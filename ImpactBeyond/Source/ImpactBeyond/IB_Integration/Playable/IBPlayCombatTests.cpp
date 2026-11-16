#include "IBPlayCombat.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "IB_S35/IBS35ReceiverComponent.h"
#include "IB_S6H/IBS6HBridge.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBPlayCombatTest, "ImpactBeyond.Integration.PlayableCombat",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBPlayCombatTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const auto& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("running world"), World)) return false;
    FActorSpawnParameters Spawn;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Origin(900000, 900000, 100000);
    auto* Source = World->SpawnActor<AActor>(AActor::StaticClass(), Origin-FVector(500,0,0), FRotator::ZeroRotator, Spawn);
    auto* Target = World->SpawnActor<AActor>(AActor::StaticClass(), Origin, FRotator::ZeroRotator, Spawn);
    auto* Body = NewObject<UBoxComponent>(Target);
    Target->SetRootComponent(Body);
    Body->SetBoxExtent(FVector(40));
    Body->SetCollisionProfileName(TEXT("BlockAll"));
    Body->RegisterComponent();
    Target->SetActorLocation(Origin);
    auto* Legacy = NewObject<UIBS35ReceiverComponent>(Target);
    Target->AddInstanceComponent(Legacy);
    Legacy->InitialHealth = 1000;
    Legacy->RegisterComponent();
    auto* Combat = NewObject<UIBPlayCombat>(Target);
    Target->AddInstanceComponent(Combat);
    Combat->RegisterComponent();
    TestTrue(TEXT("canonical owner initializes"), Combat->InitializeReceiver());
    TestTrue(TEXT("both ingress ports are bound"), Legacy->HasCanonicalIngress() && Combat->Response->HasConsumerBinding());
    TestEqual(TEXT("legacy health normalizes to S4 fraction"), Combat->GetHealth(), 100.f);
    int32 ImpactCount = 0;
    FGuid LastCause;
    Combat->OnCanonicalImpact.AddLambda([&](FGuid Cause, AActor* Instigator, FVector Point, float Magnitude)
    {
        ++ImpactCount;
        LastCause = Cause;
        TestTrue(TEXT("canonical event has a stable cause"), Cause.IsValid());
        TestTrue(TEXT("canonical event retains instigator"), Instigator == Source);
        TestTrue(TEXT("canonical event retains contact"), Point.Equals(Origin));
        TestTrue(TEXT("canonical event has positive magnitude"), Magnitude > 0);
    });
    FHitResult Trace(Target, Body, Origin, FVector::ForwardVector);
    Trace.bBlockingHit = true;
    FIBS35DamageMessage Contact;
    Contact.AttackId = FGuid::NewGuid();
    Contact.InstigatorId = FGuid::NewGuid();
    Contact.InstigatorActor = Source;
    Contact.TargetId = Combat->GetTargetLife();
    Contact.TargetPart = Body;
    Contact.Contact = Origin;
    Contact.HealthDelta = -25;
    Contact.DamageKind = EIBS35DamageKind::QuickPunch;
    TestTrue(TEXT("legacy producer enters canonical owner"), Legacy->Receive(Contact, Body));
    TestEqual(TEXT("single S4 health subtraction"), Combat->GetHealth(), 75.f);
    TestEqual(TEXT("legacy health is a mirror"), Legacy->Health, 75.f);
    TestFalse(TEXT("repeated S35 event is rejected"), Legacy->Receive(Contact, Body));
    TestEqual(TEXT("duplicate cannot reduce health"), Combat->GetHealth(), 75.f);
    TestEqual(TEXT("physical ingress publishes only accepted impact"), ImpactCount, 1);
    TestEqual(TEXT("S35 cause survives dispatch"), LastCause, Contact.AttackId);
    const auto Native = UIBS4ResponseComponent::MakeHit(World, Source, EIBS4Weapon::Bullet, 20, Trace);
    TestTrue(TEXT("native S4 ingress retains serial through canonical owner"), Combat->Response->Receive(Native));
    TestEqual(TEXT("mixed producer health"), Combat->GetHealth(), 55.f);
    TestFalse(TEXT("native replay is rejected"), Combat->DispatchS4(Native, Trace));
    TestEqual(TEXT("native ingress publishes once"), ImpactCount, 2);
    const FGuid NativeCause = LastCause;
    TestEqual(TEXT("native cause uses the process-wide serial identity"), NativeCause, UIBPlayCombat::NativeCauseId(Native.AttackId));
    auto* OtherTarget = World->SpawnActor<AActor>(AActor::StaticClass(), Origin+FVector(0,500,0), FRotator::ZeroRotator, Spawn);
    auto* OtherBody = NewObject<UBoxComponent>(OtherTarget);
    OtherTarget->SetRootComponent(OtherBody);
    OtherBody->SetBoxExtent(FVector(40));
    OtherBody->SetCollisionProfileName(TEXT("BlockAll"));
    OtherBody->RegisterComponent();
    OtherTarget->SetActorLocation(Origin+FVector(0,500,0));
    auto* OtherCombat = NewObject<UIBPlayCombat>(OtherTarget);
    OtherTarget->AddInstanceComponent(OtherCombat);
    OtherCombat->RegisterComponent();
    TestTrue(TEXT("second blast recipient initializes independently"), OtherCombat->InitializeReceiver());
    FGuid OtherCause;
    int32 OtherImpactCount = 0;
    OtherCombat->OnCanonicalImpact.AddLambda([&](FGuid Cause, AActor*, FVector, float)
    {
        OtherCause = Cause;
        ++OtherImpactCount;
    });
    FHitResult OtherTrace(OtherTarget, OtherBody, OtherTarget->GetActorLocation(), FVector::ForwardVector);
    OtherTrace.bBlockingHit = true;
    auto SameBlast = Native;
    SameBlast.Part = OtherBody;
    SameBlast.Point = OtherTarget->GetActorLocation();
    TestTrue(TEXT("same native blast damages another receiver"), OtherCombat->DispatchS4(SameBlast, OtherTrace));
    TestEqual(TEXT("second receiver applies its own damage once"), OtherCombat->GetHealth(), 80.f);
    TestEqual(TEXT("multi-target native attack retains one civic cause"), OtherCause, NativeCause);
    TestFalse(TEXT("second recipient rejects duplicate delivery"), OtherCombat->DispatchS4(SameBlast, OtherTrace));
    TestEqual(TEXT("second recipient publishes once"), OtherImpactCount, 1);
    TestFalse(TEXT("invalid native serial has no civic identity"), UIBPlayCombat::NativeCauseId(0).IsValid());
    TestTrue(TEXT("native identity retains high serial bits"), UIBPlayCombat::NativeCauseId(1) != UIBPlayCombat::NativeCauseId((int64(1) << 32) + 1));
    OtherTarget->Destroy();
    const auto Queued = UIBS4ResponseComponent::MakeHit(World, Source, EIBS4Weapon::Bullet, 20, Trace);
    TestTrue(TEXT("separate native attacks have separate civic causes"), NativeCause != UIBPlayCombat::NativeCauseId(Queued.AttackId));
    const FGuid PreviousLife = Contact.TargetId;
    Legacy->ResetReceiver();
    TestTrue(TEXT("reset creates a new receiver life"), Combat->GetTargetLife() != PreviousLife);
    TestEqual(TEXT("reset restores normalized health"), Combat->GetHealth(), 100.f);
    TestFalse(TEXT("queued S4 packet from old life is refused"), Combat->Response->Receive(Queued));
    TestFalse(TEXT("old S35 target life is refused"), Combat->DispatchS35(Contact, Trace));
    Contact.AttackId = FGuid::NewGuid();
    Contact.TargetId = Combat->GetTargetLife();
    Contact.HealthDelta = -40;
    Contact.DamageKind = EIBS35DamageKind::ThrownObject;
    Contact.bPhysicsResolved = true;
    TestTrue(TEXT("new physical throw is accepted"), Legacy->Receive(Contact, Body));
    TestEqual(TEXT("physical throw reduces one health owner"), Combat->GetHealth(), 60.f);
    TestEqual(TEXT("physical throw mirrors health"), Legacy->Health, 60.f);

    auto* Hero = World->SpawnActor<ACharacter>(ACharacter::StaticClass(), Origin+FVector(0,2000,0), FRotator::ZeroRotator, Spawn);
    auto* Powers = NewObject<UIBS6HBridge>(Hero);
    Hero->AddInstanceComponent(Powers);
    Powers->RegisterComponent();
    auto* HeroCombat = NewObject<UIBPlayCombat>(Hero);
    Hero->AddInstanceComponent(HeroCombat);
    HeroCombat->RegisterComponent();
    FHitResult HeroTrace(Hero, Hero->GetCapsuleComponent(), Hero->GetActorLocation(), FVector::ForwardVector);
    HeroTrace.bBlockingHit = true;
    auto HeroHit = UIBS4ResponseComponent::MakeHit(World, Source, EIBS4Weapon::Bullet, 40, HeroTrace);
    TestTrue(TEXT("H01 receives contact"), HeroCombat->DispatchS4(HeroHit, HeroTrace));
    TestEqual(TEXT("H01 protection is applied by bound S4"), HeroCombat->GetHealth(), 100.f);
    TestTrue(TEXT("switch to H02 for shield"), Powers->SelectHero(1));
    TestTrue(TEXT("equip H02 shield"), Powers->EquipAbility(201));
    TestTrue(TEXT("spawn H02 shield"), Powers->ActivateEquipped());
    HeroHit = UIBS4ResponseComponent::MakeHit(World, Source, EIBS4Weapon::Bullet, 40, HeroTrace);
    TestTrue(TEXT("shield contact accepted"), HeroCombat->DispatchS4(HeroHit, HeroTrace));
    TestEqual(TEXT("shield absorbs before health"), HeroCombat->GetHealth(), 100.f);
    TestFalse(TEXT("shield duplicate rejected before absorption"), HeroCombat->DispatchS4(HeroHit, HeroTrace));
    HeroHit = UIBS4ResponseComponent::MakeHit(World, Source, EIBS4Weapon::Bullet, 80, HeroTrace);
    TestTrue(TEXT("second shield contact accepted"), HeroCombat->DispatchS4(HeroHit, HeroTrace));
    TestEqual(TEXT("only unabsorbed damage reduces HP"), HeroCombat->GetHealth(), 80.f);
    TestEqual(TEXT("hero S35 health mirrors shield outcome"), HeroCombat->ContactReceiver->Health, 80.f);
    Hero->Destroy();
    Target->Destroy();
    Source->Destroy();
    return true;
}
#endif
