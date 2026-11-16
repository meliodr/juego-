#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IB_S35/IBS35Types.h"
#include "IB_S35/IBS35Policy.h"
#include "IB_S35/IBS35PunchComponentPolicy.h"
#include "IBS35PunchComponent.generated.h"

UENUM(BlueprintType)
enum class EIBS35PunchStyle : uint8 { Quick, Heavy, Uppercut };

DECLARE_DELEGATE_RetVal_TwoParams(bool, FIBS35PunchCanonicalStrike, const FIBS35DamageMessage&, const FHitResult&);

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FIBS35PunchProfile Uppercut;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDrawContacts = true;
    // Disable fallback only after a montage with IBS35Contact notify is actually connected.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseNotifyContact = false;
    UPROPERTY(BlueprintAssignable) FIBS35AttackCue OnAttackCue;
    UPROPERTY(BlueprintAssignable) FIBS35DamageEvent OnHit;
    // Optional composition-root route. Rejection never falls back to a second damage owner.
    FIBS35PunchCanonicalStrike CanonicalStrike;
    UFUNCTION(BlueprintCallable) bool BeginPunch(bool bHeavy);
    UFUNCTION(BlueprintCallable) bool BeginUppercut();
    UFUNCTION(BlueprintCallable) bool BeginChargedPunch(float Charge01);
    UFUNCTION(BlueprintCallable) void CancelAttack();
    UFUNCTION(BlueprintCallable) void NotifyContact();
    UFUNCTION(BlueprintPure) bool IsAttacking() const { return bAttacking; }
    UFUNCTION(BlueprintPure) float GetCooldown() const { return bAttacking ? float(FMath::Max(0.0, Timing.Duration() - Age)) : 0.f; }
    UFUNCTION(BlueprintPure) float GetAttackDuration() const { return float(Timing.Duration()); }
    UFUNCTION(BlueprintPure) int32 GetComboIndex() const { return Combo.Index; }
    UFUNCTION(BlueprintPure) EIBS35PunchStyle GetAttackStyle() const { return Style; }
    virtual void TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* Function) override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void TraceContact(double FromAlpha, double ToAlpha);
    bool BeginAttack(EIBS35PunchStyle InStyle, float Charge01 = -1.f);
    void FinishAttack(bool bCancelled);
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
    bool bHasContactPosition = false;
    EIBS35PunchStyle Style = EIBS35PunchStyle::Quick;
    IBS35::PunchCombo Combo;
    TWeakObjectPtr<class AIBS35Object> AttackTool;
    FVector ToolStart = FVector::ZeroVector;
    FVector AttackOrigin = FVector::ZeroVector;
    FVector AttackDirection = FVector::ForwardVector;
    FVector AttackUp = FVector::UpVector;
    FVector LastContactPosition = FVector::ZeroVector;
};
