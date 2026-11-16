#if WITH_DEV_AUTOMATION_TESTS
#include "ImpactLandingComponent.h"
#include "IB_S35/IBS35LabTarget.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Math/RotationMatrix.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactRadialLandingTest, "ImpactBeyond.Landing.RadialCanonical",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactRadialLandingTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("Game world"), World)) return false;
    FActorSpawnParameters Spawn;
    Spawn.ObjectFlags |= RF_Transient;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Center(40000000,40000000,40000000);
    float ReferenceDamage = 0;
    for (const FVector Up : {FVector::UpVector, FVector::ForwardVector, -FVector::ForwardVector})
    for (const float Step : {.01f,.05f,.2f})
    {
        struct FCleanup
        {
            TArray<TWeakObjectPtr<AActor>> Actors;
            ~FCleanup() { for (const auto& Actor : Actors) if (Actor.IsValid()) Actor->Destroy(); }
        } Cleanup;
        const FVector Forward = FVector::VectorPlaneProject(FVector(1,1,0), Up).GetSafeNormal();
        const FVector Right = FVector::CrossProduct(Up, Forward);
        const FQuat Rotation = FRotationMatrix::MakeFromXZ(Forward,Up).ToQuat();
        auto MakeBox = [&](FVector Position, FVector Extent)
        {
            AActor* Actor = World->SpawnActor<AActor>(Spawn); Cleanup.Actors.Add(Actor);
            UBoxComponent* Box = NewObject<UBoxComponent>(Actor); Actor->SetRootComponent(Box); Actor->AddInstanceComponent(Box);
            Box->InitBoxExtent(Extent); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
            Actor->SetActorLocationAndRotation(Position,Rotation,false,nullptr,ETeleportType::TeleportPhysics);
            return Box;
        };
        UBoxComponent* Floor = MakeBox(Center-Up*20,FVector(2000,2000,20));
        MakeBox(Center-Forward*150+Up*90,FVector(25,180,90));
        ACharacter* Hero = World->SpawnActor<ACharacter>(Center+Up*650,Rotation.Rotator(),Spawn); Cleanup.Actors.Add(Hero);
        APlayerController* Controller = World->SpawnActor<APlayerController>(Spawn); Cleanup.Actors.Add(Controller);
        if (!Hero || !Controller) return false;
        Controller->Possess(Hero);
        Hero->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        auto* Landing = NewObject<UImpactLandingComponent>(Hero);
        Hero->AddInstanceComponent(Landing); Landing->RegisterComponent(); Landing->bUseLocalGravity = true;
        auto MakeTarget = [&](FVector Position)
        {
            auto* Target = World->SpawnActor<AIBS35LabTarget>(Position,Rotation.Rotator(),Spawn); Cleanup.Actors.Add(Target);
            Target->Body->SetBoxExtent(FVector(25,35,35)); return Target;
        };
        AIBS35LabTarget* Near = MakeTarget(Center+Forward*250+Up*60);
        AIBS35LabTarget* Blocked = MakeTarget(Center-Forward*350+Up*60);
        AIBS35LabTarget* Far = MakeTarget(Center+Right*1100+Up*60);
        int Calls = 0;
        FIBS35DamageMessage Captured;
        Landing->CanonicalStrike.BindLambda([&](const FIBS35DamageMessage& Message,const FHitResult& Hit)
        {
            ++Calls; Captured = Message;
            TestTrue(TEXT("LOS excludes shielded actor"),Hit.GetActor()!=Blocked);
            TestTrue(TEXT("Radius excludes distant actor"),Hit.GetActor()!=Far);
            return true; // Composition owns damage; this fixture deliberately applies none.
        });
        TestTrue(TEXT("Oriented capsule finds radial floor"),Landing->StartImpactDive());
        TestTrue(TEXT("Dive velocity points down local gravity"),FVector::DotProduct(Hero->GetVelocity(),-Up)>2000);
        TestTrue(TEXT("Dive active"),Landing->IsDiving());
        TestEqual(TEXT("Dive start applies no damage"),Calls,0);
        const int Count = FMath::RoundToInt(.2f/Step);
        for (int Index=0;Index<Count;++Index)
            static_cast<UActorComponent*>(Landing)->TickComponent(Step,LEVELTICK_All,nullptr);
        Hero->GetCharacterMovement()->Velocity = FVector::ZeroVector; // Landed may arrive after velocity was cleared.
        FHitResult Landed(Floor->GetOwner(),Floor,Center,Up); Landed.bBlockingHit=true;
        Hero->LandedDelegate.Broadcast(Landed);
        TestEqual(TEXT("One visible receiver reacts"),Calls,1);
        TestTrue(TEXT("Shockwave snapshots contacted life"),Captured.TargetId==Near->Receiver->TargetId);
        TestTrue(TEXT("Shockwave identity is valid"),Captured.AttackId.IsValid());
        TestTrue(TEXT("Impulse lifts away from local surface"),FVector::DotProduct(Captured.Impulse,Up)>0);
        if (ReferenceDamage==0) ReferenceDamage=Captured.HealthDelta;
        TestTrue(TEXT("Damage independent of tick subdivision and planet orientation"),FMath::IsNearlyEqual(Captured.HealthDelta,ReferenceDamage,.1f));
        Hero->LandedDelegate.Broadcast(Landed);
        TestEqual(TEXT("Repeated landed callback does not repeat wave"),Calls,1);
        TestFalse(TEXT("Recovering prevents immediate repeat"),Landing->StartImpactDive());
        TestEqual(TEXT("Canonical ownership avoids legacy damage"),Near->Receiver->AcceptedHits,0);
    }
    return true;
}
#endif
