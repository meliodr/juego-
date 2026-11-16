#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IBS4Policy.h"
#include "IBS4ResponseComponent.generated.h"

class UPrimitiveComponent;
class UIBS4ResponseComponent;

UENUM(BlueprintType)
enum class EIBS4Weapon : uint8 { Bullet, Missile, Laser, Punch };
UENUM(BlueprintType)
enum class EIBS4Surface : uint8 { Metal, Concrete, Glass };

USTRUCT(BlueprintType)
struct FIBS4Hit
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int64 AttackId = 0;
    UPROPERTY(BlueprintReadOnly) double IssuedAt = 0;
    UPROPERTY(BlueprintReadOnly) EIBS4Weapon Weapon = EIBS4Weapon::Bullet;
    UPROPERTY(BlueprintReadOnly) float Amount = 0;
    UPROPERTY(BlueprintReadOnly) FVector Point = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FVector Direction = FVector::ForwardVector;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<AActor> Instigator = nullptr;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<UPrimitiveComponent> Part = nullptr;
};
USTRUCT(BlueprintType)
struct FIBS4Outcome
{
    GENERATED_BODY()
    // Policy-filtered request for structural resistance; never a second health mutation.
    UPROPERTY(BlueprintReadOnly) float RequestedHealthDelta = 0;
    // Actual change in stored HP after clamping and float rounding.
    UPROPERTY(BlueprintReadOnly) float HealthDelta = 0;
    // Requested CMC velocity change in cm/s; not measured displacement/velocity.
    // This is NOT kg*cm/s or an impulse for Chaos bodies.
    UPROPERTY(BlueprintReadOnly) FVector DeltaVelocity = FVector::ZeroVector;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIBS4ResponseEvent, const FIBS4Hit&, Hit, const FIBS4Outcome&, Outcome);
// C++ seam only. The sole canonical adapter owns translation/routing; S4 owns
// no canonical enum, GUID map, target registry, or second global damage engine.
DECLARE_DELEGATE_TwoParams(FIBS4IngressRouter, UIBS4ResponseComponent&, const FIBS4Hit&);
// Returns remaining positive damage after canonical dedup. Cannot increase damage.
DECLARE_DELEGATE_RetVal_TwoParams(float, FIBS4BoundDamageFilter, const FIBS4Hit&, float);

/** Only lab participants opt in. Never attached to the shared H01 automatically. */
UCLASS(ClassGroup=(IBS4), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS4ResponseComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bH01 = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") EIBS4Surface Surface = EIBS4Surface::Metal;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") float Health = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="0", ClampMax="1200", Units="cm/s")) float MissilePushSpeed = 1000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="0.5")) float PushCooldown = 1.1f;
    UPROPERTY(BlueprintAssignable) FIBS4ResponseEvent OnResponse;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") int32 AcceptedHits = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") int32 AppliedPushes = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") FIBS4Outcome LastOutcome;
    UFUNCTION(BlueprintCallable, Category="S4") bool Receive(const FIBS4Hit& Hit);
    UFUNCTION(BlueprintCallable, Category="S4") void RestoreHealth();
    // Explicit single consumer. A missing/expired router fails closed, never
    // silently falls back to standalone mutation. Bind/unbind preserve policy.
    bool BindConsumer(UObject* Owner, FIBS4IngressRouter Router);
    bool UnbindConsumer(UObject* Owner);
    bool SetBoundDamageFilter(UObject* Owner, FIBS4BoundDamageFilter Filter);
    bool ConsumeBoundHit(UObject* Owner, const FIBS4Hit& Hit, bool bPhysicsAlreadyResolved);
    bool HasConsumerBinding() const { return bConsumerBound; }
    // Use once per translated contact, cache the result in the canonical adapter.
    // Do not allocate a new ID on redelivery or truncate a canonical GUID.
    static int64 AllocateAttackId();
    static FIBS4Hit MakeHit(UWorld* World, AActor* Instigator, EIBS4Weapon Weapon, float Amount, const FHitResult& Trace);
private:
    bool ReceiveInternal(const FIBS4Hit& Hit, bool bPhysicsAlreadyResolved, bool bProtectAllH01);
    IBS4::Receiver Policy;
    TWeakObjectPtr<UObject> ConsumerOwner;
    FIBS4IngressRouter ConsumerRouter;
    FIBS4BoundDamageFilter DamageFilter;
    bool bConsumerBound = false;
    bool bRouting = false;
    bool bConsuming = false;
    bool bRouteCommitted = false;
    int64 RoutedAttackId = 0;
    double RoutedIssuedAt = 0;
};
