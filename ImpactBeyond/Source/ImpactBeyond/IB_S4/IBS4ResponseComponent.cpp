#include "IBS4ResponseComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <atomic>

int64 UIBS4ResponseComponent::AllocateAttackId()
{
    static std::atomic<uint64> Sequence{0};
    const uint64 Next = ++Sequence;
    return Next <= static_cast<uint64>(MAX_int64) ? static_cast<int64>(Next) : 0;
}

FIBS4Hit UIBS4ResponseComponent::MakeHit(UWorld* World, AActor* Instigator, EIBS4Weapon Weapon, float Amount, const FHitResult& Trace)
{
    FIBS4Hit Hit;
    if (!World) return Hit;
    Hit.AttackId = AllocateAttackId();
    Hit.IssuedAt = World->GetTimeSeconds();
    Hit.Instigator = Instigator;
    Hit.Weapon = Weapon;
    Hit.Amount = Amount;
    Hit.Point = Trace.ImpactPoint;
    Hit.Direction = (Trace.TraceEnd - Trace.TraceStart).GetSafeNormal();
    Hit.Part = Trace.GetComponent();
    return Hit;
}

bool UIBS4ResponseComponent::Receive(const FIBS4Hit& Hit)
{
    if (bConsuming || bRouting) return false;
    if (!bConsumerBound) return ReceiveInternal(Hit, false, false);
    if (!ConsumerOwner.IsValid() || !ConsumerRouter.IsBound()) return false;
    TGuardValue<bool> RoutingGuard(bRouting, true);
    bRouteCommitted = false;
    RoutedAttackId = Hit.AttackId;
    RoutedIssuedAt = Hit.IssuedAt;
    ConsumerRouter.Execute(*this, Hit);
    // The router cannot claim acceptance without the S4 consumer committing it.
    return bRouteCommitted;
}

bool UIBS4ResponseComponent::BindConsumer(UObject* Owner, FIBS4IngressRouter Router)
{
    if (bConsumerBound || bRouting || bConsuming || !IsValid(Owner) || !Router.IsBound()) return false;
    ConsumerOwner = Owner;
    ConsumerRouter = MoveTemp(Router);
    bConsumerBound = true;
    return true;
}

bool UIBS4ResponseComponent::UnbindConsumer(UObject* Owner)
{
    if (!bConsumerBound || bRouting || bConsuming || !IsValid(Owner) || ConsumerOwner.Get() != Owner) return false;
    ConsumerRouter.Unbind();
    DamageFilter.Unbind();
    ConsumerOwner.Reset();
    bConsumerBound = false;
    return true;
}

bool UIBS4ResponseComponent::SetBoundDamageFilter(UObject* Owner, FIBS4BoundDamageFilter Filter)
{
    if (!bConsumerBound || bRouting || bConsuming || !IsValid(Owner) || ConsumerOwner.Get() != Owner) return false;
    DamageFilter = MoveTemp(Filter);
    return true;
}

bool UIBS4ResponseComponent::ConsumeBoundHit(UObject* Owner, const FIBS4Hit& Hit, bool bPhysicsAlreadyResolved)
{
    if (!bConsumerBound || bConsuming || !IsValid(Owner) || ConsumerOwner.Get() != Owner) return false;
    // A native routed contact must keep its original serial/time. A router may
    // reject it, but cannot manufacture a second contact inside the same call.
    if (bRouting && (bRouteCommitted || Hit.AttackId != RoutedAttackId || Hit.IssuedAt != RoutedIssuedAt)) return false;
    const bool Accepted = ReceiveInternal(Hit, bPhysicsAlreadyResolved, true);
    if (bRouting && Accepted) bRouteCommitted = true;
    return Accepted;
}

bool UIBS4ResponseComponent::ReceiveInternal(const FIBS4Hit& Hit, bool bPhysicsAlreadyResolved, bool bProtectAllH01)
{
    TGuardValue<bool> ConsumingGuard(bConsuming, true);
    if (!GetWorld() || Hit.AttackId <= 0 || Hit.Point.ContainsNaN() || Hit.Direction.ContainsNaN() || !IBS4::validHealth(Health)) return false;
    if (Hit.Part && (!IsValid(Hit.Part) || Hit.Part->GetOwner() != GetOwner())) return false;
    IBS4::Weapon Weapon;
    // Explicitly translate this lab's enum. Never cast S3.5 ordinals into this API.
    switch (Hit.Weapon)
    {
    case EIBS4Weapon::Bullet: Weapon = IBS4::Weapon::Bullet; break;
    case EIBS4Weapon::Missile: Weapon = IBS4::Weapon::Missile; break;
    case EIBS4Weapon::Laser: Weapon = IBS4::Weapon::Laser; break;
    case EIBS4Weapon::Punch: Weapon = IBS4::Weapon::Punch; break;
    default: return false;
    }
    const auto Result = Policy.receive(static_cast<uint64>(Hit.AttackId), Hit.IssuedAt, GetWorld()->GetTimeSeconds(),
        Weapon, bH01, Hit.Amount, MissilePushSpeed, PushCooldown, bPhysicsAlreadyResolved, bProtectAllH01);
    if (!Result.accepted) return false;
    ++AcceptedHits;
    const float RequestedDamage = static_cast<float>(-Result.healthDelta);
    float RemainingDamage = RequestedDamage;
    if (bConsumerBound && DamageFilter.IsBound())
    {
        RemainingDamage = DamageFilter.Execute(Hit, RequestedDamage);
        RemainingDamage = FMath::IsFinite(RemainingDamage) ? FMath::Clamp(RemainingDamage, 0.f, RequestedDamage) : 0.f;
    }
    const IBS4::HealthChange Change = IBS4::resolveHealth(Health, -RemainingDamage);
    Health = Change.next;
    FIBS4Outcome Outcome;
    Outcome.RequestedHealthDelta = -RemainingDamage;
    Outcome.HealthDelta = Change.delta;
    if (Result.pushSpeed > 0)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            auto* Movement = Character->GetCharacterMovement();
            const FVector Up = -Movement->GetGravityDirection();
            FVector Direction = FVector::VectorPlaneProject(Hit.Direction, Up).GetSafeNormal();
            if (Direction.IsNearlyZero()) Direction = FVector::VectorPlaneProject(Character->GetActorForwardVector(), Up).GetSafeNormal();
            Direction += Up * .16;
            // CMC owns swept movement; no teleport, ragdoll, input lock, or forced Walking.
            Outcome.DeltaVelocity = Direction.GetSafeNormal() * Result.pushSpeed;
            Movement->AddImpulse(Outcome.DeltaVelocity, true);
            ++AppliedPushes;
        }
    }
    LastOutcome = Outcome;
    OnResponse.Broadcast(Hit, Outcome);
    UE_LOG(LogTemp, Display, TEXT("IBS4 Hit id=%lld instigator=%s target=%s weapon=%d requestedDelta=%.1f healthDelta=%.1f health=%.1f deltaV_cm_s=%.1f"),
        Hit.AttackId, *GetNameSafe(Hit.Instigator), *GetNameSafe(GetOwner()), static_cast<int32>(Hit.Weapon),
        Outcome.RequestedHealthDelta, Outcome.HealthDelta, Health, Outcome.DeltaVelocity.Size());
    return true;
}

void UIBS4ResponseComponent::RestoreHealth()
{
    Health = 100;
    // Keep the replay ledger and cooldown: a reset must not make an in-flight hit new.
}
