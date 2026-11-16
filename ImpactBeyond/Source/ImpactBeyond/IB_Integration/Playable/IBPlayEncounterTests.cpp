#include "IBPlayEncounter.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "IB_S4/IBS4Threat.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35GripComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBPlayableEncounterTest, "ImpactBeyond.S7.PreparedEncounter",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBPlayableEncounterTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("Game world exists"), World)) return false;
    FActorSpawnParameters Spawn;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Ground(1700000,1700000,1700000);
    auto* Hero = World->SpawnActor<APawn>(APawn::StaticClass(), Ground, FRotator::ZeroRotator, Spawn);
    auto* Encounter = World->SpawnActor<AIBPlayEncounter>(AIBPlayEncounter::StaticClass(), Ground, FRotator::ZeroRotator, Spawn);
    if (!Hero || !Encounter) { if (Hero) Hero->Destroy(); if (Encounter) Encounter->Destroy(); return false; }
    auto* HeroBody=NewObject<UBoxComponent>(Hero); Hero->SetRootComponent(HeroBody); Hero->AddInstanceComponent(HeroBody);
    HeroBody->SetBoxExtent(FVector(20)); HeroBody->SetCollisionProfileName(TEXT("Pawn")); HeroBody->RegisterComponent();
    Hero->SetActorLocation(Ground);
    TestTrue(TEXT("Radial region spawns"), Encounter->SpawnRegion(TEXT("QA_Encounter"), Ground-FVector(10000,0,0), Ground, FVector::ForwardVector, Hero));
    TestFalse(TEXT("Absent presentation assets are reported"), Encounter->HasRequiredVisualAssets());
    TestEqual(TEXT("Six prepared targets"), Encounter->PreparedTargets.Num(), 6);
    TestEqual(TEXT("Five damageable autonomous threats"), Encounter->Threats.Num(), 5);
    TSet<FTickFunction*> Visiting, Complete;
    TFunction<bool(FTickFunction*)> HasAcyclicTicks = [&](FTickFunction* Tick)
    {
        if (!Tick || Complete.Contains(Tick)) return true;
        if (Visiting.Contains(Tick)) return false;
        Visiting.Add(Tick);
        for (FTickPrerequisite& Prerequisite : Tick->GetPrerequisites())
            if (!HasAcyclicTicks(Prerequisite.Get())) return false;
        Visiting.Remove(Tick);
        Complete.Add(Tick);
        return true;
    };
    for (AIBS4Threat* Threat : Encounter->Threats)
    {
        TestTrue(TEXT("Spawned threat tick prerequisites have no cycle"), HasAcyclicTicks(&Threat->PrimaryActorTick));
        TestTrue(TEXT("Spawned movement tick prerequisites have no cycle"), HasAcyclicTicks(&Threat->GetCharacterMovement()->PrimaryComponentTick));
    }
    AIBS4PreparedTarget* Crater = nullptr;
    AIBS4PreparedTarget* Wall = nullptr;
    for (AIBS4PreparedTarget* Target : Encounter->PreparedTargets)
    {
        if (Target->Kind == EIBS4TargetKind::Crater) Crater = Target;
        if (Target->ObjectId.ToString().EndsWith(TEXT(".Wall"))) Wall = Target;
    }
    if (Crater && Wall)
    {
        const FTransform Frame = Encounter->GetActorTransform();
        FHitResult Before, After;
        World->LineTraceSingleByChannel(Before, Frame.TransformPosition(FVector(650,0,100)),
            Frame.TransformPosition(FVector(650,0,-120)), ECC_Visibility);
        TestTrue(TEXT("Unbroken crater has continuous floor"), Before.GetActor() == Crater);
        TestTrue(TEXT("Crater breaks"), Crater->SetBrokenState(true));
        World->LineTraceSingleByChannel(After, Frame.TransformPosition(FVector(650,0,100)),
            Frame.TransformPosition(FVector(650,0,-120)), ECC_Visibility);
        TestTrue(TEXT("Broken crater remains walkable"), After.GetActor() == Crater);
        TestTrue(TEXT("Crater is a real 60 cm depression"), FMath::IsNearlyEqual(FVector::Dist(Before.ImpactPoint,After.ImpactPoint),60.0,0.2));
        const auto TraceWall = [&]()
        {
            FHitResult Hit;
            World->LineTraceSingleByChannel(Hit, Frame.TransformPosition(FVector(1300,0,120)),
                Frame.TransformPosition(FVector(1500,0,120)), ECC_Visibility);
            return Hit.GetActor() == Wall;
        };
        TestTrue(TEXT("Wall initially blocks passage"), TraceWall());
        Wall->SetBrokenState(true);
        TestFalse(TEXT("Wall opens physical passage"), TraceWall());
        TestTrue(TEXT("Unoccupied wall can restore"), Wall->ResetTarget());
        FHitResult Contact(Wall, nullptr, Wall->GetActorLocation(), FVector::ForwardVector);
        TestTrue(TEXT("Partial structural hit"), Wall->Response->Receive(UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Punch,20,Contact)));
        FIBS4PreparedSnapshot Saved;
        TestTrue(TEXT("Partial damage captures"), Wall->CaptureSnapshot(Saved));
        TestEqual(TEXT("Partial resistance persists"), Saved.StructuralDamage, 20.f);
        Wall->ResetTarget();
        TestTrue(TEXT("Partial damage can preflight"), Wall->CanRestoreSnapshot(Saved));
        TestEqual(TEXT("Preflight never writes health"), Wall->Response->Health,100.f);
        TestTrue(TEXT("Partial damage restores"), Wall->RestoreSnapshot(Saved));
        TestEqual(TEXT("One health owner restores"), Wall->Response->Health,80.f);
        auto Invalid = Saved; Invalid.Health = -1;
        TestFalse(TEXT("Corrupt health rejected"), Wall->RestoreSnapshot(Invalid));
        TestEqual(TEXT("Rejected hydration leaves HP unchanged"), Wall->Response->Health,80.f);
        TArray<FIBS4PreparedSnapshot> Snapshots;
        TestTrue(TEXT("Whole region captures"), Encounter->CapturePreparedState(Snapshots));
        TestTrue(TEXT("Prepared broken states restore"), Encounter->RestorePreparedState(Snapshots));
        TestTrue(TEXT("Crater stays damaged after hydration"), Crater->bBroken);
    }
    else AddError(TEXT("Missing expected prepared targets"));
    for(AIBS4PreparedTarget* Target:Encounter->PreparedTargets) if(Target->Kind==EIBS4TargetKind::Robot)
    {
        TestTrue(TEXT("Cannon breaks into an available part"),Target->SetBrokenState(true));
        AIBS35Object* Part=Target->GetSharedLoosePart();
        if(TestNotNull(TEXT("Cannon joins the shared grip system"),Part))
        {
            TestTrue(TEXT("Conversion keeps exactly one loose cannon"),Target->GetSharedLoosePart()==Part);
            TestFalse(TEXT("Former cannon collider is disabled"),Target->GetPart()->IsCollisionEnabled());
            const FTransform RadialFrame=Encounter->GetActorTransform();
            Part->SetActorLocation(RadialFrame.TransformPosition(FVector(0,0,700)),false,nullptr,ETeleportType::TeleportPhysics);
            Hero->SetActorLocationAndRotation(Part->GetActorLocation()+RadialFrame.GetUnitAxis(EAxis::X)*200,RadialFrame.GetRotation());
            auto* Grip=NewObject<UIBS35GripComponent>(Hero); Hero->AddInstanceComponent(Grip); Grip->RegisterComponent();
            TestTrue(TEXT("Shared grip can lift the detached cannon"),Grip->TryGrab(Part));
            FIBS4PreparedSnapshot State;
            TestFalse(TEXT("Held cannon rejects save"),Target->CaptureSnapshot(State));
            TestFalse(TEXT("Held cannon rejects reset"),Target->ResetTarget());
            Grip->Drop();
            TestTrue(TEXT("Released cannon captures"),Target->CaptureSnapshot(State));
            TestTrue(TEXT("Save captures the shared cannon position"),State.PartTransform.GetLocation().Equals(Part->GetActorLocation(),.1));
        }
    }
    Encounter->Destroy(); Hero->Destroy();
    return true;
}
#endif
