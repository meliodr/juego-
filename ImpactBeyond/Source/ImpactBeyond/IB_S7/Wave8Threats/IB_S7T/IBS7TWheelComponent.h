#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "../Radial/PowerWheel.h"
#include "IBS7TWheelComponent.generated.h"
class APlayerController;
class UInputComponent;

USTRUCT(BlueprintType)
struct FIBS7TSlot {
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AbilityId=0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Label;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName IconId=TEXT("PLACEHOLDER_CIRCLE");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Cooldown=0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAvailable=true;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIBS7TSelection,int32,AbilityId);
DECLARE_DELEGATE_RetVal_OneParam(bool, FIBS7TSelectionProvider, int32);

/** Explicit opt-in bindings; broadcasts selection only, never activates powers. */
UCLASS(Blueprintable, ClassGroup=(IBS7T), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS7TWheelComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UIBS7TWheelComponent();
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UFUNCTION(BlueprintCallable, Category="S7") bool ConfigureSlots(const TArray<FIBS7TSlot>& Descriptors);
    UFUNCTION(BlueprintCallable, Category="S7") bool UpdateSlots(const TArray<FIBS7TSlot>& Descriptors,int32 EquippedAbility);
    UFUNCTION(BlueprintCallable, Category="S7") bool BindController(APlayerController* Controller, bool bInstallLabBindings = true);
    UFUNCTION(BlueprintCallable, Category="S7") void UnbindController();
    UFUNCTION(BlueprintCallable, Category="S7") void Cancel();
    UFUNCTION(BlueprintCallable, Category="S7") bool SetCooldown(int32 AbilityId,float Seconds);
    UFUNCTION(BlueprintCallable, Category="S7") void OpenWheel(bool bGamepad);
    UFUNCTION(BlueprintCallable, Category="S7") void ConfirmWheel(bool bGamepad);
    UFUNCTION(BlueprintCallable, Category="S7") bool SelectPrevious();
    FIBS7TSelectionProvider SelectionProvider;
    // Provider must revalidate the current hero/context. Default fails closed.
    UFUNCTION(BlueprintNativeEvent, Category="S7") bool CommitSelection(int32 AbilityId);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") FKey MouseOpenKey=EKeys::F9;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") FKey PadOpenKey=EKeys::Gamepad_DPad_Up;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") bool bInputAllowed=false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") bool bOpen=false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") int32 HoveredSlot=-1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") int32 SelectedAbility=0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") TArray<FIBS7TSlot> Slots;
    UPROPERTY(BlueprintAssignable, Category="S7") FIBS7TSelection OnSelectionChanged;
private:
    IBS7T::PowerWheel Wheel;
    UPROPERTY() TObjectPtr<UInputComponent> LocalInput;
    UPROPERTY() TObjectPtr<APlayerController> BoundController;
    bool bPad=false;
    bool bOwnLookLock=false;
    FVector2D MouseOffset=FVector2D::ZeroVector;
    void OpenMouse();
    void OpenPad();
    void Open(bool Gamepad);
    void Confirm();
    void ConfirmMouse();
    void ConfirmPad();
    void Refresh();
    void ReleaseLookLock();
};
