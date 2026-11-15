#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IB_S35/IBS35Policy.h"
#include "IB_S35/IBS35Types.h"
#include "IBS35ReceiverComponent.generated.h"

class UPrimitiveComponent;

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
    UFUNCTION(BlueprintCallable) void ResetReceiver();
protected:
    virtual void BeginPlay() override;
private:
    IBS35::HitLedger<FGuid> Ledger;
};
