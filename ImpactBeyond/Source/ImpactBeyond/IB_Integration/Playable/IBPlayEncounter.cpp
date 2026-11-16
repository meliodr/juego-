#include "IBPlayEncounter.h"
#include "IBPlayCombat.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "IB_S4/IBS4Threat.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AIBPlayEncounter::AIBPlayEncounter()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Cube = Shape.Object;
}

bool AIBPlayEncounter::HasRequiredVisualAssets() const
{
    return OfficerMesh && RivalMesh && TankMesh && DroneMesh && RobotBodyMesh && RobotCannonMesh
        && ConcreteMaterial && GroundMaterial && GlassMaterial && MetalMaterial;
}

bool AIBPlayEncounter::SpawnRegion(FName Id, FVector Center, FVector Ground, FVector Up, APawn* Hero)
{
    if (!GetWorld() || !RegionId.IsNone() || Id.IsNone() || !IsValid(Hero) || Center.ContainsNaN()
        || Ground.ContainsNaN() || Up.ContainsNaN() || !Up.Normalize() || !Cube) return false;
    RegionId = Id;
    const FVector Forward = FVector::VectorPlaneProject(FVector::ForwardVector, Up).GetSafeNormal();
    const FQuat Basis = FRotationMatrix::MakeFromXZ(Forward.IsNearlyZero() ? FVector::RightVector : Forward, Up).ToQuat();
    // The crater descends 60 cm. Lift the plaza 80 cm so planet ground never plugs its hole.
    const FTransform Frame(Basis, Ground + Up * 80);
    SetActorTransform(Frame);
    const auto WorldPose = [&Frame](FVector Local, FQuat Rotation = FQuat::Identity)
    { return FTransform(Frame.GetRotation() * Rotation, Frame.TransformPosition(Local)); };
    bool Valid = true;
    const auto Box = [this, &WorldPose, &Valid](FVector Position, FVector Size, UMaterialInterface* Material)
    {
        const FTransform Pose = WorldPose(Position);
        FActorSpawnParameters Params; Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        auto* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Pose, Params);
        if (!Actor) { Valid = false; return; }
        OwnedActors.Add(Actor);
        auto* Mesh = Actor->GetStaticMeshComponent();
        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(Cube);
        Mesh->SetWorldScale3D(Size / 100);
        Mesh->SetCollisionProfileName(TEXT("BlockAll"));
        if (Material) Mesh->SetMaterial(0, Material);
    };
    // Four collision slabs surround the exact five-by-five metre crater opening.
    Box(FVector(-800,0,-50), FVector(2400,3600,100), GroundMaterial);
    Box(FVector(1700,0,-50), FVector(1600,3600,100), GroundMaterial);
    Box(FVector(650,-1025,-50), FVector(500,1550,100), GroundMaterial);
    Box(FVector(650,1025,-50), FVector(500,1550,100), GroundMaterial);
    // Walkable 40 cm steps connect both open edges with the surrounding city surface.
    Box(FVector(-2100,0,-60), FVector(200,3600,40), GroundMaterial);
    Box(FVector(2600,0,-60), FVector(200,3600,40), GroundMaterial);
    const auto Target = [this, &WorldPose, &Valid, Center](const TCHAR* Name, EIBS4TargetKind Kind, FVector Local,
        UMaterialInterface* Material, FQuat Rotation = FQuat::Identity)
    {
        const FTransform Pose = WorldPose(Local, Rotation);
        auto* Actor = GetWorld()->SpawnActorDeferred<AIBS4PreparedTarget>(AIBS4PreparedTarget::StaticClass(), Pose, this,
            nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Actor) { Valid = false; return Actor; }
        Actor->Kind = Kind;
        Actor->ObjectId = FName(*FString::Printf(TEXT("%s.%s"), *RegionId.ToString(), Name));
        Actor->bRadialGravity = true;
        Actor->PlanetCenter = Center;
        Actor->bHideRobotShapes = Kind == EIBS4TargetKind::Robot && RobotBodyMesh && RobotCannonMesh;
        Actor->FinishSpawning(Pose);
        PreparedTargets.Add(Actor);
        OwnedActors.Add(Actor);
        TInlineComponentArray<UStaticMeshComponent*> Meshes(Actor);
        if (Material) for (auto* Mesh : Meshes) Mesh->SetMaterial(0, Material);
        return Actor;
    };
    Target(TEXT("Wall"), EIBS4TargetKind::Wall, FVector(1400,0,0), ConcreteMaterial);
    Target(TEXT("Glass"), EIBS4TargetKind::Glass, FVector(1000,650,0), GlassMaterial);
    Target(TEXT("Door"), EIBS4TargetKind::Door, FVector(1400,650,0), MetalMaterial);
    Target(TEXT("Crater"), EIBS4TargetKind::Crater, FVector(650,0,0), GroundMaterial);
    Target(TEXT("Roof"), EIBS4TargetKind::Wall, FVector(1800,-900,500), ConcreteMaterial,
        FQuat(FVector::RightVector, -PI / 2));
    Box(FVector(1490,-1075,250), FVector(60,60,500), ConcreteMaterial);
    Box(FVector(1490,-725,250), FVector(60,60,500), ConcreteMaterial);
    AIBS4PreparedTarget* Robot = Target(TEXT("RobotCannon"), EIBS4TargetKind::Robot, FVector(1000,-1100,0), MetalMaterial);
    const auto Threat = [this, &WorldPose, &Valid, Center, Hero](EIBS4ThreatRole ThreatRole, FVector Local,
        USkeletalMesh* Skin, UStaticMesh* StaticSkin, bool Hostile)
    {
        const FTransform Pose = WorldPose(Local);
        auto* Actor = GetWorld()->SpawnActorDeferred<AIBS4Threat>(AIBS4Threat::StaticClass(), Pose, this,
            nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Actor) { Valid = false; return Actor; }
        Actor->ThreatRole = ThreatRole;
        Actor->bTank = ThreatRole == EIBS4ThreatRole::Tank;
        Actor->bRadialGravity = true;
        Actor->PlanetCenter = Center;
        Actor->bRegionReady = false;
        Actor->bHostileOnSight = Hostile;
        Actor->Target = Hero;
        if (Skin)
        {
            Actor->SetSharedOutfit(Skin);
            Actor->GetMesh()->SetRelativeLocation(FVector(0,0,-90));
            Actor->GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
        }
        if (StaticSkin)
        {
            Actor->TankVisual->SetStaticMesh(StaticSkin);
            const FVector Extent = StaticSkin->GetBounds().BoxExtent;
            const float Size = ThreatRole == EIBS4ThreatRole::Tank ? 220.f : 90.f;
            const double Scale = Size / FMath::Max(1.0, FMath::Max3(Extent.X, Extent.Y, Extent.Z));
            Actor->TankVisual->SetRelativeScale3D(FVector(Scale));
            if (ThreatRole == EIBS4ThreatRole::Tank)
                Actor->TankVisual->SetRelativeLocation(-StaticSkin->GetBounds().Origin * Scale + FVector(0,0,Extent.Z * Scale - 90));
        }
        Actor->FinishSpawning(Pose);
        OwnedActors.Add(Actor);
        if (ThreatRole != EIBS4ThreatRole::Robot) Threats.Add(Actor);
        Actor->IdleAnimation = ThreatRole == EIBS4ThreatRole::Officer ? OfficerIdle.Get() : RivalIdle.Get();
        Actor->WalkAnimation = ThreatRole == EIBS4ThreatRole::Officer ? OfficerWalk.Get() : RivalIdle.Get();
        Actor->AttackAnimation = ThreatRole == EIBS4ThreatRole::Officer ? OfficerAttack.Get() : RivalIdle.Get();
        return Actor;
    };
    Threat(EIBS4ThreatRole::Officer, FVector(-700,1100,90), OfficerMesh, nullptr, false);
    Threat(EIBS4ThreatRole::Officer, FVector(0,1100,90), OfficerMesh, nullptr, false);
    Threat(EIBS4ThreatRole::Tank, FVector(-1100,0,90), nullptr, TankMesh, false);
    Threat(EIBS4ThreatRole::Drone, FVector(0,-1000,600), nullptr, DroneMesh, true);
    Threat(EIBS4ThreatRole::AerialRival, FVector(-900,-900,650), RivalMesh, nullptr, true);
    if (Robot)
    {
        // A non-colliding fire controller uses the prepared robot's single HP source.
        RobotController = Threat(EIBS4ThreatRole::Robot, FVector(1000,-1100,90), nullptr, nullptr, true);
        if (RobotController)
        {
            RobotController->PreparedWeapon = Robot;
            RobotController->SetActorEnableCollision(false);
            RobotController->SetActorHiddenInGame(true);
        }
        const auto Skin = [Robot](UStaticMesh* Mesh, USceneComponent* Parent, FVector Location, float Radius)
        {
            if (!Mesh || !Parent) return;
            auto* Visual = NewObject<UStaticMeshComponent>(Robot);
            Visual->SetStaticMesh(Mesh);
            Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Visual->SetupAttachment(Parent);
            Visual->SetAbsolute(false, false, true);
            Visual->SetRelativeLocation(Location);
            Visual->SetWorldScale3D(FVector(Radius / FMath::Max(1.0, Mesh->GetBounds().BoxExtent.GetMax())));
            Robot->AddInstanceComponent(Visual);
            Visual->RegisterComponent();
        };
        Skin(RobotBodyMesh, Robot->GetRootComponent(), FVector(0,0,85), 100);
        Skin(RobotCannonMesh, Robot->GetPart(), FVector::ZeroVector, 50);
    }
    if (!Valid) ClearEncounter();
    return Valid;
}

void AIBPlayEncounter::SetRegionReady(bool Ready)
{
    for (AIBS4Threat* Threat : Threats) if (IsValid(Threat)) Threat->bRegionReady = Ready;
    if (IsValid(RobotController)) RobotController->bRegionReady = Ready;
}

void AIBPlayEncounter::Retarget(APawn* Hero)
{
    for (AIBS4Threat* Threat : Threats) if (IsValid(Threat)) Threat->Target = Hero;
    if (IsValid(RobotController)) RobotController->Target = Hero;
}

bool AIBPlayEncounter::ResetEncounter()
{
    bool Restored = true;
    for (AIBS4PreparedTarget* Target : PreparedTargets) if (IsValid(Target)) Restored &= Target->ResetTarget();
    for (AIBS4Threat* Threat : Threats) if (IsValid(Threat)) { Threat->Response->RestoreHealth(); Threat->ClearAlert(); }
    if (IsValid(RobotController)) RobotController->ClearAlert();
    return Restored;
}

bool AIBPlayEncounter::CapturePreparedState(TArray<FIBS4PreparedSnapshot>& Snapshots) const
{
    TArray<FIBS4PreparedSnapshot> Captured;
    for (AIBS4PreparedTarget* Target : PreparedTargets)
    {
        if (!IsValid(Target)) return false;
        FIBS4PreparedSnapshot Snapshot;
        if (!Target->CaptureSnapshot(Snapshot)) return false;
        Captured.Add(Snapshot);
    }
    Snapshots = MoveTemp(Captured);
    return true;
}

bool AIBPlayEncounter::RestorePreparedState(const TArray<FIBS4PreparedSnapshot>& Snapshots)
{
    if (Snapshots.Num() != PreparedTargets.Num()) return false;
    TSet<FName> Seen;
    for (const auto& Snapshot : Snapshots)
    {
        if (Seen.Contains(Snapshot.ObjectId) || !PreparedTargets.ContainsByPredicate([&Snapshot](const auto& Target)
            { return IsValid(Target.Get()) && Target->ObjectId == Snapshot.ObjectId && Target->Kind == Snapshot.Kind; })) return false;
        Seen.Add(Snapshot.ObjectId);
        auto* Match=PreparedTargets.FindByPredicate([&Snapshot](const auto& Target){return Target->ObjectId==Snapshot.ObjectId;});
        if(!(*Match)->CanRestoreSnapshot(Snapshot)) return false;
    }
    bool Restored = true;
    for (const auto& Snapshot : Snapshots)
    {
        auto* Target = PreparedTargets.FindByPredicate([&Snapshot](const auto& Entry) { return Entry->ObjectId == Snapshot.ObjectId; });
        const bool Hydrated=(*Target)->RestoreSnapshot(Snapshot);
        Restored &= Hydrated;
        if(Hydrated) if(auto* Combat=(*Target)->FindComponentByClass<UIBPlayCombat>())
        {
            Combat->ResetCombat();
            Combat->Response->Health=Snapshot.Health;
            Combat->ContactReceiver->Health=Snapshot.Health;
        }
    }
    return Restored;
}

void AIBPlayEncounter::ClearEncounter()
{
    for (AActor* Actor : OwnedActors) if (IsValid(Actor)) Actor->Destroy();
    OwnedActors.Reset(); PreparedTargets.Reset(); Threats.Reset(); RobotController = nullptr; RegionId = NAME_None;
}

void AIBPlayEncounter::EndPlay(const EEndPlayReason::Type Reason)
{
    ClearEncounter();
    Super::EndPlay(Reason);
}
