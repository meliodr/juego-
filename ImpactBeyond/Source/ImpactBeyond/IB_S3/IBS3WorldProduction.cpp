#include "IBS3World.h"
#include "IBS3Geometry.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"

void AIBS3Hub::BuildProduction(const FIBS3City& City, const FVector& Center, double Radius)
{
    const IBS3::Frame Frame = IBS3::frame({FMath::DegreesToRadians(City.LatitudeDegrees), FMath::DegreesToRadians(City.LongitudeDegrees)});
    const FVector Up(Frame.up.x, Frame.up.y, Frame.up.z);
    const FVector East(Frame.east.x, Frame.east.y, Frame.east.z);
    const FVector North(Frame.north.x, Frame.north.y, Frame.north.z);
    const bool bPort = City.CityId == TEXT("CITY_PORT");
    const bool bCoast = City.CityId == TEXT("CITY_COAST");
    const bool bMountain = City.CityId == TEXT("CITY_MOUNTAIN");
    TArray<UMaterialInterface*> Materials;
    for (auto& Material : ProductionMaterials) Materials.Add(Material.LoadSynchronous());
    if (Materials[0]) Pavement->SetMaterial(0, Materials[0]);
    if (Materials[1]) LandingPad->SetMaterial(0, Materials[1]);
    if (Materials[3]) Buildings->SetMaterial(0, Materials[3]);

    auto Batch = [&](const TCHAR* Name, UStaticMesh* Mesh, UMaterialInterface* Material, bool bCollision, int32 CullDistance)
    {
        const FName ComponentName = MakeUniqueObjectName(this, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(Name));
        UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, ComponentName);
        AddInstanceComponent(Component);
        Component->SetupAttachment(RootComponent);
        Component->SetMobility(EComponentMobility::Movable);
        Component->SetStaticMesh(Mesh);
        Component->SetCollisionProfileName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
        Component->SetCanEverAffectNavigation(false);
        Component->SetCullDistances(CullDistance * 3 / 4, CullDistance);
        if (Material) Component->SetMaterial(0, Material);
        Component->RegisterComponent();
        ProductionBatches.Add(Component);
        return Component;
    };
    TArray<UHierarchicalInstancedStaticMeshComponent*> Facades;
    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FString Name = FString::Printf(TEXT("FacadeModule%d"), Index);
        Facades.Add(Batch(*Name, ProductionMeshes[Index].LoadSynchronous(), nullptr, false, 100000));
    }

    // Same bounds-aware module placement as Apply-RealisticFacades.py, in each radial frame.
    auto Panel = [&](int32 Module, FVector TargetCenter, FVector Tangent, FVector Normal, double Width, double Height)
    {
        UHierarchicalInstancedStaticMeshComponent* Component = Facades[Module];
        if (!Component->GetStaticMesh()) return;
        const FBoxSphereBounds Bounds = Component->GetStaticMesh()->GetBounds();
        const bool bWidthX = Bounds.BoxExtent.X > Bounds.BoxExtent.Y;
        const double HalfWidth = bWidthX ? Bounds.BoxExtent.X : Bounds.BoxExtent.Y;
        if (HalfWidth <= UE_SMALL_NUMBER || Bounds.BoxExtent.Z <= UE_SMALL_NUMBER) return;
        const FVector Scale = bWidthX ? FVector(Width / (2 * HalfWidth), 1, Height / (2 * Bounds.BoxExtent.Z))
            : FVector(1, Width / (2 * HalfWidth), Height / (2 * Bounds.BoxExtent.Z));
        const FVector NativeX = bWidthX ? Tangent : FVector::CrossProduct(Tangent, Normal);
        const FQuat Rotation = FRotationMatrix::MakeFromXZ(NativeX, Normal).ToQuat();
        const FVector Pivot = Rotation.RotateVector(Bounds.Origin * Scale);
        Component->AddInstance(FTransform(Rotation, TargetCenter - Pivot, Scale), true);
    };
    UStaticMesh* Cube = Buildings->GetStaticMesh();
    auto* Sidewalk = Batch(TEXT("PavedSidewalks"), Cube, Materials[1], true, 120000);
    auto* Roofs = Batch(TEXT("RoofDetails"), Cube, Materials[2], false, 65000);
    auto* Green = Batch(TEXT("ParkGround"), Cube, Materials[4], true, 100000);
    auto* Metal = Batch(TEXT("CityLandmarks"), Cube, Materials[5], true, 120000);
    auto* Markings = Batch(TEXT("RoadMarkings"), Cube, Materials[6], false, 40000);
    auto* Water = Batch(TEXT("HarbourWater"), Cube, Materials[7], false, 160000);

    auto Box = [&](UHierarchicalInstancedStaticMeshComponent* Component, double X, double Y, double Z, FVector Dimensions)
    {
        const FVector Normal = (Up * Radius + East * X + North * Y).GetSafeNormal();
        const FVector Position = Center + Normal * (Radius + City.SurfaceOffsetCm + Z);
        Component->AddInstance(FTransform(FRotationMatrix::MakeFromZX(Normal, East).ToQuat(), Position, Dimensions / 100.0), true);
    };

    for (int32 Index = 0; Index < Buildings->GetInstanceCount(); ++Index)
    {
        FTransform Building;
        Buildings->GetInstanceTransform(Index, Building, true);
        const FVector U = Building.GetUnitAxis(EAxis::Z);
        const FVector X = Building.GetUnitAxis(EAxis::X);
        const FVector Y = Building.GetUnitAxis(EAxis::Y);
        const FVector Half = Building.GetScale3D() * 50.0;
        const FVector Base = Building.GetLocation() - U * Half.Z;
        const int32 Stories = FMath::Clamp(FMath::RoundToInt(Half.Z * 2.0 / 360.0), 2, 20);
        const double StoryHeight = Half.Z * 2.0 / Stories;
        const int32 Bays = 6;
        for (int32 Side = 0; Side < 4; ++Side)
        {
            const FVector Outward = Side == 0 ? X : Side == 1 ? -X : Side == 2 ? Y : -Y;
            const FVector Tangent = FVector::CrossProduct(U, Outward);
            const double Width = Side < 2 ? Half.Y * 2 : Half.X * 2;
            const double Depth = Side < 2 ? Half.X : Half.Y;
            const double BayWidth = Width / Bays;
            for (int32 Bay = 0; Bay < Bays; ++Bay)
            {
                const FVector BayBase = Base + Outward * (Depth + 20.0) + Tangent * ((Bay + 0.5) * BayWidth - Width / 2.0);
                for (int32 Story = 0; Story < Stories; ++Story)
                {
                    const int32 Module = Story == 0 && Bay == Bays / 2 ? 2 : (Index + Bay) % 2;
                    Panel(Module, BayBase + U * ((Story + 0.5) * StoryHeight), Tangent, U, BayWidth, StoryHeight);
                }
                Panel(3, BayBase + U * (Half.Z * 2.0 - 10.0), Tangent, U, BayWidth, 28.0);
            }
        }
        FTransform Roof = Building;
        Roof.SetTranslation(Base + U * (Half.Z * 2.0 + 10.0));
        Roof.SetScale3D(FVector(Half.X / 50.0, Half.Y / 50.0, 0.25));
        Roofs->AddInstance(Roof, true);
        FTransform Apron = Building;
        Apron.SetTranslation(Base + U * 6.0);
        Apron.SetScale3D(FVector((Half.X + 350.0) / 50.0, (Half.Y + 350.0) / 50.0, 0.12));
        Sidewalk->AddInstance(Apron, true);
    }

    // Open central avenues keep the landing zone and all QA fixtures accessible.
    for (int32 Mark = -14; Mark <= 14; ++Mark)
    {
        if (FMath::Abs(Mark) < 2) continue;
        Box(Markings, Mark * 2000.0, 0, 12, FVector(700, 24, 2));
        Box(Markings, 0, Mark * 2000.0, 12, FVector(24, 700, 2));
    }
    Box(Green, -22000, -22000, 8, FVector(9000, 9000, 16));
    Box(Sidewalk, -22000, -22000, 20, FVector(1000, 9000, 20));
    Box(Sidewalk, -22000, -22000, 20, FVector(9000, 1000, 20));
    auto* Trees = Batch(TEXT("ParkTrees"), ProductionMeshes[4].LoadSynchronous(), nullptr, false, 35000);
    if (UStaticMesh* TreeMesh = Trees->GetStaticMesh())
    {
        const FBoxSphereBounds Bounds = TreeMesh->GetBounds();
        const double Scale = 1000.0 / FMath::Max(1.0, Bounds.BoxExtent.Z * 2.0);
        for (int32 Row = 0; Row < 4; ++Row) for (int32 Column = 0; Column < 4; ++Column)
        {
            const double X = -25200 + Row * 2100.0;
            const double Y = -25200 + Column * 2100.0;
            const FVector Normal = (Up * Radius + East * X + North * Y).GetSafeNormal();
            const FQuat Rotation = FRotationMatrix::MakeFromZX(Normal, East).ToQuat();
            const FVector Ground = Center + Normal * (Radius + City.SurfaceOffsetCm + 20.0);
            const FVector Offset = Rotation.RotateVector(FVector(Bounds.Origin.X, Bounds.Origin.Y, Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale);
            Trees->AddInstance(FTransform(Rotation, Ground - Offset, FVector(Scale)), true);
        }
    }

    if (bPort)
    {
        // Three accessible quays and two gantry cranes identify the industrial hub.
        for (int32 Pier = -1; Pier <= 1; ++Pier)
            Box(Sidewalk, Pier * 10000, 25000, 25, FVector(3800, 14000, 50));
        for (int32 Crane = 0; Crane < 2; ++Crane)
        {
            const double X = -11000 + Crane * 22000.0;
            Box(Metal, X, 22000, 2200, FVector(180, 180, 4400));
            Box(Metal, X + 2800, 22000, 4300, FVector(5800, 180, 180));
            Box(Metal, X + 5200, 22000, 3400, FVector(50, 50, 1800));
        }
    }
    else if (bCoast)
    {
        Box(Sidewalk, 0, 25000, 25, FVector(58000, 3000, 50));
        for (int32 Bench = -4; Bench <= 4; ++Bench)
        {
            Box(Roofs, Bench * 5500, 25500, 65, FVector(240, 80, 30));
            Box(Roofs, Bench * 5500, 25545, 110, FVector(240, 20, 90));
        }
    }
    else if (bMountain)
    {
        Box(Sidewalk, 24000, 0, 1800, FVector(3000, 50000, 90));
        for (int32 Pillar = -2; Pillar <= 2; ++Pillar)
            Box(Metal, 24000, Pillar * 10000, 900, FVector(500, 500, 1800));
        Box(Sidewalk, 24000, 23000, 1850, FVector(6500, 5000, 100));
    }
    else
    {
        Box(Sidewalk, -7000, 7000, 25, FVector(5000, 5000, 50));
        Box(Metal, -7000, 7000, 1000, FVector(350, 350, 2000));
        Box(Metal, -7000, 7000, 2100, FVector(900, 900, 200));
    }
    if (bPort || bCoast)
    {
        for (int32 X = -8; X <= 8; ++X) for (int32 Y = 7; Y <= 14; ++Y)
            Box(Water, X * 5000.0, Y * 5000.0, 12.0, FVector(5100, 5100, 10));
    }
}
