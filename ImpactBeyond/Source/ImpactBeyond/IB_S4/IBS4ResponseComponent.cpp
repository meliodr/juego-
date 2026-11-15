#include "IBS4ResponseComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <atomic>

FIBS4Hit UIBS4ResponseComponent::MakeHit(UWorld* World, AActor* Instigator, EIBS4Weapon Weapon, float Amount, const FHitResult& Trace)
{
    static std::atomic<int64> Sequence{0};
    FIBS4Hit Hit;
    Hit.AttackId = ++Sequence;
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
        Weapon, bH01, Hit.Amount, MissilePushSpeed, PushCooldown);
    if (!Result.accepted) return false;
    ++AcceptedHits;
    const IBS4::HealthChange Change = IBS4::resolveHealth(Health, Result.healthDelta);
    Health = Change.next;
    FIBS4Outcome Outcome;
    Outcome.RequestedHealthDelta = static_cast<float>(Result.healthDelta);
    Outcome.HealthDelta = Change.delta;
    if (Result.pushSpeed > 0)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            FVector Direction(Hit.Direction.X, Hit.Direction.Y, 0);
            Direction = Direction.GetSafeNormal(0.001, FVector::ForwardVector);
            Direction.Z = 0.16; // never a downward missile impulse, including at floor level
            auto* Movement = Character->GetCharacterMovement();
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
