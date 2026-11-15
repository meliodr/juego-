// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImpactBeyondCharacter.h"
#include "ImpactCircuitComponent.h"
#include "ImpactGripMesh.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ImpactBeyond.h"

AImpactBeyondCharacter::AImpactBeyondCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UImpactGripMesh>(ACharacter::MeshComponentName))
{
	Circuit = CreateDefaultSubobject<UImpactCircuitComponent>(TEXT("Circuit"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> HoverAsset(
		TEXT("/Game/IB_Flight/A_FlightHover.A_FlightHover"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> CruiseAsset(
		TEXT("/Game/IB_Flight/A_FlightCruise.A_FlightCruise"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> CarryAsset(
		TEXT("/Game/IB_Flight/A_FlightCarry.A_FlightCarry"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> TakeoffAsset(
		TEXT("/Game/IB_Flight/A_FlightTakeoff.A_FlightTakeoff"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> CarryTakeoffAsset(
		TEXT("/Game/IB_Flight/A_FlightCarryTakeoff.A_FlightCarryTakeoff"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ThrowAsset(
		TEXT("/Game/IB_Flight/A_FlightThrow.A_FlightThrow"));
	HoverPose = HoverAsset.Object;
	TravelPose = CruiseAsset.Object;
	CarryPose = CarryAsset.Object;
	TakeoffPose = TakeoffAsset.Object;
	CarryTakeoffPose = CarryTakeoffAsset.Object;
	ThrowPose = ThrowAsset.Object;

	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->BrakingDecelerationFlying = 1400.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AImpactBeyondCharacter::BeginPlay()
{
	Super::BeginPlay();
	BaseCameraArmLength = CameraBoom->TargetArmLength;
	BaseFieldOfView = FollowCamera->FieldOfView;
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	BaseGroundFriction = Movement->GroundFriction;
	BaseBrakingFriction = Movement->BrakingFriction;
	BaseGroundAcceleration = Movement->MaxAcceleration;
	BaseGroundBraking = Movement->BrakingDecelerationWalking;
	bBaseSeparateBrakingFriction = Movement->bUseSeparateBrakingFriction;
	if (UImpactGripMesh* GripMesh = Cast<UImpactGripMesh>(GetMesh()))
	{
		GripMesh->SetCircuit(Circuit);
	}
}

void AImpactBeyondCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AImpactBeyondCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AImpactBeyondCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AImpactBeyondCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AImpactBeyondCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AImpactBeyondCharacter::Look);

		PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &AImpactBeyondCharacter::ToggleFlight);
		PlayerInputComponent->BindKey(EKeys::Gamepad_RightThumbstick, IE_Pressed, this, &AImpactBeyondCharacter::ToggleFlight);
		PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AImpactBeyondCharacter::IncreaseFlightSpeed);
		PlayerInputComponent->BindKey(EKeys::Gamepad_RightShoulder, IE_Pressed, this, &AImpactBeyondCharacter::IncreaseFlightSpeed);
		PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AImpactBeyondCharacter::DecreaseFlightSpeed);
		PlayerInputComponent->BindKey(EKeys::Gamepad_LeftShoulder, IE_Pressed, this, &AImpactBeyondCharacter::DecreaseFlightSpeed);
		PlayerInputComponent->BindKey(EKeys::G, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::ToggleGrab);
		PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::ToggleGrab);
		PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::BeginThrowCharge);
		PlayerInputComponent->BindKey(EKeys::T, IE_Released, Circuit.Get(), &UImpactCircuitComponent::ReleaseThrowCharge);
		PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::BeginThrowCharge);
		PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Released, Circuit.Get(), &UImpactCircuitComponent::ReleaseThrowCharge);
		PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::ResetCircuit);
		PlayerInputComponent->BindKey(EKeys::Gamepad_DPad_Down, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::ResetCircuit);
	}
	else
	{
		UE_LOG(LogImpactBeyond, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AImpactBeyondCharacter::ToggleFlight()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement->IsFlying())
	{
		Movement->SetMovementMode(MOVE_Falling);
		return;
	}

	const bool bTakingOff = Movement->IsMovingOnGround();
	const FVector GroundVelocity = Movement->Velocity;
	TakeoffRemaining = bTakingOff ? TakeoffDuration : 0.0f;
	StopJumping();
	Movement->SetMovementMode(MOVE_Flying);
	SetFlightLevel(0);
	if (bTakingOff)
	{
		// Let the movement component sweep the capsule during liftoff.
		Movement->Velocity = FVector(GroundVelocity.X, GroundVelocity.Y, 450.0f);
	}
}

void AImpactBeyondCharacter::IncreaseFlightSpeed()
{
	SetFlightLevel(FlightLevel + 1);
}

void AImpactBeyondCharacter::DecreaseFlightSpeed()
{
	SetFlightLevel(FlightLevel - 1);
}

void AImpactBeyondCharacter::SetFlightLevel(int32 Level)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement->IsFlying() || FlightSpeeds.IsEmpty())
	{
		return;
	}
	FlightLevel = FMath::Clamp(Level, 0, FlightSpeeds.Num() - 1);
	Movement->MaxFlySpeed = FMath::Max(1.0f, FlightSpeeds[FlightLevel]);
}

bool AImpactBeyondCharacter::IsFlightBrakeHeld() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	return PlayerController && (PlayerController->IsInputKeyDown(EKeys::X)
		|| PlayerController->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left));
}

void AImpactBeyondCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (GetCharacterMovement()->IsFlying())
	{
		// The template's ground AnimBP would feed horizontal airspeed into its gait blend.
		if (!bPresentingFlight && HoverPose && TravelPose)
		{
			GroundMeshTransform = CharacterMesh->GetRelativeTransform();
			FlightFacing = GetActorRotation();
			PreviousTravelYaw = FlightFacing.Yaw;
			FlightBank = 0.0f;
			bUsingTravelPose = false;
			bPresentingFlight = true;
			bPlayingGroundThrow = false;
			CharacterMesh->GlobalAnimRateScale = 1.0f;
			UpdateFlightPresentation(0.0f);
		}
	}
	else if (bPresentingFlight)
	{
		bPresentingFlight = false;
		TakeoffRemaining = 0.0f;
		CharacterMesh->SetRelativeTransform(GroundMeshTransform);
		if (UImpactGripMesh* GripMesh = Cast<UImpactGripMesh>(CharacterMesh))
		{
			GripMesh->BeginFlightPoseBlend();
		}
		CharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
}

void AImpactBeyondCharacter::UpdateActionAnimation(UAnimSequence* Animation, bool bLoop, float Rate)
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	UAnimSingleNodeInstance* AnimationInstance = CharacterMesh->GetSingleNodeInstance();
	if (!AnimationInstance || AnimationInstance->GetCurrentAsset() != Animation)
	{
		if (UImpactGripMesh* GripMesh = Cast<UImpactGripMesh>(CharacterMesh))
		{
			GripMesh->BeginFlightPoseBlend(Animation == ThrowPose.Get() ? 0.08f : 0.18f);
		}
		CharacterMesh->PlayAnimation(Animation, bLoop);
	}
	CharacterMesh->SetPlayRate(Rate);
}

void AImpactBeyondCharacter::UpdateFlightPresentation(float DeltaSeconds)
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	const FVector Velocity = GetCharacterMovement()->Velocity;
	const float HorizontalSpeed = Velocity.Size2D();
	const float GripWeight = Circuit->GetGripWeight();
	// Hysteresis prevents pose flicker at the boundary between hover and travel.
	const bool bTravel = HorizontalSpeed >= (bUsingTravelPose ? 120.0f : 200.0f);
	bUsingTravelPose = bTravel;
	const bool bCarrying = GripWeight > 0.1f;
	const bool bTakingOff = TakeoffRemaining > 0.0f;
	UAnimSequence* LaunchPose = bCarrying ? CarryTakeoffPose.Get() : TakeoffPose.Get();
	UAnimSequence* FlightAnimation = bCarrying && CarryPose ? CarryPose.Get()
		: (bTravel ? TravelPose.Get() : HoverPose.Get());
	if (Circuit->IsPlayingThrow() && ThrowPose)
	{
		UpdateActionAnimation(ThrowPose.Get(), false);
	}
	else if (bTakingOff && LaunchPose)
	{
		UpdateActionAnimation(LaunchPose, false, LaunchPose->GetPlayLength() / TakeoffDuration);
	}
	else
	{
		UpdateActionAnimation(FlightAnimation, true, FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, GetCharacterMovement()->MaxFlySpeed), FVector2D(0.75f, 1.15f), HorizontalSpeed));
	}

	const FRotator TravelDirection = Velocity.Rotation();
	FRotator TargetFacing = FlightFacing;
	if (bTravel)
	{
		TargetFacing.Yaw = TravelDirection.Yaw;
	}
	if (bCarrying)
	{
		TargetFacing.Yaw = GetActorRotation().Yaw;
	}
	float Lean = FMath::GetMappedRangeValueClamped(
		FVector2D(180.0f, 1500.0f), FVector2D(0.0f, 65.0f), HorizontalSpeed);
	if (bTakingOff)
	{
		Lean *= 1.0f - TakeoffRemaining / TakeoffDuration;
	}
	if (IsFlightBrakeHeld())
	{
		Lean *= 0.2f;
	}
	// Use the trajectory pitch and filtered turn rate, following ManOfSteel's UFlight.
	const float VerticalBlend = FMath::SmoothStep(70.0, 600.0, Velocity.Size());
	const float TrajectoryPitch = FMath::ClampAngle(TravelDirection.Pitch, -85.0f, 85.0f);
	const float BodyPitch = FMath::Clamp(TrajectoryPitch * VerticalBlend - Lean, -85.0f, 35.0f);
	TargetFacing.Pitch = FMath::Lerp(BodyPitch, 0.0f, GripWeight);
	const float TurnRate = bTravel && DeltaSeconds > UE_SMALL_NUMBER
		? FMath::FindDeltaAngleDegrees(PreviousTravelYaw, TravelDirection.Yaw) / DeltaSeconds : 0.0f;
	PreviousTravelYaw = bTravel ? TravelDirection.Yaw : TargetFacing.Yaw;
	FlightBank = FMath::FInterpTo(FlightBank, FMath::Clamp(TurnRate / 180.0f, -1.0f, 1.0f), DeltaSeconds, 5.0f);
	TargetFacing.Roll = -28.0f * FlightBank * (1.0f - GripWeight);
	FlightFacing = FMath::RInterpTo(FlightFacing, TargetFacing, DeltaSeconds, 9.0f);
	FlightFacing.Yaw = FMath::Lerp(FlightFacing.Yaw,
		FlightFacing.Yaw + FMath::FindDeltaAngleDegrees(FlightFacing.Yaw, TargetFacing.Yaw), GripWeight);

	// Rotate the mesh about the capsule center, retaining the mannequin's template offset.
	const FQuat VisualRotation = GetActorQuat().Inverse() * FlightFacing.Quaternion();
	CharacterMesh->SetRelativeLocationAndRotation(VisualRotation.RotateVector(GroundMeshTransform.GetLocation()),
		VisualRotation * GroundMeshTransform.GetRotation());
}

void AImpactBeyondCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UpdateGroundMovement(DeltaSeconds, PlayerController);
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength,
		BaseCameraArmLength + 175.0f * Circuit->GetGripWeight(), DeltaSeconds, 4.0f);
	TakeoffRemaining = FMath::Max(0.0f, TakeoffRemaining - DeltaSeconds);
	if (bPresentingFlight)
	{
		UpdateFlightPresentation(DeltaSeconds);
	}
	else if (Circuit->IsPlayingThrow() && ThrowPose)
	{
		bPlayingGroundThrow = true;
		GetMesh()->GlobalAnimRateScale = 1.0f;
		UpdateActionAnimation(ThrowPose.Get(), false);
	}
	else if (bPlayingGroundThrow)
	{
		bPlayingGroundThrow = false;
		CastChecked<UImpactGripMesh>(GetMesh())->BeginFlightPoseBlend();
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
	if (!Movement->IsFlying() || !PlayerController || !IsLocallyControlled())
	{
		return;
	}

	const bool bBraking = IsFlightBrakeHeld();
	Movement->BrakingDecelerationFlying = bBraking ? 4200.0f : (TakeoffRemaining > 0.0f ? 500.0f : 1400.0f);
	if (bBraking)
	{
		ConsumeMovementInputVector();
	}
	else
	{
		const float Up = PlayerController->IsInputKeyDown(EKeys::SpaceBar) ? 1.0f
			: PlayerController->GetInputAnalogKeyState(EKeys::Gamepad_RightTriggerAxis);
		const float Down = PlayerController->IsInputKeyDown(EKeys::C) ? 1.0f
			: PlayerController->GetInputAnalogKeyState(EKeys::Gamepad_LeftTriggerAxis);
		if (Down - Up > 0.12f && Movement->Velocity.Z <= 0.0f)
		{
			FFindFloorResult Floor;
			Movement->FindFloor(GetActorLocation(), Floor, false);
			if (Floor.IsWalkableFloor() && Floor.GetDistanceToFloor() <= 4.0f)
			{
				Movement->SetMovementMode(MOVE_Walking);
				return;
			}
		}
		AddMovementInput(FVector::UpVector, Up - Down);
	}

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1041, 0.1f, FColor::Cyan,
			FString::Printf(TEXT("VUELO %d/%d | %.1f m/s%s"), FlightLevel + 1, FlightSpeeds.Num(),
				Movement->Velocity.Size() / 100.0f, bBraking ? TEXT(" | FRENO") : TEXT("")));
	}
#endif
}

void AImpactBeyondCharacter::UpdateGroundMovement(float DeltaSeconds, const APlayerController* PlayerController)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const bool bGrounded = Movement->IsMovingOnGround();
	const bool bSprinting = bGrounded && PlayerController && (PlayerController->IsInputKeyDown(EKeys::LeftShift)
		|| PlayerController->IsInputKeyDown(EKeys::Gamepad_LeftThumbstick));
	const bool bWalking = bGrounded && PlayerController && PlayerController->IsInputKeyDown(EKeys::LeftControl);
	const float GroundSpeed = Movement->Velocity.Size2D();
	const bool bFastGroundMovement = bGrounded && (bSprinting || GroundSpeed > RunSpeed + 30.0f);
	Movement->MaxWalkSpeed = bSprinting ? SprintSpeed : (bWalking ? WalkSpeed : RunSpeed);
	Movement->MaxAcceleration = Movement->IsFlying() ? 3200.0f + 1150.0f * FlightLevel
		: (bFastGroundMovement ? 2600.0f : BaseGroundAcceleration);
	Movement->GroundFriction = bFastGroundMovement ? 3.0f : BaseGroundFriction;
	Movement->BrakingDecelerationWalking = bFastGroundMovement ? 2200.0f : BaseGroundBraking;
	Movement->bUseSeparateBrakingFriction = bFastGroundMovement || bBaseSeparateBrakingFriction;
	Movement->BrakingFriction = bFastGroundMovement ? 0.4f : BaseBrakingFriction;

	const float SpeedAlpha = bGrounded ? FMath::GetMappedRangeValueClamped(
		FVector2D(RunSpeed, SprintSpeed), FVector2D(0.0f, 1.0f), GroundSpeed) : 0.0f;
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView,
		BaseFieldOfView + 7.0f * SpeedAlpha, DeltaSeconds, 3.5f));
	GetMesh()->GlobalAnimRateScale = FMath::FInterpTo(GetMesh()->GlobalAnimRateScale,
		1.0f + 0.85f * SpeedAlpha, DeltaSeconds, 8.0f);
}

void AImpactBeyondCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AImpactBeyondCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AImpactBeyondCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		if (GetCharacterMovement()->IsFlying() && IsFlightBrakeHeld())
		{
			return;
		}
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AImpactBeyondCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AImpactBeyondCharacter::DoJumpStart()
{
	// signal the character to jump
	if (!GetCharacterMovement()->IsFlying())
	{
		Jump();
	}
}

void AImpactBeyondCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
