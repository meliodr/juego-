#if WITH_DEV_AUTOMATION_TESTS
#include "IBS4ResponseComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS4ConsumerBindingTest, "ImpactBeyond.S4.Consumer.Binding",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FS4ConsumerBindingTest::RunTest(const FString&)
{
    UWorld* World = nullptr;
    for (const auto& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("game world required; no render validation implied"), World)) return false;
    auto* Hero = World->SpawnActor<ACharacter>(FVector(120000, 40000, 1000), FRotator::ZeroRotator);
    auto* Owner = World->SpawnActor<AActor>();
    auto* Stranger = World->SpawnActor<AActor>();
    if (!Hero || !Owner || !Stranger)
    {
        AddError(TEXT("consumer fixtures did not spawn"));
        if (Hero) Hero->Destroy();
        if (Owner) Owner->Destroy();
        if (Stranger) Stranger->Destroy();
        return false;
    }
    auto* Response = NewObject<UIBS4ResponseComponent>(Hero);
    Response->RegisterComponent();
    Response->bH01 = true;
    Response->Health = 37;
    Hero->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    FIBS4IngressRouter Empty;
    TestFalse(TEXT("empty router rejected"), Response->BindConsumer(Owner, Empty));
    FIBS4IngressRouter Router;
    int32 Routes = 0;
    Router.BindLambda([&](UIBS4ResponseComponent& Target, const FIBS4Hit& Hit)
    {
        ++Routes;
        TestFalse(TEXT("router cannot reenter normal ingress"), Target.Receive(Hit));
        TestFalse(TEXT("cannot unbind inside callback"), Target.UnbindConsumer(Owner));
        auto Replacement = Hit; Replacement.AttackId = UIBS4ResponseComponent::AllocateAttackId();
        TestFalse(TEXT("router cannot substitute a new contact serial"), Target.ConsumeBoundHit(Owner, Replacement, false));
        TestTrue(TEXT("bound native contact consumed"), Target.ConsumeBoundHit(Owner, Hit, false));
        TestFalse(TEXT("one response per routed contact"), Target.ConsumeBoundHit(Owner, Hit, false));
    });
    TestTrue(TEXT("bind sole consumer"), Response->BindConsumer(Owner, Router));
    TestFalse(TEXT("second binding refused"), Response->BindConsumer(Stranger, Router));
    FHitResult Trace;
    auto Punch = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Punch, 60, Trace);
    TestFalse(TEXT("foreign owner cannot consume"), Response->ConsumeBoundHit(Stranger, Punch, false));
    TestTrue(TEXT("normal ingress reaches canonical callback fixture"), Response->Receive(Punch));
    TestEqual(TEXT("one callback"), Routes, 1);
    TestEqual(TEXT("one accepted outcome"), Response->AcceptedHits, 1);
    TestEqual(TEXT("bound H01 HP invariant at non-default value"), Response->Health, 37.f);
    TestEqual(TEXT("H01 structural damage also filtered"), Response->LastOutcome.RequestedHealthDelta, 0.f);
    TestFalse(TEXT("canonical replay rejected"), Response->ConsumeBoundHit(Owner, Punch, false));
    auto Missile = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Missile, 999, Trace);
    TestTrue(TEXT("resolved missile accepted once"), Response->ConsumeBoundHit(Owner, Missile, true));
    TestEqual(TEXT("resolved missile adds no CMC impulse"), Response->AppliedPushes, 0);
    TestTrue(TEXT("resolved missile reports zero delta velocity"), Response->LastOutcome.DeltaVelocity.IsZero());
    Missile = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Missile, 999, Trace);
    TestTrue(TEXT("rapid native missile routed"), Response->Receive(Missile));
    TestEqual(TEXT("shared cooldown prevents second push"), Response->AppliedPushes, 0);
    TestTrue(TEXT("binding does not force Walking"), Hero->GetCharacterMovement()->MovementMode == MOVE_Flying);
    TestFalse(TEXT("foreign owner cannot unbind"), Response->UnbindConsumer(Stranger));
    TestTrue(TEXT("owner unbinds explicitly"), Response->UnbindConsumer(Owner));
    TestFalse(TEXT("bound endpoint closed after unbind"), Response->ConsumeBoundHit(Owner, Punch, false));
    TestFalse(TEXT("unbind preserves replay ledger"), Response->Receive(Punch));
    TestTrue(TEXT("rebind allowed without resetting policy"), Response->BindConsumer(Owner, Router));
    TestFalse(TEXT("rebind preserves replay ledger"), Response->ConsumeBoundHit(Owner, Punch, false));

    TestTrue(TEXT("unbind for expired-owner fixture"), Response->UnbindConsumer(Owner));
    FIBS4IngressRouter Never;
    Never.BindLambda([this](UIBS4ResponseComponent&, const FIBS4Hit&) { AddError(TEXT("dead owner's callback executed")); });
    TestTrue(TEXT("bind expiring owner"), Response->BindConsumer(Owner, Never));
    Owner->Destroy();
    const auto AfterOwner = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Bullet, 10, Trace);
    TestFalse(TEXT("expired owner fails closed"), Response->Receive(AfterOwner));
    TestTrue(TEXT("expired binding does not fall back to standalone"), Response->HasConsumerBinding());
    TestEqual(TEXT("final H01 HP invariant"), Response->Health, 37.f);
    Hero->Destroy(); Stranger->Destroy();
    return true;
}
#endif
