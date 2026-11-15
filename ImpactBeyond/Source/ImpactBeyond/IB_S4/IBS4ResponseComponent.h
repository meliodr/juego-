#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IBS4Policy.h"
#include "IBS4ResponseComponent.generated.h"

class UPrimitiveComponent;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIBS4ResponseEvent, const FIBS4Hit&, Hit, float, HealthDelta);

/** Only lab participants opt in. Never attached to the shared H01 automatically. */
UCLASS(ClassGroup=(IBS4), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS4ResponseComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bH01 = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") EIBS4Surface Surface = EIBS4Surface::Metal;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") float Health = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="0", ClampMax="1200")) float MissilePushSpeed = 1000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="0.5")) float PushCooldown = 1.1f;
    UPROPERTY(BlueprintAssignable) FIBS4ResponseEvent OnResponse;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") int32 AcceptedHits = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") int32 AppliedPushes = 0;
    UFUNCTION(BlueprintCallable, Category="S4") bool Receive(const FIBS4Hit& Hit);
    UFUNCTION(BlueprintCallable, Category="S4") void RestoreHealth();
    static FIBS4Hit MakeHit(UWorld* World, AActor* Instigator, EIBS4Weapon Weapon, float Amount, const FHitResult& Trace);
private:
    IBS4::Receiver Policy;
};
