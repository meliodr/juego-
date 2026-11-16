#include "IBS4LabPawn.h"
#include "IBS4PowerComponent.h"
#include "IBS4PresentationComponent.h"
#include "IBS4Threat.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

UIBS4FlightIntentComponent::UIBS4FlightIntentComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}
void UIBS4FlightIntentComponent::SetFlightIntent(bool Flying)
{
    bFlightIntent = Flying;
    if (auto* Character = Cast<ACharacter>(GetOwner()))
        Character->GetCharacterMovement()->SetMovementMode(Flying ? MOVE_Flying : MOVE_Falling);
}
void UIBS4FlightIntentComponent::TickComponent(float Dt, ELevelTick Type, FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt, Type, Fn);
    auto* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !bFlightIntent) return;
    auto* Movement = Character->GetCharacterMovement();
    if (Movement->MovementMode != MOVE_Flying) Movement->SetMovementMode(MOVE_Flying);
    // Native swept movement remains responsible for walls/floor. Correct velocity,
    // never position, when the capsule is near the prepared floor or a slope.
    const float Half = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const FVector Feet = Character->GetActorLocation() - FVector(0, 0, Half - 10);
    FHitResult Ground;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4LowFlight), false, Character);
    if (GetWorld()->SweepSingleByChannel(Ground, Feet, Feet - FVector(0, 0, 80), FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeSphere(10), Query) && Ground.ImpactNormal.Z > 0.55)
    {
        const float Clearance = FMath::Max(0.f, Ground.Distance);
        if (Clearance < 50 && Movement->Velocity.Z < 120)
            Movement->Velocity.Z = FMath::FInterpTo(Movement->Velocity.Z, (50 - Clearance) * 5, Dt, 18);
    }
}
AIBS4LabPawn::AIBS4LabPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(38, 90);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    Response = CreateDefaultSubobject<UIBS4ResponseComponent>(TEXT("Response")); Response->bH01 = true;
    Powers = CreateDefaultSubobject<UIBS4PowerComponent>(TEXT("Powers"));
    Flight = CreateDefaultSubobject<UIBS4FlightIntentComponent>(TEXT("FlightIntent"));
    CreateDefaultSubobject<UIBS4PresentationComponent>(TEXT("Feedback"));
    Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    Boom->SetupAttachment(RootComponent); Boom->TargetArmLength = 360; Boom->bUsePawnControlRotation = true;
    Boom->SocketOffset = FVector(0, 50, 65);
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); Camera->SetupAttachment(Boom);
    GetCharacterMovement()->MaxFlySpeed = 1400;
    GetCharacterMovement()->MaxAcceleration = 2200;
    GetCharacterMovement()->BrakingDecelerationFlying = 900;
    GetCharacterMovement()->BrakingFrictionFactor = 0;
    GetCharacterMovement()->MaxWalkSpeed = 500;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;
}
void AIBS4LabPawn::BeginPlay()
{
    Super::BeginPlay();
    Flight->SetFlightIntent(true);
    GetCharacterMovement()->AddTickPrerequisiteComponent(Flight);
    Response->OnResponse.AddDynamic(this, &AIBS4LabPawn::React);
}
void AIBS4LabPawn::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    auto* Enhanced = Cast<UEnhancedInputComponent>(Input);
    auto* PC = Cast<APlayerController>(GetController());
    if (!Enhanced || !PC || !PC->GetLocalPlayer()) return;
    auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!Subsystem) return;
    Mapping = NewObject<UInputMappingContext>(this);
    const auto Axis = [this](FKey Positive, FKey Negative, FKey Stick)
    {
        auto* Action = NewObject<UInputAction>(this); Action->ValueType = EInputActionValueType::Axis1D;
        Action->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
        Actions.Add(Action);
        Mapping->MapKey(Action, Positive);
        if (Negative.IsValid()) Mapping->MapKey(Action, Negative).Modifiers.Add(NewObject<UInputModifierNegate>(Mapping));
        if (Stick.IsValid()) Mapping->MapKey(Action, Stick).Modifiers.Add(NewObject<UInputModifierDeadZone>(Mapping));
        return Action;
    };
    Enhanced->BindAction(Axis(EKeys::W, EKeys::S, EKeys::Gamepad_LeftY), ETriggerEvent::Triggered, this, &AIBS4LabPawn::Forward);
    Enhanced->BindAction(Axis(EKeys::D, EKeys::A, EKeys::Gamepad_LeftX), ETriggerEvent::Triggered, this, &AIBS4LabPawn::Right);
    auto* Up = Axis(EKeys::SpaceBar, EKeys::LeftControl, EKeys::Gamepad_RightShoulder);
    Mapping->MapKey(Up, EKeys::Gamepad_LeftShoulder).Modifiers.Add(NewObject<UInputModifierNegate>(Mapping));
    Enhanced->BindAction(Up, ETriggerEvent::Triggered, this, &AIBS4LabPawn::Vertical);
    Enhanced->BindAction(Axis(EKeys::MouseX, FKey(), FKey()), ETriggerEvent::Triggered, this, &AIBS4LabPawn::Yaw);
    Enhanced->BindAction(Axis(EKeys::MouseY, FKey(), FKey()), ETriggerEvent::Triggered, this, &AIBS4LabPawn::Pitch);
    Enhanced->BindAction(Axis(EKeys::Gamepad_RightX, FKey(), FKey()), ETriggerEvent::Triggered, this, &AIBS4LabPawn::YawRate);
    Enhanced->BindAction(Axis(EKeys::Gamepad_RightY, FKey(), FKey()), ETriggerEvent::Triggered, this, &AIBS4LabPawn::PitchRate);
    const auto Button = [this](FKey Key, FKey Pad)
    {
        auto* Action = NewObject<UInputAction>(this); Actions.Add(Action);
        Mapping->MapKey(Action, Key); Mapping->MapKey(Action, Pad); return Action;
    };
    auto* Fire = Button(EKeys::LeftMouseButton, EKeys::Gamepad_RightTrigger);
    Enhanced->BindAction(Fire, ETriggerEvent::Started, this, &AIBS4LabPawn::Trigger);
    Enhanced->BindAction(Fire, ETriggerEvent::Completed, this, &AIBS4LabPawn::StopTrigger);
    Enhanced->BindAction(Fire, ETriggerEvent::Canceled, this, &AIBS4LabPawn::StopTrigger);
    Enhanced->BindAction(Button(EKeys::E, EKeys::Gamepad_FaceButton_Left), ETriggerEvent::Started, this, &AIBS4LabPawn::Grab);
    Enhanced->BindAction(Button(EKeys::Q, EKeys::Gamepad_FaceButton_Top), ETriggerEvent::Started, this, &AIBS4LabPawn::Punch);
    Enhanced->BindAction(Button(EKeys::F, EKeys::Gamepad_RightThumbstick), ETriggerEvent::Started, this, &AIBS4LabPawn::ToggleFlight);
    Enhanced->BindAction(Button(EKeys::R, EKeys::Gamepad_Special_Left), ETriggerEvent::Started, this, &AIBS4LabPawn::ResetLab);
    Enhanced->BindAction(Button(EKeys::G, EKeys::Gamepad_Special_Right), ETriggerEvent::Started, this, &AIBS4LabPawn::TestAlert);
    Subsystem->AddMappingContext(Mapping, 100);
}
void AIBS4LabPawn::Forward(const FInputActionValue& V) { AddMovementInput(FRotator(0, GetControlRotation().Yaw, 0).Vector(), V.Get<float>()); }
void AIBS4LabPawn::Right(const FInputActionValue& V) { AddMovementInput(FRotationMatrix(FRotator(0, GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::Y), V.Get<float>()); }
void AIBS4LabPawn::Vertical(const FInputActionValue& V) { if (Flight->bFlightIntent) AddMovementInput(FVector::UpVector, V.Get<float>()); }
void AIBS4LabPawn::Yaw(const FInputActionValue& V) { AddControllerYawInput(V.Get<float>()); }
void AIBS4LabPawn::Pitch(const FInputActionValue& V) { AddControllerPitchInput(-V.Get<float>()); }
void AIBS4LabPawn::YawRate(const FInputActionValue& V) { AddControllerYawInput(V.Get<float>() * GetWorld()->GetDeltaSeconds() * 110); }
void AIBS4LabPawn::PitchRate(const FInputActionValue& V) { AddControllerPitchInput(-V.Get<float>() * GetWorld()->GetDeltaSeconds() * 90); }
void AIBS4LabPawn::Trigger() { if (Powers->IsHolding()) Powers->Release(true); else Powers->SetLaser(true); }
void AIBS4LabPawn::StopTrigger() { Powers->SetLaser(false); }
void AIBS4LabPawn::Grab() { Powers->ToggleTelekinesis(); }
void AIBS4LabPawn::Punch() { Powers->Punch(); }
void AIBS4LabPawn::ToggleFlight() { Flight->SetFlightIntent(!Flight->bFlightIntent); }
void AIBS4LabPawn::ResetLab() { Powers->Release(); UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this))); }
void AIBS4LabPawn::TestAlert()
{
    FHitResult Dummy;
    const FIBS4Hit Incident = UIBS4ResponseComponent::MakeHit(GetWorld(), this, EIBS4Weapon::Punch, 0, Dummy);
    for (TActorIterator<AIBS4Threat> It(GetWorld()); It; ++It) It->ReportIncident(Incident.AttackId, this, true);
    UE_LOG(LogTemp, Display, TEXT("IBS4 explicit player-requested test alert id=%lld"), Incident.AttackId);
}
void AIBS4LabPawn::React(const FIBS4Hit& Hit, const FIBS4Outcome&)
{
    if (Hit.Weapon == EIBS4Weapon::Missile && Response->AppliedPushes != LastShakePushCount)
    {
        LastShakePushCount = Response->AppliedPushes;
        ShakeTime = 0.16f;
    }
}
void AIBS4LabPawn::Tick(float Dt)
{
    Super::Tick(Dt);
    ShakeTime = FMath::Max(0.f, ShakeTime - Dt);
    const float NewOffset = ShakeTime > 0 ? FMath::Sin(ShakeTime * 95) * FMath::Clamp(MicroShake, 0.f, 0.4f) * ShakeTime / 0.16f : 0;
    Camera->AddLocalRotation(FRotator(NewOffset - CameraShakeOffset, 0, 0));
    CameraShakeOffset = NewOffset;
}
void AIBS4LabPawn::EndPlay(const EEndPlayReason::Type Reason)
{
    if (auto* PC = Cast<APlayerController>(GetController())) if (auto* Local = PC->GetLocalPlayer())
        if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Local)) Subsystem->RemoveMappingContext(Mapping);
    Super::EndPlay(Reason);
}
