#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SaveGame.h"
#include "IB_S35/IBS35Types.h"
#include "IB_S4/IBS4ResponseComponent.h"
#include "IB_S4/IBS4PreparedTarget.h"
#include "IB_S7/Wave8Threats/IB_S7T/IBS7TThreatActor.h"
#include "IBPlaySession.generated.h"

class AIBS3PlanetWorld;
class AIBS5WorldLife;
class AIBPlayCharacter;
class UIBPlayArt;
class AIBS35Object;
class AIBPlayEncounter;
class UIBPlayCombat;

USTRUCT()
struct FIBPlayObjectState
{
    GENERATED_BODY()
    UPROPERTY() FName Id;
    UPROPERTY() int32 MeshIndex=0;
    UPROPERTY() FTransform Transform;
    UPROPERTY() FVector Velocity=FVector::ZeroVector;
    UPROPERTY() FVector AngularVelocity=FVector::ZeroVector;
    UPROPERTY() float Health=100;
};
UCLASS()
class IMPACTBEYOND_API UIBPlaySave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 Version=1;
    UPROPERTY() int32 Hero=0;
    UPROPERTY() FTransform HeroTransform;
    UPROPERTY() TArray<FIBPlayObjectState> Objects;
    UPROPERTY() TArray<FIBS7TThreatSnapshot> Threats;
    UPROPERTY() TArray<FIBS4PreparedSnapshot> Prepared;
};

/** Small composition root. Owns regional encounters and only serializable world deltas. */
UCLASS()
class IMPACTBEYOND_API AIBPlaySession : public AActor
{
    GENERATED_BODY()
public:
    AIBPlaySession();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    UPROPERTY() TObjectPtr<AIBS3PlanetWorld> Planet;
    UPROPERTY() TObjectPtr<AIBS5WorldLife> Life;
    UPROPERTY() TObjectPtr<UIBPlayArt> Art;
    UPROPERTY() TObjectPtr<AIBPlayCharacter> Hero;
    UPROPERTY(EditAnywhere) bool bPersistence=true;
    UPROPERTY(EditAnywhere) FString SaveSlot=TEXT("IB_Playable_S7_v1");
    UFUNCTION(BlueprintCallable) bool SaveSession();
    bool CaptureSnapshot(UIBPlaySave*& Out);
    bool RestoreSnapshot(const UIBPlaySave* Snapshot);
    void CycleDayLength();
    bool DispatchS35(const FIBS35DamageMessage& Input,const FHitResult& Hit);
    bool DispatchS4(const FIBS4Hit& Input,const FHitResult& Hit);
    void RegisterTarget(AActor* Actor);
private:
    FGuid SessionId;
    TSet<FName> SpawnedRegions;
    TSet<TWeakObjectPtr<UIBPlayCombat>> Reporters;
    UPROPERTY() TMap<FName,TObjectPtr<AIBS35Object>> Objects;
    UPROPERTY() TArray<TObjectPtr<AIBS7TThreatActor>> Threats;
    UPROPERTY() TArray<TObjectPtr<AIBPlayEncounter>> Encounters;
    UPROPERTY() TObjectPtr<UIBPlaySave> Loaded;
    double NextUpdate=0;
    int32 DayLengthIndex=1;
    bool bRestoredHero=false;
    void SpawnRegion(FName Region,FVector Center,FVector Up);
    void SpawnThreat(FName Identity,bool bKaiju,FVector Location,FVector Up);
    void SpawnObject(FName Id,int32 MeshIndex,const FTransform& Transform,const FIBPlayObjectState* Previous);
    void RestoreNearbyObjects();
};
