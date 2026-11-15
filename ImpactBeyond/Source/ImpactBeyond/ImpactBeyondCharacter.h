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

	UPROPERTY(EditDefaultsOnly, Category="Movement|Ground", meta=(ClampMin="1", Units="cm/s"))
	float SprintSpeed = 700.0f;

	/** Maximum speed of each flight level in centimeters per second. */
	UPROPERTY(EditDefaultsOnly, Category="Movement|Flight")
	TArray<float> FlightSpeeds = {900.0f, 1800.0f, 3200.0f};

public:

	/** Constructor */
	AImpactBeyondCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UImpactCircuitComponent> Circuit;

	void ToggleFlight();
	void IncreaseFlightSpeed();
	void DecreaseFlightSpeed();
	void SetFlightLevel(int32 Level);
	bool IsFlightBrakeHeld() const;

	void UpdateFlightPresentation(float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<UAnimSequence> HoverPose;

	UPROPERTY()
	TObjectPtr<UAnimSequence> TravelPose;

	FTransform GroundMeshTransform;
	FRotator FlightFacing = FRotator::ZeroRotator;
	bool bPresentingFlight = false;
	bool bUsingTravelPose = false;
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
};

