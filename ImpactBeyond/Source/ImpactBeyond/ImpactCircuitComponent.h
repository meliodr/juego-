#pragma once

#include "Components/ActorComponent.h"
#include "ImpactCircuitComponent.generated.h"

class ACharacter;
class UStaticMeshComponent;
class UStaticMesh;
struct FImpactCircuitCanonical;
struct FImpactCircuitCanonicalDeleter
{
	void operator()(FImpactCircuitCanonical* Ptr) const;
};

UCLASS()
class UImpactCircuitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UImpactCircuitComponent();
	virtual ~UImpactCircuitComponent() override;
	void ToggleGrab();
	void ThrowCar();
	void BeginThrowCharge();
	void ReleaseThrowCharge();
	void ResetCircuit();
	bool GetGripTargets(FVector& LeftWorld, FVector& RightWorld) const;
	float GetGripWeight() const;
	bool IsChargingThrow() const { return bChargingThrow; }
	float GetThrowCharge() const;
	bool IsPlayingThrow() const;
	bool IsCarHeld() const;
	int32 GetCanonicalAcceptedHits() const;
	float GetCanonicalTargetHealth() const;

	static constexpr float ThrowReleaseTime = 0.18f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	enum class ECarState : uint8 { Free, Lifting, Held, WindingUp, Thrown };

	void ReleaseCar(ECarState NextState);
	void StartThrow(float Charge);
	void FitCarAppearance(UStaticMeshComponent* Chassis, UStaticMesh* Appearance);
	UStaticMeshComponent* FindGrabbable() const;

	UPROPERTY(EditDefaultsOnly, Category="Circuit|Throw", meta=(ClampMin="0.1", Units="s"))
	float FullChargeTime = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category="Circuit|Throw", meta=(ClampMin="1", Units="cm/s"))
	float MinimumThrowSpeed = 1300.0f;

	UPROPERTY(EditDefaultsOnly, Category="Circuit|Throw", meta=(ClampMin="1", Units="cm/s"))
	float MaximumThrowSpeed = 6800.0f;

	UPROPERTY(EditDefaultsOnly, Category="Circuit|Throw", meta=(ClampMin="1", Units="cm/s"))
	float PanelImpactThreshold = 1950.0f;

	UFUNCTION()
	void OnCarHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	TObjectPtr<ACharacter> Hero;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Car;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CircuitCar;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LaunchedVehicle;
	UPROPERTY()
	TObjectPtr<UStaticMesh> TaxiAppearance;
	UPROPERTY()
	TObjectPtr<UStaticMesh> ElectricAppearance;

	TArray<TWeakObjectPtr<UStaticMeshComponent>> Grabbables;
	TArray<TWeakObjectPtr<UStaticMeshComponent>> Vehicles;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CarAppearance;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Panel;

	ECarState CarState = ECarState::Free;
	float PhaseTime = 0.0f;
	float BlockedTime = 0.0f;
	float ChargeTime = 0.0f;
	float VehicleGripInset = 0.0f;
	float LaunchSpeed = 0.0f;
	bool bChargingThrow = false;
	FVector PrePhysicsCarVelocity = FVector::ZeroVector;
	FVector LiftOffset = FVector::ZeroVector;
	FVector ChassisExtent = FVector::ZeroVector;
	FQuat CarCarryRotationOffset = FQuat::Identity;
	FVector LeftChassisGrip = FVector::ZeroVector;
	FVector RightChassisGrip = FVector::ZeroVector;
	TUniquePtr<FImpactCircuitCanonical, FImpactCircuitCanonicalDeleter> Canonical;
	FGuid ThrowAttackId;
};
