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
        || !TargetId.IsValid() || !IsValid(Body) || Body->GetOwner() != GetOwner()
        || Message.TargetPart.IsStale() || (Message.TargetPart.IsValid() && Message.TargetPart.Get() != Body)
        || !IBS35::ValidDamageRequest(int(Message.DamageKind), int(EIBS35DamageKind::ThrownObject),
            Message.HealthDelta, Message.AttackSeconds, Health, InitialHealth)
        || Message.Impulse.ContainsNaN() || Message.Contact.ContainsNaN() || Message.RelativeVelocity.ContainsNaN()
        || !FMath::IsFinite(MaxDeltaSpeed) || MaxDeltaSpeed < 0
        || !Ledger.Accept(Message.AttackId)) return false;
    Message.DamageResponseProfile = ResponseProfile;
    Message.TargetPart = Body;
    Message.RequestedHealthDelta = Message.HealthDelta;
    Message.HealthDelta = static_cast<float>(IBS35::AppliedDamageDelta(Health, Message.HealthDelta, InitialHealth));
    Health += Message.HealthDelta;
    ++AcceptedHits;
    if (!Message.bPhysicsResolved && IsValid(Body) && Body->GetOwner() == GetOwner() && Body->IsSimulatingPhysics())
    {
        Message.Impulse = Message.Impulse.GetClampedToMaxSize(Body->GetMass() * MaxDeltaSpeed);
        Body->AddImpulseAtLocation(Message.Impulse, Message.Contact);
    }
    UE_LOG(LogTemp, Display, TEXT("IBS35 hit attack=%s target=%s part=%s kind=%d requestedDelta=%.2f healthDelta=%.2f hp=%.2f t=%.4f air=%d contact=%s velocity=%s material=%s physical=%d"),
        *Message.AttackId.ToString(), *TargetId.ToString(), *GetNameSafe(Body), int32(Message.DamageKind), Message.RequestedHealthDelta, Message.HealthDelta, Health,
        Message.AttackSeconds, Message.bAirborne, *Message.Contact.ToString(), *Message.RelativeVelocity.ToString(),
        *Message.SurfaceMaterial.ToString(), Message.bPhysicsResolved);
    OnDamage.Broadcast(Message);
    return true;
}
