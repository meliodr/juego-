#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IBPlayArt.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UMaterialInterface;
class UAnimSequence;

/** Cooked references to the small, licensed production selection. Source packs stay local. */
UCLASS()
class IMPACTBEYOND_API UIBPlayArt : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<USkeletalMesh>> Heroes;
    UPROPERTY(EditAnywhere) TObjectPtr<USkeletalMesh> CitizenMesh;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<UMaterialInterface>> HeroAccents;
    UPROPERTY(EditAnywhere) TObjectPtr<UMaterialInterface> EnergyMaterial;
    UPROPERTY(EditAnywhere) TObjectPtr<USkeletalMesh> Robot;
    UPROPERTY(EditAnywhere) TObjectPtr<USkeletalMesh> Kaiju;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<UAnimSequence>> RobotAnimations;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<UAnimSequence>> KaijuAnimations;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<UStaticMesh>> RobotParts;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<UStaticMesh>> KaijuParts;
    UPROPERTY(EditAnywhere) TArray<FName> RobotPartBones;
    UPROPERTY(EditAnywhere) TArray<FName> KaijuPartBones;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<UStaticMesh>> Props;
    UPROPERTY(EditAnywhere) TObjectPtr<UStaticMesh> Tank;
    UPROPERTY(EditAnywhere) TObjectPtr<UStaticMesh> Drone;
    UPROPERTY(EditAnywhere) TObjectPtr<UStaticMesh> SmallRobot;
    UPROPERTY(EditAnywhere) TObjectPtr<UStaticMesh> Cannon;
    UPROPERTY(EditAnywhere) TObjectPtr<UMaterialInterface> Concrete;
    UPROPERTY(EditAnywhere) TObjectPtr<UMaterialInterface> Ground;
    UPROPERTY(EditAnywhere) TObjectPtr<UMaterialInterface> Glass;
    UPROPERTY(EditAnywhere) TObjectPtr<UMaterialInterface> Metal;
};
