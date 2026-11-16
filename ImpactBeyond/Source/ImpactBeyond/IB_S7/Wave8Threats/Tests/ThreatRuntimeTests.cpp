#include "../IB_S7T/IBS7TThreatActor.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "IB_S35/IBS35Object.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS7RuntimeContractTest, "ImpactBeyond.S7.RuntimeContract",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBS7RuntimeContractTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("Game world exists"), World)) return false;
    FActorSpawnParameters Spawn;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Threat = World->SpawnActor<AIBS7TThreatActor>(AIBS7TThreatActor::StaticClass(),
        FVector(900000, 900000, 900000), FRotator::ZeroRotator, Spawn);
    if (!TestNotNull(TEXT("Threat spawns"), Threat)) return false;
    // Only this fixture uses an engine cube; production module assets are supplied by the world author.
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    Threat->bUseSkeletalParts = true;
    TestTrue(TEXT("External module accepts an explicit absent skeletal mapping"),
        Threat->ConfigurePartArt(EIBS7TPart::Hammer, Cube, NAME_None));
    TestTrue(TEXT("External module remains visible alongside a production rig"), Threat->Modules[0]->IsVisible());
    TestFalse(TEXT("Invalid named skeletal mapping cannot silently replace external art"),
        Threat->ConfigurePartArt(EIBS7TPart::Hammer, Cube, TEXT("QA_MissingBone")));
    TestTrue(TEXT("Failed bone mapping preserves external module"), Threat->PartBones[0].IsNone());
    Threat->SetRegionReady(true);
    TestTrue(TEXT("Identity configured"), Threat->ConfigureIdentity(TEXT("QA_S7_Robot"), FGuid::NewGuid(), FVector::ZeroVector));
    UPrimitiveComponent* Part = Threat->GetPartCollider(EIBS7TPart::Hammer);
    FHitResult Hit(Threat, Part, Part->GetComponentLocation(), FVector::UpVector);
    TestTrue(TEXT("Intact part is not a detachable candidate"), Threat->FindExposedPart(Hit) == EIBS7TPart::Body);
    FIBS35DamageMessage Message;
    Message.AttackId = FGuid::NewGuid();
    Message.InstigatorId = FGuid::NewGuid();
    Message.TargetId = Threat->GetTargetLifeId();
    Message.TargetPart = Part;
    Message.HealthDelta = -60;
    Message.DamageKind = EIBS35DamageKind::HeavyPunch;
    Message.Contact = Hit.ImpactPoint;
    TestTrue(TEXT("Canonical part impact accepted"), Threat->AcceptCanonicalImpact(Message, Hit));
    TestFalse(TEXT("Canonical impact cannot repeat"), Threat->AcceptCanonicalImpact(Message, Hit));
    TestEqual(TEXT("Part impact does not also damage body"), Threat->BodyHealth, 500.f);
    TestFalse(TEXT("Legacy direct ingress blocked once canonical owner exists"),
        Threat->ReceiveImpact(Threat->Generation, 999, EIBS7TPart::Body, 100));
    TestTrue(TEXT("Exact exposed hit resolves prepared part"), Threat->FindExposedPart(Hit) == EIBS7TPart::Hammer);
    FHitResult Nearby(Threat,Threat->Body,Part->GetComponentLocation()+FVector(0,0,90),FVector::UpVector);
    TestTrue(TEXT("Nearby body hit resolves exposed joint within tolerance"), Threat->FindExposedPart(Nearby) == EIBS7TPart::Hammer);
    FHitResult Far(Threat,Threat->Body,Part->GetComponentLocation()+FVector(10000,0,0),FVector::UpVector);
    TestNull(TEXT("Distant body hit cannot detach"), Threat->TryDetachAt(Far));
    AIBS35Object* Piece = Threat->TryDetachAt(Nearby);
    TestNull(TEXT("Repeated interaction cannot spawn another loose piece"), Threat->TryDetachAt(Hit));
    if (TestNotNull(TEXT("Loose module uses shared grabbable actor"), Piece))
    {
        TestTrue(TEXT("Loose module simulates physics"), Piece->Body->IsSimulatingPhysics());
        TestTrue(TEXT("Loose module uses loose-part profile"), Piece->Profile.ObjectClass == EIBS35ObjectClass::LoosePart);
        TestTrue(TEXT("Loose external module preserves its configured mesh"), Piece->Appearance->GetStaticMesh() == Cube);
    }
    TestTrue(TEXT("Attached collider removed"), Part->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
    TestFalse(TEXT("Detached external module is hidden on the original body"), Threat->Modules[0]->IsVisible());
    FIBS7TThreatSnapshot Snapshot;
    TestTrue(TEXT("Snapshot captured"), Threat->CaptureSnapshot(Snapshot));
    TestTrue(TEXT("Snapshot contains physical module"), Snapshot.Parts.Num() == 2 && Snapshot.Parts[0].bLoosePiece);
    const FGuid OldLife = Threat->GetTargetLifeId();
    const FHitResult BodyHit(Threat,Threat->Body,Threat->GetActorLocation(),FVector::UpVector);
    const FIBS4Hit OldNative = UIBS4ResponseComponent::MakeHit(World,Threat,EIBS4Weapon::Laser,10,BodyHit);
    TestTrue(TEXT("Snapshot preflight accepts"), Threat->CanRestoreSnapshot(Snapshot));
    TestTrue(TEXT("Preflight never renews live identity"), Threat->GetTargetLifeId() == OldLife);
    TestTrue(TEXT("Preflight keeps existing physical piece"), Threat->GetLoosePiece(EIBS7TPart::Hammer) == Piece);
    TestTrue(TEXT("Snapshot restored"), Threat->RestoreSnapshot(Snapshot));
    TestTrue(TEXT("Restore renews target life"), Threat->GetTargetLifeId() != OldLife);
    TestFalse(TEXT("Old contact rejected after hydration"), Threat->AcceptCanonicalImpact(Message, Hit));
    TestFalse(TEXT("Old native contact rejected even if host supplies renewed life"), Threat->AcceptCanonicalS4Impact(OldNative,Threat->GetTargetLifeId()));
    const FIBS4Hit FreshNative = UIBS4ResponseComponent::MakeHit(World,Threat,EIBS4Weapon::Laser,10,BodyHit);
    TestTrue(TEXT("New native contact accepted in restored life"), Threat->AcceptCanonicalS4Impact(FreshNative,Threat->GetTargetLifeId()));
    auto Invalid = Snapshot;
    Invalid.Health = -1;
    TestFalse(TEXT("Invalid snapshot fails preflight"), Threat->CanRestoreSnapshot(Invalid));
    TestFalse(TEXT("Invalid snapshot rejected"), Threat->RestoreSnapshot(Invalid));
    TestEqual(TEXT("Invalid restore preserves health"), Threat->BodyHealth, 490.f);
    TestTrue(TEXT("Threat can reset after external module detachment"), Threat->ResetThreat());
    TestTrue(TEXT("Reset restores the attached external module"), Threat->Modules[0]->IsVisible());
    Threat->Destroy();
    return true;
}
#endif
