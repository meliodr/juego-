#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IB_S35/IBS35Types.h"
#include "IB_S35/IBS35Policy.h"
#include "IBS35PunchComponent.generated.h"

USTRUCT(BlueprintType)
struct FIBS35PunchProfile
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Windup = 0.09f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Active = 0.08f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Recovery = 0.18f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage = 20;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DeltaSpeed = 350;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Reach = 140;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 18;
};

UCLASS(ClassGroup=(IBS35), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS35PunchComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UIBS35PunchComponent();
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FIBS35PunchProfile Quick;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FIBS35PunchProfile Heavy;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDrawContacts = true;
    // Disable fallback only after a montage with IBS35Contact notify is actually connected.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseNotifyContact = false;
    UPROPERTY(BlueprintAssignable) FIBS35AttackCue OnAttackCue;
    UPROPERTY(BlueprintAssignable) FIBS35DamageEvent OnHit;
    UFUNCTION(BlueprintCallable) bool BeginPunch(bool bHeavy);
    UFUNCTION(BlueprintCallable) void CancelAttack();
    UFUNCTION(BlueprintCallable) void NotifyContact();
    UFUNCTION(BlueprintPure) bool IsAttacking() const { return bAttacking; }
    virtual void TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* Function) override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void TraceContact(double FromAlpha, double ToAlpha);
    FIBS35PunchProfile Current;
    IBS35::AttackTiming Timing{0.09, 0.08, 0.18};
    FGuid AttackId;
    FGuid InstigatorId;
    double Age = 0;
    bool bAttacking = false;
    bool bHeavyAttack = false;
    bool bTool = false;
    bool bContactCue = false;
    bool bPathBlocked = false;
    TWeakObjectPtr<class AIBS35Object> AttackTool;
    FVector ToolStart = FVector::ZeroVector;
    FVector AttackOrigin = FVector::ZeroVector;
    FVector AttackDirection = FVector::ForwardVector;
};
