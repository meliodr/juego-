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

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	struct FTrailPoint
	{
		FVector Left;
		FVector Right;
		double Time;
	};

	void UpdateWind(float DeltaSeconds, float Speed, bool bFlying, bool bTurbo, const FVector& Right);
	void CreateWind();
	void UpdateTrail(float DeltaSeconds, float Speed, bool bFlying, bool bTurbo, const FVector& Right);
	void ClearTrail();

	UPROPERTY(EditDefaultsOnly, Category="Flight Feedback", meta=(ClampMin="0.0", ClampMax="1.0"))
	float WindVolumeScale = 0.72f;

	UPROPERTY(EditDefaultsOnly, Category="Flight Feedback")
	bool bDrawTrail = true;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> WindAudio;

	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> WindWave;

	UPROPERTY(Transient)
	TObjectPtr<ULineBatchComponent> FlightTrail;

	TArray<int16> WindLoop;
	TArray<FTrailPoint, TInlineAllocator<12>> Trail;
	float WindVolume = 0.0f;
	float WindPitch = 0.82f;
	float TurboBurst = 0.0f;
	bool bWasTurbo = false;
	float NearbyWall = 0.0f;
	float WallAccent = 0.0f;
	float WallCheckRemaining = 0.0f;
	float TrailAccumulator = 0.0f;
};
