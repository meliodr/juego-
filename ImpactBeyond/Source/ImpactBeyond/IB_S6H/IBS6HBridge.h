#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IB_S35/IBS35Types.h"
#include "IBS6HBridge.generated.h"

class UInputComponent;
class UStaticMesh;
class UMaterialInterface;
class USoundBase;
class UIBS6HSelectorWidget;
struct FIBS6HRuntime;

USTRUCT(BlueprintType)
struct FIBS6HAbility
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 Id = 0;
    UPROPERTY(BlueprintReadOnly) FText Label;
    UPROPERTY(BlueprintReadOnly) float Cooldown = 0;
    UPROPERTY(BlueprintReadOnly) float EnergyCost = 0;
    UPROPERTY(BlueprintReadOnly) bool bAvailable = false;
};

USTRUCT(BlueprintType)
struct FIBS6HHeroDescriptor
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 Id = 0;
    UPROPERTY(BlueprintReadOnly) FText Label;
    UPROPERTY(BlueprintReadOnly) FText Description;
    UPROPERTY(BlueprintReadOnly) FLinearColor Color = FLinearColor::White;
};

// The integration owner resolves the target life and routes through ImpactContract.
// False means no target mutation. Presentation and legacy receivers never apply damage.
DECLARE_DELEGATE_RetVal_TwoParams(bool, FIBS6HCanonicalStrike, const FIBS35DamageMessage&, const FHitResult&);
DECLARE_DELEGATE_RetVal_OneParam(bool, FIBS6HHostAbility, int32);
DECLARE_DELEGATE_OneParam(FIBS6HHostReleaseAbility, int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIBS6HHeroChanged, int32, HeroId);

/** Opt-in bridge on an existing ACharacter. No pawn replacement or global mappings. */
UCLASS(ClassGroup=(IBS6H), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS6HBridge : public UActorComponent
{
    GENERATED_BODY()
public:
    UIBS6HBridge();
    UIBS6HBridge(FVTableHelper& Helper);
    virtual ~UIBS6HBridge() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn) override;
    // Default denies blink without an operational S3 world. Only enable explicitly in a nonstreamed S2 lab.
    UPROPERTY(EditAnywhere, Category="S6|Safety") bool bAllowNonPlanetLab = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S6") FString LastResult;
    UPROPERTY(EditAnywhere, Category="S6|Presentation") TObjectPtr<UStaticMesh> EffectMesh;
    UPROPERTY(EditAnywhere, Category="S6|Presentation") TObjectPtr<UMaterialInterface> EnergyMaterial;
    UPROPERTY(EditAnywhere, Category="S6|Presentation") TObjectPtr<USoundBase> ActivationSound;
    UPROPERTY(BlueprintAssignable, Category="S6") FIBS6HHeroChanged OnHeroChanged;
    FIBS6HCanonicalStrike CanonicalStrike;
    FIBS6HHostAbility HostAbility;
    FIBS6HHostReleaseAbility HostReleaseAbility;
    UFUNCTION(BlueprintPure, Category="S6") TArray<FIBS6HHeroDescriptor> GetHeroDescriptors() const;
    UFUNCTION(BlueprintPure, Category="S6") TArray<FIBS6HAbility> GetAbilities() const;
    UFUNCTION(BlueprintPure, Category="S6") float Energy() const;
    UFUNCTION(BlueprintPure, Category="S6") int32 EquippedAbility() const { return Equipped; }
    UFUNCTION(BlueprintCallable, Category="S6") bool EquipAbility(int32 AbilityId);
    UFUNCTION(BlueprintCallable, Category="S6") bool ActivateEquipped();
    UFUNCTION(BlueprintCallable, Category="S6") void ReleaseEquipped();
    UFUNCTION(BlueprintCallable, Category="S6") void NotifyIncomingHit(FVector Source);
    UFUNCTION(BlueprintCallable, Category="S6") void ResetAbilities();
    UFUNCTION(BlueprintPure, Category="S6") bool IsSelectorOpen() const;
    UFUNCTION(BlueprintPure, Category="S6") int32 SelectorFocus() const;
    UFUNCTION(BlueprintCallable, Category="S6") void CloseSelector();
    UFUNCTION(BlueprintCallable, Category="S6") bool SelectHero(int32 Index);
    UFUNCTION(BlueprintPure, Category="S6") int32 ActiveHero() const;
    UFUNCTION(BlueprintCallable, Category="S6") void Shield();
    UFUNCTION(BlueprintCallable, Category="S6") void Construct();
    UFUNCTION(BlueprintCallable, Category="S6") void Blink();
    UFUNCTION(BlueprintCallable, Category="S6") bool Grab(AActor* Target);
    UFUNCTION(BlueprintCallable, Category="S6") bool Strike(AActor* Target);
    // Receiver calls once per canonical deduplicated hit; return is absorption, not remaining damage.
    UFUNCTION(BlueprintCallable, Category="S6") float Absorb(const FGuid& Attack,float Damage);
    UFUNCTION(BlueprintCallable, Category="S6") void OpenSelector();
    // Control: 0 previous,1 next,2 confirm,3 back,4/5/6 H01/H02/H03 card. Feed release too.
    UFUNCTION(BlueprintCallable, Category="S6") void SelectorInput(int32 Control,bool Down);
    // Explicit opt-in F6 selector, F7 shield, F8 construct, F9 blink. No gamepad global override.
    void BindLabInput(UInputComponent* Input);
private:
    TUniquePtr<FIBS6HRuntime> Runtime;
    TWeakObjectPtr<UInputComponent> BoundInput;
    UPROPERTY(Transient) TObjectPtr<UIBS6HSelectorWidget> SelectorWidget;
    UPROPERTY(Transient) TObjectPtr<USoundBase> GeneratedSound;
    int32 Equipped = 101;
    float StrikeCooldown = 0;
    float ShieldCooldown = 0;
    float ConstructCooldown = 0;
    FVector LastAttackSource = FVector::ZeroVector;
    double LastIncomingAt = -100;
    void RefreshSelector();
    void PresentAbility(bool bBlink);
    void PublishHeroChange(int32 PreviousHero);
    void UnbindLabInput();
    void Previous();
    void Next();
    void Confirm();
    void Back();
};
