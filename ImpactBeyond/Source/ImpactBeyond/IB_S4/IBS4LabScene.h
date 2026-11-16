#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "IBS4ResponseComponent.h"
#include "IBS4LabScene.generated.h"

class AIBS4PreparedTarget;
class AIBS4Threat;
class USkeletalMesh;
class UStaticMesh;
class UAnimInstance;

UCLASS()
class IMPACTBEYOND_API UIBS4LabSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) int32 Version = 1;
    UPROPERTY(SaveGame) TMap<FName, bool> Broken;
    UPROPERTY(SaveGame) FTransform DetachedCannon;
};

/** Reproducible test fixtures. Geometry is explicitly proxy art, never the city. */
UCLASS()
class IMPACTBEYOND_API AIBS4LabScene : public AActor
{
    GENERATED_BODY()
public:
    AIBS4LabScene();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    UPROPERTY(EditAnywhere, Category="S4|Approved assets") TObjectPtr<USkeletalMesh> HumanMesh;
    UPROPERTY(EditAnywhere, Category="S4|Approved assets") TSubclassOf<UAnimInstance> HumanAnim;
    UPROPERTY(EditAnywhere, Category="S4|Approved assets") TObjectPtr<UStaticMesh> TankMesh;
    UPROPERTY(EditAnywhere, Category="S4|Approved assets") FVector TankScale = FVector(1);
    UFUNCTION(BlueprintCallable, Category="S4") bool SaveLab();
    UFUNCTION(BlueprintCallable, Category="S4") bool LoadLab();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") bool bUsingTankProxy = true;
private:
    UPROPERTY() TArray<TObjectPtr<AIBS4PreparedTarget>> Targets;
    UPROPERTY() TArray<TObjectPtr<AIBS4Threat>> Threats;
    UPROPERTY() TObjectPtr<AIBS4PreparedTarget> Robot;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    double RobotNextWarning = 0;
    double RobotFireAt = -1;
    bool bRobotAlert = false;
    UFUNCTION() void Incident(const FIBS4Hit& Hit, const FIBS4Outcome& Outcome);
};

UCLASS()
class IMPACTBEYOND_API AIBS4LabHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class IMPACTBEYOND_API AIBS4LabGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AIBS4LabGameMode();
};
