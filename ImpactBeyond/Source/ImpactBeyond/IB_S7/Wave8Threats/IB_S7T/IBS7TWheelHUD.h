#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "IBS7TWheelHUD.generated.h"
/** Optional isolated-map HUD. Does not replace the production HUD automatically. */
UCLASS()
class IMPACTBEYOND_API AIBS7TWheelHUD : public AHUD {
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
