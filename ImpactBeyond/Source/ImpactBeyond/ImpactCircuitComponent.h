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

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class ECarState : uint8 { Free, Held, Thrown };

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
};
