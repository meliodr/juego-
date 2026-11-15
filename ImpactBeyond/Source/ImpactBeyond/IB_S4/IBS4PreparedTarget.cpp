#include "IBS4PreparedTarget.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AIBS4PreparedTarget::AIBS4PreparedTarget()
{
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
        Place(0, FVector(0, 0, 85), FVector(100, 90, 130));
        Place(1, FVector(80, 0, 135), FVector(100, 30, 30));
        bCannonEnabled = !bBroken;
        if (bBroken)
        {
            UStaticMeshComponent* Part = Pieces[1];
            Part->ComponentTags.AddUnique(TEXT("IBS4Grabbable"));
            Part->SetCollisionProfileName(TEXT("PhysicsActor"));
            Part->SetUseCCD(true);
            Part->SetMassOverrideInKg(NAME_None, 25);
            Part->SetSimulatePhysics(true);
        }
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

void AIBS4PreparedTarget::HandleResponse(const FIBS4Hit& Hit, float Delta)
{
    if (bBroken || Kind == EIBS4TargetKind::MaterialCoupon) return;
    // Only the prepared cannon joint can disable the cannon; a torso hit is not dismemberment.
    if (Kind == EIBS4TargetKind::Robot && Hit.Part != Pieces[1]) return;
    AccumulatedDamage += FMath::Max(0.f, -Delta);
    if (AccumulatedDamage >= BreakThreshold && SetBrokenState(true))
    {
        PresentBreak(Hit.Point, Response->Surface);
        if (Kind != EIBS4TargetKind::Robot && Kind != EIBS4TargetKind::Crater)
        {
            for (int32 I = 0; I < 3; ++I)
            {
                auto* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(Hit.Point + FVector(30, I * 35 - 35, 40), FRotator::ZeroRotator);
                if (!Fragment) continue;
                auto* Mesh = Fragment->GetStaticMeshComponent();
                Mesh->SetMobility(EComponentMobility::Movable);
                Mesh->SetStaticMesh(Cube);
                Mesh->SetWorldScale3D(FVector(0.15, 0.2, 0.25));
                Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
                Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
                Mesh->SetSimulatePhysics(true);
                Mesh->SetPhysicsLinearVelocity(FVector(150, (I - 1) * 80, 140));
                Fragment->SetLifeSpan(4);
                Debris.Add(Fragment);
            }
        }
    }
}

bool AIBS4PreparedTarget::IsRestoreBlocked() const
{
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_Pawn);
    Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Restore), false, this);
    for (AActor* Fragment : Debris) if (IsValid(Fragment)) Query.AddIgnoredActor(Fragment);
    const FVector Extent = Kind == EIBS4TargetKind::Crater ? FVector(250, 250, 90) : FVector(120, 240, 180);
    const FVector Center = Kind == EIBS4TargetKind::Crater ? FVector(0, 0, -20) : FVector(0, 0, 180);
    return GetWorld()->OverlapAnyTestByObjectType(GetActorTransform().TransformPosition(Center), GetActorQuat(),
        Objects, FCollisionShape::MakeBox(Extent * GetActorScale3D().GetAbs()), Query);
}

bool AIBS4PreparedTarget::SetBrokenState(bool Broken)
{
    if (Broken == bBroken) return false;
    if (!Broken && GetWorld() && IsRestoreBlocked()) return false;
    // A held cannon must be released before restoring its attachment.
    if (!Broken && Pieces[1]->ComponentHasTag(TEXT("IBS4Held"))) return false;
    ClearDebris();
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
void AIBS4PreparedTarget::ClearDebris()
{
    for (AActor* Fragment : Debris) if (IsValid(Fragment)) Fragment->Destroy();
    Debris.Reset();
}
void AIBS4PreparedTarget::EndPlay(const EEndPlayReason::Type Reason)
{
    ClearDebris();
    Super::EndPlay(Reason);
}
