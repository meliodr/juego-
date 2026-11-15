#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IBS3Manifest.h"
#include "IBS3World.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class USkyAtmosphereComponent;

UCLASS()
class IMPACTBEYOND_API AIBS3Prop : public AActor
{
    GENERATED_BODY()
public:
    AIBS3Prop();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Mesh;
    FIBS3ObjectDelta State;
    void Restore(const FIBS3ObjectDelta& Delta, const FVector& WorldOrigin);
    FIBS3ObjectDelta Capture(const FVector& WorldOrigin) const;
    void Interact(FVector Direction);
};

UCLASS()
class IMPACTBEYOND_API AIBS3Hub : public AActor
{
    GENERATED_BODY()
public:
    AIBS3Hub();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Pavement;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Buildings;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> LandingPad;
    bool Build(const FIBS3City& City, const FVector& Center, double Radius);
    bool bCollisionReady = false;
};

UCLASS()
class IMPACTBEYOND_API AIBS3PlanetWorld : public AActor
{
    GENERATED_BODY()
public:
    AIBS3PlanetWorld();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TObjectPtr<UIBS3WorldManifest> Manifest;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> GroundCollision;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> PlanetProxy;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> NearSurface;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USkyAtmosphereComponent> Atmosphere;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName ActiveCity;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName DestinationCity;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TSet<FName> Discovered;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Status;
    UPROPERTY(EditAnywhere) FString SaveSlot = TEXT("IB_S3_PlanetLab_v1");
    UFUNCTION(BlueprintCallable) bool SaveState();
    UFUNCTION(BlueprintCallable) void CycleDestination();
    FVector CityPosition(const FIBS3City& City) const;
    FVector OriginOffset() const;
    FName RegionAt(FVector Position) const;
    double P95Seconds() const;
    bool IsReady(FName City) const;
    double AllowedSpeed(FVector Position, double RequestedSpeed) const;
    void UpdateResidency(FVector Position, double Speed);
private:
    UPROPERTY() TMap<FName,TObjectPtr<AIBS3Hub>> Hubs;
    UPROPERTY() TMap<FName,TObjectPtr<AIBS3Prop>> Props;
    UPROPERTY() TMap<FName,FIBS3ObjectDelta> Deltas;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> CityProxies;
    TArray<double> LoadSamples;
    TMap<FName,double> LoadStartedAt;
    bool bValid = false;
    bool bSaveCompatible = true;
    double NextResidencyUpdate = 0;
    void LoadState();
    void SeedObjects();
    void SnapshotProps();
};
