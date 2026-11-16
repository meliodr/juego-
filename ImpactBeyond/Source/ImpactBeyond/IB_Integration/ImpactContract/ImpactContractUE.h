#pragma once

// Optional UE glue: include only after integrating BOTH labs. No auto-registration,
// generated code, new module, or modifications to either lab's public API.
#include "ImpactContract.h"
#include "IB_S35/IBS35Types.h"
#include "IB_S4/IBS4ResponseComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

namespace IBImpactUE
{
inline IBImpact::Guid Id(const FGuid& V) { return {V.A, V.B, V.C, V.D}; }
inline IBImpact::Vector Vec(const FVector& V) { return {V.X, V.Y, V.Z}; }
inline FVector Vec(const IBImpact::Vector& V) { return FVector(V.X, V.Y, V.Z); }
inline bool Kind(EIBS35DamageKind In, IBImpact::S35Kind& Out)
{
    switch (In)
    {
    case EIBS35DamageKind::QuickPunch: Out = IBImpact::S35Kind::QuickPunch; return true;
    case EIBS35DamageKind::HeavyPunch: Out = IBImpact::S35Kind::HeavyPunch; return true;
    case EIBS35DamageKind::HeldTool: Out = IBImpact::S35Kind::HeldTool; return true;
    case EIBS35DamageKind::ThrownObject: Out = IBImpact::S35Kind::ThrownObject; return true;
    default: return false;
    }
}
inline bool Kind(EIBS4Weapon In, IBImpact::S4Weapon& Out)
{
    switch (In)
    {
    case EIBS4Weapon::Bullet: Out = IBImpact::S4Weapon::Bullet; return true;
    case EIBS4Weapon::Missile: Out = IBImpact::S4Weapon::Missile; return true;
    case EIBS4Weapon::Laser: Out = IBImpact::S4Weapon::Laser; return true;
    case EIBS4Weapon::Punch: Out = IBImpact::S4Weapon::Punch; return true;
    default: return false;
    }
}
inline EIBS4Weapon Weapon(IBImpact::S4Weapon In)
{
    switch (In)
    {
    case IBImpact::S4Weapon::Bullet: return EIBS4Weapon::Bullet;
    case IBImpact::S4Weapon::Missile: return EIBS4Weapon::Missile;
    case IBImpact::S4Weapon::Laser: return EIBS4Weapon::Laser;
    case IBImpact::S4Weapon::Punch: return EIBS4Weapon::Punch;
    }
    checkNoEntry(); return EIBS4Weapon::Punch;
}
struct Record
{
    IBImpact::Envelope Request;
    IBImpact::RouteResult Result;
    // Local lookup hints only. Do not serialize pointers or feed them into identity keys.
    TWeakObjectPtr<AActor> Instigator;
    TWeakObjectPtr<UPrimitiveComponent> Part;
};
struct S4Identity
{
    // Capture alongside the contact/queued packet, NOT from the current target at replay time.
    FGuid SessionId;
    FGuid TargetId;
    FGuid InstigatorId;
    FGuid PartId;
    bool bPhysicsResolved = false; // Captured with the contact; never add a second S4 push.
};

// Own ONE instance per S4 receiver life, game thread. Route ALL mutating ingress
// through it. Existing S4 OnResponse continues to drive prepared destruction.
// S6/S7 consume Accepted Records as notifications, NEVER as another damage call.
class S4Bridge
{
    TWeakObjectPtr<UIBS4ResponseComponent> Receiver;
    TWeakObjectPtr<UObject> BindingOwner;
    FGuid SessionId, TargetId;
    FName Profile;
    IBImpact::TargetRouter<> Router;
    struct Binding
    {
        IBImpact::AttackId Attack;
        TWeakObjectPtr<AActor> Instigator;
        TWeakObjectPtr<UPrimitiveComponent> Part;
    };
    TArray<Binding> Bindings;

    Record Dispatch(IBImpact::Envelope Request, AActor* Instigator, UPrimitiveComponent* Part, double MaxDeltaSpeed)
    {
        check(IsInGameThread());
        Record Out; Out.Request = Request; Out.Instigator = Instigator; Out.Part = Part;
        auto* Target = Receiver.Get();
        if (!IsValid(Target) || !BindingOwner.IsValid() || !Target->HasConsumerBinding()
            || !Target->GetWorld() || !Request.Valid()
            || !FMath::IsFinite(MaxDeltaSpeed) || MaxDeltaSpeed < 0
            || (Part && (!IsValid(Part) || Part->GetOwner() != Target->GetOwner()))
            || (Instigator && !IsValid(Instigator))) return Out;
        // Freeze local identity alongside the portable request. A retry may not substitute a part/actor.
        Binding* Existing = Bindings.FindByPredicate([&](const Binding& B) { return B.Attack == Request.Attack; });
        if (Existing)
        {
            if (Existing->Part.IsStale() || Existing->Instigator.IsStale()
                || Existing->Part.Get() != Part || Existing->Instigator.Get() != Instigator) return Out;
        }
        else
        {
            if (Bindings.Num() >= 4096) { Out.Result.State = IBImpact::Status::Capacity; return Out; }
            Bindings.Add({Request.Attack, Instigator, Part});
        }
        const double Now = Target->GetWorld()->GetTimeSeconds();
        Out.Result = Router.Route(Request, Now,
            [&](const IBImpact::Envelope& E)
            {
                FHitResult Trace; Trace.ImpactPoint = Vec(E.Contact);
                // Sole allocator shared with S4's original producers. Never fabricate a serial.
                const auto Packet = UIBS4ResponseComponent::MakeHit(Target->GetWorld(), Instigator,
                    EIBS4Weapon::Punch, 0, Trace);
                return IBImpact::S4Packet{Packet.AttackId, Packet.IssuedAt};
            },
            [&](const IBImpact::Envelope& E, const IBImpact::S4Packet& Packet)
            {
                IBImpact::Outcome Applied;
                IBImpact::S4Weapon Mapped;
                if (!IBImpact::ToS4Weapon(E.Kind, Mapped)) return Applied;
                // Both legacy entry points use float amounts; reject unrepresentable values.
                if (-E.RequestedHealthDelta > TNumericLimits<float>::Max()) return Applied;
                FIBS4Hit Hit;
                Hit.AttackId = Packet.Serial; Hit.IssuedAt = Packet.IssuedAt;
                Hit.Weapon = Weapon(Mapped); Hit.Amount = static_cast<float>(-E.RequestedHealthDelta);
                Hit.Point = Vec(E.Contact);
                Hit.Direction = Vec(E.Direction).GetSafeNormal();
                Hit.Instigator = Instigator; Hit.Part = Part;
                // Receive is ingress: calling it here recurses through the binding.
                // Only this owner's bound endpoint activates H01 protection and commits.
                if (!Target->ConsumeBoundHit(BindingOwner.Get(), Hit, E.PhysicsResolved)) return Applied;
                Applied.Accepted = true;
                Applied.RequestedHealthDelta = Target->LastOutcome.RequestedHealthDelta;
                Applied.HealthDelta = Target->LastOutcome.HealthDelta;
                Applied.DeltaVelocity = Vec(Target->LastOutcome.DeltaVelocity);
                const bool Simulating = IsValid(Part) && Part->IsSimulatingPhysics();
                const auto Physical = IBImpact::PlanPhysicalImpulse(E, Simulating, Simulating ? Part->GetMass() : 0, MaxDeltaSpeed);
                Applied.PhysicalResponse = Physical.Disposition;
                if (Physical.Disposition == IBImpact::ImpulseDisposition::RequestedPhysical)
                {
                    Part->AddImpulseAtLocation(Vec(Physical.Impulse), Vec(E.Contact));
                    Applied.Impulse = Physical.Impulse;
                }
                Out.Request = E; // On retry report the frozen accepted request, not new caller values.
                return Applied;
            });
        return Out;
    }
public:
    S4Bridge(UIBS4ResponseComponent& Target, UObject& Owner, FGuid WorldSession, FGuid TargetLife, FName ResponseProfile)
        : Receiver(&Target), BindingOwner(&Owner), SessionId(WorldSession), TargetId(TargetLife), Profile(ResponseProfile),
          Router(Id(WorldSession), Id(TargetLife)) {}

    // Call at contact BEFORE any UIBS35ReceiverComponent::Receive. Raw HealthDelta
    // is the damage command here; RequestedHealthDelta is only populated by old receivers.
    Record ReceiveS35(const FIBS35DamageMessage& Input, UPrimitiveComponent* Body, FGuid PartId,
                      double MaxDeltaSpeed = 2000)
    {
        IBImpact::Envelope E;
        auto* Target = Receiver.Get();
        IBImpact::S35Kind Mapped;
        if (!IsValid(Target) || !Target->GetWorld() || Input.TargetId != TargetId || !IsValid(Body)
            || !PartId.IsValid() || Input.TargetPart.IsStale()
            || (Input.TargetPart.IsValid() && Input.TargetPart.Get() != Body)
            || !Kind(Input.DamageKind, Mapped) || !IBImpact::SetS35Request(Mapped, Input.HealthDelta, E)) return {};
        E.Attack = {IBImpact::Origin::S35, Id(SessionId), Id(Input.AttackId), 0};
        E.TargetId = Id(TargetId); E.InstigatorId = Id(Input.InstigatorId); E.PartId = Id(PartId);
        E.Contact = Vec(Input.Contact); E.Impulse = Vec(Input.Impulse);
        E.RelativeVelocity = Vec(Input.RelativeVelocity); E.HasRelativeVelocity = true;
        E.Direction = Vec(Input.RelativeVelocity.GetSafeNormal());
        E.AttackAgeSeconds = Input.AttackSeconds; E.HasAttackAge = true;
        E.ContactWorldSeconds = Target->GetWorld()->GetTimeSeconds();
        E.PhysicsResolved = Input.bPhysicsResolved; E.Airborne = Input.bAirborne;
        E.DamageResponseProfile = Profile.IsNone() ? "" : TCHAR_TO_UTF8(*Profile.ToString());
        E.SurfaceMaterial = TCHAR_TO_UTF8(*Input.SurfaceMaterial.ToString());
        return Dispatch(E, Input.InstigatorActor.Get(), Body, MaxDeltaSpeed);
    }

    // Replaces a direct S4 Receive call. Never call AFTER S4 has already applied it.
    Record ReceiveS4(const FIBS4Hit& Input, const S4Identity& Identity)
    {
        IBImpact::Envelope E;
        IBImpact::S4Weapon Mapped;
        if (Identity.SessionId != SessionId || Identity.TargetId != TargetId
            || !Kind(Input.Weapon, Mapped) || !IBImpact::SetS4Request(Mapped, Input.Amount, E)
            || (Input.Part && !Identity.PartId.IsValid())) return {};
        E.Attack = {IBImpact::Origin::S4, Id(SessionId), {}, Input.AttackId};
        E.TargetId = Id(TargetId); E.InstigatorId = Id(Identity.InstigatorId); E.PartId = Id(Identity.PartId);
        E.ContactWorldSeconds = Input.IssuedAt; E.Contact = Vec(Input.Point);
        E.Direction = Vec(Input.Direction); // Unitless; no relative speed supplied by S4.
        E.PhysicsResolved = Identity.bPhysicsResolved;
        E.DamageResponseProfile = Profile.IsNone() ? "" : TCHAR_TO_UTF8(*Profile.ToString());
        return Dispatch(E, Input.Instigator, Input.Part, 0);
    }
};
}
