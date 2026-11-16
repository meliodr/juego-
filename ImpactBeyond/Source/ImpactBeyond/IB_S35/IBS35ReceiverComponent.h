#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IB_S35/IBS35Policy.h"
#include "IB_S35/IBS35Types.h"
#include "IBS35ReceiverComponent.generated.h"

class UPrimitiveComponent;
DECLARE_DELEGATE_RetVal_TwoParams(bool, FIBS35CanonicalIngress, FIBS35DamageMessage&, UPrimitiveComponent*);
DECLARE_MULTICAST_DELEGATE(FIBS35ReceiverReset);

UCLASS(ClassGroup=(IBS35), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS35ReceiverComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ResponseProfile = TEXT("LabTarget");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float InitialHealth = 1000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxDeltaSpeed = 2000;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Health = 1000;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGuid TargetId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 AcceptedHits = 0;
    UPROPERTY(BlueprintAssignable) FIBS35DamageEvent OnDamage;
    bool Receive(FIBS35DamageMessage& Message, UPrimitiveComponent* Body);
    bool BindCanonicalIngress(UObject* Owner, FIBS35CanonicalIngress Ingress);
    bool UnbindCanonicalIngress(UObject* Owner);
    bool HasCanonicalIngress() const { return bCanonicalBound; }
    FIBS35ReceiverReset OnReceiverReset;
    UFUNCTION(BlueprintCallable) void ResetReceiver();
protected:
    virtual void BeginPlay() override;
private:
    IBS35::HitLedger<FGuid> Ledger;
    TWeakObjectPtr<UObject> CanonicalOwner;
    FIBS35CanonicalIngress CanonicalIngress;
    bool bCanonicalBound = false;
    bool bRouting = false;
};
