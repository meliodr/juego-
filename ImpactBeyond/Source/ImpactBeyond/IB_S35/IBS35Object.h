#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IB_S35/IBS35Policy.h"
#include "IB_S35/IBS35Types.h"
#include "IBS35Object.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UIBS35GripComponent;
class UIBS35ReceiverComponent;

UCLASS()
class IMPACTBEYOND_API AIBS35Object : public AActor
{
    GENERATED_BODY()
public:
    AIBS35Object();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UBoxComponent> Body;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Appearance;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UIBS35ReceiverComponent> Receiver;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FIBS35ObjectProfile Profile;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UIBS35ObjectProfileAsset> ProfileAsset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRadialGravity = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector PlanetCenter = FVector::ZeroVector;
    UFUNCTION(BlueprintCallable) bool ResetObject();
    /** Clear transient attack identity before restoring physics; held objects are refused. */
    UFUNCTION(BlueprintCallable) bool ClearLaunchForRestore();
    UFUNCTION(BlueprintCallable, CallInEditor) void RefreshProfile();
    UFUNCTION(BlueprintPure) bool IsHeld() const { return Lease.Carrier != 0; }
    IBS35::HoldState GetHoldState() const { return Lease.State; }
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    friend class UIBS35GripComponent;
    bool Claim(UIBS35GripComponent* Grip);
    bool Release(UIBS35GripComponent* Grip, bool bLaunch, const FVector& Velocity);
    bool ClearCarrierOverlap();
    void RestoreBody();
    UFUNCTION() void OnBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
    IBS35::Lease Lease;
    TWeakObjectPtr<UIBS35GripComponent> Holder;
    TWeakObjectPtr<AActor> IgnoredCarrier;
    bool bBodyIgnoredCarrier = false;
    bool bCarrierIgnoredBody = false;
    bool bSavedSimulation = true;
    FTransform StartTransform;
    FGuid LaunchId;
    FGuid LaunchInstigator;
    TWeakObjectPtr<AActor> LaunchInstigatorActor;
    float FlightAge = 0;
    FVector PrePhysicsVelocity = FVector::ZeroVector;
};
