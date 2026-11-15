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
	HoverPose = HoverAsset.Object;
	TravelPose = CruiseAsset.Object;
	CarryPose = CarryAsset.Object;

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
		PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::ThrowCar);
		PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, Circuit.Get(), &UImpactCircuitComponent::ThrowCar);
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
	StopJumping();
	Movement->SetMovementMode(MOVE_Flying);
	SetFlightLevel(0);
	if (bTakingOff)
	{
		// Let the movement component sweep the capsule during liftoff.
		Movement->Velocity.Z = 450.0f;
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
		if (!bPresentingFlight && CharacterMesh->GetAnimationMode() == EAnimationMode::AnimationBlueprint
			&& HoverPose && TravelPose)
		{
			GroundMeshTransform = CharacterMesh->GetRelativeTransform();
			FlightFacing = GetActorRotation();
			bUsingTravelPose = false;
			bPresentingFlight = true;
			UpdateFlightPresentation(0.0f);
		}
	}
	else if (bPresentingFlight)
	{
		bPresentingFlight = false;
		CharacterMesh->SetRelativeTransform(GroundMeshTransform);
		if (UImpactGripMesh* GripMesh = Cast<UImpactGripMesh>(CharacterMesh))
		{
			GripMesh->BeginFlightPoseBlend();
		}
		CharacterMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
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
	UAnimSequence* FlightAnimation = GripWeight > 0.1f && CarryPose ? CarryPose.Get()
		: (bTravel ? TravelPose.Get() : HoverPose.Get());
	UAnimSingleNodeInstance* AnimationInstance = CharacterMesh->GetSingleNodeInstance();
	if (!AnimationInstance || AnimationInstance->GetCurrentAsset() != FlightAnimation)
	{
		if (UImpactGripMesh* GripMesh = Cast<UImpactGripMesh>(CharacterMesh))
		{
			GripMesh->BeginFlightPoseBlend();
		}
		CharacterMesh->PlayAnimation(FlightAnimation, true);
	}
	CharacterMesh->SetPlayRate(FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, 3200.0f), FVector2D(0.65f, 1.15f), HorizontalSpeed));

	FRotator TargetFacing = FlightFacing;
	if (bTravel)
	{
		TargetFacing.Yaw = Velocity.Rotation().Yaw;
	}
	const float Lean = FMath::GetMappedRangeValueClamped(
		FVector2D(180.0f, 1500.0f), FVector2D(0.0f, 65.0f), HorizontalSpeed);
	// Pitch up slightly during a climb and down during a dive, even without forward input.
	const float VerticalPitch = FMath::Clamp(Velocity.Z / 60.0f, -15.0f, 15.0f);
	const FVector FacingRight = FRotationMatrix(FRotator(0.0f, FlightFacing.Yaw, 0.0f)).GetUnitAxis(EAxis::Y);
	TargetFacing.Pitch = FMath::Lerp(VerticalPitch - Lean, FMath::Clamp(VerticalPitch, -6.0f, 6.0f), GripWeight);
	TargetFacing.Roll = FMath::Clamp(FVector::DotProduct(Velocity, FacingRight) / 40.0f, -22.0f, 22.0f)
		* FMath::Lerp(1.0f, 0.28f, GripWeight);
	FlightFacing = FMath::RInterpTo(FlightFacing, TargetFacing, DeltaSeconds, 6.0f);

	// Rotate the mesh about the capsule center, retaining the mannequin's template offset.
	const FQuat VisualRotation = GetActorQuat().Inverse() * FlightFacing.Quaternion();
	CharacterMesh->SetRelativeLocationAndRotation(VisualRotation.RotateVector(GroundMeshTransform.GetLocation()),
		VisualRotation * GroundMeshTransform.GetRotation());
}

void AImpactBeyondCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength,
		BaseCameraArmLength + 175.0f * Circuit->GetGripWeight(), DeltaSeconds, 4.0f);
	if (bPresentingFlight)
	{
		UpdateFlightPresentation(DeltaSeconds);
	}
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!Movement->IsFlying() || !PlayerController || !IsLocallyControlled())
	{
		return;
	}

	const bool bBraking = IsFlightBrakeHeld();
	Movement->BrakingDecelerationFlying = bBraking ? 4200.0f : 1400.0f;
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
		// Read held modifiers when processing movement so a missed release cannot latch a gait.
		UCharacterMovementComponent* Movement = GetCharacterMovement();
		Movement->MaxWalkSpeed = RunSpeed;
		if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (Movement->IsMovingOnGround())
			{
				if (PlayerController->IsInputKeyDown(EKeys::LeftShift)
					|| PlayerController->IsInputKeyDown(EKeys::Gamepad_LeftThumbstick))
				{
					Movement->MaxWalkSpeed = SprintSpeed;
				}
				else if (PlayerController->IsInputKeyDown(EKeys::LeftControl))
				{
					Movement->MaxWalkSpeed = WalkSpeed;
				}
			}
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
