#pragma once

#include "Components/ActorComponent.h"
#include "ImpactFlightFeedback.generated.h"

class UAudioComponent;
class ULineBatchComponent;
class USoundWaveProcedural;

/** Local flight audio and a short trail driven by actual speed, with no effect on movement. */
UCLASS()
class UImpactFlightFeedback : public UActorComponent
{
	GENERATED_BODY()

public:
	void UpdateFlightFeedback(float DeltaSeconds, bool bFlying, bool bTurbo);

	UPROPERTY(EditDefaultsOnly, Category="Flight Feedback", meta=(ClampMin="0.0", ClampMax="1.0"))
	float WindVolumeScale = 0.55f;

	UPROPERTY(EditDefaultsOnly, Category="Flight Feedback")
	bool bDrawTrail = true;

	UPROPERTY(EditDefaultsOnly, Category="Flight Feedback", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TrailOpacityScale = 0.65f;

	/** Air strokes are bounded relative to their current hand anchor, even during travel/teleports. */
	UPROPERTY(EditDefaultsOnly, Category="Flight Feedback", meta=(ClampMin="50.0", ClampMax="400.0", Units="cm"))
	float MaxTrailLengthCm = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	float ObservedSpeedCmS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	float ObservedAccelerationCmS2 = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	float SpeedStrength = 0.0f;

	/** A gust requires positive measured acceleration, not just a held turbo button. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	float BoostStrength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	int32 LastDrawnSegments = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	float LastTrailExtentCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	int32 DiscontinuityResets = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Flight Feedback|Observed")
	bool bUsingHandAnchors = false;

	float GetWindVolume() const { return WindVolume; }
	float GetWindPitch() const { return WindPitch; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	friend class FImpactFlightFeedbackPresentationTest;
	void AdvancePresentation(float DeltaSeconds, float Speed, bool bFlying, bool bTurbo, bool bDiscontinuity);
	FVector ResolveFlowDirection(const FVector& Velocity, const FVector& Fallback, bool bReset);
	void UpdateWind(float DeltaSeconds, bool bFlying, const FVector& Right);
	void CreateWind();
	void UpdateTrail(const FVector& Direction, const FVector& Right);
	void ClearTrail();

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> WindAudio;

	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> WindWave;

	UPROPERTY(Transient)
	TObjectPtr<ULineBatchComponent> FlightTrail;

	TArray<int16> WindLoop;
	FVector PreviousLocation = FVector::ZeroVector;
	FVector LastFlowDirection = FVector::ForwardVector;
	bool bHasMotionSample = false;
	float PreviousSpeed = 0.0f;
	float FlowPhase = 0.0f;
	float WindVolume = 0.0f;
	float WindPitch = 0.82f;
	float NearbyWall = 0.0f;
	float WallAccent = 0.0f;
	float WallCheckRemaining = 0.0f;
};
