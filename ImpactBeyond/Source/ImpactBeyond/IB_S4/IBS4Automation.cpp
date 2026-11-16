#if WITH_DEV_AUTOMATION_TESTS
#include "IBS4LabPawn.h"
#include "IBS4PreparedTarget.h"
#include "IBS4ResponseComponent.h"
#include "IBS4Threat.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

namespace
{
UWorld* LabWorld()
{
    for (const auto& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            if (Cast<AIBS4LabPawn>(UGameplayStatics::GetPlayerPawn(Context.World(), 0))) return Context.World();
    return nullptr;
}
class FS4Frame final : public IAutomationLatentCommand
{
    TFunction<bool()> Step;
    uint64 Previous = MAX_uint64;
public:
    explicit FS4Frame(TFunction<bool()> In) : Step(MoveTemp(In)) {}
    virtual bool Update() override
    {
        if (Previous == GFrameCounter) return false;
        Previous = GFrameCounter;
        return Step();
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS4CollisionTest, "ImpactBeyond.S4.PreparedCollision",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FS4CollisionTest::RunTest(const FString&)
{
    UWorld* World = LabWorld();
    if (!TestNotNull(TEXT("S4 game world loaded"), World)) return false;
    const FVector Origin(100000, 0, 0);
    const auto Spawn = [World](EIBS4TargetKind Kind, FVector Position)
    {
        auto* Actor = World->SpawnActorDeferred<AIBS4PreparedTarget>(AIBS4PreparedTarget::StaticClass(), FTransform(Position));
        Actor->Kind = Kind;
        UGameplayStatics::FinishSpawningActor(Actor, FTransform(Position));
        return Actor;
    };
    auto* Wall = Spawn(EIBS4TargetKind::Wall, Origin);
    auto* Crater = Spawn(EIBS4TargetKind::Crater, Origin + FVector(0, 2000, 0));
    auto* Robot = Spawn(EIBS4TargetKind::Robot, Origin + FVector(0, 4000, 0));
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FS4Frame>(
        [=, this, Step = 0]() mutable
        {
            FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Acceptance), false);
            FHitResult Hit;
            const int32 Phase = Step % 4;
            if (Phase == 0)
            {
                TestTrue(TEXT("intact wall blocks capsule"), World->SweepSingleByChannel(Hit, Origin + FVector(-250, 0, 130),
                    Origin + FVector(250, 0, 130), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(38, 90), Query));
                TestTrue(TEXT("wall enters broken state"), Wall->SetBrokenState(true));
                TestTrue(TEXT("crater enters broken state"), Crater->SetBrokenState(true));
                TestTrue(TEXT("cannon detaches once"), Robot->SetBrokenState(true));
                TestFalse(TEXT("cannon cannot detach twice"), Robot->SetBrokenState(true));
            }
            else if (Phase == 1)
            {
                TestFalse(TEXT("broken wall is capsule-transitable"), World->SweepSingleByChannel(Hit, Origin + FVector(-250, 0, 130),
                    Origin + FVector(250, 0, 130), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(38, 90), Query));
                const FVector Center = Crater->GetActorLocation();
                TestTrue(TEXT("crater retains floor collision"), World->LineTraceSingleByChannel(Hit, Center + FVector(0, 0, 200),
                    Center - FVector(0, 0, 300), ECC_Visibility, Query));
                TestTrue(TEXT("crater center is 60 cm deep"), FMath::IsNearlyEqual(Hit.ImpactPoint.Z, Center.Z - 60, 1.0));
                TestFalse(TEXT("detached cannon disables attack"), Robot->bCannonEnabled);
                TestTrue(TEXT("detached cannon is physical"), Robot->GetPart()->IsSimulatingPhysics());
                TestTrue(TEXT("detached cannon is interactable"), Robot->GetPart()->ComponentHasTag(TEXT("IBS4Grabbable")));
            }
            else if (Phase == 2)
            {
                TestTrue(TEXT("wall reset"), Wall->ResetTarget());
                TestTrue(TEXT("crater reset"), Crater->ResetTarget());
                TestTrue(TEXT("robot reset"), Robot->ResetTarget());
            }
            else
            {
                const FVector Center = Crater->GetActorLocation();
                World->LineTraceSingleByChannel(Hit, Center + FVector(0, 0, 200), Center - FVector(0, 0, 300), ECC_Visibility, Query);
                TestTrue(TEXT("reset floor is flush"), FMath::IsNearlyEqual(Hit.ImpactPoint.Z, Center.Z, 1.0));
                TestTrue(TEXT("cannon capability restored"), Robot->bCannonEnabled);
            }
            if (++Step < 80) return false;
            Wall->Destroy(); Crater->Destroy(); Robot->Destroy();
            return true;
        }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS4H01Test, "ImpactBeyond.S4.H01Response",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FS4H01Test::RunTest(const FString&)
{
    UWorld* World = LabWorld();
    if (!TestNotNull(TEXT("S4 game world loaded"), World)) return false;
    auto* Pawn = Cast<AIBS4LabPawn>(UGameplayStatics::GetPlayerPawn(World, 0));
    const float OriginalHealth = Pawn->Response->Health;
    for (int32 I = 0; I < 20; ++I)
    {
        FHitResult Trace;
        auto Hit = UIBS4ResponseComponent::MakeHit(World, nullptr, EIBS4Weapon::Bullet, 999, Trace);
        TestTrue(TEXT("bullet accepted"), Pawn->Response->Receive(Hit));
        TestFalse(TEXT("bullet duplicate rejected"), Pawn->Response->Receive(Hit));
        TestEqual(TEXT("H01 has no bullet health loss"), Pawn->Response->Health, OriginalHealth);
    }
    AddInfo(TEXT("This suite does not certify audiovisual bullet impact or human feel."));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS4ResponseContractTest, "ImpactBeyond.S4.ResponseContract",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FS4ResponseContractTest::RunTest(const FString&)
{
    UWorld* World = LabWorld();
    if (!TestNotNull(TEXT("S4 game world loaded"), World)) return false;
    auto* Target = World->SpawnActor<AIBS4PreparedTarget>(FVector(100000, 20000, 0), FRotator::ZeroRotator);
    auto* Foreign = World->SpawnActor<AIBS4PreparedTarget>(FVector(100000, 22000, 0), FRotator::ZeroRotator);
    if (!Target || !Foreign)
    {
        AddError(TEXT("Response fixtures failed to spawn"));
        if (Target) Target->Destroy();
        if (Foreign) Foreign->Destroy();
        return false;
    }
    Target->Kind = EIBS4TargetKind::MaterialCoupon;
    FHitResult Trace;
    auto Packet = UIBS4ResponseComponent::MakeHit(World, nullptr, EIBS4Weapon::Punch, 60, Trace);
    Packet.Part = Foreign->GetPart();
    TestFalse(TEXT("foreign component rejected before mutation"), Target->Response->Receive(Packet));
    TestEqual(TEXT("foreign component did not consume ID"), Target->Response->AcceptedHits, 0);
    Packet.Part = Target->GetPart();
    Packet.Weapon = static_cast<EIBS4Weapon>(255);
    TestFalse(TEXT("unknown enum rejected"), Target->Response->Receive(Packet));
    Packet.Weapon = EIBS4Weapon::Punch;
    Target->Response->Health = -1;
    TestFalse(TEXT("invalid stored health rejected before ledger"), Target->Response->Receive(Packet));
    Target->Response->Health = 10;
    TestTrue(TEXT("corrected packet keeps same unused ID"), Target->Response->Receive(Packet));
    TestEqual(TEXT("requested structural delta preserved"), Target->Response->LastOutcome.RequestedHealthDelta, -60.f);
    TestEqual(TEXT("actual HP loss clamped"), Target->Response->LastOutcome.HealthDelta, -10.f);
    TestEqual(TEXT("stored HP is zero"), Target->Response->Health, 0.f);
    TestTrue(TEXT("Punch has no implicit Missile delta velocity"), Target->Response->LastOutcome.DeltaVelocity.IsZero());
    TestFalse(TEXT("duplicate packet cannot mutate or republish"), Target->Response->Receive(Packet));
    Target->Response->RestoreHealth();
    TestFalse(TEXT("RestoreHealth does not create a new attack lifetime"), Target->Response->Receive(Packet));
    auto* Robot = World->SpawnActorDeferred<AIBS4PreparedTarget>(AIBS4PreparedTarget::StaticClass(), FTransform(FVector(100000, 24000, 0)));
    if (Robot)
    {
        Robot->Kind = EIBS4TargetKind::Robot;
        UGameplayStatics::FinishSpawningActor(Robot, FTransform(FVector(100000, 24000, 0)));
        TArray<UStaticMeshComponent*> Pieces;
        Robot->GetComponents(Pieces);
        UPrimitiveComponent* Torso = nullptr;
        for (auto* Piece : Pieces) if (Piece != Robot->GetPart() && Piece->IsVisible()) { Torso = Piece; break; }
        TestNotNull(TEXT("robot torso identified"), Torso);
        if (Torso)
        {
            auto BodyHit = UIBS4ResponseComponent::MakeHit(World, nullptr, EIBS4Weapon::Punch, 100, Trace);
            BodyHit.Part = Torso;
            TestTrue(TEXT("torso damage accepted"), Robot->Response->Receive(BodyHit));
            TestTrue(TEXT("torso hit does not detach cannon"), Robot->bCannonEnabled);
            TestEqual(TEXT("torso HP depleted"), Robot->Response->Health, 0.f);
            auto PartHit = UIBS4ResponseComponent::MakeHit(World, nullptr, EIBS4Weapon::Punch, 45, Trace);
            PartHit.Part = Robot->GetPart();
            TestTrue(TEXT("cannon hit after torso depleted accepted"), Robot->Response->Receive(PartHit));
            TestFalse(TEXT("structural request still disables cannon at zero HP"), Robot->bCannonEnabled);
            TestEqual(TEXT("cannon structural hit invents no HP loss"), Robot->Response->LastOutcome.HealthDelta, 0.f);
            TestFalse(TEXT("duplicate cannon hit rejected"), Robot->Response->Receive(PartHit));
        }
        Robot->Destroy();
    }
    else AddError(TEXT("Robot damage fixture failed to spawn"));
    Target->Destroy(); Foreign->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS4RestoreOccupantsTest, "ImpactBeyond.S4.RestoreOccupantsAndCannon",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FS4RestoreOccupantsTest::RunTest(const FString&)
{
    UWorld* World = LabWorld();
    if (!TestNotNull(TEXT("S4 game world loaded"), World)) return false;
    const FVector Origin(100000, 30000, 0);
    auto* Crater = World->SpawnActorDeferred<AIBS4PreparedTarget>(AIBS4PreparedTarget::StaticClass(), FTransform(Origin));
    auto* Robot = World->SpawnActorDeferred<AIBS4PreparedTarget>(AIBS4PreparedTarget::StaticClass(), FTransform(Origin + FVector(0, 2000, 0)));
    if (!Crater || !Robot)
    {
        AddError(TEXT("Restore fixtures failed to spawn"));
        if (Crater) Crater->Destroy();
        if (Robot) Robot->Destroy();
        return false;
    }
    Crater->Kind = EIBS4TargetKind::Crater;
    Robot->Kind = EIBS4TargetKind::Robot;
    UGameplayStatics::FinishSpawningActor(Crater, FTransform(Origin));
    UGameplayStatics::FinishSpawningActor(Robot, FTransform(Origin + FVector(0, 2000, 0)));
    auto* Occupant = World->SpawnActor<AActor>();
    auto* Box = NewObject<UBoxComponent>(Occupant);
    Occupant->SetRootComponent(Box);
    Box->InitBoxExtent(FVector(8));
    Box->SetCollisionProfileName(TEXT("BlockAllDynamic")); // explicitly NON-simulating WorldDynamic
    Box->SetMobility(EComponentMobility::Movable);
    Occupant->SetActorLocation(Origin + FVector(0, 0, -30));
    Box->RegisterComponent();
    Crater->SetBrokenState(true);
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FS4Frame>(
        [=, this, Step = 0]() mutable
        {
            if (Step == 0)
            {
                TestFalse(TEXT("kinematic occupant prevents rising floor"), Crater->ResetTarget());
                TestTrue(TEXT("failed reset leaves crater broken"), Crater->bBroken);
                Occupant->SetActorLocation(Origin + FVector(0, 0, 70));
            }
            else if (Step == 1)
            {
                TestTrue(TEXT("hover above floor permits reset"), Crater->ResetTarget());
                TestTrue(TEXT("cannon detaches"), Robot->SetBrokenState(true));
                Robot->GetPart()->ComponentTags.AddUnique(TEXT("IBS4Held"));
            }
            else if (Step == 2)
            {
                TestFalse(TEXT("held cannon prevents reset"), Robot->ResetTarget());
                TestFalse(TEXT("held detached cannon does not re-enable attack"), Robot->bCannonEnabled);
                FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4CannonTorso), false);
                Query.AddIgnoredComponent(Robot->GetPart());
                FHitResult Hit;
                const FVector At = Robot->GetActorLocation();
                TestTrue(TEXT("ignoring cannon preserves torso blocker"), World->LineTraceSingleByChannel(Hit,
                    At + FVector(250, 0, 85), At + FVector(0, 0, 85), ECC_Visibility, Query));
                TestTrue(TEXT("torso belongs to same robot"), Hit.GetActor() == Robot);
                Robot->GetPart()->ComponentTags.Remove(TEXT("IBS4Held"));
                TestTrue(TEXT("released cannon resets"), Robot->ResetTarget());
                TestFalse(TEXT("reset has no loose simulated cannon"), Robot->GetPart()->IsSimulatingPhysics());
                TestTrue(TEXT("reset restores cannon capability"), Robot->bCannonEnabled);
                Crater->Destroy(); Robot->Destroy(); Occupant->Destroy();
                return true;
            }
            ++Step;
            return false;
        }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS4MissileFlightTest, "ImpactBeyond.S4.MissileFlightRecovery",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FS4MissileFlightTest::RunTest(const FString&)
{
    UWorld* World = LabWorld();
    if (!TestNotNull(TEXT("S4 game world loaded"), World)) return false;
    auto* Pawn = Cast<AIBS4LabPawn>(UGameplayStatics::GetPlayerPawn(World, 0));
    const FTransform Saved = Pawn->GetActorTransform();
    const bool SavedIntent = Pawn->Flight->bFlightIntent;
    const FVector SavedVelocity = Pawn->GetVelocity();
    const FVector Origin(100000, 10000, 0);
    auto* Floor = World->SpawnActor<AActor>();
    auto* Box = NewObject<UBoxComponent>(Floor);
    Floor->SetRootComponent(Box);
    Box->InitBoxExtent(FVector(10000, 3000, 50));
    Box->SetCollisionProfileName(TEXT("BlockAll"));
    Floor->SetActorLocation(Origin - FVector(0, 0, 50)); Box->RegisterComponent();
    Pawn->Flight->SetFlightIntent(true);
    const float InitialHealth = Pawn->Response->Health;
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FS4Frame>(
        [=, this, Trial = 0, Phase = 0, Began = 0.0, HitsBefore = 0, PushesBefore = 0,
         MaxTravel = 0.0, Start = FVector::ZeroVector, Projectile = TWeakObjectPtr<AIBS4Missile>()]() mutable
        {
            const double Now = World->GetTimeSeconds();
            if (Phase == 0)
            {
                Start = Origin + FVector(0, 0, Trial % 2 ? 390 : 140);
                Pawn->SetActorLocation(Start, false, nullptr, ETeleportType::TeleportPhysics);
                Pawn->GetCharacterMovement()->StopMovementImmediately();
                HitsBefore = Pawn->Response->AcceptedHits; PushesBefore = Pawn->Response->AppliedPushes;
                MaxTravel = 0;
                Projectile = World->SpawnActor<AIBS4Missile>(Start - FVector(800, 0, 0), FRotator::ZeroRotator);
                TestTrue(TEXT("missile spawned"), Projectile.IsValid());
                Began = Now; Phase = 1;
                return false;
            }
            MaxTravel = FMath::Max(MaxTravel, FVector::Dist2D(Start, Pawn->GetActorLocation()));
            TestTrue(TEXT("flight intent retains airborne movement"), Pawn->GetCharacterMovement()->MovementMode == MOVE_Flying);
            TestTrue(TEXT("capsule stays above prepared floor"), Pawn->GetActorLocation().Z >= 89);
            TestTrue(TEXT("capsule collision remains enabled"), Pawn->GetCapsuleComponent()->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics);
            TestEqual(TEXT("missile has zero H01 health loss"), Pawn->Response->Health, InitialHealth);
            if (Now - Began < 2.8) return false;
            TestEqual(TEXT("one projectile yields one accepted hit"), Pawn->Response->AcceptedHits - HitsBefore, 1);
            TestEqual(TEXT("one projectile yields one push"), Pawn->Response->AppliedPushes - PushesBefore, 1);
            TestTrue(TEXT("knockback measures 3-8 m in clear lab lane"), MaxTravel >= 300 && MaxTravel <= 800);
            TestTrue(TEXT("recoverable velocity"), Pawn->GetVelocity().Size() < 120);
            AddInfo(FString::Printf(TEXT("IBS4 missile trial=%d travel_cm=%.2f end_speed=%.2f"), Trial + 1, MaxTravel, Pawn->GetVelocity().Size()));
            if (Projectile.IsValid()) Projectile->Destroy();
            if (++Trial < 20) { Phase = 0; return false; }
            Pawn->SetActorTransform(Saved, false, nullptr, ETeleportType::TeleportPhysics);
            Pawn->Flight->SetFlightIntent(SavedIntent);
            Pawn->GetCharacterMovement()->Velocity = SavedVelocity;
            Floor->Destroy();
            return true;
        }));
    AddInfo(TEXT("Twenty real projectiles, alternating 0.5 m / 3 m clearance. Does not replace free-flight or gamepad QA."));
    return true;
}
#endif
