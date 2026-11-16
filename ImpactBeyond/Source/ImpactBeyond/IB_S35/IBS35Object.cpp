#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

AIBS35Object::AIBS35Object()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    Body = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicalBox"));
    SetRootComponent(Body);
    Body->SetMobility(EComponentMobility::Movable);
    Body->SetCollisionProfileName(TEXT("PhysicsActor"));
    Body->SetCollisionResponseToAllChannels(ECR_Block);
    Body->SetUseCCD(true);
    Body->SetNotifyRigidBodyCollision(true);
    Appearance = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Appearance"));
    Appearance->SetupAttachment(Body);
    Appearance->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Cosmetic only; Body remains blocking.
    Receiver = CreateDefaultSubobject<UIBS35ReceiverComponent>(TEXT("Receiver"));
}

void AIBS35Object::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (ProfileAsset) Profile = ProfileAsset->Profile;
    if (!Profile.IsValid()) return;
    Body->SetBoxExtent(Profile.HalfExtent);
    Body->SetMassOverrideInKg(NAME_None, Profile.MassKg, true);
    if (UStaticMesh* Mesh = Appearance->GetStaticMesh())
    {
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector Extent = Bounds.BoxExtent.ComponentMax(FVector(1));
        const bool bTurn = (Extent.X > Extent.Y) != (Profile.HalfExtent.X > Profile.HalfExtent.Y);
        const FQuat Rotation = bTurn ? FQuat(FVector::UpVector, UE_HALF_PI) : FQuat::Identity;
        const FVector Target = bTurn ? FVector(Profile.HalfExtent.Y, Profile.HalfExtent.X, Profile.HalfExtent.Z) : Profile.HalfExtent;
        const FVector Scale = Target / Extent;
        Appearance->SetRelativeRotation(Rotation);
        Appearance->SetRelativeScale3D(Scale);
        Appearance->SetRelativeLocation(-Rotation.RotateVector(Bounds.Origin * Scale));
    }
}

void AIBS35Object::RefreshProfile()
{
    if (!HasActorBegunPlay()) OnConstruction(GetActorTransform());
}

void AIBS35Object::BeginPlay()
{
    Super::BeginPlay();
    StartTransform = GetActorTransform();
    if (bRadialGravity) Body->SetEnableGravity(false);
    Body->OnComponentHit.AddDynamic(this, &AIBS35Object::OnBodyHit);
    if (Profile.IsValid()) Body->SetSimulatePhysics(true);
}

bool AIBS35Object::Claim(UIBS35GripComponent* Grip)
{
    if (!IsValid(Grip) || !IsValid(Grip->GetOwner()) || !Profile.IsValid()
        || !Body->IsSimulatingPhysics() || Body->GetCollisionEnabled() != ECollisionEnabled::QueryAndPhysics
        || Body->GetCollisionResponseToChannel(ECC_WorldStatic) != ECR_Block
        || Body->GetCollisionResponseToChannel(ECC_WorldDynamic) != ECR_Block
        || !GetActorScale3D().Equals(FVector::OneVector, 0.001f)
        || !Lease.Claim(Grip->GetUniqueID())) return false;
    Holder = Grip;
    IgnoredCarrier = Grip->GetOwner();
    bSavedSimulation = Body->IsSimulatingPhysics();
    bBodyIgnoredCarrier = Body->GetMoveIgnoreActors().Contains(Grip->GetOwner());
    Body->IgnoreActorWhenMoving(Grip->GetOwner(), true);
    if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Grip->GetOwner()->GetRootComponent()))
    {
        bCarrierIgnoredBody = Root->GetMoveIgnoreActors().Contains(this);
        Root->IgnoreActorWhenMoving(this, true);
    }
    Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Body->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    Body->SetSimulatePhysics(false);
    LaunchId.Invalidate();
    return true;
}

void AIBS35Object::RestoreBody()
{
    if (AActor* Carrier = IgnoredCarrier.Get())
    {
        if (!bBodyIgnoredCarrier) Body->IgnoreActorWhenMoving(Carrier, false);
        if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Carrier->GetRootComponent()))
            if (!bCarrierIgnoredBody) Root->IgnoreActorWhenMoving(this, false);
    }
    IgnoredCarrier.Reset(); Holder.Reset();
    Body->SetSimulatePhysics(bSavedSimulation);
    Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Body->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    Body->WakeAllRigidBodies();
}

bool AIBS35Object::Release(UIBS35GripComponent* Grip, bool bLaunch, const FVector& Velocity)
{
    if (!Grip || Lease.Carrier != Grip->GetUniqueID() || Velocity.ContainsNaN() || !ClearCarrierOverlap()
        || !Lease.Release(Grip->GetUniqueID(), bLaunch)) return false;
    RestoreBody();
    if (bLaunch)
    {
        LaunchId = FGuid::NewGuid(); LaunchInstigator = Grip->InstigatorId; FlightAge = 0;
        LaunchInstigatorActor = Grip->GetOwner();
        PrePhysicsVelocity = Velocity;
        Body->AddImpulse(Velocity * Body->GetMass()); // Physical impulse, no mass-independent flag.
    }
    else LaunchId.Invalidate(); // Drop/cancel never reuses charge or inherited flight velocity.
    return true;
}

bool AIBS35Object::ClearCarrierOverlap()
{
    AActor* Carrier = IgnoredCarrier.Get();
    UPrimitiveComponent* Root = Carrier ? Cast<UPrimitiveComponent>(Carrier->GetRootComponent()) : nullptr;
    if (!Root) return true;
    const FCollisionShape Shape = FCollisionShape::MakeBox(Body->GetScaledBoxExtent());
    if (!Root->OverlapComponent(Body->GetComponentLocation(), Body->GetComponentQuat(), Shape)) return true;
    // Never reactivate physics inside the carrier. Separation also sweeps against the world.
    const FVector Up = Carrier->GetActorUpVector();
    const double CarrierRadius = FVector::DotProduct(Root->Bounds.BoxExtent, Up.GetAbs());
    const double CargoRadius = FVector::DotProduct(Body->Bounds.BoxExtent, Up.GetAbs());
    const double Separation = FVector::DotProduct(Body->GetComponentLocation() - Root->Bounds.Origin, Up);
    FHitResult Hit;
    Body->MoveComponent(Up * FMath::Max(0.0, CarrierRadius + CargoRadius + 4 - Separation),
        Body->GetComponentQuat(), true, &Hit);
    return !Root->OverlapComponent(Body->GetComponentLocation(), Body->GetComponentQuat(), Shape);
}

bool AIBS35Object::ClearLaunchForRestore()
{
    if (IsHeld()) return false;
    LaunchId.Invalidate();
    LaunchInstigator.Invalidate();
    LaunchInstigatorActor.Reset();
    FlightAge = 0;
    PrePhysicsVelocity = FVector::ZeroVector;
    Lease.Reset();
    // Snapshot restore owns transforms, simulation and velocities. This method
    // neither claims a carrier nor changes the physical state supplied by it.
    return true;
}

bool AIBS35Object::ResetObject()
{
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS35Reset), false, this);
    if (GetWorld()->OverlapBlockingTestByChannel(StartTransform.GetLocation(), StartTransform.GetRotation(),
        Body->GetCollisionObjectType(), FCollisionShape::MakeBox(Body->GetScaledBoxExtent()), Query)) return false;
    if (UIBS35GripComponent* Grip = Holder.Get()) Grip->Drop();
    else if (Lease.Carrier && ClearCarrierOverlap()) { RestoreBody(); Lease.Reset(); }
    if (Lease.Carrier) return false;
    Lease.Reset(); LaunchId.Invalidate(); FlightAge = 0;
    Body->SetSimulatePhysics(false);
    SetActorTransform(StartTransform, false, nullptr, ETeleportType::TeleportPhysics);
    Body->SetSimulatePhysics(true);
    Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Body->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
    Receiver->ResetReceiver();
    return true;
}

void AIBS35Object::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bRadialGravity && Body->IsSimulatingPhysics())
    {
        Body->SetEnableGravity(false);
        Body->AddForce((PlanetCenter - GetActorLocation()).GetSafeNormal() * 980, NAME_None, true);
    }
    if (Lease.Carrier && !Holder.IsValid() && ClearCarrierOverlap()) { RestoreBody(); Lease.Reset(); LaunchId.Invalidate(); }
    if (LaunchId.IsValid())
    {
        FlightAge += DeltaSeconds;
        PrePhysicsVelocity = Body->GetPhysicsLinearVelocity();
        if (FlightAge > 10) LaunchId.Invalidate();
    }
}

void AIBS35Object::EndPlay(const EEndPlayReason::Type Reason)
{
    if (UIBS35GripComponent* Grip = Holder.Get()) Grip->Drop();
    Super::EndPlay(Reason);
}

void AIBS35Object::OnBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!LaunchId.IsValid() || !IsValid(OtherActor) || !IsValid(OtherComponent)) return;
    auto* Target = OtherActor->FindComponentByClass<UIBS35ReceiverComponent>();
    if (!Target) return;
    const FVector Relative = PrePhysicsVelocity - OtherComponent->GetComponentVelocity();
    const float ClosingSpeed = FMath::Max(0.0, -FVector::DotProduct(Relative, Hit.ImpactNormal));
    if (ClosingSpeed < 200) return;
    FIBS35DamageMessage Message;
    Message.AttackId = LaunchId; Message.InstigatorId = LaunchInstigator; Message.TargetId = Target->TargetId;
    Message.InstigatorActor = LaunchInstigatorActor;
    Message.TargetPart = OtherComponent;
    Message.DamageKind = EIBS35DamageKind::ThrownObject;
    Message.HealthDelta = -FMath::Clamp(Body->GetMass() * ClosingSpeed * ClosingSpeed / 20000000.f, 1.f, 300.f);
    Message.Impulse = -Hit.ImpactNormal * FMath::Abs(FVector::DotProduct(NormalImpulse, Hit.ImpactNormal));
    Message.Contact = Hit.ImpactPoint; Message.RelativeVelocity = Relative; Message.AttackSeconds = FlightAge;
    Message.SurfaceMaterial = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->GetFName() : NAME_None;
    Message.bPhysicsResolved = true;
    Target->Receive(Message, OtherComponent);
}
