#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IBS35GripComponent.generated.h"

class AIBS35Object;

UCLASS(ClassGroup=(IBS35), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS35GripComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UIBS35GripComponent();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid InstigatorId;
    UFUNCTION(BlueprintCallable) bool TryGrab(AIBS35Object* Object, bool bTelekinesis = false);
    UFUNCTION(BlueprintCallable) bool GrabNearest();
    UFUNCTION(BlueprintCallable) void Drop();
    UFUNCTION(BlueprintCallable) bool BeginCharge();
    UFUNCTION(BlueprintCallable) void CancelCharge();
    UFUNCTION(BlueprintCallable) bool CommitThrow(FVector Direction);
    UFUNCTION(BlueprintPure) AIBS35Object* GetHeld() const { return Held.Get(); }
    UFUNCTION(BlueprintPure) bool GetGripTargets(FVector& LeftWorld, FVector& RightWorld) const;
    UFUNCTION(BlueprintPure) float GetGripWeight() const;
    UFUNCTION(BlueprintPure) float GetGestureLoad() const;
    UFUNCTION(BlueprintPure) bool IsCharging() const;
    bool IsTelekinetic() const { return bRemoteCarry && Held.IsValid(); }
    // Tool motion still goes through the real root box sweep.
    bool SweepTool(const FVector& Delta, FHitResult& Hit);
    void SetToolActive(bool bActive) { bToolActive = bActive; }
    virtual void TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* Function) override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    TWeakObjectPtr<AIBS35Object> Held;
    float HeldSeconds = 0;
    float ChargeSeconds = 0;
    FVector LiftStart = FVector::ZeroVector;
    bool bToolActive = false;
    bool bRemoteCarry = false;
};
