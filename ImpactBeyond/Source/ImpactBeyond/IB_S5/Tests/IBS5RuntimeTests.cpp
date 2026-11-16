#include "IB_S5/IBS5WorldLife.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS5WorldLifeTest, "ImpactBeyond.S5.LifeAndPersistence",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBS5WorldLifeTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("Game world"), World)) return false;

    const FVector Origin(1000000, 1000000, 1000000);
    AActor* Floor = World->SpawnActor<AActor>();
    UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
    Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(3000, 3000, 10));
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionProfileName(TEXT("BlockAll"));
    Box->RegisterComponent();
    Floor->SetActorLocation(Origin - FVector(0, 0, 10));

    ACharacter* Helper = World->SpawnActor<ACharacter>(Origin + FVector(0, 0, 100), FRotator::ZeroRotator);
    AIBS5WorldLife* Life = World->SpawnActorDeferred<AIBS5WorldLife>(AIBS5WorldLife::StaticClass(), FTransform(Origin));
    if (!TestNotNull(TEXT("Life actor"), Life) || !TestNotNull(TEXT("Helper pawn"), Helper))
    {
        if (Life) Life->Destroy();
        if (Helper) Helper->Destroy();
        Floor->Destroy();
        return false;
    }
    Life->bEnablePersistence = false;
    Life->FinishSpawning(FTransform(Origin));
    Life->SetObserver(Helper);
    Life->SetRegionContext(TEXT("QA_S5"), Origin, FVector::UpVector, false);
    TestEqual(TEXT("Unready region has no citizens"), Life->GetCitizenCount(), 0);
    Life->SetRegionContext(TEXT("QA_S5"), Origin, FVector::UpVector, true);
    TestTrue(TEXT("Ready region has 3-6 real capsule citizens"), Life->GetCitizenCount() >= 3 && Life->GetCitizenCount() <= 6);
    const int32 Count = Life->GetCitizenCount();
    Life->SetRegionContext(TEXT("QA_S5"), Origin, FVector::UpVector, true);
    TestEqual(TEXT("Repeated context creates no duplicate"), Life->GetCitizenCount(), Count);
    Life->SetSolarHour(12.0);
    Life->SetClockPaused(true);
    Life->Tick(46.0f);
    TestEqual(TEXT("Clock pause"), Life->GetLocalSolarHour(), 12.0);
    Helper->SetActorLocation(Life->GetIncidentLocation());
    TestTrue(TEXT("Pre-existing optional incident resolves by explicit help"), Life->TryResolveIncident(Helper));
    TestFalse(TEXT("Incident cannot award twice"), Life->TryResolveIncident(Helper));
    const double EarnedReputation = Life->Reputation;
    const FGuid EnemyCause = FGuid::NewGuid();
    TestTrue(TEXT("Enemy disturbance accepted"), Life->ReportDisturbance(Origin, false, 3.0f, EnemyCause));
    Life->Tick(3.0f);
    TestEqual(TEXT("Enemy cause does not penalize player"), Life->Reputation, EarnedReputation);
    const FGuid PlayerCause = FGuid::NewGuid();
    TestTrue(TEXT("Player disturbance accepted"), Life->ReportDisturbance(Origin, true, 3.0f, PlayerCause));
    TestFalse(TEXT("Fragments of same cause deduplicated"), Life->ReportDisturbance(Origin, true, 3.0f, PlayerCause));
    TestEqual(TEXT("Witness report has delay"), Life->Reputation, EarnedReputation);
    Life->Tick(3.0f);
    TestEqual(TEXT("One delayed penalty"), Life->Reputation, EarnedReputation - 6.0);

    Life->bEnablePersistence = true;
    Life->SaveSlot = TEXT("IB_S5_QA_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    Life->SetSolarHour(22.5);
    TestTrue(TEXT("Save world"), Life->SaveWorld());
    Life->SetSolarHour(2.0);
    TestTrue(TEXT("Load world"), Life->LoadWorld());
    TestEqual(TEXT("Clock restored"), Life->GetLocalSolarHour(), 22.5);
    TestFalse(TEXT("Cause remains retired after load"), Life->ReportDisturbance(Origin, true, 3.0f, PlayerCause));
    Life->SetRegionContext(TEXT("QA_S5_OTHER"), Origin, FVector::UpVector, false);
    TestEqual(TEXT("Leaving ready region releases civilians"), Life->GetCitizenCount(), 0);
    Life->SetRegionContext(TEXT("QA_S5"), Origin, FVector::UpVector, true);
    TestEqual(TEXT("Reentry has same bounded population"), Life->GetCitizenCount(), Count);
    Life->bEnablePersistence = false;
    const FString TestSlot = Life->SaveSlot;
    Life->Destroy();
    Helper->Destroy();
    Floor->Destroy();
    TestTrue(TEXT("Remove test-owned slot"), UGameplayStatics::DeleteGameInSlot(TestSlot, 0));
    return true;
}
#endif
