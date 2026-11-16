#include "IBS4PreparedTarget.h"
#include "IB_S35/IBS35Object.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AIBS4PreparedTarget::AIBS4PreparedTarget()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Response = CreateDefaultSubobject<UIBS4ResponseComponent>(TEXT("Response"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Cube = Mesh.Object;
    for (int32 I = 0; I < 25; ++I)
    {
        auto* Piece = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Prepared_%02d"), I));
        Piece->SetupAttachment(RootComponent);
        Piece->SetStaticMesh(Cube);
        Piece->SetMobility(EComponentMobility::Movable);
        Piece->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Pieces.Add(Piece);
    }
}

void AIBS4PreparedTarget::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Configure();
}

void AIBS4PreparedTarget::BeginPlay()
{
    Super::BeginPlay();
    Response->OnResponse.AddDynamic(this, &AIBS4PreparedTarget::HandleResponse);
    Configure();
}

void AIBS4PreparedTarget::Configure()
{
    for (UStaticMeshComponent* Piece : Pieces)
    {
        if (Piece->IsSimulatingPhysics())
        {
            Piece->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Piece->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        }
        Piece->SetSimulatePhysics(false);
        Piece->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        Piece->SetRelativeRotation(FRotator::ZeroRotator);
        Piece->SetVisibility(false);
        Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Piece->ComponentTags.Remove(TEXT("IBS4Grabbable"));
    }
    const auto Place = [this](int32 I, FVector Position, FVector Dimensions)
    {
        UStaticMeshComponent* Piece = Pieces[I];
        Piece->SetRelativeLocation(Position);
        Piece->SetRelativeScale3D(Dimensions / 100.0);
        Piece->SetVisibility(true);
        Piece->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Piece->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    };
    if (Kind == EIBS4TargetKind::Crater)
    {
        Response->Surface = EIBS4Surface::Concrete;
        int32 I = 0;
        for (int X = -2; X <= 2; ++X) for (int Y = -2; Y <= 2; ++Y)
        {
            const double Depth = bBroken ? IBS4::craterDepth(X, Y) : 0;
            // Every tile extends below the deepest depression; there is always a floor.
            Place(I++, FVector(X * 100, Y * 100, -80 - Depth / 2), FVector(100, 100, 160 - Depth));
        }
    }
    else if (Kind == EIBS4TargetKind::Robot)
    {
        Response->Surface = EIBS4Surface::Metal;
        const auto Body = IBS4::robotBodyBox();
        const auto Cannon = IBS4::cannonAttachmentBox();
        Place(0, FVector(Body.x, Body.y, Body.z), FVector(Body.hx, Body.hy, Body.hz) * 2);
        // Keep the cannon AND its conservative carry sweep outside the torso.
        Place(1, FVector(Cannon.x, Cannon.y, Cannon.z), FVector(Cannon.hx, Cannon.hy, Cannon.hz) * 2);
        bCannonEnabled = !bBroken;
        if (bBroken)
        {
            UStaticMeshComponent* Part = Pieces[1];
            Part->ComponentTags.AddUnique(TEXT("IBS4Grabbable"));
            Part->SetCollisionProfileName(TEXT("PhysicsActor"));
            Part->SetUseCCD(true);
            Part->SetMassOverrideInKg(NAME_None, 25);
            Part->SetSimulatePhysics(true);
            Part->SetEnableGravity(!bRadialGravity);
        }
        if (bHideRobotShapes) { Pieces[0]->SetVisibility(false); Pieces[1]->SetVisibility(false); }
    }
    else if (Kind == EIBS4TargetKind::Wall)
    {
        Response->Surface = EIBS4Surface::Concrete;
        // 220 cm wide x 260 cm high passage; frame remains collidable.
        Place(0, FVector(0, -175, 180), FVector(30, 130, 360));
        Place(1, FVector(0, 175, 180), FVector(30, 130, 360));
        Place(2, FVector(0, 0, 310), FVector(30, 220, 100));
        if (!bBroken) Place(3, FVector(0, 0, 130), FVector(30, 220, 260));
    }
    else
    {
        if (Kind == EIBS4TargetKind::Glass) Response->Surface = EIBS4Surface::Glass;
        if (Kind == EIBS4TargetKind::Door) Response->Surface = EIBS4Surface::Metal;
        if (!bBroken || Kind == EIBS4TargetKind::MaterialCoupon)
            Place(0, FVector(0, 0, 120), FVector(20, 150, 240));
    }
}

void AIBS4PreparedTarget::HandleResponse(const FIBS4Hit& Hit, const FIBS4Outcome& Outcome)
{
    if (bBroken || Kind == EIBS4TargetKind::MaterialCoupon) return;
    // Only the prepared cannon joint can disable the cannon; a torso hit is not dismemberment.
    if (Kind == EIBS4TargetKind::Robot && Hit.Part != Pieces[1]) return;
    // Structural resistance is independent of remaining HP. A depleted torso
    // must not make its intact cannon immune, and this never mutates HP twice.
    AccumulatedDamage += FMath::Max(0.f, -Outcome.RequestedHealthDelta);
    if (AccumulatedDamage >= BreakThreshold && SetBrokenState(true))
    {
        PresentBreak(Hit.Point, Response->Surface);
        if (Kind != EIBS4TargetKind::Robot && Kind != EIBS4TargetKind::Crater)
        {
            for (int32 I = 0; I < 3; ++I)
            {
                auto* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(Hit.Point + GetActorQuat().RotateVector(FVector(30, I * 35 - 35, 40)), GetActorRotation());
                if (!Fragment) continue;
                auto* Mesh = Fragment->GetStaticMeshComponent();
                Mesh->SetMobility(EComponentMobility::Movable);
                Mesh->SetStaticMesh(Cube);
                for (int32 MaterialIndex = 0; MaterialIndex < Pieces[0]->GetNumMaterials(); ++MaterialIndex)
                    Mesh->SetMaterial(MaterialIndex, Pieces[0]->GetMaterial(MaterialIndex));
                Mesh->SetWorldScale3D(FVector(0.15, 0.2, 0.25));
                Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
                Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
                Mesh->SetSimulatePhysics(true);
                Mesh->SetEnableGravity(!bRadialGravity);
                Mesh->SetPhysicsLinearVelocity(GetActorQuat().RotateVector(FVector(150, (I - 1) * 80, 140)));
                Fragment->SetLifeSpan(4);
                Debris.Add(Fragment);
            }
        }
    }
}

void AIBS4PreparedTarget::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bRadialGravity) return;
    const auto ApplyGravity = [this](UPrimitiveComponent* Body)
    {
        if (!Body || !Body->IsSimulatingPhysics() || Body->ComponentHasTag(TEXT("IBS4Held"))) return;
        Body->AddForce((PlanetCenter - Body->GetComponentLocation()).GetSafeNormal() * 980, NAME_None, true);
    };
    if (Kind == EIBS4TargetKind::Robot && bBroken) ApplyGravity(Pieces[1]);
    for (AActor* Fragment : Debris)
        if (IsValid(Fragment)) ApplyGravity(Cast<UPrimitiveComponent>(Fragment->GetRootComponent()));
}

bool AIBS4PreparedTarget::CaptureSnapshot(FIBS4PreparedSnapshot& Snapshot) const
{
    if (ObjectId.IsNone() || Pieces[1]->ComponentHasTag(TEXT("IBS4Held")) || (IsValid(SharedLoosePart)&&SharedLoosePart->IsHeld())) return false;
    Snapshot = FIBS4PreparedSnapshot{};
    Snapshot.ObjectId = ObjectId;
    Snapshot.Kind = Kind;
    Snapshot.bBroken = bBroken;
    Snapshot.Health = Response->Health;
    Snapshot.StructuralDamage = AccumulatedDamage;
    if (Kind == EIBS4TargetKind::Robot && bBroken)
    {
        Snapshot.PartTransform = Pieces[1]->GetComponentTransform();
        Snapshot.PartVelocity = Pieces[1]->GetPhysicsLinearVelocity();
        Snapshot.PartAngularVelocity = Pieces[1]->GetPhysicsAngularVelocityInDegrees();
        if(IsValid(SharedLoosePart))
        {
            Snapshot.PartTransform.SetLocation(SharedLoosePart->GetActorLocation());
            Snapshot.PartTransform.SetRotation(SharedLoosePart->GetActorQuat());
            Snapshot.PartVelocity=SharedLoosePart->Body->GetPhysicsLinearVelocity();
            Snapshot.PartAngularVelocity=SharedLoosePart->Body->GetPhysicsAngularVelocityInDegrees();
        }
    }
    return true;
}

bool AIBS4PreparedTarget::CanRestoreSnapshot(const FIBS4PreparedSnapshot& Snapshot) const
{
    if (!GetWorld() || Snapshot.Version != 1 || ObjectId.IsNone() || Snapshot.ObjectId != ObjectId || Snapshot.Kind != Kind
        || static_cast<uint8>(Kind) > static_cast<uint8>(EIBS4TargetKind::MaterialCoupon)
        || !IBS4::validHealth(Snapshot.Health) || !FMath::IsFinite(Snapshot.StructuralDamage)
        || Snapshot.StructuralDamage < 0 || Snapshot.StructuralDamage > 100000
        || !Snapshot.PartTransform.IsValid() || Snapshot.PartVelocity.ContainsNaN() || Snapshot.PartAngularVelocity.ContainsNaN()
        || Snapshot.PartVelocity.SizeSquared() > FMath::Square(100000.f)
        || Snapshot.PartAngularVelocity.SizeSquared() > FMath::Square(100000.f)
        || Pieces[1]->ComponentHasTag(TEXT("IBS4Held")) || (IsValid(SharedLoosePart)&&SharedLoosePart->IsHeld())) return false;
    if (Kind == EIBS4TargetKind::Robot && Snapshot.bBroken)
    {
        const FVector Scale = Snapshot.PartTransform.GetScale3D();
        if (!Scale.Equals(FVector(1,0.3,0.3) * GetActorScale3D(),0.001)) return false;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4PartRestore), false, this);
        if(IsValid(SharedLoosePart)) Query.AddIgnoredActor(SharedLoosePart);
        if (GetWorld()->OverlapBlockingTestByChannel(Snapshot.PartTransform.GetLocation(), Snapshot.PartTransform.GetRotation(),
            ECC_WorldDynamic, FCollisionShape::MakeBox(Scale.GetAbs() * 49.9), Query)) return false;
    }
    if (!Snapshot.bBroken && bBroken && IsRestoreBlocked()) return false;
    return true;
}

bool AIBS4PreparedTarget::RestoreSnapshot(const FIBS4PreparedSnapshot& Snapshot)
{
    if (!CanRestoreSnapshot(Snapshot)) return false;
    if (Snapshot.bBroken != bBroken && !SetBrokenState(Snapshot.bBroken)) return false;
    ClearDebris();
    // Persistence restores the same health owner; it is not a second damage path.
    Response->Health = Snapshot.Health;
    AccumulatedDamage = Snapshot.StructuralDamage;
    if (Kind == EIBS4TargetKind::Robot && bBroken)
    {
        Pieces[1]->SetWorldTransform(Snapshot.PartTransform, false, nullptr, ETeleportType::TeleportPhysics);
        Pieces[1]->SetPhysicsLinearVelocity(Snapshot.PartVelocity);
        Pieces[1]->SetPhysicsAngularVelocityInDegrees(Snapshot.PartAngularVelocity);
        if(IsValid(SharedLoosePart))
        {
            if(!SharedLoosePart->ClearLaunchForRestore()) return false;
            SharedLoosePart->SetActorLocationAndRotation(Snapshot.PartTransform.GetLocation(),Snapshot.PartTransform.GetRotation(),false,nullptr,ETeleportType::TeleportPhysics);
            SharedLoosePart->Body->SetPhysicsLinearVelocity(Snapshot.PartVelocity);
            SharedLoosePart->Body->SetPhysicsAngularVelocityInDegrees(Snapshot.PartAngularVelocity);
        }
    }
    return true;
}

bool AIBS4PreparedTarget::IsRestoreBlocked() const
{
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Restore), false, this);
    for (AActor* Fragment : Debris) if (IsValid(Fragment)) Query.AddIgnoredActor(Fragment);
    const auto Blocked = [this, &Query](const IBS4::RestoreBox& Box)
    {
        if (Box.hz <= 0) return false;
        // Query only newly occupied volume, with a 1 mm contact tolerance. Include
        // every blocker of WorldDynamic, including kinematic props and vehicles.
        const FVector Extent = (FVector(Box.hx, Box.hy, Box.hz) - FVector(0.1)) * GetActorScale3D().GetAbs();
        return GetWorld()->OverlapBlockingTestByChannel(GetActorTransform().TransformPosition(FVector(Box.x, Box.y, Box.z)),
            GetActorQuat(), ECC_WorldDynamic, FCollisionShape::MakeBox(Extent), Query);
    };
    if (Kind == EIBS4TargetKind::Crater)
    {
        for (int X = -1; X <= 1; ++X) for (int Y = -1; Y <= 1; ++Y)
            if (Blocked(IBS4::craterRestoreVolume(X, Y))) return true;
        return false;
    }
    if (Kind == EIBS4TargetKind::Robot) return Blocked(IBS4::cannonAttachmentBox());
    if (Kind == EIBS4TargetKind::Wall) return Blocked({0, 0, 130, 15, 110, 130});
    return Blocked({0, 0, 120, 10, 75, 120});
}

bool AIBS4PreparedTarget::SetBrokenState(bool Broken)
{
    if (Broken == bBroken) return false;
    if(IsValid(SharedLoosePart)&&SharedLoosePart->IsHeld()) return false;
    // A held cannon must be released before restoring its attachment.
    if (!Broken && !IBS4::canRestore(GetWorld() && IsRestoreBlocked(), Pieces[1]->ComponentHasTag(TEXT("IBS4Held")))) return false;
    ClearDebris();
    if(IsValid(SharedLoosePart)) { SharedLoosePart->Destroy(); SharedLoosePart=nullptr; }
    for(USceneComponent* Child:Pieces[1]->GetAttachChildren()) Child->SetVisibility(true,true);
    bBroken = Broken;
    Configure();
    return true;
}

bool AIBS4PreparedTarget::ResetTarget()
{
    if (bBroken && !SetBrokenState(false)) return false;
    ClearDebris();
    AccumulatedDamage = 0;
    Response->RestoreHealth();
    return true;
}

UPrimitiveComponent* AIBS4PreparedTarget::GetPart() const { return Pieces[1]; }
AIBS35Object* AIBS4PreparedTarget::GetSharedLoosePart()
{
    if(IsValid(SharedLoosePart)) return SharedLoosePart;
    if(Kind!=EIBS4TargetKind::Robot||!bBroken||!GetWorld()||Pieces[1]->ComponentHasTag(TEXT("IBS4Held"))) return nullptr;
    UStaticMeshComponent* Source=Pieces[1];
    for(USceneComponent* Child:Pieces[1]->GetAttachChildren())
        if(auto* Mesh=Cast<UStaticMeshComponent>(Child); Mesh&&Mesh->GetStaticMesh()) { Source=Mesh; break; }
    const FTransform Pose(Pieces[1]->GetComponentQuat(),Pieces[1]->GetComponentLocation());
    auto* Loose=GetWorld()->SpawnActorDeferred<AIBS35Object>(AIBS35Object::StaticClass(),Pose,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if(!Loose) return nullptr;
    Loose->Profile=FIBS35ObjectProfile::Preset(EIBS35ObjectClass::LoosePart);
    Loose->Profile.HalfExtent=Pieces[1]->Bounds.BoxExtent.ComponentMax(FVector(5));
    Loose->Profile.HalfExtent=FVector(50,15,15)*GetActorScale3D().GetAbs();
    Loose->Profile.MassKg=25;
    Loose->Profile.Variant=FName(*FString::Printf(TEXT("%s.Cannon"),*ObjectId.ToString()));
    Loose->bRadialGravity=bRadialGravity; Loose->PlanetCenter=PlanetCenter;
    Loose->Appearance->SetStaticMesh(Source->GetStaticMesh());
    for(int32 I=0;I<Source->GetNumMaterials();++I) Loose->Appearance->SetMaterial(I,Source->GetMaterial(I));
    const FVector Velocity=Pieces[1]->GetPhysicsLinearVelocity();
    Pieces[1]->SetSimulatePhysics(false);
    Pieces[1]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Pieces[1]->SetVisibility(false,true);
    Loose->FinishSpawning(Pose);
    Loose->Body->SetPhysicsLinearVelocity(Velocity);
    SharedLoosePart=Loose;
    return Loose;
}
void AIBS4PreparedTarget::ClearDebris()
{
    for (AActor* Fragment : Debris) if (IsValid(Fragment)) Fragment->Destroy();
    Debris.Reset();
}
void AIBS4PreparedTarget::EndPlay(const EEndPlayReason::Type Reason)
{
    ClearDebris();
    if(IsValid(SharedLoosePart)) SharedLoosePart->Destroy();
    Super::EndPlay(Reason);
}
