#include "IBS3World.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3ProductionCityTest, "ImpactBeyond.S3.ProductionCities",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBS3ProductionCityTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("Game world"), World)) return false;
    UIBS3WorldManifest* Manifest = NewObject<UIBS3WorldManifest>();
    const FVector Center(100000000, 100000000, 100000000);
    for (const FIBS3City& City : Manifest->Cities)
    {
        AIBS3Hub* Hub = World->SpawnActor<AIBS3Hub>();
        if (!TestNotNull(TEXT("City spawned"), Hub)) return false;
        Hub->bProductionVisuals = true;
        TestTrue(City.DisplayName() + TEXT(" collision ready"), Hub->Build(City, Center, Manifest->RadiusCm));
        TInlineComponentArray<UHierarchicalInstancedStaticMeshComponent*> Batches;
        Hub->GetComponents(Batches);
        int32 FacadeCount = 0;
        int32 TreeCount = 0;
        for (const auto* Batch : Batches)
        {
            if (Batch->GetName().StartsWith(TEXT("FacadeModule")))
            {
                TestNotNull(TEXT("Imported facade mesh resolves"), Batch->GetStaticMesh().Get());
                FacadeCount += Batch->GetInstanceCount();
                TestEqual(TEXT("Facades use the existing building collision"), Batch->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
            }
            if (Batch->GetName().StartsWith(TEXT("ParkTrees"))) TreeCount = Batch->GetInstanceCount();
        }
        TestTrue(TEXT("Facade instances bounded"), FacadeCount > 100 && FacadeCount < 10000);
        TestEqual(TEXT("Trees bounded per hub"), TreeCount, 16);
        TestTrue(TEXT("Display name excludes internal ID"), !City.DisplayName().StartsWith(TEXT("CITY_")));
        const int32 BatchCount = Batches.Num();
        TestTrue(TEXT("Rebuilding collision succeeds"), Hub->Build(City, Center, Manifest->RadiusCm));
        Batches.Reset();
        Hub->GetComponents(Batches);
        TestEqual(TEXT("Rebuild does not duplicate components"), Batches.Num(), BatchCount);
        Hub->Destroy();
    }
    return true;
}
#endif
