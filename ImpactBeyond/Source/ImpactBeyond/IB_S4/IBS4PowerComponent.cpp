#include "IBS4PowerComponent.h"
#include "IBS4ResponseComponent.h"
#include "IBS4PresentationComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

UIBS4PowerComponent::UIBS4PowerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}
void UIBS4PowerComponent::BeginPlay()
{
    Super::BeginPlay();
    Handle = NewObject<UPhysicsHandleComponent>(GetOwner(), TEXT("S4CreativeHandle"));
    GetOwner()->AddInstanceComponent(Handle);
    Handle->RegisterComponent();
    Handle->SetLinearStiffness(1800);
    Handle->SetLinearDamping(180);
    Handle->SetInterpolationSpeed(12);
    Feedback = GetOwner()->FindComponentByClass<UIBS4PresentationComponent>();
}
bool UIBS4PowerComponent::Aim(float Range, FHitResult& Hit, FVector& Start, FVector& End) const
{
    Start = GetOwner()->GetActorLocation();
    End = Start;
    const auto* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !Pawn->GetController()) return false;
    FVector View; FRotator Rotation;
    Pawn->GetController()->GetPlayerViewPoint(View, Rotation);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Aim), false, Pawn);
    FHitResult CameraHit;
    const FVector CameraEnd = View + Rotation.Vector() * Range;
    GetWorld()->LineTraceSingleByChannel(CameraHit, View, CameraEnd, ECC_Visibility, Query);
    End = CameraHit.bBlockingHit ? CameraHit.ImpactPoint : CameraEnd;
    // The camera may see around a corner; the power must still clear the owner's eyes.
    Start = Pawn->GetActorLocation() + Pawn->GetActorUpVector() * 55;
    GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + (End - Start).GetClampedToMaxSize(Range), ECC_Visibility, Query);
    End = Hit.bBlockingHit ? Hit.ImpactPoint : Hit.TraceEnd;
    return Hit.bBlockingHit;
}
bool UIBS4PowerComponent::ToggleTelekinesis()
{
    if (IsHolding()) { Release(); return true; }
    FHitResult Hit; FVector Start, End;
    if (!Handle || !Aim(GrabRange, Hit, Start, End)) return false;
    UPrimitiveComponent* Candidate = Hit.GetComponent();
    if (!Candidate || !Candidate->IsSimulatingPhysics() || Candidate->GetMass() > MaxMass ||
        !Candidate->ComponentHasTag(TEXT("IBS4Grabbable")) || Candidate->ComponentHasTag(TEXT("IBS4Held"))) return false;
    HeldRadius = Candidate->Bounds.SphereRadius;
    if (!FMath::IsFinite(HeldRadius) || HeldRadius > 160 || HeldRadius < 1) return false;
    Candidate->ComponentTags.Add(TEXT("IBS4Held"));
    bPreviousCCD = Candidate->BodyInstance.bUseCCD;
    Candidate->SetUseCCD(true);
    Held = Candidate;
    SetLaser(false);
    // Center grab + conservative bounding sphere covers rotation and the entire cargo.
    Handle->GrabComponentAtLocation(Candidate, NAME_None, Candidate->Bounds.Origin);
    if (Handle->GetGrabbedComponent() != Candidate) { Release(); return false; }
    return true;
}
void UIBS4PowerComponent::Release(bool Throw)
{
    UPrimitiveComponent* Body = Held.Get();
    if (Handle) Handle->ReleaseComponent();
    Held.Reset();
    if (!Body) return;
    Body->ComponentTags.Remove(TEXT("IBS4Held"));
    Body->SetUseCCD(bPreviousCCD);
    if (Throw)
    {
        const auto* Pawn = Cast<APawn>(GetOwner());
        if (!Pawn) return;
        const FVector Direction = Pawn->GetControlRotation().Vector();
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Throw), false, GetOwner());
        Query.AddIgnoredComponent(Body);
        FHitResult Block;
        if (!GetWorld()->SweepSingleByChannel(Block, Body->Bounds.Origin, Body->Bounds.Origin + Direction * (HeldRadius + 30),
            FQuat::Identity, ECC_PhysicsBody, FCollisionShape::MakeSphere(HeldRadius), Query))
        {
            // Lab props already have CCD enabled; restore the original setting on release.
            Body->AddImpulse(Direction * 1600 - Body->GetPhysicsLinearVelocity(), NAME_None, true);
        }
    }
}
void UIBS4PowerComponent::SetLaser(bool Active)
{
    bLaser = Active && !IsHolding();
    if (!bLaser && Feedback) Feedback->Beam(FVector::ZeroVector, FVector::ZeroVector, false);
}
void UIBS4PowerComponent::Punch()
{
    if (IsHolding()) { Release(true); return; }
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now < NextPunch) return;
    NextPunch = Now + 0.45;
    FHitResult Hit; FVector Start, End;
    if (!Aim(330, Hit, Start, End)) return;
    if (auto* Receiver = Hit.GetActor()->FindComponentByClass<UIBS4ResponseComponent>())
    {
        Receiver->Receive(UIBS4ResponseComponent::MakeHit(GetWorld(), GetOwner(), EIBS4Weapon::Punch, 45, Hit));
        if (Feedback) Feedback->Impact(Hit.ImpactPoint, Receiver->Surface, true);
    }
}
void UIBS4PowerComponent::TickComponent(float Dt, ELevelTick Type, FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt, Type, Fn);
    const auto* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn) return;
    if (UPrimitiveComponent* Body = Held.Get())
    {
        if (!Body->IsSimulatingPhysics() || !Body->ComponentHasTag(TEXT("IBS4Grabbable"))) { Release(); return; }
        const FVector Origin = Pawn->GetActorLocation() + Pawn->GetActorUpVector() * 55;
        const FVector Desired = Origin + Pawn->GetControlRotation().Vector() * FMath::Max(CarryDistance, HeldRadius + 100);
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS4Cargo), false, Pawn);
        // A detached cannon still belongs to its robot actor. Ignore just the held
        // component so that the torso remains a blocker for sight/sweep/throw.
        Query.AddIgnoredComponent(Body);
        FHitResult Block, Sight;
        // Check both owner-to-target and actual cargo-to-target paths.
        const bool Occluded = GetWorld()->LineTraceSingleByChannel(Sight, Origin, Body->Bounds.Origin, ECC_Visibility, Query);
        const bool Blocked = GetWorld()->SweepSingleByChannel(Block, Body->Bounds.Origin, Desired, FQuat::Identity,
            ECC_PhysicsBody, FCollisionShape::MakeSphere(HeldRadius + 2), Query);
        if (Occluded || Block.bStartPenetrating || FVector::DistSquared(Origin, Body->Bounds.Origin) > FMath::Square(GrabRange + 150))
        { Release(); return; }
        const FVector Safe = Blocked ? FMath::Lerp(Body->Bounds.Origin, Desired, FMath::Max(0.f, Block.Time - 0.02f)) : Desired;
        Handle->SetTargetLocation(Safe);
        if (Blocked && FVector::DistSquared(Body->Bounds.Origin, Desired) > FMath::Square(180.f)) Release();
    }
    else if (Handle && Handle->GetGrabbedComponent()) Release();
    const double Now = GetWorld()->GetTimeSeconds();
    if (!bLaser || IsHolding() || Now < NextLaser) return;
    NextLaser = Now + 0.1; // no catch-up burst after a hitch, one trace/target per fixed sample
    FHitResult Hit; FVector Start, End;
    const bool Contact = Aim(3500, Hit, Start, End);
    if (Feedback) Feedback->Beam(Start, End, true);
    if (Contact)
    {
        if (CanonicalStrike.IsBound())
        {
            const bool Accepted = CanonicalStrike.Execute(UIBS4ResponseComponent::MakeHit(GetWorld(), GetOwner(), EIBS4Weapon::Laser, 9, Hit), Hit);
            if (Accepted && Feedback) Feedback->Impact(End, EIBS4Surface::Metal, false);
            return;
        }
        if (auto* Receiver = Hit.GetActor()->FindComponentByClass<UIBS4ResponseComponent>())
        {
            const float Amount = static_cast<float>(IBS4::laserDamage(static_cast<IBS4::Surface>(Receiver->Surface), 0.1));
            Receiver->Receive(UIBS4ResponseComponent::MakeHit(GetWorld(), GetOwner(), EIBS4Weapon::Laser, Amount, Hit));
            if (Feedback) Feedback->Impact(End, Receiver->Surface, false);
        }
    }
}
void UIBS4PowerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    Release();
    Super::EndPlay(Reason);
}
