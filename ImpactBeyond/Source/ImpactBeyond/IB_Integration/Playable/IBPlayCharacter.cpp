#include "IBPlayCharacter.h"
#include "IBPlayArt.h"
#include "IBPlayCombat.h"
#include "IBPlaySession.h"
#include "IB_S3/IBS3World.h"
#include "IB_S3/IBS3MapWidget.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35PunchComponent.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S4/IBS4PowerComponent.h"
#include "IB_S4/IBS4PreparedTarget.h"
#include "IB_S4/IBS4PresentationComponent.h"
#include "IB_S5/IBS5WorldLife.h"
#include "IB_S6H/IBS6HBridge.h"
#include "IB_S7/Wave8Threats/IB_S7T/IBS7TWheelComponent.h"
#include "IB_S7/Wave8Threats/IB_S7T/IBS7TThreatActor.h"
#include "ImpactFlightFeedback.h"
#include "ImpactLandingComponent.h"
#include "ImpactGripMesh.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/BlendSpace.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"

AIBPlayCharacter::AIBPlayCharacter(const FObjectInitializer& Init)
    : Super(Init.SetDefaultSubobjectClass<UImpactGripMesh>(ACharacter::MeshComponentName))
{
    Grip=CreateDefaultSubobject<UIBS35GripComponent>(TEXT("SharedGrip"));
    Punches=CreateDefaultSubobject<UIBS35PunchComponent>(TEXT("TimedMelee"));
    Punches->bDrawContacts=false;
    Heroes=CreateDefaultSubobject<UIBS6HBridge>(TEXT("HeroAbilities"));
    Wheel=CreateDefaultSubobject<UIBS7TWheelComponent>(TEXT("PowerSelection"));
    Response=CreateDefaultSubobject<UIBS4ResponseComponent>(TEXT("HeroResponse"));
    Combat=CreateDefaultSubobject<UIBPlayCombat>(TEXT("CanonicalCombat"));
    Powers=CreateDefaultSubobject<UIBS4PowerComponent>(TEXT("SharedPowers"));
    Effects=CreateDefaultSubobject<UIBS4PresentationComponent>(TEXT("PowerEffects"));
    FlightFeedback=CreateDefaultSubobject<UImpactFlightFeedback>(TEXT("FlightFeedback"));
    Landing=CreateDefaultSubobject<UImpactLandingComponent>(TEXT("ImpactLanding"));
    Landing->bUseLocalGravity=true;
    HeroVisual=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeroAppearance"));
    HeroVisual->SetupAttachment(GetMesh());
    HeroVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeroVisual->SetLeaderPoseComponent(GetMesh());
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    GetMesh()->SetSkeletalMesh(Manny.Object);
    GetMesh()->SetRelativeLocation(FVector(0,0,-96));
    GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
    GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    FlightPoses.SetNum(5);
    const TCHAR* Paths[]={TEXT("/Game/IB_Flight/A_FlightCruise"),TEXT("/Game/IB_Flight/A_FlightSpeed2"),
        TEXT("/Game/IB_Flight/A_FlightSpeed3"),TEXT("/Game/IB_Flight/A_FlightSpeed4"),TEXT("/Game/IB_Flight/A_FlightSpeed5")};
    for(int32 I=0;I<5;++I) FlightPoses[I]=ConstructorHelpers::FObjectFinder<UAnimSequence>(Paths[I]).Object;
    IdlePose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle")).Object;
    WalkPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd")).Object;
    RunPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd")).Object;
    GroundLocomotion=ConstructorHelpers::FObjectFinder<UBlendSpace>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run")).Object;
    JumpPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Jump")).Object;
    FallPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Fall_Loop")).Object;
    LandPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Land")).Object;
    TakeoffPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/IB_Flight/A_FlightTakeoff")).Object;
    CarryTakeoffPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/IB_Flight/A_FlightCarryTakeoff")).Object;
    CarryPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/IB_Flight/A_FlightCarry")).Object;
    BrakePose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/IB_Flight/A_FlightBrake")).Object;
    ThrowPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/IB_Flight/A_FlightThrow")).Object;
    for(const TCHAR* Path:{TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01"),TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02"),TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03")})
        AttackPoses.Add(ConstructorHelpers::FObjectFinder<UAnimSequence>(Path).Object);
    HeavyPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_ChargedAttack")).Object;
    HoverPose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/IB_Flight/A_FlightHover")).Object;
    DivePose=ConstructorHelpers::FObjectFinder<UAnimSequence>(TEXT("/Game/IB_Flight/A_ImpactDive")).Object;
    Arm->TargetArmLength=420;
    Arm->bEnableCameraLag=true; Arm->CameraLagSpeed=14; Arm->CameraLagMaxDistance=45;
    Arm->bEnableCameraRotationLag=false;
    Camera->FieldOfView=85;
    Camera->PostProcessSettings.bOverride_MotionBlurAmount=true;
    Camera->PostProcessSettings.MotionBlurAmount=0;
    GetCharacterMovement()->MaxWalkSpeed=650;
    GetCharacterMovement()->JumpZVelocity=650;
}
void AIBPlayCharacter::BeginPlay()
{
    Super::BeginPlay();
    if(Map) Map->bCompactHud=true;
    Art=LoadObject<UIBPlayArt>(nullptr,TEXT("/Game/IB_Playable/DA_PlayArt.DA_PlayArt"));
    for(TActorIterator<AIBPlaySession> It(GetWorld());It;++It) { Session=*It; break; }
    CastChecked<UImpactGripMesh>(GetMesh())->SetSharedGrip(Grip);
    Combat->InitializeReceiver();
    Heroes->CanonicalStrike.BindUObject(this,&AIBPlayCharacter::DispatchS35);
    Punches->CanonicalStrike.BindUObject(this,&AIBPlayCharacter::DispatchS35);
    Punches->OnAttackCue.AddDynamic(this,&AIBPlayCharacter::OnPunchCue);
    Landing->CanonicalStrike.BindUObject(this,&AIBPlayCharacter::DispatchS35);
    Heroes->HostAbility.BindUObject(this,&AIBPlayCharacter::HostAbility);
    Heroes->HostReleaseAbility.BindUObject(this,&AIBPlayCharacter::ReleaseHostAbility);
    Heroes->OnHeroChanged.AddDynamic(this,&AIBPlayCharacter::HeroChanged);
    Powers->CanonicalStrike.BindUObject(this,&AIBPlayCharacter::DispatchS4);
    Wheel->SelectionProvider.BindUObject(Heroes,&UIBS6HBridge::EquipAbility);
    if(APlayerController* PC=Cast<APlayerController>(Controller))
    {
        Wheel->BindController(PC,false); Wheel->bInputAllowed=true;
        PC->SetInputMode(FInputModeGameOnly()); PC->bShowMouseCursor=false;
    }
    if(Art) Heroes->EnergyMaterial=Art->EnergyMaterial;
    HeroChanged(Heroes->ActiveHero());
    ShowNotice(TEXT("Bienvenido a IMPACT: BEYOND. F1 muestra los controles."));
}
bool AIBPlayCharacter::IsGameplayLocked() const { return bSettings || Heroes->IsSelectorOpen() || Wheel->bOpen; }
FRotator AIBPlayCharacter::GetBaseAimRotation() const { return Camera?Camera->GetComponentRotation():Super::GetBaseAimRotation(); }
void AIBPlayCharacter::ShowNotice(const FString& Text) { Notice=Text; NoticeSeconds=5; }
double AIBPlayCharacter::RequestedFlightSpeed() const
{
    static const double Speeds[]={1800,4000,8000,14000,22000};
    double Speed=bTraversal&&Planet?Planet->Manifest->TraversalSpeedCmS:Speeds[FMath::Clamp(FlightGear,0,4)]*(bTurbo?1.3:1.0);
    if(Grip->GetHeld()) Speed=FMath::Min(Speed,1600.0);
    return Speed;
}
bool AIBPlayCharacter::PrepareMovement()
{
    if(IsGameplayLocked()) { ConsumeMovementInputVector(); GetCharacterMovement()->StopMovementImmediately(); }
    return Super::PrepareMovement();
}
void AIBPlayCharacter::ReadDigitalMovement(APlayerController* PC)
{
    if(IsGameplayLocked()||Landing->IsImpacting()) return;
    bBrake=PC->IsInputKeyDown(EKeys::X)||PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left);
    bTurbo=PC->IsInputKeyDown(EKeys::LeftShift)||PC->IsInputKeyDown(EKeys::Gamepad_LeftThumbstick);
    const float F=FMath::Clamp(float(PC->IsInputKeyDown(EKeys::W))-float(PC->IsInputKeyDown(EKeys::S))+PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftY),-1.f,1.f);
    const float R=FMath::Clamp(float(PC->IsInputKeyDown(EKeys::D))-float(PC->IsInputKeyDown(EKeys::A))+PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftX),-1.f,1.f);
    Forward(F); Right(R);
    const float Vertical=float(PC->IsInputKeyDown(EKeys::SpaceBar))-float(PC->IsInputKeyDown(EKeys::LeftControl))
        +PC->GetInputAnalogKeyState(EKeys::Gamepad_RightTriggerAxis)-PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftTriggerAxis);
    Ascend(Vertical);
    GetCharacterMovement()->MaxWalkSpeed=bTurbo?1800:650;
}
void AIBPlayCharacter::Tick(float Dt)
{
    // Brake release must still be read while the base movement method consumes input.
    if(APlayerController* PC=Cast<APlayerController>(Controller))
    {
        bBrake=IsGameplayLocked()||PC->IsInputKeyDown(EKeys::X)||PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left);
        const bool Both=PC->GetInputAnalogKeyState(EKeys::Gamepad_RightTriggerAxis)>.8f&&PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftTriggerAxis)>.8f;
        if(Both&&!bDivePadLatch) ImpactDive();
        bDivePadLatch=Both;
    }
    Super::Tick(Dt);
    if(bSelectorWasOpen!=Heroes->IsSelectorOpen())
    {
        bSelectorWasOpen=Heroes->IsSelectorOpen();
        if(APlayerController* PC=Cast<APlayerController>(Controller))
        {
            PC->bShowMouseCursor=bSelectorWasOpen;
            if(bSelectorWasOpen) PC->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
            else PC->SetInputMode(FInputModeGameOnly());
        }
    }
    if(!Session) for(TActorIterator<AIBPlaySession> It(GetWorld());It;++It) { Session=*It; break; }
    NoticeSeconds=FMath::Max(0.f,NoticeSeconds-Dt);
    if(bChargingPunch) PunchCharge=FMath::Min(1.f,PunchCharge+Dt/.9f);
    WheelRefresh-=Dt; if(WheelRefresh<=0 && !Wheel->bOpen) { RefreshWheel(); WheelRefresh=.15f; }
    UpdateAnimation(Dt);
    FlightFeedback->UpdateFlightFeedback(Dt,GetCharacterMovement()->IsFlying(),bTurbo&&!bBrake&&!bTraversal&&!Grip->GetHeld());
    UpdatePresentationCamera(Dt);
    if(bLaserActive && IsGameplayLocked()) Release();
}
void AIBPlayCharacter::UpdateAnimation(float Dt)
{
    const UCharacterMovementComponent* Move=GetCharacterMovement();
    const FVector Velocity=Move->Velocity;
    const FQuat ActorFrame=GetActorQuat();
    const FVector Up=GetActorUpVector();
    const FVector TangentVelocity=FVector::VectorPlaneProject(Velocity,Up);
    const float Speed=Velocity.Size(), GroundSpeed=TangentVelocity.Size();
    const float RiseSpeed=FVector::DotProduct(Velocity,Up);
    const bool Flying=Move->IsFlying(), Falling=Move->IsFalling();
    const bool Carrying=Grip->GetHeld()&&!Grip->IsTelekinetic();
    const bool Attacking=Punches->IsAttacking(), Diving=Landing->IsDiving();
    const bool TakingOff=Flying&&PreviousPresentationMode!=MOVE_Flying;
    const bool TouchingDown=Move->IsMovingOnGround()&&PreviousPresentationMode==MOVE_Falling;
    constexpr float TakeoffDuration=.43f;
    TakeoffRemaining=TakingOff?TakeoffDuration:FMath::Max(0.f,TakeoffRemaining-Dt);
    if(!Flying) TakeoffRemaining=0;
    const float LandDuration=GroundSpeed>200?.18f:.28f;
    LandRemaining=TouchingDown?LandDuration:FMath::Max(0.f,LandRemaining-Dt);
    if(Flying||Falling) LandRemaining=0;
    ThrowAnimation=FMath::Max(0.f,ThrowAnimation-Dt);

    // The speed selector limits movement; the visible pose follows achieved speed, including traversal.
    bUsingTravelPose=Flying&&Speed>=(bUsingTravelPose?120.f:200.f);
    static constexpr float PoseBoundaries[]={2900,6000,11000,18000};
    int32 SpeedPose=Presentation.FlightPose;
    while(SpeedPose<4&&Speed>PoseBoundaries[SpeedPose]*1.04f) ++SpeedPose;
    while(SpeedPose>0&&Speed<PoseBoundaries[SpeedPose-1]*.96f) --SpeedPose;
    UAnimationAsset* Next=GroundLocomotion.Get();
    if(!Next) Next=GroundSpeed>450?RunPose.Get():GroundSpeed>35?WalkPose.Get():IdlePose.Get();
    EIBPlayMotion Motion=EIBPlayMotion::Ground;
    bool Loop=true;
    float Rate=FMath::Clamp(GroundSpeed/600.f,1.f,3.f);
    if(Falling)
    {
        Motion=RiseSpeed>40?EIBPlayMotion::Jump:EIBPlayMotion::Fall;
        Next=RiseSpeed>40?JumpPose.Get():FallPose.Get(); Loop=RiseSpeed<=40; Rate=1;
    }
    else if(LandRemaining>0)
    {
        Motion=EIBPlayMotion::Land; Next=LandPose; Loop=false;
        Rate=LandPose?LandPose->GetPlayLength()/LandDuration:1;
    }
    if(Flying)
    {
        Motion=bUsingTravelPose?EIBPlayMotion::Flight:EIBPlayMotion::Hover;
        Next=bUsingTravelPose?FlightPoses[SpeedPose].Get():HoverPose.Get();
        Rate=FMath::GetMappedRangeValueClamped(FVector2D(0,22000),FVector2D(.85f,1.2f),Speed);
        if(Carrying) { Motion=EIBPlayMotion::Carry; Next=CarryPose; Rate=1; }
        if(TakeoffRemaining>0)
        {
            Motion=EIBPlayMotion::Takeoff; Next=Carrying?CarryTakeoffPose.Get():TakeoffPose.Get(); Loop=false;
            const UAnimSequence* Launch=Cast<UAnimSequence>(Next);
            Rate=Launch?Launch->GetPlayLength()/TakeoffDuration:1;
        }
        if(bBrake&&!Carrying) { Motion=EIBPlayMotion::Brake; Next=BrakePose; Loop=false; Rate=1; }
    }
    if(ThrowAnimation>0) { Motion=EIBPlayMotion::Throw; Next=ThrowPose; Loop=false; Rate=1; }
    if(Attacking)
    {
        Motion=EIBPlayMotion::Attack; Loop=false;
        UAnimSequence* Attack=Punches->GetAttackStyle()==EIBS35PunchStyle::Heavy?HeavyPose.Get():AttackPoses[FMath::Clamp(Punches->GetComboIndex()-1,0,2)].Get();
        Next=Attack; Rate=Attack?Attack->GetPlayLength()/FMath::Max(.1f,Punches->GetAttackDuration()):1;
    }
    if(Diving) { Motion=EIBPlayMotion::Dive; Next=DivePose; Loop=false; Rate=1; }

    // Transport the previous visual frame with local gravity before following travel direction.
    // Camera yaw and a curved planet must not masquerade as a banking turn.
    const bool HasPreviousFrame=PreviousPresentationMode!=MOVE_None;
    const FQuat Transport=HasPreviousFrame?FQuat::FindBetweenNormals(PreviousPresentationUp,Up):FQuat::Identity;
    if(HasPreviousFrame) VisualFacing=(ActorFrame.Inverse()*Transport*PreviousPresentationFrame*VisualFacing).GetNormalized();
    const FVector LocalVelocity=ActorFrame.UnrotateVector(Velocity);
    const float TravelYaw=GroundSpeed>35?FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y,LocalVelocity.X)):VisualFacing.Rotator().Yaw;
    const bool AimFacing=Carrying||Attacking||ThrowAnimation>0||bLaserActive;
    FRotator TargetFacing(0,AimFacing?0:TravelYaw,0);
    const FVector TravelDirection=TangentVelocity.GetSafeNormal();
    float TurnRate=0;
    if(Flying&&GroundSpeed>200&&!PreviousTravelDirection.IsNearlyZero()&&Dt>UE_SMALL_NUMBER)
    {
        const FVector Before=Transport.RotateVector(PreviousTravelDirection);
        const float Turn=FMath::Atan2(FVector::DotProduct(Up,FVector::CrossProduct(Before,TravelDirection)),FVector::DotProduct(Before,TravelDirection));
        TurnRate=FMath::RadiansToDegrees(Turn)/Dt;
    }
    if(Flying&&!Diving)
    {
        float Lean=FMath::GetMappedRangeValueClamped(FVector2D(180,1800),FVector2D(0,65),GroundSpeed);
        if(TakeoffRemaining>0) Lean*=1-TakeoffRemaining/TakeoffDuration;
        if(bBrake) Lean*=.2f;
        const float TrajectoryPitch=FMath::RadiansToDegrees(FMath::Atan2(RiseSpeed,GroundSpeed));
        TargetFacing.Pitch=FMath::Clamp(TrajectoryPitch*FMath::SmoothStep(70.f,600.f,Speed)-Lean,-85.f,35.f);
        if(AimFacing) TargetFacing.Pitch=0;
    }
    const float BankTarget=Flying&&!AimFacing&&!Diving?FMath::Clamp(TurnRate/180.f,-1.f,1.f)*(bBrake?.25f:1.f):0;
    FlightBank=FMath::FInterpTo(FlightBank,BankTarget,Dt,5);
    TargetFacing.Roll=-28*FlightBank;
    VisualFacing=FQuat::Slerp(VisualFacing,TargetFacing.Quaternion(),1-FMath::Exp(-9.f*FMath::Max(0.f,Dt))).GetNormalized();
    const FVector MeshOffset(0,0,-GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
    GetMesh()->SetRelativeLocationAndRotation(VisualFacing.RotateVector(MeshOffset),VisualFacing*FRotator(0,-90,0).Quaternion());
    // Blend-space direction is relative to the lagging visual heading, not the camera/capsule.
    const float Direction=GroundSpeed>1?FMath::FindDeltaAngleDegrees(VisualFacing.Rotator().Yaw,TravelYaw):0;

    const bool ActionRestart=(Motion==EIBPlayMotion::Attack&&AttackSerial!=PresentedAttackSerial)
        ||(Motion==EIBPlayMotion::Throw&&ThrowSerial!=PresentedThrowSerial);
    if(Next&&(Next!=Playing||Motion!=Presentation.Motion||ActionRestart))
    {
        CastChecked<UImpactGripMesh>(GetMesh())->BeginFlightPoseBlend(Motion==EIBPlayMotion::Throw?.08f:.16f);
        GetMesh()->PlayAnimation(Next,Loop); Playing=Next; ++Presentation.Transition;
    }
    // Rates must also change while the same clip stays selected (walk -> sprint, climb -> cruise).
    if(UAnimSingleNodeInstance* Animation=GetMesh()->GetSingleNodeInstance())
    {
        Animation->SetPlayRate(Rate); Animation->SetLooping(Loop);
        if(Next==GroundLocomotion) Animation->SetBlendSpacePosition(FVector(Direction,FMath::Min(GroundSpeed,600.f),0));
    }
    Presentation.Motion=Motion; Presentation.Speed=Speed; Presentation.GroundSpeed=GroundSpeed;
    Presentation.BlendDirection=Direction; Presentation.PlaybackRate=Rate; Presentation.FlightPose=SpeedPose;
    Presentation.VisualRotation=VisualFacing.Rotator();
    PresentedAttackSerial=AttackSerial; PresentedThrowSerial=ThrowSerial;
    PreviousPresentationMode=Move->MovementMode; PreviousPresentationFrame=ActorFrame; PreviousPresentationUp=Up;
    PreviousTravelDirection=GroundSpeed>200?TravelDirection:FVector::ZeroVector;
}
void AIBPlayCharacter::OnPunchCue(FName Phase,float NormalizedTime)
{
    if(Phase==TEXT("Windup")) ++AttackSerial;
}
void AIBPlayCharacter::UpdatePresentationCamera(float Dt)
{
    const bool Flying=GetCharacterMovement()->IsFlying();
    const bool Carrying=Grip->GetHeld()!=nullptr;
    const float SpeedAlpha=(Flying||Landing->IsDiving())?FMath::Sqrt(FMath::Clamp(float(GetVelocity().Size())/22000.f,0.f,1.f)):0;
    Camera->FieldOfView=FMath::FInterpTo(Camera->FieldOfView,85+12*SpeedAlpha,Dt,4);
    Arm->TargetArmLength=FMath::FInterpTo(Arm->TargetArmLength,Carrying?510.f:420.f+50*SpeedAlpha,Dt,4);
    Arm->SocketOffset=FMath::VInterpTo(Arm->SocketOffset,FVector(0,0,Carrying?45:15),Dt,4);
}
FString AIBPlayCharacter::GetMovementStatusText() const
{
    if(Landing->IsDiving()) return TEXT("Impacto");
    if(GetCharacterMovement()->IsFlying())
    {
        FString State=bBrake?TEXT("Freno"):Grip->GetHeld()?TEXT("Carga"):bTraversal?TEXT("Travesia"):bTurbo?TEXT("Turbo"):TEXT("Vuelo");
        return FString::Printf(TEXT("%s %d/5"),*State,FlightGear+1);
    }
    if(GetCharacterMovement()->IsFalling()) return FVector::DotProduct(GetVelocity(),GetActorUpVector())>40?TEXT("Salto"):TEXT("Caida");
    return GetVelocity().Size()>1000?TEXT("Sprint"):TEXT("A pie");
}
void AIBPlayCharacter::RefreshWheel()
{
    TArray<FIBS7TSlot> Slots;
    for(const auto& A:Heroes->GetAbilities())
    {
        FIBS7TSlot S; S.AbilityId=A.Id; S.Label=A.Label; S.Cooldown=A.Cooldown; S.bAvailable=A.bAvailable; Slots.Add(S);
        if(A.Id>=101&&A.Id<=103) { Slots.Last().Cooldown=Punches->GetCooldown(); Slots.Last().bAvailable&=!Punches->IsAttacking(); }
    }
    Wheel->UpdateSlots(Slots,Heroes->EquippedAbility());
}
void AIBPlayCharacter::HeroChanged(int32 Id)
{
    bChargingPunch=false; Release(); Landing->CancelImpactDive(); Punches->CancelAttack(); Grip->Drop(); Wheel->Cancel();
    ThrowAnimation=0; Playing=nullptr;
    Response->bH01=Id==0;
    if(Art && Art->Heroes.IsValidIndex(Id) && Art->Heroes[Id])
    {
        HeroVisual->SetSkeletalMesh(Art->Heroes[Id]);
        HeroVisual->SetLeaderPoseComponent(GetMesh(),true);
        GetMesh()->SetVisibility(false,false); HeroVisual->SetVisibility(true);
    }
    RefreshWheel();
}
bool AIBPlayCharacter::DispatchS35(const FIBS35DamageMessage& Input,const FHitResult& Hit)
{
    return Session && Session->DispatchS35(Input,Hit);
}
bool AIBPlayCharacter::DispatchS4(const FIBS4Hit& Input,const FHitResult& Hit)
{
    return Session && Session->DispatchS4(Input,Hit);
}
bool AIBPlayCharacter::HostAbility(int32 Id)
{
    if(IsGameplayLocked()) return false;
    if(Id==104) { if(Grip->GetHeld()) return false; Powers->SetLaser(true); bLaserActive=true; return true; }
    if(Id==105)
    {
        if(Grip->GetHeld()) { Grip->Drop(); return true; }
        return GrabAimedPart(true);
    }
    if(Id==106)
    {
        if(!GetCharacterMovement()->IsFlying() || Grip->GetHeld() || !Planet) return false;
        const FVector End=GetActorLocation()+TangentForward*1000;
        if(!Planet->CanTraverse(GetActorLocation(),End,96)) return false;
        GetCharacterMovement()->AddImpulse(TangentForward*3000,true); return true;
    }
    return false;
}
void AIBPlayCharacter::ReleaseHostAbility(int32 Id) { if(Id==104) { Powers->SetLaser(false); bLaserActive=false; } }
void AIBPlayCharacter::Flight() { if(!IsGameplayLocked()&&!Landing->IsImpacting()) { ToggleFlight(); bTraversal=false; } }
void AIBPlayCharacter::GearUp() { if(Heroes->IsSelectorOpen()) SelectorNext(); else if(!IsGameplayLocked()) FlightGear=FMath::Min(4,FlightGear+1); }
void AIBPlayCharacter::GearDown() { if(Heroes->IsSelectorOpen()) SelectorPrevious(); else if(!IsGameplayLocked()) FlightGear=FMath::Max(0,FlightGear-1); }
void AIBPlayCharacter::Travel() { if(Heroes->IsSelectorOpen()) SelectorNext(); else if(!IsGameplayLocked() && GetCharacterMovement()->IsFlying()) ToggleTraversal(); }
void AIBPlayCharacter::Grab()
{
    if(Heroes->IsSelectorOpen()) { Heroes->CloseSelector(); return; }
    if(Wheel->bOpen) { Wheel->Cancel(); return; }
    if(IsGameplayLocked()||Landing->IsImpacting()) return;
    if(Grip->GetHeld()) Grip->Drop(); else if(!GrabAimedPart(false)&&!Grip->GrabNearest()) InteractWorld();
}
bool AIBPlayCharacter::GrabAimedPart(bool bTelekinesis)
{
    FHitResult Hit; FCollisionQueryParams Q(SCENE_QUERY_STAT(IBPlayGrabAim),false,this);
    const FVector Start=Camera->GetComponentLocation();
    const double Range=bTelekinesis?900:400;
    if(!GetWorld()->LineTraceSingleByChannel(Hit,Start,Start+Camera->GetForwardVector()*(Range+Arm->TargetArmLength+100),ECC_Visibility,Q)
        ||FVector::DistSquared(GetActorLocation(),Hit.ImpactPoint)>FMath::Square(Range+50)) return false;
    AIBS35Object* Loose=Cast<AIBS35Object>(Hit.GetActor());
    bool Detached=false;
    if(auto* Threat=Cast<AIBS7TThreatActor>(Hit.GetActor()))
    {
        Loose=Threat->TryDetachAt(Hit);
        Detached=Loose!=nullptr;
    }
    if(auto* Prepared=Cast<AIBS4PreparedTarget>(Hit.GetActor()); Prepared&&Hit.GetComponent()==Prepared->GetPart())
        Loose=Prepared->GetSharedLoosePart();
    if(Loose&&Grip->TryGrab(Loose,bTelekinesis)) return true;
    if(Detached) ShowNotice(TEXT("Pieza arrancada. Acercate para levantarla."));
    return Detached;
}
void AIBPlayCharacter::ThrowStart() { if(!IsGameplayLocked()) Grip->BeginCharge(); }
void AIBPlayCharacter::ThrowRelease()
{
    if(IsGameplayLocked()) { Grip->CancelCharge(); return; }
    if(Grip->CommitThrow(Camera->GetForwardVector()))
    {
        ThrowAnimation=ThrowPose?ThrowPose->GetPlayLength():.4f;
        ++ThrowSerial;
    }
}
void AIBPlayCharacter::Activate()
{
    if(IsGameplayLocked()) return;
    switch(Heroes->EquippedAbility())
    {
    case 101: Punches->BeginPunch(false); break;
    case 102: Heavy(); break;
    case 103: Punches->BeginUppercut(); break;
    default: Heroes->ActivateEquipped(); break;
    }
}
void AIBPlayCharacter::Release()
{
    if(bChargingPunch) { bChargingPunch=false; if(!IsGameplayLocked()) Punches->BeginChargedPunch(PunchCharge); }
    Heroes->ReleaseEquipped(); Powers->SetLaser(false); bLaserActive=false;
}
void AIBPlayCharacter::Quick() { if(!IsGameplayLocked()) Punches->BeginPunch(false); }
void AIBPlayCharacter::Heavy() { if(!IsGameplayLocked()&&!Punches->IsAttacking()&&!Grip->GetHeld()) { bChargingPunch=true; PunchCharge=0; } }
void AIBPlayCharacter::OpenWheelMouse() { if(!Heroes->IsSelectorOpen()&&!bSettings) { bChargingPunch=false; Release(); RefreshWheel(); Wheel->OpenWheel(false); } }
void AIBPlayCharacter::CloseWheelMouse() { Wheel->ConfirmWheel(false); }
void AIBPlayCharacter::OpenWheelPad() { if(!Heroes->IsSelectorOpen()&&!bSettings) { bChargingPunch=false; Release(); RefreshWheel(); Wheel->OpenWheel(true); } }
void AIBPlayCharacter::CloseWheelPad() { Wheel->ConfirmWheel(true); }
void AIBPlayCharacter::Selector() { if(!Wheel->bOpen&&!bSettings) { bChargingPunch=false; Release(); Punches->CancelAttack(); Heroes->OpenSelector(); } }
void AIBPlayCharacter::Back()
{
    if(Wheel->bOpen) { Wheel->Cancel(); return; }
    if(Heroes->IsSelectorOpen()) { Heroes->CloseSelector(); return; }
    bChargingPunch=false; Release(); Punches->CancelAttack(); bSettings=!bSettings;
}
void AIBPlayCharacter::SaveGame() { if(Session) ShowNotice(Session->SaveSession()?TEXT("Partida guardada."):TEXT("Suelta los objetos antes de guardar.")); }
void AIBPlayCharacter::InteractWorld() { if(Session&&Session->Life && Session->Life->TryResolveIncident(this)) ShowNotice(TEXT("Persona a salvo.")); }
void AIBPlayCharacter::ToggleHelp() { bHelp=!bHelp; }
void AIBPlayCharacter::NextDestination() { if(!IsGameplayLocked()) Destination(); }
void AIBPlayCharacter::GlobalMap() { if(!IsGameplayLocked()) ToggleMap(); }
void AIBPlayCharacter::ClockSpeed() { if(Session) Session->CycleDayLength(); }
void AIBPlayCharacter::ClockHour() { if(Session&&Session->Life) Session->Life->AdvanceSolarHours(6); }
void AIBPlayCharacter::Consequences() { if(Session&&Session->Life) Session->Life->bConsequencesEnabled=!Session->Life->bConsequencesEnabled; }
void AIBPlayCharacter::JumpPressed()
{
    if(Heroes->IsSelectorOpen()) { Heroes->SelectorInput(2,true); Heroes->SelectorInput(2,false); }
    else if(!IsGameplayLocked()&&!Landing->IsImpacting()&&!GetCharacterMovement()->IsFlying())
    {
        auto* Movement=GetCharacterMovement();
        const FVector Up=GetActorUpVector();
        const double TangentSpeed=FVector::VectorPlaneProject(GetVelocity(),Up).Size();
        if(Movement->IsMovingOnGround()&&bTurbo&&!Grip->GetHeld()&&TangentSpeed>=1450)
        {
            const double LaunchSpeed=FMath::Sqrt(2.0*FMath::Abs(Movement->GetGravityZ())*1500.0);
            Movement->SetMovementMode(MOVE_Falling);
            Movement->Velocity=FVector::VectorPlaneProject(GetVelocity(),Up)*.92+Up*LaunchSpeed;
        }
        else Jump();
    }
}
void AIBPlayCharacter::JumpReleased() { StopJumping(); }
void AIBPlayCharacter::SelectorPrevious() { if(Heroes->IsSelectorOpen()) { Heroes->SelectorInput(0,true); Heroes->SelectorInput(0,false); } }
void AIBPlayCharacter::SelectorNext() { if(Heroes->IsSelectorOpen()) { Heroes->SelectorInput(1,true); Heroes->SelectorInput(1,false); } }
void AIBPlayCharacter::PreviousPower() { if(Heroes->IsSelectorOpen()) SelectorPrevious(); else if(!IsGameplayLocked()) Wheel->SelectPrevious(); }
void AIBPlayCharacter::ImpactDive()
{
    if(IsGameplayLocked()||Grip->GetHeld()) return;
    bChargingPunch=false; Release(); Punches->CancelAttack(); Landing->StartImpactDive();
}
void AIBPlayCharacter::LookX(float V) { if(!IsGameplayLocked()) Turn(V); }
void AIBPlayCharacter::LookY(float V) { if(!IsGameplayLocked()) Look(V); }
void AIBPlayCharacter::StickX(float V) { if(!IsGameplayLocked()) TurnStick(V); }
void AIBPlayCharacter::StickY(float V) { if(!IsGameplayLocked()) LookStick(V); }
void AIBPlayCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    ACharacter::SetupPlayerInputComponent(Input);
    Input->BindAxisKey(EKeys::MouseX,this,&AIBPlayCharacter::LookX); Input->BindAxisKey(EKeys::MouseY,this,&AIBPlayCharacter::LookY);
    Input->BindAxisKey(EKeys::Gamepad_RightX,this,&AIBPlayCharacter::StickX); Input->BindAxisKey(EKeys::Gamepad_RightY,this,&AIBPlayCharacter::StickY);
    auto Press=[&](FKey K,void(AIBPlayCharacter::*F)()){ Input->BindKey(K,IE_Pressed,this,F); };
    auto Up=[&](FKey K,void(AIBPlayCharacter::*F)()){ Input->BindKey(K,IE_Released,this,F); };
    Press(EKeys::F,&AIBPlayCharacter::Flight); Press(EKeys::Gamepad_RightThumbstick,&AIBPlayCharacter::Flight);
    Press(EKeys::E,&AIBPlayCharacter::GearUp); Press(EKeys::Gamepad_RightShoulder,&AIBPlayCharacter::GearUp);
    Press(EKeys::Q,&AIBPlayCharacter::GearDown); Press(EKeys::Gamepad_LeftShoulder,&AIBPlayCharacter::GearDown);
    Press(EKeys::B,&AIBPlayCharacter::Travel); Press(EKeys::Gamepad_DPad_Right,&AIBPlayCharacter::Travel);
    Press(EKeys::G,&AIBPlayCharacter::Grab); Press(EKeys::Gamepad_FaceButton_Right,&AIBPlayCharacter::Grab);
    Press(EKeys::T,&AIBPlayCharacter::ThrowStart); Up(EKeys::T,&AIBPlayCharacter::ThrowRelease);
    Press(EKeys::Gamepad_FaceButton_Top,&AIBPlayCharacter::ThrowStart); Up(EKeys::Gamepad_FaceButton_Top,&AIBPlayCharacter::ThrowRelease);
    Press(EKeys::LeftMouseButton,&AIBPlayCharacter::Activate); Up(EKeys::LeftMouseButton,&AIBPlayCharacter::Release);
    Press(EKeys::Gamepad_DPad_Down,&AIBPlayCharacter::Activate); Up(EKeys::Gamepad_DPad_Down,&AIBPlayCharacter::Release);
    Press(EKeys::RightMouseButton,&AIBPlayCharacter::Heavy);
    Up(EKeys::RightMouseButton,&AIBPlayCharacter::Release);
    Press(EKeys::Tab,&AIBPlayCharacter::OpenWheelMouse); Up(EKeys::Tab,&AIBPlayCharacter::CloseWheelMouse);
    Press(EKeys::Gamepad_DPad_Up,&AIBPlayCharacter::OpenWheelPad); Up(EKeys::Gamepad_DPad_Up,&AIBPlayCharacter::CloseWheelPad);
    Press(EKeys::H,&AIBPlayCharacter::Selector); Press(EKeys::Gamepad_Special_Right,&AIBPlayCharacter::Selector);
    Press(EKeys::Escape,&AIBPlayCharacter::Back);
    Press(EKeys::V,&AIBPlayCharacter::ImpactDive);
    Press(EKeys::Left,&AIBPlayCharacter::SelectorPrevious); Press(EKeys::A,&AIBPlayCharacter::SelectorPrevious);
    Press(EKeys::Right,&AIBPlayCharacter::SelectorNext); Press(EKeys::D,&AIBPlayCharacter::SelectorNext);
    Press(EKeys::Enter,&AIBPlayCharacter::JumpPressed); Press(EKeys::Gamepad_DPad_Left,&AIBPlayCharacter::PreviousPower);
    Press(EKeys::M,&AIBPlayCharacter::GlobalMap); Press(EKeys::Gamepad_Special_Left,&AIBPlayCharacter::GlobalMap);
    Press(EKeys::N,&AIBPlayCharacter::NextDestination);
    Press(EKeys::F5,&AIBPlayCharacter::SaveGame); Press(EKeys::F1,&AIBPlayCharacter::ToggleHelp);
    Press(EKeys::F2,&AIBPlayCharacter::ClockSpeed); Press(EKeys::F3,&AIBPlayCharacter::ClockHour); Press(EKeys::F4,&AIBPlayCharacter::Consequences);
    Press(EKeys::SpaceBar,&AIBPlayCharacter::JumpPressed); Up(EKeys::SpaceBar,&AIBPlayCharacter::JumpReleased);
    Press(EKeys::Gamepad_FaceButton_Bottom,&AIBPlayCharacter::JumpPressed); Up(EKeys::Gamepad_FaceButton_Bottom,&AIBPlayCharacter::JumpReleased);
}

void AIBPlayHUD::DrawHUD()
{
    Super::DrawHUD();
    AIBPlayCharacter* P=PlayerOwner?Cast<AIBPlayCharacter>(PlayerOwner->GetPawn()):nullptr;
    if(!Canvas||!P) return;
    const float W=Canvas->ClipX,H=Canvas->ClipY;
    const FLinearColor Cyan(.3f,.9f,1.f),White(.92f,.95f,1.f),Panel(.02f,.03f,.05f,.86f);
    DrawRect(Panel,22,20,410,106);
    const auto Heroes=P->Heroes->GetHeroDescriptors();
    const int32 Id=P->Heroes->ActiveHero();
    const FString Name=Heroes.IsValidIndex(Id)?Heroes[Id].Label.ToString():TEXT("Heroe");
    DrawText(TEXT("IMPACT: BEYOND"),Cyan,38,30,nullptr,1.3);
    DrawText(FString::Printf(TEXT("%s   Vida %.0f   Energia %.0f"),*Name,P->Response->Health,P->Heroes->Energy()),White,38,59);
    DrawText(FString::Printf(TEXT("%s   %.0f km/h"),*P->GetMovementStatusText(),P->GetVelocity().Size()*.036),White,38,85);
    if(!P->bHelp) DrawText(TEXT("F1 controles"),White,24,H-30);
    if(P->Session&&P->Session->Life) DrawText(P->Session->Life->GetStatusText(),White,24,136);
    DrawLine(W/2-6,H/2,W/2+6,H/2,Cyan); DrawLine(W/2,H/2-6,W/2,H/2+6,Cyan);
    if(P->NoticeSeconds>0) { DrawRect(Panel,W*.25f,H-100,W*.5f,40); DrawText(P->Notice,White,W*.25f+16,H-90); }
    if(P->bHelp)
    {
        DrawRect(Panel,22,H-230,460,204);
        const TCHAR* Lines[]={TEXT("CONTROLES   F1 ocultar"),TEXT("WASD mover | raton mirar | espacio saltar / subir"),
            TEXT("F / R3 volar | E,Q / R1,L1 velocidad | X freno"),TEXT("Ctrl bajar | Shift turbo | B crucero planetario"),
            TEXT("G / circulo agarrar, soltar o rescatar"),TEXT("T / triangulo cargar y soltar para lanzar"),
            TEXT("Clic / D-pad abajo usar poder seleccionado"),TEXT("TAB / D-pad arriba mantener rueda"),
            TEXT("H / Options heroes | M mapa | N destino | F5 guardar")};
        for(int32 I=0;I<9;++I) DrawText(Lines[I],I?White:Cyan,36,H-219+I*20);
    }
    if(P->bSettings)
    {
        DrawRect(Panel,W*.28f,H*.3f,W*.44f,230);
        DrawText(TEXT("AJUSTES DEL MUNDO"),Cyan,W*.3f,H*.32f,nullptr,1.4);
        DrawText(TEXT("F2 ciclo de dia 18 / 36 / 72 minutos\nF3 adelantar seis horas\nF4 activar / desactivar consecuencias\nF5 guardar\nEsc volver"),White,W*.3f,H*.39f);
    }
    if(P->Wheel->bOpen)
    {
        DrawRect(Panel,W*.19f,H*.14f,W*.62f,H*.72f);
        DrawText(TEXT("ELIGE UN PODER"),Cyan,W/2-80,H/2-24,nullptr,1.4);
        DrawText(TEXT("Suelta para equipar"),White,W/2-70,H/2+7);
        const float Radius=FMath::Min(W,H)*.25f;
        for(int32 I=0;I<P->Wheel->Slots.Num();++I)
        {
            const auto& S=P->Wheel->Slots[I]; const float A=2*PI*I/6;
            const float X=W/2+FMath::Cos(A)*Radius,Y=H/2-FMath::Sin(A)*Radius;
            const FLinearColor C=I==P->Wheel->HoveredSlot?Cyan:S.bAvailable?White:FLinearColor::Gray;
            DrawRect(FLinearColor(.08,.12,.16,.96),X-95,Y-30,190,60);
            DrawText(S.Label.ToString(),C,X-85,Y-22);
            DrawText(S.Cooldown>0?FString::Printf(TEXT("%.1f s"),S.Cooldown):TEXT("Listo"),C,X-85,Y+3);
        }
    }
}
AIBPlayGameMode::AIBPlayGameMode() { DefaultPawnClass=AIBPlayCharacter::StaticClass(); HUDClass=AIBPlayHUD::StaticClass(); }
void AIBPlayGameMode::StartPlay()
{
    bool Found=false; for(TActorIterator<AIBPlaySession> It(GetWorld());It;++It) { Found=true; break; }
    if(!Found) GetWorld()->SpawnActor<AIBPlaySession>();
    Super::StartPlay();
}
