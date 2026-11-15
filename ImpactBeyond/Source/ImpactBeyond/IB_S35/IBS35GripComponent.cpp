#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35Object.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UIBS35GripComponent::UIBS35GripComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UIBS35GripComponent::BeginPlay()
{
    Super::BeginPlay(); InstigatorId = FGuid::NewGuid();
    if (ACharacter* Hero = Cast<ACharacter>(GetOwner())) AddTickPrerequisiteComponent(Hero->GetCharacterMovement());
}

bool UIBS35GripComponent::TryGrab(AIBS35Object* Object)
{
    if (Held.IsValid() || !IsValid(Object) || !GetOwner() || Object->GetWorld() != GetWorld()) return false;
    const FVector Origin = GetOwner()->GetActorLocation();
    if (FVector::DistSquared(Origin, Object->GetActorLocation()) > FMath::Square(Object->Profile.GrabRange)) return false;
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS35Grab), false, GetOwner());
    // The path to the contact must be unobstructed. Transport additionally sweeps the whole box.
    if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Object->GetActorLocation(), ECC_Visibility, Query)
        && Hit.GetActor() != Object) return false;
    if (!Object->Claim(this)) return false;
    Held = Object; HeldSeconds = ChargeSeconds = 0; LiftStart = Object->GetActorLocation();
    return true;
}

bool UIBS35GripComponent::GrabNearest()
{
    TArray<AIBS35Object*> Candidates;
    for (TActorIterator<AIBS35Object> It(GetWorld()); It; ++It) if (!It->IsHeld()) Candidates.Add(*It);
    const FVector Origin = GetOwner()->GetActorLocation();
    Candidates.Sort([Origin](const AIBS35Object& A, const AIBS35Object& B)
    { return FVector::DistSquared(Origin,A.GetActorLocation()) < FVector::DistSquared(Origin,B.GetActorLocation()); });
    for (AIBS35Object* Object : Candidates) if (TryGrab(Object)) return true;
    return false;
}

void UIBS35GripComponent::Drop()
{
    CancelCharge(); bToolActive = false;
    if (AIBS35Object* Object = Held.Get())
        if (!Object->Release(this, false, FVector::ZeroVector)) return;
    Held.Reset(); ChargeSeconds = HeldSeconds = 0; bToolActive = false;
}

bool UIBS35GripComponent::BeginCharge()
{
    AIBS35Object* Object = Held.Get();
    if (!Object || HeldSeconds < Object->Profile.LiftSeconds || !Object->Lease.BeginCharge(GetUniqueID())) return false;
    ChargeSeconds = 0; return true;
}

void UIBS35GripComponent::CancelCharge()
{
    if (AIBS35Object* Object = Held.Get()) Object->Lease.CancelCharge(GetUniqueID());
    ChargeSeconds = 0;
}

bool UIBS35GripComponent::CommitThrow(FVector Direction)
{
    AIBS35Object* Object = Held.Get();
    if (!Object || !IsCharging() || Direction.ContainsNaN() || Direction.IsNearlyZero()) return false;
    const auto& P = Object->Profile;
    const double Speed = IBS35::ThrowSpeed(Object->Body->GetMass(), ChargeSeconds / 0.9, P.MinThrowSpeed, P.MaxThrowSpeed);
    if (Speed <= 0 || !Object->Release(this, true, Direction.GetSafeNormal() * Speed)) return false;
    Held.Reset(); ChargeSeconds = HeldSeconds = 0; return true;
}

bool UIBS35GripComponent::IsCharging() const
{
    const AIBS35Object* Object = Held.Get();
    return Object && Object->GetHoldState() == IBS35::HoldState::Charging;
}

bool UIBS35GripComponent::GetGripTargets(FVector& LeftWorld, FVector& RightWorld) const
{
    const AIBS35Object* Object = Held.Get();
    if (!Object) return false;
    const auto& P = Object->Profile;
    LeftWorld = !P.LeftSocket.IsNone() && Object->Appearance->DoesSocketExist(P.LeftSocket)
        ? Object->Appearance->GetSocketLocation(P.LeftSocket) : Object->Body->GetComponentTransform().TransformPosition(P.LeftContact);
    RightWorld = !P.RightSocket.IsNone() && Object->Appearance->DoesSocketExist(P.RightSocket)
        ? Object->Appearance->GetSocketLocation(P.RightSocket) : Object->Body->GetComponentTransform().TransformPosition(P.RightContact);
    return true;
}

float UIBS35GripComponent::GetGripWeight() const
{
    const AIBS35Object* Object = Held.Get();
    return Object ? FMath::SmoothStep(0.f, Object->Profile.LiftSeconds, HeldSeconds) : 0;
}

float UIBS35GripComponent::GetGestureLoad() const
{
    const AIBS35Object* Object = Held.Get();
    return Object ? FMath::Clamp(Object->Profile.MassKg / 2200, 0.1f, 1.f) : 0;
}

bool UIBS35GripComponent::SweepTool(const FVector& Delta, FHitResult& Hit)
{
    AIBS35Object* Object = Held.Get();
    if (!Object || !Object->Profile.bMeleeTool || IsCharging() || Delta.ContainsNaN()) return false;
    Object->Body->MoveComponent(Delta, Object->Body->GetComponentQuat(), true, &Hit);
    return Hit.bBlockingHit;
}

void UIBS35GripComponent::TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* Function)
{
    Super::TickComponent(Dt, TickType, Function);
    AIBS35Object* Object = Held.Get();
    if (!Object || Dt <= 0 || !FMath::IsFinite(Dt)) return;
    if (!Object->Profile.IsValid() || !Object->GetActorScale3D().Equals(FVector::OneVector, 0.001f)) { Drop(); return; }
    HeldSeconds += Dt;
    if (IsCharging()) ChargeSeconds = FMath::Min(ChargeSeconds + Dt, 0.9f);
    const auto& P = Object->Profile;
    const FVector Target = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 35
        + FVector(0,0,Object->Body->Bounds.BoxExtent.Z + 100);
    if (bToolActive)
    {
        if (FVector::DistSquared(Object->GetActorLocation(), Target) > FMath::Square(P.MaxCarryError + 200)) Drop();
        return;
    }
    const float Alpha = FMath::SmoothStep(0.f, P.LiftSeconds, HeldSeconds);
    // Raise clear of the capsule before translating over it; never lift diagonally through the body.
    const FVector Raised(LiftStart.X, LiftStart.Y, FMath::Max(LiftStart.Z, Target.Z));
    const FVector Desired = Alpha < 0.6f ? FMath::Lerp(LiftStart, Raised, Alpha / 0.6f)
        : FMath::Lerp(Raised, Target, (Alpha - 0.6f) / 0.4f);
    const FVector Current = Object->Body->GetComponentLocation();
    if (Alpha >= 1 && FVector::DistSquared(Current, Target) > FMath::Square(P.MaxCarryError + P.CarrySpeed * Dt))
    { Drop(); return; }
    FHitResult Hit;
    Object->Body->MoveComponent((Desired - Current).GetClampedToMaxSize(P.CarrySpeed * Dt),
        Object->Body->GetComponentQuat(), true, &Hit);
    // Fixed orientation means no unswept rotation can cut through the world.
    if ((Hit.bBlockingHit || Alpha >= 1) && FVector::DistSquared(Object->GetActorLocation(), Desired) > FMath::Square(P.MaxCarryError)) Drop();
}

void UIBS35GripComponent::EndPlay(const EEndPlayReason::Type Reason) { Drop(); Super::EndPlay(Reason); }
