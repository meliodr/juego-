#include "IBS3World.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "IBS3Geometry.h"
#include "CollisionShape.h"
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
        UHierarchicalInstancedStaticMeshComponent* Infill = nullptr;
        for (auto* Batch : Batches)
        {
            if (Batch->GetName().StartsWith(TEXT("FacadeModule")))
            {
                TestNotNull(TEXT("Imported facade mesh resolves"), Batch->GetStaticMesh().Get());
                FacadeCount += Batch->GetInstanceCount();
                TestEqual(TEXT("Facades use the existing building collision"), Batch->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
            }
            if (Batch->GetName().StartsWith(TEXT("ParkTrees"))) TreeCount = Batch->GetInstanceCount();
            if (Batch->GetName().StartsWith(TEXT("InfillBuildings"))) Infill = Batch;
            if (Batch->GetName().StartsWith(TEXT("StreetSurfaces")))
            {
                TestEqual(TEXT("Road dressing retains the existing floor collision"),Batch->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
                for (int32 Index=0;Index<Batch->GetInstanceCount();++Index)
                {
                    FTransform Strip;
                    Batch->GetInstanceTransform(Index,Strip,true);
                    for (int32 X : {-1,1}) for (int32 Y : {-1,1})
                    {
                        const FVector Corner=Strip.TransformPosition(FVector(X*50,Y*50,50));
                        const double Height=FVector::Dist(Corner,Center)-Manifest->RadiusCm-City.SurfaceOffsetCm;
                        TestTrue(TEXT("Street corners follow the curved ground instead of floating"),Height>=0 && Height<30);
                    }
                }
            }
        }
        TestTrue(TEXT("Facade instances bounded"), FacadeCount > 100 && FacadeCount < 12000);
        TestTrue(TEXT("Park and street trees stay within the per-hub budget"), TreeCount > 16 && TreeCount <= 48);
        if (TestNotNull(TEXT("Near-view buildings exist"), Infill))
        {
            TestTrue(TEXT("Near-view building body budget"), Infill->GetInstanceCount() >= 6 && Infill->GetInstanceCount() <= 8);
            for (int32 Index = 0; Index < Infill->GetInstanceCount(); ++Index)
            {
                FTransform Building;
                Infill->GetInstanceTransform(Index, Building, true);
                const FVector Across = Building.GetUnitAxis(EAxis::X)*(Building.GetScale3D().X*50+250);
                FHitResult Hit;
                TestTrue(TEXT("Added building has a real blocking body"), World->LineTraceSingleByChannel(Hit,
                    Building.GetLocation()+Across, Building.GetLocation()-Across, ECC_Visibility));
                TestTrue(TEXT("Added building's visible footprint owns its collision"), Hit.GetComponent()==Infill);
            }
        }
        // Exercise actual scene collision at the gameplay positions, independently
        // of the lot-placement predicate. This catches a facade/collision edit that
        // obstructs the spawn, cargo rows, encounter or either heavy-threat lane.
        const IBS3::Frame Frame = IBS3::frame({FMath::DegreesToRadians(City.LatitudeDegrees),FMath::DegreesToRadians(City.LongitudeDegrees)});
        const FVector Up(Frame.up.x,Frame.up.y,Frame.up.z), East(Frame.east.x,Frame.east.y,Frame.east.z), North(Frame.north.x,Frame.north.y,Frame.north.z);
        const auto Surface = [&](double X, double Y)
        {
            const FVector Normal=(Up*Manifest->RadiusCm+East*X+North*Y).GetSafeNormal();
            return Center+Normal*(Manifest->RadiusCm+City.SurfaceOffsetCm+180);
        };
        const FQuat CapsuleRotation=FQuat::FindBetweenNormals(FVector::UpVector,Up);
        const auto ClearRoute = [&](FVector A, FVector B, const TCHAR* Label)
        {
            TestFalse(Label,World->SweepTestByChannel(A,B,CapsuleRotation,ECC_Pawn,FCollisionShape::MakeCapsule(42,96)));
        };
        ClearRoute(Surface(0,-10000),Surface(0,10000),TEXT("North/south flight and heavy-threat corridor stays clear"));
        ClearRoute(Surface(-7000,0),Surface(7000,0),TEXT("Spawn cross-street stays clear"));
        for (double X : {-1450.0,1450.0})
            ClearRoute(Surface(X,-14000),Surface(X,-11000),TEXT("Approach sidewalks stay outside the infill building footprints"));
        for (double Y : {-1800.0,1800.0,3500.0,6000.0})
            ClearRoute(Surface(-3500,Y),Surface(3500,Y),TEXT("Cargo and encounter approaches stay clear"));
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
