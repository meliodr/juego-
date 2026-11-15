#pragma once

#include "Components/ActorComponent.h"
#include "ImpactLandingComponent.generated.h"

class ACharacter;
class ULineBatchComponent;

/** Swept aerial impact and a small, authored set of world reactions. */
UCLASS()
class UImpactLandingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UImpactLandingComponent();
	void StartImpact();
	bool CanStartImpact() const;
	bool IsImpacting() const;
	bool IsDiving() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class EPhase : uint8 { Ready, Diving, Recovering };

	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);
	void ApplyShockwave(const FVector& Origin, float ImpactSpeed);
	void DrawShockwave() const;
	void PlayImpactSound(const FVector& Origin);

	UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(ClampMin="200", ClampMax="1000"))
	float Radius = 700.0f;

	UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(ClampMin="1000", ClampMax="4000"))
	float DiveSpeed = 2200.0f;

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> Hero;

	UPROPERTY(Transient)
	TObjectPtr<ULineBatchComponent> ShockwaveLines;

	EPhase Phase = EPhase::Ready;
	float PhaseTime = 0.0f;
	double EarliestStartTime = 0.0;
	FVector ImpactOrigin = FVector::ZeroVector;
	FVector ImpactNormal = FVector::UpVector;
};
