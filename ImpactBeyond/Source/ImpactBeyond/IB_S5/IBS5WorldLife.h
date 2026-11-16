#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SaveGame.h"
#include "IBS5Policy.h"
#include "IBS5WorldLife.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UTextRenderComponent;
class UAnimSequence;
class UPointLightComponent;
class ADirectionalLight;
class USkyLightComponent;
class UAudioComponent;
class USoundWaveProcedural;
class UStaticMeshComponent;
class USkeletalMesh;

USTRUCT()
struct FIBS5RegionLife
{
    GENERATED_BODY()
    UPROPERTY() double Alert = 0.0;
    UPROPERTY() double Reputation = 0.0;
    UPROPERTY() double Threat = 0.0;
    UPROPERTY() int32 Rescued = 0;
    UPROPERTY() double ReportDelay = 0.0;
    UPROPERTY() double PendingHeat = 0.0;
    UPROPERTY() double PendingReputation = 0.0;
};

UCLASS()
class IMPACTBEYOND_API UIBS5WorldSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 Version = 1;
    UPROPERTY() double SolarPhase = 0.5;
    UPROPERTY() double DayMinutes = 36.0;
    UPROPERTY() bool bClockPaused = false;
    UPROPERTY() bool bConsequencesEnabled = true;
    UPROPERTY() double Reputation = 0.0;
    UPROPERTY() TMap<FName, FIBS5RegionLife> Regions;
    UPROPERTY() FName IncidentRegion;
    UPROPERTY() uint8 IncidentStage = 0;
    UPROPERTY() double IncidentRemaining = 45.0;
    UPROPERTY() int32 IncidentSequence = 0;
    UPROPERTY() bool bIncidentPlayerCaused = false;
    UPROPERTY() bool bIncidentRescue = false;
    UPROPERTY() TArray<FGuid> RecordedCauses;
};

/** A few existing mannequin citizens, moved by swept capsules on real ground. */
UCLASS()
class IMPACTBEYOND_API AIBS5Citizen : public AActor
{
    GENERATED_BODY()
public:
    AIBS5Citizen();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCapsuleComponent> Capsule;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USkeletalMeshComponent> Mesh;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USkeletalMeshComponent> Outfit;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
    void UpdatePresentation(const FString& State, FVector LocalUp, FVector Facing, bool bMoving, bool bFleeing);
    void SetAppearance(USkeletalMesh* Appearance);
private:
    UPROPERTY() TObjectPtr<UAnimSequence> Idle;
    UPROPERTY() TObjectPtr<UAnimSequence> Walk;
    UPROPERTY() TObjectPtr<UAnimSequence> Run;
    UPROPERTY() TObjectPtr<UAnimSequence> Playing;
};

/** One actor per world. The host supplies the collision-ready region context. */
UCLASS()
class IMPACTBEYOND_API AIBS5WorldLife : public AActor
{
    GENERATED_BODY()
public:
    AIBS5WorldLife();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    void ConfigurePlanet(FVector Center, bool bInSpherical);
    void SetRegionContext(FName RegionId, FVector GroundCenter, FVector LocalUp, bool bCollisionReady);
    void SetObserver(APawn* Pawn);
    UFUNCTION(BlueprintCallable) void SetCitizenAppearance(USkeletalMesh* Appearance);
    UFUNCTION(BlueprintCallable) void SetDayLengthMinutes(double Minutes);
    UFUNCTION(BlueprintCallable) void SetSolarHour(double Hour);
    UFUNCTION(BlueprintCallable) void AdvanceSolarHours(float Hours);
    UFUNCTION(BlueprintCallable) void SetClockPaused(bool bPaused);
    UFUNCTION(BlueprintCallable) bool SaveWorld();
    UFUNCTION(BlueprintCallable) bool LoadWorld();
    UFUNCTION(BlueprintCallable) bool TryResolveIncident(APawn* Helper);
    UFUNCTION(BlueprintCallable) bool RequestIncident();
    // CausationId is the original impact ID shared by all of its fragments.
    bool ReportDisturbance(FVector Location, bool bPlayerCaused, float Severity, const FGuid& CausationId, bool bCivilianHarm=true);
    UFUNCTION(BlueprintPure) double GetLocalSolarHour() const;
    UFUNCTION(BlueprintPure) FString GetStatusText() const;
    UFUNCTION(BlueprintPure) int32 GetCitizenCount() const { return Citizens.Num(); }
    UFUNCTION(BlueprintPure) FVector GetIncidentLocation() const;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S5") bool bConsequencesEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S5") bool bEnablePersistence = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S5") FString SaveSlot = TEXT("IB_S5_World_v1");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S5", meta=(ClampMin="3", ClampMax="6")) int32 CitizenBudget = 4;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S5") double Reputation = 0.0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S5") FName ActiveRegion;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S5") bool bRegionReady = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S5") int32 SaveFailures = 0;

private:
    IBS5::Clock Clock;
    IBS5::Incident Incident;
    FName IncidentRegion;
    FVector PlanetCenter = FVector::ZeroVector;
    FVector RegionCenter = FVector::ZeroVector;
    FVector Up = FVector::UpVector;
    FVector East = FVector::ForwardVector;
    FVector North = FVector::RightVector;
    FVector DisturbancePoint = FVector::ZeroVector;
    bool bSpherical = false;
    bool bPersistenceCompatible = true;
    double PanicSeconds = 0.0;
    double LightTimer = 0.0;
    double SaveTimer = 0.0;
    double RoutineTime = 0.0;
    UPROPERTY() TMap<FName, FIBS5RegionLife> Regions;
    UPROPERTY() TArray<FGuid> RecordedCauses;
    TSet<FGuid> SeenCauses;
    UPROPERTY() TArray<TObjectPtr<AIBS5Citizen>> Citizens;
    UPROPERTY() TObjectPtr<USkeletalMesh> CitizenAppearance;
    UPROPERTY() TArray<TObjectPtr<UPointLightComponent>> StreetLights;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> LampPosts;
    UPROPERTY() TObjectPtr<UAudioComponent> Ambience;
    UPROPERTY() TObjectPtr<USoundWaveProcedural> AmbienceWave;
    FRandomStream AmbientRandom = FRandomStream(5105);
    int64 AmbientSample = 0;
    UPROPERTY() TObjectPtr<ADirectionalLight> Sun;
    UPROPERTY() TObjectPtr<USkyLightComponent> SkyLight;
    TWeakObjectPtr<APawn> Observer;
    float OriginalSunIntensity = 8.0f;
    float OriginalSkyIntensity = 1.0f;
    FRotator OriginalSunRotation = FRotator::ZeroRotator;
    void ClearCitizens();
    void SpawnCitizens();
    void UpdateCitizens(float DeltaSeconds);
    void UpdateLighting();
    void UpdateAmbience(float Daylight);
    bool GroundPoint(FVector Candidate, FVector& Result) const;
    FVector LocalUpAt(FVector Position) const;
};
