#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IBPlayReviewMode.generated.h"
/** QA-only view of licensed CC0 scenery and Epic's template mannequin. */
UCLASS()
class IMPACTBEYOND_API AIBPlayReviewGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AIBPlayReviewGameMode();
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    bool ValidateScene(FString& Reason) const;
    bool bReviewReady = false;
};
