#pragma once

#include "Components/ActorComponent.h"
#include "IB_S35/IBS35Types.h"
#include "ImpactLandingComponent.generated.h"

class ACharacter;
class ULineBatchComponent;
DECLARE_DELEGATE_RetVal_TwoParams(bool, FImpactLandingCanonicalStrike, const FIBS35DamageMessage&, const FHitResult&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FImpactLandingEvent, float, Speed, FVector, Origin, FVector, Normal);

/** Swept aerial impact and a small, authored set of world reactions. */
UCLASS()
class IMPACTBEYOND_API UImpactLandingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UImpactLandingComponent();
	void StartImpact();
	UFUNCTION(BlueprintCallable) bool StartImpactDive();
	UFUNCTION(BlueprintCallable) void CancelImpactDive();
	bool CanStartImpact() const;
	bool IsImpacting() const;
	bool IsDiving() const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Impact") bool bUseLocalGravity = false;
	UPROPERTY(BlueprintAssignable, Category="Impact") FImpactLandingEvent OnImpact;
	// When bound, all reactions use this damage owner; no direct impulse/panel fallback.
	FImpactLandingCanonicalStrike CanonicalStrike;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class EPhase : uint8 { Ready, Diving, Recovering };

	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);
	void ApplyShockwave(const FVector& Origin, float ImpactSpeed);
	void DrawShockwave() const;
	void PlayImpactSound(const FVector& Origin);
	FVector LocalUp() const;
	bool IsWalkableImpact(const FHitResult& Hit) const;

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
	FGuid AttackId;
	FGuid InstigatorId;
	float LastDescentSpeed = 0;
};
