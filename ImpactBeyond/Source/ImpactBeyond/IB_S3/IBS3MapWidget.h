#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IBS3MapWidget.generated.h"
class AIBS3Character;
class APawn;
class AIBS3PlanetWorld;

// Native UMG widget; optional designer child is authored exclusively under /Game/IB_S3.
UCLASS()
class IMPACTBEYOND_API UIBS3MapWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly) TObjectPtr<AIBS3Character> Pilot;
    /** Production HUD owns health/abilities; this widget supplies only navigation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCompactHud = false;
    UFUNCTION(BlueprintCallable) void SetMapContext(APawn* Pawn, AIBS3PlanetWorld* World);
    UFUNCTION(BlueprintCallable) void SetGlobalMapVisible(bool bVisible) { bGlobalMapVisible = bVisible; }
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& Geometry,float DeltaSeconds) override;
    virtual int32 NativePaint(const FPaintArgs& Args,const FGeometry& Geometry,const FSlateRect& Culling,
        FSlateWindowElementList& Elements,int32 Layer,const FWidgetStyle& Style,bool ParentEnabled) const override;
private:
    struct Icon { FVector2D Global,Local,East,North; FString Label; double Radius=0; bool Discovered=false,Destination=false,Ambiguous=false; };
    TArray<Icon> Icons;
    FVector2D PlayerGlobal;
    FVector2D NorthDirection;
    FString Telemetry,DestinationText,RegionText;
    float Refresh=0;
    UPROPERTY() TObjectPtr<APawn> Observer;
    UPROPERTY() TObjectPtr<AIBS3PlanetWorld> Planet;
    bool bProduction = false;
    bool bGlobalMapVisible = false;
    double LastLongitude = 0.0;
    double MapRangeCm = 200000.0;
};
