#include "IBS4ResponseComponent.h"
#include "Engine/World.h"
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
    if (!GetWorld() || Hit.AttackId <= 0 || Hit.Point.ContainsNaN() || Hit.Direction.ContainsNaN()) return false;
    const auto Result = Policy.receive(static_cast<uint64>(Hit.AttackId), Hit.IssuedAt, GetWorld()->GetTimeSeconds(),
        static_cast<IBS4::Weapon>(Hit.Weapon), bH01, Hit.Amount, MissilePushSpeed, PushCooldown);
    if (!Result.accepted) return false;
    ++AcceptedHits;
    Health = FMath::Clamp(Health + static_cast<float>(Result.healthDelta), 0.f, 100.f);
    if (Result.pushSpeed > 0)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            FVector Direction(Hit.Direction.X, Hit.Direction.Y, 0);
            Direction = Direction.GetSafeNormal(0.001, FVector::ForwardVector);
            Direction.Z = 0.16; // never a downward missile impulse, including at floor level
            auto* Movement = Character->GetCharacterMovement();
            // CMC owns swept movement; no teleport, ragdoll, input lock, or forced Walking.
            Movement->AddImpulse(Direction.GetSafeNormal() * Result.pushSpeed, true);
            ++AppliedPushes;
        }
    }
    OnResponse.Broadcast(Hit, static_cast<float>(Result.healthDelta));
    UE_LOG(LogTemp, Display, TEXT("IBS4 Hit id=%lld instigator=%s target=%s weapon=%d healthDelta=%.1f health=%.1f push=%.1f"),
        Hit.AttackId, *GetNameSafe(Hit.Instigator), *GetNameSafe(GetOwner()), static_cast<int32>(Hit.Weapon),
        Result.healthDelta, Health, Result.pushSpeed);
    return true;
}

void UIBS4ResponseComponent::RestoreHealth()
{
    Health = 100;
    // Keep the replay ledger and cooldown: a reset must not make an in-flight hit new.
}
