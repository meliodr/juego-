#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IB_S4/IBS4PreparedTarget.h"
#include "IBPlayEncounter.generated.h"

class AIBS4Threat;
class APawn;
class UStaticMesh;
class USkeletalMesh;
class UAnimSequence;
class UMaterialInterface;

/** Reuses S4 targets and threat policies in a local tangent frame on the planet. */
UCLASS()
class IMPACTBEYOND_API AIBPlayEncounter : public AActor
{
    GENERATED_BODY()
public:
    AIBPlayEncounter();
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    // Supply licensed presentation assets before SpawnRegion. Collision and HP remain S4-owned.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<USkeletalMesh> OfficerMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<USkeletalMesh> RivalMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UAnimSequence> OfficerIdle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UAnimSequence> OfficerWalk;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UAnimSequence> OfficerAttack;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UAnimSequence> RivalIdle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UStaticMesh> TankMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UStaticMesh> DroneMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UStaticMesh> RobotBodyMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UStaticMesh> RobotCannonMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UMaterialInterface> ConcreteMaterial;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UMaterialInterface> GroundMaterial;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UMaterialInterface> GlassMaterial;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Encounter|Art") TObjectPtr<UMaterialInterface> MetalMaterial;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Encounter") TArray<TObjectPtr<AIBS4PreparedTarget>> PreparedTargets;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Encounter") TArray<TObjectPtr<AIBS4Threat>> Threats;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Encounter") FName RegionId;
    UFUNCTION(BlueprintCallable, Category="Encounter") bool SpawnRegion(FName Id, FVector PlanetCenter, FVector GroundCenter, FVector LocalUp, APawn* Hero);
    UFUNCTION(BlueprintCallable, Category="Encounter") void SetRegionReady(bool bReady);
    UFUNCTION(BlueprintCallable, Category="Encounter") void Retarget(APawn* Hero);
    UFUNCTION(BlueprintCallable, Category="Encounter") bool ResetEncounter();
    UFUNCTION(BlueprintPure, Category="Encounter") bool HasRequiredVisualAssets() const;
    UFUNCTION(BlueprintCallable, Category="Encounter") bool CapturePreparedState(TArray<FIBS4PreparedSnapshot>& Snapshots) const;
    // Returns false on an occupied/held restoration; successful independent targets stay restored.
    UFUNCTION(BlueprintCallable, Category="Encounter") bool RestorePreparedState(const TArray<FIBS4PreparedSnapshot>& Snapshots);
private:
    UPROPERTY() TArray<TObjectPtr<AActor>> OwnedActors;
    UPROPERTY() TObjectPtr<AIBS4Threat> RobotController;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    void ClearEncounter();
};
