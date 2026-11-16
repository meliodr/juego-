#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IBS6HSelectorWidget.generated.h"

class UIBS6HBridge;
class UButton;
class UTextBlock;

/** Small native UMG selector, usable with pointer or the host's virtual controls. */
UCLASS()
class IMPACTBEYOND_API UIBS6HSelectorWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetBridge(UIBS6HBridge* InBridge);
    void Refresh();
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
private:
    UPROPERTY() TObjectPtr<UIBS6HBridge> Bridge;
    UPROPERTY() TArray<TObjectPtr<UButton>> Cards;
    UPROPERTY() TObjectPtr<UTextBlock> Status;
    UFUNCTION() void First();
    UFUNCTION() void Second();
    UFUNCTION() void Third();
    UFUNCTION() void Confirm();
    UFUNCTION() void Back();
    void Choose(int32 Index);
};
