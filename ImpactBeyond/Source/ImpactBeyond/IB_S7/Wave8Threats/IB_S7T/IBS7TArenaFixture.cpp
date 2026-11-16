#include "IBS7TArenaFixture.h"
#include "IBS7TThreatActor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#if __has_include("IB_S4/IBS4PreparedTarget.h")
#include "IB_S4/IBS4PreparedTarget.h"
#define IBS7T_FIXTURE_S4 1
#else
#define IBS7T_FIXTURE_S4 0
#endif
AIBS7TArenaFixture::AIBS7TArenaFixture()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("FixtureRoot")));
}
void AIBS7TArenaFixture::BeginPlay()
{
    Super::BeginPlay();
    if(!bEnableFixture) return;
#if IBS7T_FIXTURE_S4
    FixtureStatus=TEXT("PARTIAL - native fixture, region gate remains CLOSED");
    for(int32 I=0; I<2; ++I) {
        const FVector Lane=GetActorLocation()+FVector(0,I*2000,0);
        // Wall in front; crater below. Aiming/orientation must be inspected by integrator.
        const FVector TargetPosition=Lane+(I?FVector(550,0,-100):FVector(350,0,0));
        auto* Patch=GetWorld()->SpawnActorDeferred<AIBS4PreparedTarget>(AIBS4PreparedTarget::StaticClass(),
            FTransform(FRotator::ZeroRotator,TargetPosition),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if(!Patch) continue;
        Patch->Kind=I?EIBS4TargetKind::Crater:EIBS4TargetKind::Wall;
        Patch->ObjectId=I?TEXT("S7FixtureCrater"):TEXT("S7FixtureWall");
        Patch->FinishSpawning(Patch->GetActorTransform()); OwnedActors.Add(Patch);
        auto* Threat=GetWorld()->SpawnActorDeferred<AIBS7TThreatActor>(AIBS7TThreatActor::StaticClass(),
            FTransform(FRotator::ZeroRotator,Lane),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if(!Threat) continue;
        Threat->bKaiju=I==1; Threat->Target=Patch; Threat->bRegionReady=false;
        Threat->FinishSpawning(Threat->GetActorTransform()); OwnedActors.Add(Threat);
    }
    if(OwnedActors.Num()==4) FixtureStatus=TEXT("READY FOR INSPECTION ONLY - set region gates after verifying placement");
#else
    FixtureStatus=TEXT("BLOCKED - S4 PreparedTarget not present at compile time; spawned nothing");
#endif
}
void AIBS7TArenaFixture::EndPlay(const EEndPlayReason::Type Reason)
{
    for(AActor* Actor:OwnedActors) if(IsValid(Actor)) Actor->Destroy();
    OwnedActors.Reset(); Super::EndPlay(Reason);
}
