#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IB_S35/IBS35Types.h"
#include "IB_S4/IBS4ResponseComponent.h"
#include "IBPlayCombat.generated.h"

class UIBS35ReceiverComponent;
class UPrimitiveComponent;
struct FIBPlayCombatRuntime;

/** Post-dedup accepted contact; also covers physical throws and native S4 ingress. */
DECLARE_MULTICAST_DELEGATE_FourParams(FIBPlayCanonicalImpact, FGuid, AActor*, FVector, float);

/** Owns the canonical receiver for one opted-in actor life. S4 is its health owner. */
UCLASS(ClassGroup=(Impact), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBPlayCombat : public UActorComponent
{
    GENERATED_BODY()
public:
    UIBPlayCombat();
    UIBPlayCombat(FVTableHelper& Helper);
    virtual ~UIBPlayCombat() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    bool InitializeReceiver();
    bool DispatchS35(const FIBS35DamageMessage& Message, const FHitResult& Hit);
    bool DispatchS4(const FIBS4Hit& Hit, const FHitResult& Trace);
    /** One civic cause for a native attack across every receiver in this process. */
    static FGuid NativeCauseId(int64 AttackSerial);
    FIBPlayCanonicalImpact OnCanonicalImpact;
    UFUNCTION(BlueprintPure) FGuid GetTargetLife() const;
    UFUNCTION(BlueprintPure) float GetHealth() const;
    UFUNCTION(BlueprintCallable) void ResetCombat();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UIBS4ResponseComponent> Response;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UIBS35ReceiverComponent> ContactReceiver;
private:
    TUniquePtr<FIBPlayCombatRuntime> Runtime;
    FDelegateHandle ResetHandle;
    bool bEndingPlay = false;
    bool RouteContact(FIBS35DamageMessage& Message, UPrimitiveComponent* Body);
    void RouteNativeS4(UIBS4ResponseComponent& Receiver, const FIBS4Hit& Hit);
    float FilterDamage(const FIBS4Hit& Hit, float Damage);
    void ReceiverReset();
    void RefreshHeroState();
};
