#include "IBS4Threat.h"
#include "IBS4PresentationComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AIBS4Missile::AIBS4Missile()
{
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("MissileCollision"));
    RootComponent = Collision;
    Collision->InitSphereRadius(12);
    Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Collision->SetNotifyRigidBodyCollision(true);
    Collision->OnComponentHit.AddDynamic(this, &AIBS4Missile::Impact);
    auto* Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissileProxy"));
    Visual->SetupAttachment(Collision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Visual->SetStaticMesh(Shape.Object);
    Visual->SetRelativeScale3D(FVector(0.24));
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    Movement->SetUpdatedComponent(Collision);
    Movement->InitialSpeed = 2500;
    Movement->MaxSpeed = 2500;
    Movement->ProjectileGravityScale = 0;
    Movement->bRotationFollowsVelocity = true;
    Movement->bForceSubStepping = true;
    InitialLifeSpan = 4;
    CreateDefaultSubobject<UIBS4PresentationComponent>(TEXT("Feedback"));
}

void AIBS4Missile::Impact(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, FVector, const FHitResult& Hit)
{
    if (bExploded) return;
    bExploded = true;
    Movement->StopMovementImmediately();
    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    const FVector Origin = Hit.ImpactPoint;
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_Pawn);
    Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Blast), false, this);
    Query.AddIgnoredActor(GetOwner());
    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, Objects, FCollisionShape::MakeSphere(350), Query);
    TSet<AActor*> Visited;
    FIBS4Hit Packet = UIBS4ResponseComponent::MakeHit(GetWorld(), GetOwner(), EIBS4Weapon::Missile, 30, Hit);
    for (const auto& Overlap : Overlaps)
    {
        AActor* Actor = Overlap.GetActor();
        if (!IsValid(Actor) || Visited.Contains(Actor) || Visited.Num() >= 16) continue;
        Visited.Add(Actor);
        const FVector Toward = Actor->GetActorLocation() - Origin;
        FHitResult Occlusion;
        // Start just outside the contacted surface; explosions cannot push through intact walls.
        if (GetWorld()->LineTraceSingleByChannel(Occlusion, Origin + Hit.ImpactNormal * 4, Actor->GetActorLocation(), ECC_Visibility, Query)
            && Occlusion.GetActor() != Actor) continue;
        Packet.Point = Origin;
        Packet.Direction = Toward.GetSafeNormal(0.001, -Hit.ImpactNormal);
        Packet.Part = Overlap.GetComponent();
        if (auto* Receiver = Actor->FindComponentByClass<UIBS4ResponseComponent>()) Receiver->Receive(Packet);
        else if (auto* Body = Overlap.GetComponent(); Body && Body->IsSimulatingPhysics())
            Body->AddImpulse(Packet.Direction * FMath::Clamp(600.f * (1.f - Toward.Size() / 350.f), 0.f, 600.f), NAME_None, true);
    }
    if (auto* Feedback = FindComponentByClass<UIBS4PresentationComponent>()) Feedback->Impact(Origin, EIBS4Surface::Concrete, true);
    PresentExplosion(Origin);
    SetLifeSpan(0.35f);
}

AIBS4Threat::AIBS4Threat()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
    Response = CreateDefaultSubobject<UIBS4ResponseComponent>(TEXT("Response"));
    TankVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TankVisual"));
    TankVisual->SetupAttachment(RootComponent);
    TankVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CreateDefaultSubobject<UIBS4PresentationComponent>(TEXT("Feedback"));
    GetCharacterMovement()->MaxWalkSpeed = 150;
    GetCapsuleComponent()->InitCapsuleSize(38, 90);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AIBS4Threat::BeginPlay()
{
    Super::BeginPlay();
    Home = GetActorLocation();
    if (bTank)
    {
        GetCharacterMovement()->DisableMovement();
        GetMesh()->SetVisibility(false);
    }
}

bool AIBS4Threat::ReportIncident(int64 IncidentId, AActor* Responsible, bool bVerified)
{
    if (IncidentId <= 0 || !IsValid(Target)) return false;
    const bool Accepted = Alert.report(static_cast<uint64>(IncidentId), Responsible == Target, bVerified, GetWorld()->GetTimeSeconds());
    if (Accepted) UE_LOG(LogTemp, Display, TEXT("IBS4 Alert incident=%lld origin=%s responder=%s"), IncidentId, *GetNameSafe(Responsible), *GetName());
    return Accepted;
}

void AIBS4Threat::ClearAlert()
{
    Alert = IBS4::Alert{};
    Burst = 0;
}

bool AIBS4Threat::HasSight(FHitResult& Trace) const
{
    if (!IsValid(Target) || FVector::DistSquared(Target->GetActorLocation(), GetActorLocation()) > FMath::Square(4000.f)) return false;
    const FVector Start = GetActorLocation() + FVector(0, 0, bTank ? 100 : 45);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Sight), false, this);
    GetWorld()->LineTraceSingleByChannel(Trace, Start, Target->GetActorLocation(), ECC_Visibility, Query);
    return Trace.bBlockingHit && Trace.GetActor() == Target;
}

void AIBS4Threat::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (Response->Health <= 0) { ClearAlert(); GetCharacterMovement()->StopMovementImmediately(); return; }
    FHitResult Trace;
    const bool Sight = HasSight(Trace);
    const bool Near = IsValid(Target) && FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) < FMath::Square(240.f);
    const double Now = GetWorld()->GetTimeSeconds();
    const auto Phase = Alert.update(Now, Sight, Near);
    static const TCHAR* Labels[] = {TEXT("Patrol"), TEXT("Warning"), TEXT("Aim"), TEXT("Fire"), TEXT("Retreat")};
    const FString Label = Labels[static_cast<uint8>(Phase)];
    if (Label != StateLabel) { StateLabel = Label; PresentState(StateLabel); }
    if (Phase == IBS4::Phase::Patrol && !bTank)
    {
        const FVector Destination = bPatrolReturn ? Home : Home + PatrolOffset;
        FVector Direction = Destination - GetActorLocation(); Direction.Z = 0;
        if (Direction.SizeSquared() < 2500) bPatrolReturn = !bPatrolReturn;
        else AddMovementInput(Direction.GetSafeNormal()); // CMC sweep, bounded open lab patrol
    }
    if (Phase == IBS4::Phase::Retreat && !bTank && IsValid(Target))
        AddMovementInput((GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D());
    if (Phase == IBS4::Phase::Aim || Phase == IBS4::Phase::Fire)
    {
        FRotator Facing = (Target->GetActorLocation() - GetActorLocation()).Rotation(); Facing.Pitch = 0;
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), Facing, DeltaSeconds, 3));
    }
    if (Phase != IBS4::Phase::Fire || !Sight || Now < NextShot) return;
    NextShot = Now + (bTank ? 3.5 : 0.5);
    if (bTank)
    {
        FActorSpawnParameters Params; Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        const FVector Start = Trace.TraceStart + (Trace.TraceEnd - Trace.TraceStart).GetSafeNormal() * 170;
        // Do not spawn beyond an intervening wall: muzzle path is also traced.
        FHitResult MuzzleHit;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Muzzle), false, this);
        if (GetWorld()->LineTraceSingleByChannel(MuzzleHit, Trace.TraceStart, Start, ECC_Visibility, Query)) return;
        auto* Missile = GetWorld()->SpawnActor<AIBS4Missile>(AIBS4Missile::StaticClass(), Start, (Target->GetActorLocation() - Start).Rotation(), Params);
        if (Missile) Missile->Collision->IgnoreActorWhenMoving(this, true);
    }
    else if (auto* Receiver = Target->FindComponentByClass<UIBS4ResponseComponent>())
        Receiver->Receive(UIBS4ResponseComponent::MakeHit(GetWorld(), this, EIBS4Weapon::Bullet, 10, Trace));
    if (auto* Feedback = FindComponentByClass<UIBS4PresentationComponent>())
    {
        Feedback->Beam(Trace.TraceStart, Trace.ImpactPoint, true);
        Feedback->Impact(Trace.ImpactPoint, EIBS4Surface::Metal, false);
    }
    ++ShotsFired;
    PresentShot(Trace.TraceStart, Trace.ImpactPoint, bTank);
    // Five-shot bursts require a new warning; no endless suppression state.
    if (++Burst >= 5) { Burst = 0; Alert.phase = IBS4::Phase::Retreat; Alert.changed = Now; }
}
