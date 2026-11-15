// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ImpactBeyondCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAnimSequence;
class UImpactCircuitComponent;
class UImpactLandingComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AImpactBeyondCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Ground", meta=(ClampMin="1", Units="cm/s"))
	float WalkSpeed = 220.0f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Ground", meta=(ClampMin="1", Units="cm/s"))
	float RunSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Ground", meta=(ClampMin="1800", ClampMax="2400", Units="cm/s"))
	float SprintSpeed = 2100.0f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Ground", meta=(ClampMin="1200", ClampMax="1800", Units="cm"))
	float SuperJumpHeight = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Ground", meta=(ClampMin="100", Units="cm/s"))
	float SuperJumpMinSpeed = 1450.0f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Flight", meta=(ClampMin="0.3", ClampMax="0.6", Units="s"))
	float TakeoffDuration = 0.433f;

	/** Maximum speed of each flight level in centimeters per second. */
	UPROPERTY(EditDefaultsOnly, Category="Movement|Flight")
	TArray<float> FlightSpeeds = {900.0f, 1800.0f, 4400.0f, 7200.0f, 10500.0f};

public:

	/** Constructor */
	AImpactBeyondCharacter(const FObjectInitializer& ObjectInitializer);

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse,
		const FHitResult& Hit) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UImpactCircuitComponent> Circuit;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UImpactLandingComponent> ImpactLanding;

	void ToggleFlight();
	void IncreaseFlightSpeed();
	void DecreaseFlightSpeed();
	void SetFlightLevel(int32 Level);
	bool IsFlightBrakeHeld() const;
	bool IsSprintHeld() const;
	void RequestImpactLanding();
	void ToggleCircuitGrab();
	void TogglePerformancePreset();

	void UpdateFlightPresentation(float DeltaSeconds);
	void UpdateGroundMovement(float DeltaSeconds, const APlayerController* PlayerController);
	void UpdateActionAnimation(UAnimSequence* Animation, bool bLoop, float Rate = 1.0f);

	UPROPERTY()
	TObjectPtr<UAnimSequence> HoverPose;

	UPROPERTY()
	TObjectPtr<UAnimSequence> TravelPose;

	UPROPERTY()
	TObjectPtr<UAnimSequence> CarryPose;

	UPROPERTY()
	TObjectPtr<UAnimSequence> TakeoffPose;

	UPROPERTY()
	TObjectPtr<UAnimSequence> CarryTakeoffPose;

	UPROPERTY()
	TObjectPtr<UAnimSequence> ThrowPose;

	UPROPERTY()
	TObjectPtr<UAnimSequence> ImpactDivePose;

	float BaseCameraArmLength = 400.0f;
	float BaseFieldOfView = 90.0f;
	FVector BaseCameraSocketOffset = FVector::ZeroVector;
	bool bBaseCameraLag = false;
	bool bBaseCameraRotationLag = false;
	float FastFlightTime = 0.0f;
	float BaseGroundFriction = 0.0f;
	float BaseBrakingFriction = 0.0f;
	float BaseGroundAcceleration = 0.0f;
	float BaseGroundBraking = 0.0f;
	float BaseAirBraking = 0.0f;
	bool bBaseSeparateBrakingFriction = false;
	float TakeoffRemaining = 0.0f;

	FTransform GroundMeshTransform;
	FRotator FlightFacing = FRotator::ZeroRotator;
	float PreviousTravelYaw = 0.0f;
	float FlightBank = 0.0f;
	bool bPresentingFlight = false;
	bool bUsingTravelPose = false;
	bool bPlayingGroundThrow = false;
	bool bPresentingImpactDive = false;
	bool bSuperJumpInProgress = false;
	FTransform ImpactDiveMeshTransform;
	int32 FlightLevel = 0;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	const UImpactCircuitComponent* GetCircuit() const { return Circuit.Get(); }
	int32 GetFlightLevel() const { return FlightLevel + 1; }
	bool CanUseImpactLanding() const;
};

