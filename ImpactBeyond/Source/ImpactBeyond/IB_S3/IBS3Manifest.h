#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/SaveGame.h"
#include "IBS3Manifest.generated.h"

USTRUCT(BlueprintType)
struct FIBS3City
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName CityId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName RegionId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double LatitudeDegrees = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double LongitudeDegrees = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double SurfaceOffsetCm = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double FootprintRadiusCm = 45000;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Seed = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double BuildingHeightScale = 1;
};

UCLASS(BlueprintType)
class IMPACTBEYOND_API UIBS3WorldManifest : public UDataAsset
{
    GENERATED_BODY()
public:
    UIBS3WorldManifest();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName BodyId = TEXT("BODY_EARTH_LAB");
    // Revision 2 fixes longitude handedness for UE forward/right/up; do not reuse revision-1 coordinates.
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Revision = 2;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="100000")) double RadiusCm = 2500000;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) double GravityCmS2 = 980;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) double TraversalSpeedCmS = 27000;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) double BrakeCmS2 = 8000;
    // Conservative starting estimate, not a measured load latency.
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double InitialLoadP95Seconds = 1.5;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double PrefetchMarginCm = 30000;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double ResidencyHysteresisCm = 40000;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) double PhysicsBubbleCm = 120000;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FIBS3City> Cities;
    bool Validate(FString& Error) const;
};

USTRUCT(BlueprintType)
struct FIBS3ObjectDelta
{
    GENERATED_BODY()
    // Key is BodyId/OriginRegionId/ObjectId. CurrentRegionId may change on transport.
    UPROPERTY(SaveGame) FName Key;
    UPROPERTY(SaveGame) FName OriginRegionId;
    UPROPERTY(SaveGame) FName CurrentRegionId;
    UPROPERTY(SaveGame) FName Kind;
    UPROPERTY(SaveGame) FTransform GlobalTransform;
    UPROPERTY(SaveGame) FVector LinearVelocity = FVector::ZeroVector;
    UPROPERTY(SaveGame) FVector AngularVelocity = FVector::ZeroVector;
    UPROPERTY(SaveGame) bool bBroken = false;
};

UCLASS()
class IMPACTBEYOND_API UIBS3SaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) int32 SchemaVersion = 1;
    UPROPERTY(SaveGame) FName BodyId;
    UPROPERTY(SaveGame) int32 ManifestRevision = 1;
    UPROPERTY(SaveGame) double RadiusCm = 0;
    UPROPERTY(SaveGame) FVector GlobalBodyCenter = FVector::ZeroVector;
    UPROPERTY(SaveGame) TArray<FName> DiscoveredCities;
    UPROPERTY(SaveGame) FName DestinationCity;
    UPROPERTY(SaveGame) TArray<FIBS3ObjectDelta> Deltas;
};
