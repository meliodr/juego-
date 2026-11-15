#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IBS4PowerComponent.generated.h"

class UPhysicsHandleComponent;
class UPrimitiveComponent;
class UIBS4PresentationComponent;

/** Experimental creative-mode adapter. Owns ONLY IBS4Grabbable bodies.
 * Replace with S3.5's agreed carry API during integration; never enable both. */
UCLASS(ClassGroup=(IBS4), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS4PowerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UIBS4PowerComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Dt, ELevelTick Type, FActorComponentTickFunction* Fn) override;
    UFUNCTION(BlueprintCallable, Category="S4") bool ToggleTelekinesis();
    UFUNCTION(BlueprintCallable, Category="S4") void Release(bool bThrow = false);
    UFUNCTION(BlueprintCallable, Category="S4") void SetLaser(bool Active);
    UFUNCTION(BlueprintCallable, Category="S4") void Punch();
    UFUNCTION(BlueprintPure, Category="S4") bool IsHolding() const { return Held.IsValid(); }
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="100", ClampMax="1200")) float GrabRange = 900;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="50", ClampMax="600")) float CarryDistance = 450;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="1", ClampMax="500")) float MaxMass = 250;
private:
    UPROPERTY() TObjectPtr<UPhysicsHandleComponent> Handle;
    UPROPERTY() TObjectPtr<UIBS4PresentationComponent> Feedback;
    TWeakObjectPtr<UPrimitiveComponent> Held;
    bool bPreviousCCD = false;
    bool bLaser = false;
    float HeldRadius = 0;
    double NextLaser = 0;
    double NextPunch = 0;
    bool Aim(float Range, FHitResult& Hit, FVector& Start, FVector& End) const;
};
