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
    // Keep the laboratory's floor bodies, but distinguish streets from planted lots.
    // Road and paving overlays below sit on this floor and add no collision bodies.
    if (Materials[4]) Pavement->SetMaterial(0, Materials[4]);
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
    auto* Streets = Batch(TEXT("StreetSurfaces"), Cube, Materials[0], false, 100000);
    auto* Walks = Batch(TEXT("PedestrianSurfaces"), Cube, Materials[1], false, 80000);
    auto* Details = Batch(TEXT("FacadeTrim"), Cube, Materials[5], false, 55000);
    auto* Infill = Batch(TEXT("InfillBuildings"), Cube, Materials[3], true, 120000);

    auto Box = [&](UHierarchicalInstancedStaticMeshComponent* Component, double X, double Y, double Z, FVector Dimensions)
    {
        const FVector Normal = (Up * Radius + East * X + North * Y).GetSafeNormal();
        const FVector Position = Center + Normal * (Radius + City.SurfaceOffsetCm + Z);
        Component->AddInstance(FTransform(FRotationMatrix::MakeFromZX(Normal, East).ToQuat(), Position, Dimensions / 100.0), true);
    };
    const auto SurfacePatch = [&](UHierarchicalInstancedStaticMeshComponent* Component, double X, double Y, double Z, FVector Size)
    {
        // A single 610 m tangent plane would float almost two metres above the
        // curved ground at its ends. Short strips follow the same radial surface
        // as the existing floor, with centimetre overlap at segment boundaries.
        const int32 CountX=FMath::Max(1,FMath::CeilToInt(Size.X/4000.0));
        const int32 CountY=FMath::Max(1,FMath::CeilToInt(Size.Y/4000.0));
        const double Width=Size.X/CountX, Length=Size.Y/CountY;
        for (int32 I=0;I<CountX;++I) for (int32 J=0;J<CountY;++J)
            Box(Component,X+(I+.5)*Width-Size.X*.5,Y+(J+.5)*Length-Size.Y*.5,Z,
                FVector(Width+2,Length+2,Size.Z));
    };

    // New lots fill the near view without moving any existing building, saved prop,
    // prepared target or threat. These reservations include the entire rotated S4
    // encounter, its approach, cargo rows and the north/south heavy-threat lane.
    const auto Overlaps = [](FVector2D A, FVector2D AH, FVector2D B, FVector2D BH)
    {
        return FMath::Abs(A.X-B.X) < AH.X+BH.X && FMath::Abs(A.Y-B.Y) < AH.Y+BH.Y;
    };
    const auto ClearLot = [&](FVector2D Position, FVector2D Half)
    {
        if (Overlaps(Position, Half, FVector2D(0,3500), FVector2D(4100,4100))
            || Overlaps(Position, Half, FVector2D::ZeroVector, FVector2D(2200,2800))
            || Overlaps(Position, Half, FVector2D::ZeroVector, FVector2D(1000,10000))
            || Overlaps(Position, Half, FVector2D(0,-7000), FVector2D(1600,2000))) return false;
        if (!bPort && !bCoast && !bMountain
            && Overlaps(Position, Half, FVector2D(-7000,7000), FVector2D(2800,2800))) return false;
        for (int32 Index = 0; Index < Buildings->GetInstanceCount(); ++Index)
        {
            FTransform Existing;
            Buildings->GetInstanceTransform(Index, Existing, true);
            const FVector Offset = Existing.GetLocation()-Center;
            const FVector2D ExistingPosition(FVector::DotProduct(Offset,East), FVector::DotProduct(Offset,North));
            const FVector Scale = Existing.GetScale3D();
            if (Overlaps(Position, Half+FVector2D(300,300), ExistingPosition, FVector2D(Scale.X*50,Scale.Y*50))) return false;
        }
        return true;
    };
    FRandomStream Architecture(City.Seed+733);
    for (int32 Side : {-1,1}) for (int32 Row = 0; Row < 4; ++Row)
    {
        const double Y = Row==0 ? -5000.0 : Row==1 ? 3500.0 : Row==2 ? 8000.0 : -12500.0;
        const double X = Side*(Row==3 ? 3100.0 : 5600.0);
        const FVector2D Size(Row==3 ? 2800.0 : Row==2 ? 2400.0 : 2600.0, Row==3 ? 2600.0 : 2800.0);
        if (!ClearLot(FVector2D(X,Y), Size*.5)) continue;
        const int32 Floors = Architecture.RandRange(bPort ? 2 : 3, bPort ? 4 : bCoast ? 5 : 7);
        const double Height = Floors*360.0+100.0;
        Box(Infill, X, Y, 10+Height*.5, FVector(Size.X,Size.Y,Height));
    }

    const auto DressBuilding = [&](const FTransform& Building, int32 Index, bool bOriginal)
    {
        const FVector U = Building.GetUnitAxis(EAxis::Z);
        const FVector X = Building.GetUnitAxis(EAxis::X);
        const FVector Y = Building.GetUnitAxis(EAxis::Y);
        const FVector Half = Building.GetScale3D() * 50.0;
        const FVector Base = Building.GetLocation() - U * Half.Z;
        const int32 Stories = FMath::Clamp(FMath::RoundToInt(Half.Z * 2.0 / 360.0), 2, 20);
        const double GroundHeight = FMath::Min(450.0, Half.Z*2.0/Stories*1.2);
        const double UpperHeight = (Half.Z*2.0-GroundHeight)/(Stories-1);
        const int32 Style = (City.Seed+Index/2)%2;
        const auto Detail = [&](FVector Position, FVector Size)
        {
            Details->AddInstance(FTransform(Building.GetRotation(), Base+X*Position.X+Y*Position.Y+U*Position.Z, Size/100.0), true);
        };
        for (int32 Side = 0; Side < 4; ++Side)
        {
            const FVector Outward = Side == 0 ? X : Side == 1 ? -X : Side == 2 ? Y : -Y;
            const FVector Tangent = FVector::CrossProduct(U, Outward);
            const double Width = Side < 2 ? Half.Y * 2 : Half.X * 2;
            const double Depth = Side < 2 ? Half.X : Half.Y;
            const int32 Bays = FMath::Clamp(FMath::RoundToInt(Width/(Style ? 600.0 : 750.0)), 3, 7);
            const double BayWidth = Width / Bays;
            for (int32 Bay = 0; Bay < Bays; ++Bay)
            {
                const FVector BayBase = Base + Outward * (Depth + 20.0) + Tangent * ((Bay + 0.5) * BayWidth - Width / 2.0);
                for (int32 Story = 0; Story < Stories; ++Story)
                {
                    const int32 Module = Story == 0 && Bay == Bays / 2 ? 2 : Story==0 ? 0 : Style;
                    const double Height = Story==0 ? GroundHeight : UpperHeight;
                    const double Level = Story==0 ? Height*.5 : GroundHeight+(Story-.5)*UpperHeight;
                    Panel(Module, BayBase+U*Level, Tangent, U, BayWidth, Height);
                }
                Panel(3, BayBase+U*GroundHeight, Tangent, U, BayWidth, 18.0);
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
        (bOriginal ? Sidewalk : Walks)->AddInstance(Apron, true);
        // A bounded roof silhouette: parapets and one/two service volumes, using
        // the existing metal/roof materials rather than new source packages.
        for (int32 Sign : {-1,1})
        {
            Detail(FVector(Sign*(Half.X-18),0,Half.Z*2+40), FVector(36,Half.Y*2,80));
            Detail(FVector(0,Sign*(Half.Y-18),Half.Z*2+40), FVector(Half.X*2,36,80));
        }
        Detail(FVector(-Half.X*.25,Half.Y*.2,Half.Z*2+110), FVector(360,260,220));
        if (Index%2==0) Detail(FVector(Half.X*.35,-Half.Y*.2,Half.Z*2+70), FVector(240,330,140));
    };
    for (int32 Index = 0; Index < Buildings->GetInstanceCount(); ++Index)
    {
        FTransform Building;
        Buildings->GetInstanceTransform(Index, Building, true);
        DressBuilding(Building, Index, true);
    }
    for (int32 Index = 0; Index < Infill->GetInstanceCount(); ++Index)
    {
        FTransform Building;
        Infill->GetInstanceTransform(Index, Building, true);
        DressBuilding(Building, Index+Buildings->GetInstanceCount(), false);
    }

    // Readable street widths and continuous footways replace a single asphalt
    // island. The broad center remains a plaza for the existing combat encounter.
    SurfacePatch(Streets, 0, 0, 13, FVector(2400,61000,2));
    SurfacePatch(Streets, 0, 0, 13, FVector(61000,2400,2));
    for (int32 Sign : {-1,1})
    {
        for (double Offset : {8000.0,16500.0,27500.0})
        {
            const double Width = Offset==8000.0 ? 1400.0 : 1800.0;
            SurfacePatch(Streets, Sign*Offset, 0, 13, FVector(Width,61000,2));
            for (int32 Edge : {-1,1})
                SurfacePatch(Walks, Sign*Offset+Edge*(Width*.5+175), 0, 15, FVector(350,61000,4));
            if (Offset!=8000.0)
                SurfacePatch(Streets, 0, Sign*Offset, 13, FVector(61000,Width,2));
        }
        SurfacePatch(Walks, Sign*3950, 2000, 15, FVector(450,16000,4));
        SurfacePatch(Walks, Sign*1450, -12500, 15, FVector(400,4000,4));
    }
    SurfacePatch(Streets, 0, -9000, 13, FVector(18000,1800,2));
    SurfacePatch(Walks, 0, 2500, 15, FVector(8000,11000,4));
    SurfacePatch(Walks, 0, -6000, 15, FVector(7800,450,4));
    for (int32 Mark = -14; Mark <= 14; ++Mark)
    {
        if (FMath::Abs(Mark) >= 3) Box(Markings, Mark*2000.0, 0, 17, FVector(700,16,1));
        if (Mark<=-5 || Mark>=5) Box(Markings, 0, Mark*2000.0, 17, FVector(16,700,1));
        for (int32 Side : {-1,1}) Box(Markings, Side*8000, Mark*2000.0, 17, FVector(14,650,1));
    }
    for (int32 Stripe = -3; Stripe <= 3; ++Stripe)
    {
        Box(Markings, Stripe*230, -7950, 17, FVector(110,360,1));
        for (int32 Side : {-1,1}) Box(Markings, Side*8000+Stripe*150, 1450, 17, FVector(80,320,1));
    }
    Box(Green, -22000, -22000, 8, FVector(9000, 9000, 16));
    Box(Sidewalk, -22000, -22000, 20, FVector(1000, 9000, 20));
    Box(Sidewalk, -22000, -22000, 20, FVector(9000, 1000, 20));
    auto* Trees = Batch(TEXT("ParkTrees"), ProductionMeshes[4].LoadSynchronous(), nullptr, false, 35000);
    if (UStaticMesh* TreeMesh = Trees->GetStaticMesh())
    {
        const FBoxSphereBounds Bounds = TreeMesh->GetBounds();
        FRandomStream Planting(City.Seed+1901);
        const auto Tree = [&](double X, double Y, double Height)
        {
            const double Scale = Height/FMath::Max(1.0, Bounds.BoxExtent.Z*2.0);
            const FVector Normal = (Up*Radius+East*X+North*Y).GetSafeNormal();
            const FQuat Rotation = FQuat(Normal,Planting.FRandRange(-PI,PI))*FRotationMatrix::MakeFromZX(Normal,East).ToQuat();
            const FVector Ground = Center+Normal*(Radius+City.SurfaceOffsetCm+20.0);
            const FVector Offset = Rotation.RotateVector(FVector(Bounds.Origin.X,Bounds.Origin.Y,Bounds.Origin.Z-Bounds.BoxExtent.Z)*Scale);
            Trees->AddInstance(FTransform(Rotation,Ground-Offset,FVector(Scale)),true);
        };
        for (int32 Row = 0; Row < 4; ++Row) for (int32 Column = 0; Column < 4; ++Column)
        {
            Tree(-25200+Row*2100.0+Planting.FRandRange(-300,300),
                -25200+Column*2100.0+Planting.FRandRange(-300,300), Planting.FRandRange(750,1150));
        }
        for (int32 Side : {-1,1})
        {
            for (double Y : {-6000.0,-2200.0,2000.0,6200.0,10000.0})
                Tree(Side*3700.0, Y+Planting.FRandRange(-180,180), Planting.FRandRange(600,850));
            for (double Y : {-15000.0,-10000.0}) Tree(Side*1650.0,Y,Planting.FRandRange(650,850));
            for (double Y : {-20000.0,-10000.0,0.0,10000.0,20000.0})
                Tree(Side*8850.0,Y+Planting.FRandRange(-250,250),Planting.FRandRange(650,950));
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
