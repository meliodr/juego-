#include "IBS6HBridge.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS6HRuntimeTest, "ImpactBeyond.S6.Runtime",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBS6HRuntimeTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const auto& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("running game world"), World)) return false;
    FActorSpawnParameters Spawn;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Origin(800000, 800000, 100000);
    auto* Character = World->SpawnActor<ACharacter>(ACharacter::StaticClass(), Origin, FRotator::ZeroRotator, Spawn);
    if (!TestNotNull(TEXT("test character"), Character)) return false;
    Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    auto* Heroes = NewObject<UIBS6HBridge>(Character);
    Character->AddInstanceComponent(Heroes);
    Heroes->bAllowNonPlanetLab = true;
    Heroes->RegisterComponent();
    TestEqual(TEXT("three hero descriptors"), Heroes->GetHeroDescriptors().Num(), 3);
    TestEqual(TEXT("six H01 slots"), Heroes->GetAbilities().Num(), 6);
    TestFalse(TEXT("H02 equipment rejected for H01"), Heroes->EquipAbility(201));
    TestTrue(TEXT("select H02"), Heroes->SelectHero(1));
    TestEqual(TEXT("six H02 slots"), Heroes->GetAbilities().Num(), 6);
    TestTrue(TEXT("equip shield"), Heroes->EquipAbility(201));
    TestTrue(TEXT("shield spawns visible blocking geometry"), Heroes->ActivateEquipped());
    TArray<UStaticMeshComponent*> Visuals;
    Character->GetComponents(Visuals);
    TestTrue(TEXT("shield has rendered mesh"), Visuals.Num() > 0);
    TestFalse(TEXT("active shield blocks hero change"), Heroes->SelectHero(2));
    const FGuid Attack = FGuid::NewGuid();
    TestEqual(TEXT("shield absorbs one attack"), Heroes->Absorb(Attack, 40), 40.f);
    TestEqual(TEXT("shield rejects repeated absorption"), Heroes->Absorb(Attack, 40), 0.f);
    TestEqual(TEXT("shield caps absorption at remaining strength"), Heroes->Absorb(FGuid::NewGuid(), 80), 60.f);
    TestEqual(TEXT("depleted shield absorbs nothing"), Heroes->Absorb(FGuid::NewGuid(), 10), 0.f);
    TestEqual(TEXT("negative damage is rejected"), Heroes->Absorb(FGuid::NewGuid(), -10), 0.f);
    TestEqual(TEXT("empty identity is rejected"), Heroes->Absorb(FGuid(), 10), 0.f);
    TestTrue(TEXT("depleted shield permits switching"), Heroes->SelectHero(0));
    Heroes->ResetAbilities();
    Visuals.Reset();
    Character->GetComponents(Visuals);
    TestEqual(TEXT("reset removes construct visuals"), Visuals.Num(), 0);
    TestTrue(TEXT("select H03"), Heroes->SelectHero(2));
    TestEqual(TEXT("six H03 slots"), Heroes->GetAbilities().Num(), 6);
    TestEqual(TEXT("switch equips default blink"), Heroes->EquippedAbility(), 301);
    Heroes->OpenSelector();
    TestTrue(TEXT("selector opens"), Heroes->IsSelectorOpen());
    TestFalse(TEXT("selector blocks powers"), Heroes->ActivateEquipped());
    Heroes->SelectorInput(4, true);
    Heroes->SelectorInput(4, false);
    Heroes->SelectorInput(2, true);
    Heroes->SelectorInput(2, false);
    TestEqual(TEXT("selector commits H01"), Heroes->ActiveHero(), 0);
    TestEqual(TEXT("selector equips matching hero power"), Heroes->EquippedAbility(), 101);
    TestFalse(TEXT("selector closes on commit"), Heroes->IsSelectorOpen());

    auto* Target = World->SpawnActor<AActor>(AActor::StaticClass(), Origin + FVector(180,0,0), FRotator::ZeroRotator, Spawn);
    auto* Body = NewObject<UBoxComponent>(Target);
    Target->SetRootComponent(Body);
    Body->SetBoxExtent(FVector(30));
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Body->SetCollisionResponseToAllChannels(ECR_Block);
    Body->RegisterComponent();
    Target->SetActorLocation(Origin + FVector(180,0,0));
    TestFalse(TEXT("unbound canonical ingress cannot damage"), Heroes->ActivateEquipped());
    int32 Accepted = 0;
    FGuid ReceivedAttack;
    Heroes->CanonicalStrike.BindLambda([&](const FIBS35DamageMessage& Message, const FHitResult& Hit) {
        TestTrue(TEXT("strike preserves hit component"), Hit.GetComponent() == Body && Message.TargetPart == Body);
        TestTrue(TEXT("strike supplies attack identity"), Message.AttackId.IsValid());
        TestTrue(TEXT("strike supplies requesting hero"), Message.InstigatorActor == Character);
        ReceivedAttack = Message.AttackId;
        ++Accepted;
        return true;
    });
    TestTrue(TEXT("bound strike dispatches"), Heroes->ActivateEquipped());
    TestFalse(TEXT("cooldown refuses repeated immediate action"), Heroes->ActivateEquipped());
    TestEqual(TEXT("only one dispatch"), Accepted, 1);
    TestTrue(TEXT("attack GUID captured"), ReceivedAttack.IsValid());
    Heroes->ResetAbilities();
    Target->Destroy();

    TestTrue(TEXT("switch to blink hero"), Heroes->SelectHero(2));
    Character->SetActorLocation(Origin);
    Character->GetCharacterMovement()->Velocity = FVector(0, 120, 30);
    Heroes->bAllowNonPlanetLab = false;
    TestFalse(TEXT("blink rejects missing region authority by default"), Heroes->ActivateEquipped());
    TestTrue(TEXT("rejected blink preserves position"), Character->GetActorLocation().Equals(Origin, .01));
    TestEqual(TEXT("rejected blink preserves energy"), Heroes->Energy(), 100.f);
    Heroes->bAllowNonPlanetLab = true;
    TestTrue(TEXT("explicit flat map blink succeeds"), Heroes->ActivateEquipped());
    TestTrue(TEXT("blink moves thirty metres"), Character->GetActorLocation().Equals(Origin + FVector(3000,0,0), .1));
    TestTrue(TEXT("blink preserves velocity"), Character->GetCharacterMovement()->Velocity.Equals(FVector(0,120,30), .01));
    TestEqual(TEXT("blink spends energy once"), Heroes->Energy(), 80.f);
    TestTrue(TEXT("cooling ability can remain equipped"), Heroes->EquipAbility(301));
    TestFalse(TEXT("cooldown prevents immediate blink replay"), Heroes->ActivateEquipped());
    Heroes->ResetAbilities();
    Character->SetActorLocation(Origin);

    auto MakeBlocker = [&](FVector Location, FVector Extent) {
        auto* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, Spawn);
        auto* Box = NewObject<UBoxComponent>(Actor);
        Actor->SetRootComponent(Box);
        Box->SetBoxExtent(Extent);
        Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Box->SetCollisionObjectType(ECC_WorldStatic);
        Box->SetCollisionResponseToAllChannels(ECR_Block);
        Box->RegisterComponent();
        Actor->SetActorLocation(Location);
        return Actor;
    };
    auto* Wall = MakeBlocker(Origin + FVector(1500,0,0), FVector(2,500,500));
    TestFalse(TEXT("full capsule sweep refuses a four-centimetre wall"), Heroes->ActivateEquipped());
    TestTrue(TEXT("blocked sweep cannot teleport through wall"), Character->GetActorLocation().Equals(Origin, .01));
    TestEqual(TEXT("blocked sweep consumes no energy"), Heroes->Energy(), 100.f);
    Wall->Destroy();
    auto* Occupied = MakeBlocker(Origin + FVector(2750,0,0), FVector(350,150,150));
    TestFalse(TEXT("occupied candidate destinations refuse blink"), Heroes->ActivateEquipped());
    Occupied->Destroy();
    Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    TestFalse(TEXT("grounded blink requires destination support"), Heroes->ActivateEquipped());
    const float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    auto* Ground = MakeBlocker(Origin + FVector(1500,0,-HalfHeight-22), FVector(2000,500,20));
    TestTrue(TEXT("grounded blink accepts support under destination"), Heroes->ActivateEquipped());
    Ground->Destroy();
    Heroes->ResetAbilities();
    Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    Character->SetActorLocation(Origin);
    Heroes->NotifyIncomingHit(Origin + FVector(200,0,0));
    TestTrue(TEXT("equip counter blink"), Heroes->EquipAbility(302));
    TestTrue(TEXT("counter blink succeeds"), Heroes->ActivateEquipped());
    TestTrue(TEXT("counter blink moves away from hit source"), Character->GetActorLocation().Equals(Origin-FVector(2500,0,0), .1));
    Heroes->ResetAbilities();
    Character->SetActorLocation(Origin);
    TestTrue(TEXT("equip reposition"), Heroes->EquipAbility(303));
    TestTrue(TEXT("lateral reposition succeeds"), Heroes->ActivateEquipped());
    TestTrue(TEXT("reposition uses local right with no movement input"), Character->GetActorLocation().Equals(Origin+FVector(0,2500,0), .1));
    Heroes->ResetAbilities();
    Character->Destroy();
    return true;
}
#endif
