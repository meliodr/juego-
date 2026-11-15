#include "IBS3Character.h"
#include "IBS3Geometry.h"
#include "IBS3World.h"
#include "IBS3MapWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/RotationMatrix.h"

namespace
{
IBS3::V3 PilotVector(FVector P) { return {P.X,P.Y,P.Z}; }
FVector PilotUE(IBS3::V3 P) { return {P.x,P.y,P.z}; }
}

void UIBS3Movement::PhysFlying(float Dt,int32 Iterations)
{
    AIBS3Character* Pilot=Cast<AIBS3Character>(CharacterOwner);
    if(!Pilot || !Pilot->Planet || !HasValidData()) return;
    AIBS3PlanetWorld* World=Pilot->Planet;
    const FVector Center=World->GetActorLocation();
    // Bound chord sagitta and collision sweep length. No SetActorLocation in flight.
    double Remaining=Dt;
    int32 Steps=0;
    while(Remaining>1.e-6 && Steps++<128)
    {
        const double Step=FMath::Min(Remaining,FMath::Min(1.0/60.0,400.0/FMath::Max(Velocity.Size(),1.0)));
        Remaining-=Step;
        const FVector P=UpdatedComponent->GetComponentLocation();
        const FVector Up=(P-Center).GetSafeNormal();
        SetGravityDirection(-Up);
        CalcVelocity(Step,0.4f,false,BrakingDecelerationFlying);
        // Vertical input controls altitude; without it, altitude is held, including over poles.
        const double RadialSpeed=FMath::Abs(FVector::DotProduct(Acceleration,Up))>1 ? FVector::DotProduct(Velocity,Up):0;
        const FVector Tangent=FVector::VectorPlaneProject(Velocity,Up);
        const IBS3::ArcStep Arc=IBS3::advance(PilotVector(P),PilotVector(Tangent),Tangent.Size()*Step,PilotVector(Center));
        const FVector NewUp=(PilotUE(Arc.position)-Center).GetSafeNormal();
        const FVector Target=PilotUE(Arc.position)+NewUp*(RadialSpeed*Step);
        const FVector Forward=PilotUE(IBS3::transport(PilotVector(Pilot->TangentForward),PilotVector(Up),PilotVector(NewUp))).GetSafeNormal();
        FHitResult Hit;
        SafeMoveUpdatedComponent(Target-P,FRotationMatrix::MakeFromXZ(Forward,NewUp).ToQuat(),true,Hit);
        if(Hit.IsValidBlockingHit())
        {
            HandleImpact(Hit,Step,Target-P); StopMovementImmediately(); break;
        }
        Velocity=PilotUE(IBS3::transport(PilotVector(Tangent),PilotVector(Up),PilotVector(NewUp)))+NewUp*RadialSpeed;
        Pilot->TangentForward=Forward; Pilot->PreviousUp=NewUp;
    }
    // On an extreme hitch, remaining motion is discarded rather than traversing unswept space.
    if(Steps>=128 && Remaining>1.e-6) { StopMovementImmediately(); World->Status=TEXT("Movement step budget exhausted: stopped safely"); }
    UpdateComponentVelocity();
}

AIBS3Character::AIBS3Character(const FObjectInitializer& Init)
    : Super(Init.SetDefaultSubobjectClass<UIBS3Movement>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick=true;
    GetCapsuleComponent()->InitCapsuleSize(42,96);
    bUseControllerRotationYaw=false; bUseControllerRotationPitch=false; bUseControllerRotationRoll=false;
    auto* Move=GetCharacterMovement(); Move->bOrientRotationToMovement=false; Move->MaxWalkSpeed=600;
    Move->MaxFlySpeed=8000; Move->MaxAcceleration=9000; Move->BrakingDecelerationFlying=8000;
    Move->bRunPhysicsWithNoController=true;
    Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("LocalCameraArm")); Arm->SetupAttachment(RootComponent);
    Arm->TargetArmLength=550; Arm->bUsePawnControlRotation=false;
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("LocalCamera")); Camera->SetupAttachment(Arm);
}
void AIBS3Character::BeginPlay()
{
    Super::BeginPlay();
    for(TActorIterator<AIBS3PlanetWorld> It(GetWorld());It;++It) { Planet=*It; break; }
    if(Planet && Planet->Manifest)
    {
        AddTickPrerequisiteActor(Planet);
        PreviousUp=(GetActorLocation()-Planet->GetActorLocation()).GetSafeNormal();
        TangentForward=PilotUE(IBS3::frame(IBS3::geographic(PilotVector(GetActorLocation()),Planet->Manifest->RadiusCm,PilotVector(Planet->GetActorLocation()))).north);
        GetCharacterMovement()->SetGravityDirection(-PreviousUp);
    }
    if(APlayerController* PC=Cast<APlayerController>(GetController()))
    {
        TSubclassOf<UIBS3MapWidget> WidgetType=MapWidgetClass;
        if(!WidgetType) WidgetType=UIBS3MapWidget::StaticClass();
        Map=CreateWidget<UIBS3MapWidget>(PC,WidgetType);
        if(Map) { Map->Pilot=this; Map->AddToViewport(); }
    }
}
void AIBS3Character::Tick(float Dt)
{
    Super::Tick(Dt); if(!Planet || !Planet->Manifest) return;
    const FVector Up=(GetActorLocation()-Planet->GetActorLocation()).GetSafeNormal();
    TangentForward=PilotUE(IBS3::transport(PilotVector(TangentForward),PilotVector(PreviousUp),PilotVector(Up))).GetSafeNormal(); PreviousUp=Up;
    auto* Move=GetCharacterMovement(); Move->SetGravityDirection(-Up);
    // CMC applies radial gravity; rotate capsule/camera frame continuously, independent of geographic heading singularities.
    FHitResult RotationHit;
    Move->MoveUpdatedComponent(FVector::ZeroVector,FRotationMatrix::MakeFromXZ(TangentForward,Up).ToQuat(),true,&RotationHit);
    Move->GravityScale=Planet->Manifest->GravityCmS2/FMath::Max(1.0,FMath::Abs(static_cast<double>(GetWorld()->GetGravityZ())));
    Move->BrakingDecelerationFlying=Planet->Manifest->BrakeCmS2;
    Move->MaxFlySpeed=Planet->AllowedSpeed(GetActorLocation(),bBrake?0:(bTraversal?Planet->Manifest->TraversalSpeedCmS:8000));
    if(bBrake) ConsumeMovementInputVector();
    LastLongitude=IBS3::geographic(PilotVector(GetActorLocation()),Planet->Manifest->RadiusCm,PilotVector(Planet->GetActorLocation()),LastLongitude).lon;
    Arm->SetRelativeRotation(FRotator(CameraPitch,0,0));
}
void AIBS3Character::Forward(float X) { if(!bBrake) AddMovementInput(TangentForward,X); }
void AIBS3Character::Right(float X) { if(!bBrake) AddMovementInput(FVector::CrossProduct(PreviousUp,TangentForward),X); }
void AIBS3Character::Ascend(float X) { if(GetCharacterMovement()->IsFlying() && !bBrake) AddMovementInput(PreviousUp,X); }
void AIBS3Character::Turn(float X) { TangentForward=FQuat(PreviousUp,FMath::DegreesToRadians(static_cast<double>(X))).RotateVector(TangentForward); }
void AIBS3Character::Look(float X) { CameraPitch=FMath::Clamp(CameraPitch-X,-85.f,80.f); }
void AIBS3Character::TurnStick(float X) { Turn(X*90*GetWorld()->GetDeltaSeconds()); }
void AIBS3Character::LookStick(float X) { Look(X*65*GetWorld()->GetDeltaSeconds()); }
void AIBS3Character::ToggleFlight()
{
    GetCharacterMovement()->SetMovementMode(GetCharacterMovement()->IsFlying()?MOVE_Falling:MOVE_Flying);
}
void AIBS3Character::ToggleTraversal() { bTraversal=!bTraversal; }
void AIBS3Character::ToggleMap() { bGlobalMap=!bGlobalMap; }
void AIBS3Character::Destination() { if(Planet) Planet->CycleDestination(); }
void AIBS3Character::Save() { if(Planet) Planet->SaveState(); }
void AIBS3Character::BrakeOn() { bBrake=true; }
void AIBS3Character::BrakeOff() { bBrake=false; }
void AIBS3Character::Interact()
{
    FCollisionQueryParams Params(SCENE_QUERY_STAT(IBS3Interaction),false,this); FHitResult Hit;
    const FVector Start=GetActorLocation();
    if(GetWorld()->SweepSingleByChannel(Hit,Start,Start+TangentForward*450,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(50),Params))
        if(AIBS3Prop* Prop=Cast<AIBS3Prop>(Hit.GetActor())) Prop->Interact(TangentForward+PreviousUp*0.25);
}
void AIBS3Character::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    // Raw axis/key bindings are local to this lab; no shared input config changes.
    Input->BindAxisKey(EKeys::W,this,&AIBS3Character::Forward);
    Input->BindAxisKey(EKeys::S).AxisDelegate.GetDelegateForManualSet().BindLambda([this](float V){Forward(-V);});
    Input->BindAxisKey(EKeys::D,this,&AIBS3Character::Right);
    Input->BindAxisKey(EKeys::A).AxisDelegate.GetDelegateForManualSet().BindLambda([this](float V){Right(-V);});
    Input->BindAxisKey(EKeys::E,this,&AIBS3Character::Ascend);
    Input->BindAxisKey(EKeys::Q).AxisDelegate.GetDelegateForManualSet().BindLambda([this](float V){Ascend(-V);});
    Input->BindAxisKey(EKeys::MouseX,this,&AIBS3Character::Turn); Input->BindAxisKey(EKeys::MouseY,this,&AIBS3Character::Look);
    Input->BindAxisKey(EKeys::Gamepad_LeftY,this,&AIBS3Character::Forward); Input->BindAxisKey(EKeys::Gamepad_LeftX,this,&AIBS3Character::Right);
    Input->BindAxisKey(EKeys::Gamepad_RightX,this,&AIBS3Character::TurnStick); Input->BindAxisKey(EKeys::Gamepad_RightY,this,&AIBS3Character::LookStick);
    Input->BindAxisKey(EKeys::Gamepad_RightTriggerAxis,this,&AIBS3Character::Ascend);
    Input->BindAxisKey(EKeys::Gamepad_LeftTriggerAxis).AxisDelegate.GetDelegateForManualSet().BindLambda([this](float V){Ascend(-V);});
    Input->BindKey(EKeys::F,IE_Pressed,this,&AIBS3Character::ToggleFlight); Input->BindKey(EKeys::Gamepad_RightThumbstick,IE_Pressed,this,&AIBS3Character::ToggleFlight);
    Input->BindKey(EKeys::LeftShift,IE_Pressed,this,&AIBS3Character::ToggleTraversal); Input->BindKey(EKeys::Gamepad_LeftThumbstick,IE_Pressed,this,&AIBS3Character::ToggleTraversal);
    Input->BindKey(EKeys::M,IE_Pressed,this,&AIBS3Character::ToggleMap); Input->BindKey(EKeys::Gamepad_Special_Left,IE_Pressed,this,&AIBS3Character::ToggleMap);
    Input->BindKey(EKeys::Tab,IE_Pressed,this,&AIBS3Character::Destination); Input->BindKey(EKeys::Gamepad_DPad_Right,IE_Pressed,this,&AIBS3Character::Destination);
    Input->BindKey(EKeys::G,IE_Pressed,this,&AIBS3Character::Interact); Input->BindKey(EKeys::Gamepad_FaceButton_Right,IE_Pressed,this,&AIBS3Character::Interact);
    Input->BindKey(EKeys::F5,IE_Pressed,this,&AIBS3Character::Save); Input->BindKey(EKeys::Gamepad_DPad_Down,IE_Pressed,this,&AIBS3Character::Save);
    Input->BindKey(EKeys::X,IE_Pressed,this,&AIBS3Character::BrakeOn); Input->BindKey(EKeys::X,IE_Released,this,&AIBS3Character::BrakeOff);
    Input->BindKey(EKeys::Gamepad_FaceButton_Left,IE_Pressed,this,&AIBS3Character::BrakeOn); Input->BindKey(EKeys::Gamepad_FaceButton_Left,IE_Released,this,&AIBS3Character::BrakeOff);
    Input->BindKey(EKeys::SpaceBar,IE_Pressed,this,&ACharacter::Jump); Input->BindKey(EKeys::SpaceBar,IE_Released,this,&ACharacter::StopJumping);
    Input->BindKey(EKeys::Gamepad_FaceButton_Bottom,IE_Pressed,this,&ACharacter::Jump); Input->BindKey(EKeys::Gamepad_FaceButton_Bottom,IE_Released,this,&ACharacter::StopJumping);
}
AIBS3GameMode::AIBS3GameMode() { DefaultPawnClass=AIBS3Character::StaticClass(); }
void AIBS3GameMode::StartPlay()
{
    // The authored lab map owns the planet and PlayerStart. Do not mutate an unrelated map.
    Super::StartPlay();
}
