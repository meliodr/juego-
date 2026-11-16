#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IBS7TArenaFixture.generated.h"
/** Opt-in fixture: exactly two stationary threats and two S4 prepared targets. No new art. */
UCLASS()
class IMPACTBEYOND_API AIBS7TArenaFixture : public AActor {
    GENERATED_BODY()
public:
    AIBS7TArenaFixture();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S7|Fixture") bool bEnableFixture=false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7|Fixture") FString FixtureStatus=TEXT("Disabled");
private:
    UPROPERTY() TArray<TObjectPtr<AActor>> OwnedActors;
};
