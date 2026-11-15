#pragma once

#include "Components/ActorComponent.h"
#include "ImpactCircuitComponent.generated.h"

class ACharacter;
class UStaticMeshComponent;

UCLASS()
class UImpactCircuitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UImpactCircuitComponent();
	void ToggleGrab();
	void ThrowCar();
	void ResetCircuit();
	bool GetGripTargets(FVector& LeftWorld, FVector& RightWorld) const;
	float GetGripWeight() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class ECarState : uint8 { Free, Lifting, Held, WindingUp, Thrown };

	void ReleaseCar(ECarState NextState);

	UFUNCTION()
	void OnCarHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	TObjectPtr<ACharacter> Hero;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Car;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Panel;

	ECarState CarState = ECarState::Free;
	float PhaseTime = 0.0f;
	float BlockedTime = 0.0f;
	FVector LiftOffset = FVector::ZeroVector;
	FVector ChassisExtent = FVector::ZeroVector;
};
