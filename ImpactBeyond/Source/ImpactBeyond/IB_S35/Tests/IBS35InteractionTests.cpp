#if WITH_DEV_AUTOMATION_TESTS

#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35PunchComponent.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35LabTarget.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include <limits>

namespace
{
UWorld* GameWorld()
{
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game) return Context.World();
    return nullptr;
}

struct FStage
{
    UWorld* World;
    TArray<TWeakObjectPtr<AActor>> Actors;
    explicit FStage(UWorld* InWorld) : World(InWorld) {}
    ~FStage() { for (auto Actor : Actors) if (Actor.IsValid()) Actor->Destroy(); }
    template<class T> T* Spawn(const FVector& Position)
    {
        FActorSpawnParameters Params;
        Params.ObjectFlags |= RF_Transient;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        T* Actor = World->SpawnActor<T>(Position, FRotator::ZeroRotator, Params);
        Actors.Add(Actor); return Actor;
    }
    AIBS35Object* Object(const FVector& Position, EIBS35ObjectClass Kind)
    {
        auto* Actor = World->SpawnActorDeferred<AIBS35Object>(AIBS35Object::StaticClass(), FTransform(Position),
            nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        Actor->SetFlags(RF_Transient);
        Actor->Profile = FIBS35ObjectProfile::Preset(Kind);
        Actor->FinishSpawning(FTransform(Position)); Actors.Add(Actor); return Actor;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS35GripTests, "ImpactBeyond.S35.GripAndReset",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBS35GripTests::RunTest(const FString& Parameters)
{
    UWorld* World = GameWorld();
    if (!TestNotNull(TEXT("Run in a game world"), World)) return false;
    FStage Stage(World);
    const FVector Origin(500000, 500000, 5000);
    auto* Carrier = Stage.Spawn<ACharacter>(Origin);
    auto* Grip = NewObject<UIBS35GripComponent>(Carrier); Grip->RegisterComponent();
    auto* Rival = Stage.Spawn<ACharacter>(Origin + FVector(0,120,0));
    auto* RivalGrip = NewObject<UIBS35GripComponent>(Rival); RivalGrip->RegisterComponent();
    for (const auto Kind : {EIBS35ObjectClass::Car, EIBS35ObjectClass::Container, EIBS35ObjectClass::Pole,
        EIBS35ObjectClass::Rock, EIBS35ObjectClass::LoosePart})
    {
        auto* Object = Stage.Object(Origin + FVector(300,0,100), Kind);
        for (int Cycle = 0; Cycle < 30; ++Cycle)
        {
            TestTrue(TEXT("Grab"), Grip->TryGrab(Object));
            TestFalse(TEXT("Rival cannot claim held object"), RivalGrip->TryGrab(Object));
            TestFalse(TEXT("One carrier cannot re-claim"), Grip->TryGrab(Object));
            for (int Frame = 0; Frame < 60; ++Frame) Grip->TickComponent(.02f, LEVELTICK_All, nullptr);
            TestTrue(TEXT("World collision retained"), Object->Body->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics);
            TestTrue(TEXT("Charge"), Grip->BeginCharge());
            Grip->CancelCharge();
            TestFalse(TEXT("Cancelled release cannot launch"), Grip->CommitThrow(FVector::ForwardVector));
            Grip->Drop();
            TestTrue(TEXT("Simulation restored"), Object->Body->IsSimulatingPhysics());
            TestFalse(TEXT("Carrier ignore restored"), Object->Body->GetMoveIgnoreActors().Contains(Carrier));
            TestTrue(TEXT("Reset"), Object->ResetObject());
            TestFalse(TEXT("Owner released"), Object->IsHeld());
        }
        Object->Destroy();
    }
    auto* Object = Stage.Object(Origin + FVector(300,0,100), EIBS35ObjectClass::Car);
    auto* Wall = Stage.Spawn<AIBS35LabTarget>(Origin + FVector(800,0,200));
    Wall->Body->SetBoxExtent(FVector(2,1000,1000));
    for (int Cycle = 0; Cycle < 20; ++Cycle)
    {
        Carrier->SetActorLocation(Origin, false, nullptr, ETeleportType::TeleportPhysics);
        TestTrue(TEXT("Reset before wall"), Object->ResetObject());
        TestTrue(TEXT("Grab before wall"), Grip->TryGrab(Object));
        for (int Frame = 0; Frame < 60; ++Frame) Grip->TickComponent(.02f, LEVELTICK_All, nullptr);
        for (int Step = 0; Step < 40 && Grip->GetHeld(); ++Step)
        {
            Carrier->SetActorLocation(Origin + FVector(Step * 35,0,0), false, nullptr, ETeleportType::TeleportPhysics);
            Grip->TickComponent(.02f, LEVELTICK_All, nullptr);
        }
        TestTrue(TEXT("Actual box did not pass thin wall"),
            Object->Body->Bounds.GetBox().Max.X <= Wall->GetActorLocation().X - 1);
        Grip->Drop();
    }
    // Destroyed carrier releases the claim and restores physics.
    Carrier->SetActorLocation(Origin, false, nullptr, ETeleportType::TeleportPhysics);
    Object->ResetObject(); Grip->TryGrab(Object); Grip->DestroyComponent();
    TestFalse(TEXT("Destroyed holder releases object"), Object->IsHeld());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS35PunchTests, "ImpactBeyond.S35.PunchAndContract",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBS35PunchTests::RunTest(const FString& Parameters)
{
    UWorld* World = GameWorld();
    if (!TestNotNull(TEXT("Run in a game world"), World)) return false;
    FStage Stage(World);
    const FVector Origin(700000,500000,5000);
    auto* Hero = Stage.Spawn<ACharacter>(Origin);
    auto* Punch = NewObject<UIBS35PunchComponent>(Hero); Punch->RegisterComponent(); Punch->bDrawContacts = false;
    auto* Target = Stage.Spawn<AIBS35LabTarget>(Origin + FVector(145,0,40));
    Target->Body->SetBoxExtent(FVector(10,80,80));
    for (const auto Mode : {MOVE_Walking, MOVE_Flying})
    {
        Hero->GetCharacterMovement()->SetMovementMode(Mode);
        for (int Attack = 0; Attack < 30; ++Attack)
        {
            const int Before = Target->Receiver->AcceptedHits;
            TestTrue(TEXT("Start punch"), Punch->BeginPunch((Attack % 2) != 0));
            TestFalse(TEXT("Cannot overlap attacks"), Punch->BeginPunch(false));
            for (int Frame = 0; Frame < 100; ++Frame) Punch->TickComponent(.01f, LEVELTICK_All, nullptr);
            TestEqual(TEXT("Exactly one damage per attack/target"), Target->Receiver->AcceptedHits, Before + 1);
        }
    }
    const int BeforeCancel = Target->Receiver->AcceptedHits;
    Punch->BeginPunch(true); Punch->CancelAttack(); Punch->TickComponent(1, LEVELTICK_All, nullptr);
    TestEqual(TEXT("Cancel during windup has no damage"), Target->Receiver->AcceptedHits, BeforeCancel);
    Punch->BeginPunch(false); Punch->TickComponent(1, LEVELTICK_All, nullptr);
    TestEqual(TEXT("Hitch still resolves one contact"), Target->Receiver->AcceptedHits, BeforeCancel + 1);
    auto* Wall = Stage.Spawn<AIBS35LabTarget>(Origin + FVector(95,0,40));
    Wall->Body->SetBoxExtent(FVector(2,100,100));
    const int BeforeWall = Target->Receiver->AcceptedHits;
    Punch->BeginPunch(false); Punch->TickComponent(1, LEVELTICK_All, nullptr);
    TestEqual(TEXT("Punch blocked by first wall"), Target->Receiver->AcceptedHits, BeforeWall);
    Punch->BeginPunch(false);
    for (int Frame = 0; Frame < 100; ++Frame) Punch->TickComponent(.01f, LEVELTICK_All, nullptr);
    TestEqual(TEXT("Later frames cannot resume beyond blocker"), Target->Receiver->AcceptedHits, BeforeWall);
    FIBS35DamageMessage Message;
    Message.AttackId = FGuid::NewGuid(); Message.InstigatorId = FGuid::NewGuid();
    Message.TargetId = Target->Receiver->TargetId; Message.HealthDelta = -10;
    TestTrue(TEXT("First message"), Target->Receiver->Receive(Message, Target->Body));
    TestFalse(TEXT("Replay"), Target->Receiver->Receive(Message, Target->Body));
    Target->Receiver->ResetReceiver();
    TestFalse(TEXT("Old target life cannot receive stale message"), Target->Receiver->Receive(Message, Target->Body));
    Message.AttackId = FGuid::NewGuid(); Message.TargetId = Target->Receiver->TargetId;
    Message.HealthDelta = std::numeric_limits<float>::quiet_NaN();
    TestFalse(TEXT("NaN rejected"), Target->Receiver->Receive(Message, Target->Body));
    return true;
}

namespace
{
// This command needs real game frames/Chaos. Preparing it is not a recorded physics pass.
class FIBS35ThrowRun final : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    FStage Stage;
    ACharacter* Carrier = nullptr;
    UIBS35GripComponent* Grip = nullptr;
    AIBS35LabTarget* Wall = nullptr;
    AIBS35Object* Object = nullptr;
    FVector Origin = FVector(900000,500000,5000);
    int32 Shot = 0;
    double Started = 0;
    double FirstHit = 0;
public:
    FIBS35ThrowRun(FAutomationTestBase* InTest, UWorld* World) : Test(InTest), Stage(World)
    {
        Carrier = Stage.Spawn<ACharacter>(Origin);
        Carrier->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        Grip = NewObject<UIBS35GripComponent>(Carrier); Grip->RegisterComponent();
        Wall = Stage.Spawn<AIBS35LabTarget>(Origin + FVector(2000,0,0));
        Wall->Body->SetBoxExtent(FVector(2,1500,1500));
    }
    bool Update() override
    {
        if (Shot >= 30) return true;
        const double Now = FPlatformTime::Seconds();
        if (!Object)
        {
            const EIBS35ObjectClass Kinds[] = {EIBS35ObjectClass::Car, EIBS35ObjectClass::Container, EIBS35ObjectClass::Rock};
            Carrier->SetActorLocation(Origin, false, nullptr, ETeleportType::TeleportPhysics);
            Carrier->GetCharacterMovement()->Velocity = FVector::ZeroVector;
            Object = Stage.Object(Origin + FVector(300,0,100), Kinds[Shot/10]);
            Wall->Receiver->ResetReceiver();
            if (!Test->TestTrue(TEXT("Throw setup grab"), Grip->TryGrab(Object))) return true;
            for (int Frame = 0; Frame < 60; ++Frame) Grip->TickComponent(.02f, LEVELTICK_All, nullptr);
            if (!Test->TestTrue(TEXT("Throw setup charge"), Grip->BeginCharge())) return true;
            for (int Frame = 0; Frame < 50; ++Frame) Grip->TickComponent(.02f, LEVELTICK_All, nullptr);
            if (!Test->TestTrue(TEXT("Explicit launch"), Grip->CommitThrow(FVector::ForwardVector))) return true;
            Test->TestTrue(TEXT("Released object simulates"), Object->Body->IsSimulatingPhysics());
            Test->TestTrue(TEXT("CCD remains enabled"), Object->Body->BodyInstance.bUseCCD);
            Started = Now; FirstHit = 0; return false;
        }
        if (Object->GetActorLocation().X > Wall->GetActorLocation().X + Object->Body->GetScaledBoxExtent().X)
        { Test->AddError(TEXT("Thrown body crossed the thin wall")); return true; }
        if (Wall->Receiver->AcceptedHits > 0 && FirstHit == 0) FirstHit = Now;
        if ((FirstHit > 0 && Now - FirstHit >= .25) || Now - Started > 2.5)
        {
            Test->TestEqual(TEXT("One physical impact per launch/target"), Wall->Receiver->AcceptedHits, 1);
            Test->TestTrue(TEXT("Native box remains before wall"), Object->Body->Bounds.GetBox().Max.X <= Wall->GetActorLocation().X + 5);
            Test->AddInfo(FString::Printf(TEXT("IBS35 throw=%d class=%d mass=%.2f hit=%d location=%s"),
                Shot+1, int32(Object->Profile.ObjectClass), Object->Body->GetMass(), Wall->Receiver->AcceptedHits, *Object->GetActorLocation().ToString()));
            Object->Destroy(); Object = nullptr; ++Shot;
        }
        return false;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS35ThrowTests, "ImpactBeyond.S35.ThrowsPhysical",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS35ThrowTests::RunTest(const FString& Parameters)
{
    UWorld* World = GameWorld();
    if (!TestNotNull(TEXT("Run in a real game world with physics"), World)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FIBS35ThrowRun(this, World));
    return true;
}

#endif
