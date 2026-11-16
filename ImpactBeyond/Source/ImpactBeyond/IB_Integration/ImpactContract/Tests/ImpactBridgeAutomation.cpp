// Compiled only in the combined checkout where the S4 consumer is present.
// Prepared regression, NOT run by the native runner and NOT yet executed in UE.
#if WITH_DEV_AUTOMATION_TESTS && __has_include("IB_S4/IBS4ResponseComponent.h")
#include "../ImpactContractUE.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactBridgeBoundTest, "ImpactBeyond.Integration.ImpactContract.BoundBridge",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FImpactBridgeBoundTest::RunTest(const FString&)
{
    UWorld* World = nullptr;
    for (const auto& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("game world required"), World)) return false;
    auto* Hero = World->SpawnActor<ACharacter>(FVector(120000, 42000, 1000), FRotator::ZeroRotator);
    auto* Owner = World->SpawnActor<AActor>();
    auto* Stranger = World->SpawnActor<AActor>();
    if (!Hero || !Owner || !Stranger)
    {
        AddError(TEXT("bridge fixtures did not spawn"));
        if (Hero) Hero->Destroy();
        if (Owner) Owner->Destroy();
        if (Stranger) Stranger->Destroy();
        return false;
    }
    auto* Response = NewObject<UIBS4ResponseComponent>(Hero);
    Response->RegisterComponent(); Response->bH01 = true; Response->Health = 37;
    const FGuid Session = FGuid::NewGuid(), Life = FGuid::NewGuid(), Instigator = FGuid::NewGuid(), Part = FGuid::NewGuid();
    IBImpactUE::S4Bridge Bridge(*Response, *Owner, Session, Life, TEXT("S4.H01"));
    IBImpactUE::S4Bridge ForeignBridge(*Response, *Stranger, Session, Life, TEXT("S4.H01"));
    IBImpactUE::S4Identity Captured{Session, Life, Instigator, Part};
    IBImpactUE::Record Last;
    FIBS4IngressRouter Ingress;
    Ingress.BindLambda([&](UIBS4ResponseComponent& Target, const FIBS4Hit& Hit)
    {
        TestFalse(TEXT("normal ingress reentry rejected"), Target.Receive(Hit));
        Last = Bridge.ReceiveS4(Hit, Captured); // actual bridge, not an endpoint fixture
    });
    TestTrue(TEXT("bind owner"), Response->BindConsumer(Owner, Ingress));
    TestFalse(TEXT("second authority rejected"), Response->BindConsumer(Stranger, Ingress));
    FHitResult Trace;
    auto Hit = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Punch, 60, Trace);
    TestTrue(TEXT("native ingress reaches real bridge and bound consumer"), Response->Receive(Hit));
    TestTrue(TEXT("bridge returns acceptance"), Last.Result.State == IBImpact::Status::Accepted);
    TestEqual(TEXT("one accepted hit"), Response->AcceptedHits, 1);
    TestEqual(TEXT("bound Punch preserves H01 HP"), Response->Health, 37.f);
    TestFalse(TEXT("native replay rejected"), Response->Receive(Hit));
    TestEqual(TEXT("replay does not add hit"), Response->AcceptedHits, 1);

    Hit = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Laser, 60, Trace);
    TestTrue(TEXT("Laser reaches bound consumer"), Response->Receive(Hit));
    TestEqual(TEXT("bound Laser preserves H01 HP"), Response->Health, 37.f);
    Captured.TargetId = FGuid::NewGuid();
    Hit = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Punch, 60, Trace);
    TestFalse(TEXT("captured wrong life rejected"), Response->Receive(Hit));
    Captured.TargetId = Life;
    Captured.bPhysicsResolved = true;
    Hit = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Missile, 60, Trace);
    TestTrue(TEXT("resolved native missile accepted"), Response->Receive(Hit));
    TestTrue(TEXT("resolved flag preserved"), Last.Request.PhysicsResolved);
    TestTrue(TEXT("no second CMC request"), Last.Result.Applied.DeltaVelocity.Zero());
    TestEqual(TEXT("no CMC application"), Response->AppliedPushes, 0);
    Captured.bPhysicsResolved = false;
    Hit = UIBS4ResponseComponent::MakeHit(World, Hero, EIBS4Weapon::Missile, 60, Trace);
    TestTrue(TEXT("rapid native missile accepted"), Response->Receive(Hit));
    TestEqual(TEXT("resolved contact reserves cooldown"), Response->AppliedPushes, 0);

    FIBS35DamageMessage S35;
    S35.AttackId = FGuid::NewGuid(); S35.TargetId = Life; S35.InstigatorId = Instigator;
    S35.InstigatorActor = Hero; S35.TargetPart = Hero->GetCapsuleComponent();
    S35.DamageKind = EIBS35DamageKind::ThrownObject; S35.HealthDelta = -60;
    S35.Impulse = FVector(120000, 0, 0); S35.bPhysicsResolved = true; S35.AttackSeconds = 5;
    TestTrue(TEXT("wrong owner cannot consume"), ForeignBridge.ReceiveS35(S35, Hero->GetCapsuleComponent(), Part).Result.State == IBImpact::Status::ReceiverRejected);
    Last = Bridge.ReceiveS35(S35, Hero->GetCapsuleComponent(), Part);
    TestTrue(TEXT("S35 throw delivered to bound endpoint"), Last.Result.State == IBImpact::Status::Accepted);
    TestEqual(TEXT("S35 throw preserves bound H01 HP"), Response->Health, 37.f);
    TestTrue(TEXT("resolved throw has no second physical impulse"), Last.Result.Applied.Impulse.Zero());
    TestTrue(TEXT("S35 duplicate sealed"), Bridge.ReceiveS35(S35, Hero->GetCapsuleComponent(), Part).Result.State == IBImpact::Status::Duplicate);
    TestTrue(TEXT("unbind before destruction"), Response->UnbindConsumer(Owner));
    S35.AttackId = FGuid::NewGuid();
    TestTrue(TEXT("unbound bridge fails closed without lab fallback"), Bridge.ReceiveS35(S35, Hero->GetCapsuleComponent(), Part).Result.State == IBImpact::Status::Invalid);
    TestTrue(TEXT("explicit rebind"), Response->BindConsumer(Owner, Ingress));
    Owner->Destroy();
    TestTrue(TEXT("expired binding owner fails closed"), Bridge.ReceiveS35(S35, Hero->GetCapsuleComponent(), Part).Result.State == IBImpact::Status::Invalid);
    TestEqual(TEXT("final H01 HP unchanged"), Response->Health, 37.f);
    // Consumer's expired owner prevents callback execution; destroy target before stack bridge.
    Hero->Destroy(); Stranger->Destroy();
    return true;
}
#endif
