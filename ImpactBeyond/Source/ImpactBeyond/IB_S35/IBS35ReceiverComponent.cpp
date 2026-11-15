#include "IB_S35/IBS35ReceiverComponent.h"
#include "Components/PrimitiveComponent.h"

void UIBS35ReceiverComponent::BeginPlay() { Super::BeginPlay(); ResetReceiver(); }

void UIBS35ReceiverComponent::ResetReceiver()
{
    TargetId = FGuid::NewGuid(); // New life: stale in-flight messages cannot hit a reset target.
    Ledger.Reset(); AcceptedHits = 0;
    Health = FMath::IsFinite(InitialHealth) ? FMath::Max(0.f, InitialHealth) : 1000;
}

bool UIBS35ReceiverComponent::Receive(FIBS35DamageMessage& Message, UPrimitiveComponent* Body)
{
    if (!Message.AttackId.IsValid() || !Message.InstigatorId.IsValid() || Message.TargetId != TargetId
        || !TargetId.IsValid() || !FMath::IsFinite(Message.HealthDelta) || Message.HealthDelta > 0
        || Message.Impulse.ContainsNaN() || Message.Contact.ContainsNaN() || Message.RelativeVelocity.ContainsNaN()
        || !FMath::IsFinite(Message.AttackSeconds) || !FMath::IsFinite(InitialHealth) || InitialHealth < 0
        || !FMath::IsFinite(MaxDeltaSpeed) || MaxDeltaSpeed < 0
        || !Ledger.Accept(Message.AttackId)) return false;
    Message.DamageResponseProfile = ResponseProfile;
    const float PreviousHealth = Health;
    Health = FMath::Clamp(Health + Message.HealthDelta, 0.f, FMath::Max(0.f, InitialHealth));
    Message.HealthDelta = Health - PreviousHealth;
    ++AcceptedHits;
    if (!Message.bPhysicsResolved && IsValid(Body) && Body->GetOwner() == GetOwner() && Body->IsSimulatingPhysics())
    {
        Message.Impulse = Message.Impulse.GetClampedToMaxSize(Body->GetMass() * MaxDeltaSpeed);
        Body->AddImpulseAtLocation(Message.Impulse, Message.Contact);
    }
    UE_LOG(LogTemp, Display, TEXT("IBS35 hit attack=%s target=%s kind=%d healthDelta=%.2f hp=%.2f t=%.4f air=%d contact=%s velocity=%s material=%s physical=%d"),
        *Message.AttackId.ToString(), *TargetId.ToString(), int32(Message.DamageKind), Message.HealthDelta, Health,
        Message.AttackSeconds, Message.bAirborne, *Message.Contact.ToString(), *Message.RelativeVelocity.ToString(),
        *Message.SurfaceMaterial.ToString(), Message.bPhysicsResolved);
    OnDamage.Broadcast(Message);
    return true;
}
