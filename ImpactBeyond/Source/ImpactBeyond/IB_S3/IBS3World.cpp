#include "IBS3World.h"
#include "IBS3Geometry.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
IBS3::V3 WorldVector(FVector P) { return {P.X,P.Y,P.Z}; }
FVector WorldUE(IBS3::V3 P) { return {P.x,P.y,P.z}; }
FTransform Surface(FVector Center,double Radius,FVector Normal,FVector Forward,double Elevation=0)
{
    return FTransform(FRotationMatrix::MakeFromZX(Normal,Forward).ToQuat(),Center+Normal*(Radius+Elevation));
}
double SafetyRadius(const FIBS3City& C)
{
    // The square street grid's corners extend beyond the default geographic city radius.
    return FMath::Max(C.FootprintRadiusCm,std::hypot(50000.0,4500.0*C.BuildingHeightScale));
}
bool ValidDelta(const FIBS3ObjectDelta& D,const FIBS3ObjectDelta& Expected)
{
    return D.Key==Expected.Key && D.OriginRegionId==Expected.OriginRegionId && D.Kind==Expected.Kind && !D.CurrentRegionId.IsNone() &&
        IBS3::validSavedPose(WorldVector(D.GlobalTransform.GetLocation()),WorldVector(D.GlobalTransform.GetScale3D()),
            D.GlobalTransform.GetRotation().SizeSquared(),WorldVector(D.LinearVelocity),WorldVector(D.AngularVelocity));
}
}

AIBS3Prop::AIBS3Prop()
{
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicalObject")); SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Cube.Object); Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetCollisionProfileName(TEXT("PhysicsActor")); Mesh->SetEnableGravity(false);
    Mesh->SetUseCCD(true); Mesh->SetLinearDamping(0.4f); Mesh->SetAngularDamping(0.5f);
}
void AIBS3Prop::Restore(const FIBS3ObjectDelta& Delta,const FVector& WorldOrigin)
{
    State=Delta; FTransform Local=Delta.GlobalTransform; Local.AddToTranslation(-WorldOrigin);
    SetActorTransform(Local,false,nullptr,ETeleportType::TeleportPhysics); // Streaming hydration only, never player motion.
    const bool Wall=State.Kind==TEXT("wall");
    Mesh->SetHiddenInGame(State.bBroken); Mesh->SetCollisionEnabled(State.bBroken?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);
    Mesh->SetSimulatePhysics(!Wall && !State.bBroken);
    if(!Wall && !State.bBroken)
    {
        Mesh->SetMassOverrideInKg(NAME_None,State.Kind==TEXT("car")?1200.f:80.f,true);
        Mesh->SetPhysicsLinearVelocity(State.LinearVelocity); Mesh->SetPhysicsAngularVelocityInRadians(State.AngularVelocity);
    }
}
FIBS3ObjectDelta AIBS3Prop::Capture(const FVector& WorldOrigin) const
{
    FIBS3ObjectDelta D=State; D.GlobalTransform=GetActorTransform(); D.GlobalTransform.AddToTranslation(WorldOrigin);
    D.LinearVelocity=Mesh->GetPhysicsLinearVelocity(); D.AngularVelocity=Mesh->GetPhysicsAngularVelocityInRadians(); return D;
}
void AIBS3Prop::Interact(FVector Direction)
{
    if(State.Kind==TEXT("wall"))
    {
        State.bBroken=true; Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); Mesh->SetHiddenInGame(true);
    }
    else Mesh->AddImpulse(Direction.GetSafeNormal()*700.f,NAME_None,true);
}

AIBS3Hub::AIBS3Hub()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("HubRoot")));
    Pavement=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CurvedStreets")); Pavement->SetupAttachment(RootComponent);
    Buildings=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TechnicalBlocks")); Buildings->SetupAttachment(RootComponent);
    LandingPad=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LandingPad")); LandingPad->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for(UStaticMeshComponent* M : {static_cast<UStaticMeshComponent*>(Pavement),static_cast<UStaticMeshComponent*>(Buildings),LandingPad.Get()})
    {
        M->SetStaticMesh(Cube.Object); M->SetMobility(EComponentMobility::Movable); M->SetCollisionProfileName(TEXT("BlockAll"));
    }
}
bool AIBS3Hub::Build(const FIBS3City& City,const FVector& Center,double Radius)
{
    const IBS3::Frame F=IBS3::frame({FMath::DegreesToRadians(City.LatitudeDegrees),FMath::DegreesToRadians(City.LongitudeDegrees)});
    const FVector U=WorldUE(F.up),E=WorldUE(F.east),N=WorldUE(F.north);
    // Local floor tiles follow radial normals; crossing a tile never moves/rebases the player.
    for(int32 X=-6;X<=6;++X) for(int32 Y=-6;Y<=6;++Y)
    {
        const FVector Normal=(U*Radius+E*(X*5000.0)+N*(Y*5000.0)).GetSafeNormal();
        FTransform T=Surface(Center,Radius,Normal,E,City.SurfaceOffsetCm-40);
        T.SetScale3D(FVector(50.4,50.4,1)); Pavement->AddInstance(T,true);
    }
    FRandomStream Random(City.Seed);
    // Leave the east-west street and north-south street open, including a central landing area.
    for(int32 X=-2;X<=2;++X) for(int32 Y=-2;Y<=2;++Y)
    {
        if(X==0 || Y==0) continue;
        const FVector Normal=(U*Radius+E*(X*11000.0)+N*(Y*11000.0)).GetSafeNormal();
        const double Height=Random.FRandRange(1200,4500)*City.BuildingHeightScale;
        FTransform T=Surface(Center,Radius,Normal,E,City.SurfaceOffsetCm+Height/2);
        T.SetScale3D(FVector(40,40,Height/100)); Buildings->AddInstance(T,true);
    }
    FTransform Pad=Surface(Center,Radius,U,E,City.SurfaceOffsetCm+25); Pad.SetScale3D(FVector(60,60,0.5));
    LandingPad->SetWorldTransform(Pad);
    return RefreshCollisionReady();
}
bool AIBS3Hub::RefreshCollisionReady()
{
    bCollisionReady=false;
    const FBodyInstance* PadBody=LandingPad->GetBodyInstance();
    if(!PadBody || !PadBody->IsValidBodyInstance()) return false;
    for(const UInstancedStaticMeshComponent* Instances:{Pavement.Get(),Buildings.Get()})
    {
        if(Instances->GetInstanceCount()==0 || !Instances->IsPhysicsStateCreated()) return false;
        for(int32 I=0;I<Instances->GetInstanceCount();++I)
        {
            const FBodyInstance* Body=Instances->GetBodyInstance(NAME_None,false,I);
            if(!Body || !Body->IsValidBodyInstance()) return false;
        }
    }
    bCollisionReady=true;
    return true;
}

AIBS3PlanetWorld::AIBS3PlanetWorld()
{
    PrimaryActorTick.bCanEverTick=true; PrimaryActorTick.TickGroup=TG_PrePhysics;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("BodyCenter")));
    GroundCollision=CreateDefaultSubobject<USphereComponent>(TEXT("AnalyticGround")); GroundCollision->SetupAttachment(RootComponent);
    GroundCollision->SetCollisionProfileName(TEXT("BlockAll")); GroundCollision->SetMobility(EComponentMobility::Movable);
    GroundCollision->SetGenerateOverlapEvents(false);
    PlanetProxy=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetVisualLOD")); PlanetProxy->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    PlanetProxy->SetStaticMesh(Sphere.Object); PlanetProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NearSurface=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NearSurfaceVisual")); NearSurface->SetupAttachment(RootComponent);
    NearSurface->SetMobility(EComponentMobility::Movable); NearSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Atmosphere=CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("PlanetAtmosphere")); Atmosphere->SetupAttachment(RootComponent);
}
FVector AIBS3PlanetWorld::OriginOffset() const { return FVector(GetWorld()->OriginLocation); }
FVector AIBS3PlanetWorld::CityPosition(const FIBS3City& C) const
{
    return WorldUE(IBS3::position({FMath::DegreesToRadians(C.LatitudeDegrees),FMath::DegreesToRadians(C.LongitudeDegrees),C.SurfaceOffsetCm},Manifest->RadiusCm,WorldVector(GetActorLocation())));
}
void AIBS3PlanetWorld::BeginPlay()
{
    Super::BeginPlay(); if(!Manifest) Manifest=NewObject<UIBS3WorldManifest>(this);
    bValid=Manifest->Validate(Status);
    if(GetActorTransform().ContainsNaN() || !GetActorScale3D().Equals(FVector::OneVector) || !GetActorQuat().Equals(FQuat::Identity))
    { bValid=false; Status=TEXT("Planet actor supports translation only: configure radius in the manifest"); }
    if(!bValid) { UE_LOG(LogTemp,Error,TEXT("IB_S3: %s"),*Status); return; }
    GroundCollision->SetSphereRadius(Manifest->RadiusCm);
    PlanetProxy->SetWorldScale3D(FVector(Manifest->RadiusCm/50.0));
    NearSurface->SetWorldScale3D(FVector(Manifest->RadiusCm/50.0));
    Atmosphere->TransformMode=ESkyAtmosphereTransformMode::PlanetCenterAtComponentTransform;
    Atmosphere->MarkRenderStateDirty();
    Atmosphere->SetBottomRadius(Manifest->RadiusCm/100000.0);
#if WITH_DEV_AUTOMATION_TESTS
    // Explicit process-restart QA uses a separate GUID-named slot, never the player's save.
    FString QASlot; FGuid QAGuid;
    if(FParse::Value(FCommandLine::Get(),TEXT("IBS3QASaveSlot="),QASlot) &&
        QASlot.StartsWith(TEXT("IB_S3_QA_Restart_")) && QASlot.Len()==49 &&
        FGuid::ParseExact(QASlot.Mid(17),EGuidFormats::Digits,QAGuid)) SaveSlot=QASlot;
#endif
    SeedObjects(); LoadState();
    for(const FIBS3City& C:Manifest->Cities)
    {
        auto* Proxy=NewObject<UStaticMeshComponent>(this); AddInstanceComponent(Proxy); Proxy->SetupAttachment(RootComponent);
        Proxy->SetStaticMesh(PlanetProxy->GetStaticMesh()); Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision); Proxy->RegisterComponent();
        Proxy->SetWorldLocation(CityPosition(C)); Proxy->SetWorldScale3D(FVector(70)); CityProxies.Add(Proxy);
    }
    if(!Manifest->Cities.ContainsByPredicate([this](const FIBS3City& C){return C.CityId==DestinationCity;})) DestinationCity=Manifest->Cities[0].CityId;
    UpdateResidency(CityPosition(Manifest->Cities[0]),0);
}
double AIBS3PlanetWorld::P95Seconds() const
{
    std::vector<double> Samples; for(double S:LoadSamples) Samples.push_back(S);
    // Keep the conservative floor until enough cold/warm samples exist. No fabricated measured p95.
    return FMath::Max(Manifest->InitialLoadP95Seconds,IBS3::percentile95(Samples,Manifest->InitialLoadP95Seconds));
}
bool AIBS3PlanetWorld::IsReady(FName City) const
{
    const auto* Hub=Hubs.Find(City); return Hub && IsValid(*Hub) && !(*Hub)->IsActorBeingDestroyed() && (*Hub)->bCollisionReady;
}
bool AIBS3PlanetWorld::IsOperational() const
{
    const FBodyInstance* GroundBody=GroundCollision->GetBodyInstance();
    return bValid && IsValid(Manifest) && GroundBody && GroundBody->IsValidBodyInstance();
}
FName AIBS3PlanetWorld::RegionAt(FVector P) const
{
    if(!IsOperational()) return NAME_None;
    const IBS3::Geo Geo=IBS3::geographic(WorldVector(P),Manifest->RadiusCm,WorldVector(GetActorLocation()));
    if(!Geo.valid) return NAME_None;
    for(const FIBS3City& C:Manifest->Cities)
        if(IBS3::surfaceDistance(WorldVector(P),WorldVector(CityPosition(C)),Manifest->RadiusCm,WorldVector(GetActorLocation()))<=C.FootprintRadiusCm)
            return C.RegionId;
    return FName(*FString::Printf(TEXT("earth.cell.%02d"),IBS3::regionCell(Geo)));
}
void AIBS3PlanetWorld::SeedObjects()
{
    for(const FIBS3City& C:Manifest->Cities)
    {
        const FVector Up=(CityPosition(C)-GetActorLocation()).GetSafeNormal();
        const FVector East=WorldUE(IBS3::frame({FMath::DegreesToRadians(C.LatitudeDegrees),FMath::DegreesToRadians(C.LongitudeDegrees)}).east);
        for(int32 I=0;I<3;++I)
        {
            const TCHAR* Kinds[]={TEXT("wall"),TEXT("car"),TEXT("object")};
            const FName Key(*FString::Printf(TEXT("%s/%s/%s01"),*Manifest->BodyId.ToString(),*C.RegionId.ToString(),Kinds[I]));
            if(Deltas.Contains(Key)) continue;
            FIBS3ObjectDelta D; D.Key=Key; D.OriginRegionId=C.RegionId; D.CurrentRegionId=C.RegionId; D.Kind=Kinds[I];
            const FVector Normal=(Up*Manifest->RadiusCm+East*(5000+I*2000)).GetSafeNormal();
            D.GlobalTransform=Surface(GetActorLocation()+OriginOffset(),Manifest->RadiusCm,Normal,East,C.SurfaceOffsetCm+(I==0?150:100));
            D.GlobalTransform.SetScale3D(I==0?FVector(0.4,10,3):(I==1?FVector(4,1.8,1.3):FVector(1)));
            Deltas.Add(Key,D);
        }
    }
}
bool AIBS3PlanetWorld::SnapshotProps()
{
    bool Valid=true;
    for(auto& Pair:Props) if(IsValid(Pair.Value))
    {
        FIBS3ObjectDelta D=Pair.Value->Capture(OriginOffset()); D.CurrentRegionId=RegionAt(Pair.Value->GetActorLocation());
        const FIBS3ObjectDelta* Expected=Deltas.Find(Pair.Key);
        if(!Expected || !ValidDelta(D,*Expected)) { Valid=false; Status=TEXT("Invalid live object state: last valid delta retained, save/unload suspended"); continue; }
        Deltas.Add(Pair.Key,D);
    }
    return Valid;
}
void AIBS3PlanetWorld::UpdateResidency(FVector P,double Speed)
{
    if(!IsOperational() || P.ContainsNaN() || !FMath::IsFinite(Speed) || Speed<0) return;
    const double Lead=IBS3::prefetch(Speed,P95Seconds(),Manifest->BrakeCmS2,Manifest->PrefetchMarginCm);
    ActiveCity=NAME_None; double Nearest=DBL_MAX;
    for(int32 I=0;I<Manifest->Cities.Num();++I)
    {
        const FIBS3City& C=Manifest->Cities[I];
        if(auto* Existing=Hubs.Find(C.CityId); Existing && (!IsValid(*Existing) || (*Existing)->IsActorBeingDestroyed()))
        { Hubs.Remove(C.CityId); LoadStartedAt.Remove(C.CityId); }
        const double D=IBS3::surfaceDistance(WorldVector(P),WorldVector(CityPosition(C)),Manifest->RadiusCm,WorldVector(GetActorLocation()));
        const double Height=FMath::Max(0.0,FVector::Distance(P,GetActorLocation())-Manifest->RadiusCm-C.SurfaceOffsetCm);
        const bool Want=IBS3::wanted(FMath::Sqrt(D*D+Height*Height)-SafetyRadius(C),Lead,Hubs.Contains(C.CityId),Manifest->ResidencyHysteresisCm);
        if(Want && !Hubs.Contains(C.CityId))
        {
            const double Start=FPlatformTime::Seconds();
            AIBS3Hub* Hub=GetWorld()->SpawnActor<AIBS3Hub>();
            if(Hub)
            {
                Hubs.Add(C.CityId,Hub); LoadStartedAt.Add(C.CityId,Start);
                Hub->Build(C,GetActorLocation(),Manifest->RadiusCm);
            }
            else Status=TEXT("Hub spawn failed: approach speed constrained");
        }
        else if(!Want && Hubs.Contains(C.CityId)) { Hubs[C.CityId]->Destroy(); Hubs.Remove(C.CityId); LoadStartedAt.Remove(C.CityId); }
        if(Hubs.Contains(C.CityId))
        {
            AIBS3Hub* Hub=Hubs[C.CityId];
            Hub->RefreshCollisionReady();
            if(Hub->bCollisionReady && LoadStartedAt.Contains(C.CityId))
            {
                LoadSamples.Add(FPlatformTime::Seconds()-LoadStartedAt[C.CityId]); LoadStartedAt.Remove(C.CityId);
                if(LoadSamples.Num()>64) LoadSamples.RemoveAt(0);
                if(Status==TEXT("Collider preparation pending: approach speed constrained")) Status=TEXT("City colliders ready");
            }
            else if(!Hub->bCollisionReady) Status=TEXT("Collider preparation pending: approach speed constrained");
        }
        if(CityProxies.IsValidIndex(I)) CityProxies[I]->SetHiddenInGame(IsReady(C.CityId));
        if(D<C.FootprintRadiusCm && Height<Manifest->PhysicsBubbleCm && D<Nearest && IsReady(C.CityId)) { Nearest=D; ActiveCity=C.CityId; }
    }
    if(!ActiveCity.IsNone()) Discovered.Add(ActiveCity);
    for(auto It=Props.CreateIterator();It;++It)
        if(!IsValid(It.Value()) || It.Value()->IsActorBeingDestroyed()) It.RemoveCurrent();
    if(!SnapshotProps()) return;
    // Object residency follows its saved global location, not its city of origin; moved cars cannot duplicate.
    for(auto& Pair:Deltas)
    {
        const FIBS3ObjectDelta& D=Pair.Value;
        const FVector Local=D.GlobalTransform.GetLocation()-OriginOffset();
        const double Distance=FVector::Distance(P,Local);
        const bool Resident=Props.Contains(Pair.Key);
        const bool Want=Distance<=Manifest->PhysicsBubbleCm+(Resident?10000:0);
        if(Want && !Resident)
        {
            AIBS3Prop* Prop=GetWorld()->SpawnActor<AIBS3Prop>();
            if(Prop) { Prop->Restore(D,OriginOffset()); Props.Add(Pair.Key,Prop); }
        }
        else if(!Want && Resident) { Props[Pair.Key]->Destroy(); Props.Remove(Pair.Key); }
    }
}
double AIBS3PlanetWorld::AllowedSpeed(FVector P,double RequestedSpeed) const
{
    if(!IsOperational() || P.ContainsNaN() || !FMath::IsFinite(RequestedSpeed) || RequestedSpeed<0) return 0;
    double Limit=RequestedSpeed;
    for(const FIBS3City& C:Manifest->Cities) if(!IsReady(C.CityId))
    {
        const double D=IBS3::surfaceDistance(WorldVector(P),WorldVector(CityPosition(C)),Manifest->RadiusCm,WorldVector(GetActorLocation()));
        const double Height=FMath::Max(0.0,FVector::Distance(P,GetActorLocation())-Manifest->RadiusCm-C.SurfaceOffsetCm);
        const double Clearance=FMath::Sqrt(D*D+Height*Height)-SafetyRadius(C)-Manifest->PrefetchMarginCm;
        Limit=FMath::Min(Limit,IBS3::safeSpeed(Clearance,P95Seconds(),Manifest->BrakeCmS2));
    }
    return Limit;
}
bool AIBS3PlanetWorld::CanTraverse(FVector Start,FVector End,double CapsuleExtentCm) const
{
    if(!IsOperational() || Start.ContainsNaN() || End.ContainsNaN() || !FMath::IsFinite(CapsuleExtentCm) || CapsuleExtentCm<0) return false;
    for(const FIBS3City& C:Manifest->Cities)
        if(!IsReady(C.CityId) && IBS3::entersUnreadyRegion(WorldVector(Start),WorldVector(End),WorldVector(CityPosition(C)),SafetyRadius(C)+CapsuleExtentCm))
            return false;
    return true;
}
void AIBS3PlanetWorld::Tick(float Dt)
{
    Super::Tick(Dt); if(!bValid) return;
    const APawn* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
    if(Pawn)
    {
        const FVector Up=(Pawn->GetActorLocation()-GetActorLocation()).GetSafeNormal();
        // A preauthored curved cap rotates as a visual LOD. It never moves the fixed analytic collision or actors.
        // No runtime mesh/physics regeneration, including during extreme traversal.
        NearSurface->SetWorldRotation(FQuat::FindBetweenNormals(FVector::UpVector,Up));
        NearSurface->SetHiddenInGame(FVector::Distance(Pawn->GetActorLocation(),GetActorLocation())-Manifest->RadiusCm>300000);
    }
    if(Pawn && GetWorld()->GetTimeSeconds()>=NextResidencyUpdate)
    { UpdateResidency(Pawn->GetActorLocation(),Pawn->GetVelocity().Size()); NextResidencyUpdate=GetWorld()->GetTimeSeconds()+0.1; }
    for(auto& Pair:Props) if(IsValid(Pair.Value) && Pair.Value->Mesh->IsSimulatingPhysics())
    {
        const FVector Down=(GetActorLocation()-Pair.Value->GetActorLocation()).GetSafeNormal();
        Pair.Value->Mesh->AddForce(Down*Manifest->GravityCmS2,NAME_None,true);
    }
}
void AIBS3PlanetWorld::LoadState()
{
    if(!UGameplayStatics::DoesSaveGameExist(SaveSlot,0)) return;
    UIBS3SaveGame* Save=Cast<UIBS3SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot,0));
    bSaveCompatible=Save && Save->SchemaVersion==1 && Save->BodyId==Manifest->BodyId && Save->ManifestRevision==Manifest->Revision &&
        FMath::IsNearlyEqual(Save->RadiusCm,Manifest->RadiusCm,0.01) && Save->GlobalBodyCenter.Equals(GetActorLocation()+OriginOffset(),0.01);
    if(!bSaveCompatible) { Status=TEXT("Save incompatible/corrupt; original slot preserved, writes disabled"); return; }
    TMap<FName,FIBS3ObjectDelta> Candidate=Deltas;
    TSet<FName> Seen;
    for(const FIBS3ObjectDelta& D:Save->Deltas)
    {
        const FIBS3ObjectDelta* Expected=Deltas.Find(D.Key);
        if(!Expected || Seen.Contains(D.Key) || !ValidDelta(D,*Expected))
        { bSaveCompatible=false; Status=TEXT("Invalid/duplicate object identity or pose: original save preserved; fresh defaults retained"); return; }
        Seen.Add(D.Key); Candidate.Add(D.Key,D);
    }
    Deltas=MoveTemp(Candidate);
    for(FName City:Save->DiscoveredCities)
        if(Manifest->Cities.ContainsByPredicate([City](const FIBS3City& C){return C.CityId==City;})) Discovered.Add(City);
    DestinationCity=Save->DestinationCity;
    Status=TEXT("S3 state loaded");
}
bool AIBS3PlanetWorld::SaveState()
{
    if(!IsOperational()) { Status=TEXT("Save unavailable: planet physics not ready"); return false; }
    if(!bSaveCompatible) { Status=TEXT("Save disabled: incompatible/corrupt original slot preserved"); return false; }
    if(!SnapshotProps()) return false;
    UIBS3SaveGame* Save=Cast<UIBS3SaveGame>(UGameplayStatics::CreateSaveGameObject(UIBS3SaveGame::StaticClass()));
    if(!Save) { Status=TEXT("Could not allocate save object"); return false; }
    Save->BodyId=Manifest->BodyId; Save->ManifestRevision=Manifest->Revision; Save->RadiusCm=Manifest->RadiusCm;
    Save->GlobalBodyCenter=GetActorLocation()+OriginOffset(); Save->DiscoveredCities=Discovered.Array(); Save->DestinationCity=DestinationCity;
    Deltas.GenerateValueArray(Save->Deltas);
    const bool Result=UGameplayStatics::SaveGameToSlot(Save,SaveSlot,0);
    Status=Result?TEXT("S3 state saved"):TEXT("Save failed; in-memory state retained"); return Result;
}
void AIBS3PlanetWorld::CycleDestination()
{
    if(!bValid) return;
    int32 Index=Manifest->Cities.IndexOfByPredicate([this](const FIBS3City& C){ return C.CityId==DestinationCity; });
    DestinationCity=Manifest->Cities[(Index+1)%Manifest->Cities.Num()].CityId;
}
void AIBS3PlanetWorld::EndPlay(const EEndPlayReason::Type Reason)
{
    // Save explicitly with F5; teardown order must not silently overwrite a good save.
    Super::EndPlay(Reason);
}
