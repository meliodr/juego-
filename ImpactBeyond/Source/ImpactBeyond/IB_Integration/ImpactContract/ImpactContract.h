#pragma once

// Offline contract v1. No engine dependency, hash/truncation of identities, or health mutation.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace IBImpact
{
struct Guid
{
    std::uint32_t A = 0, B = 0, C = 0, D = 0;
    bool Valid() const { return A || B || C || D; }
    bool operator==(const Guid& R) const { return A == R.A && B == R.B && C == R.C && D == R.D; }
};
enum class Origin : std::uint8_t { S35, S4 };
enum class DamageKind : std::uint8_t { QuickPunch, HeavyPunch, HeldTool, ThrownObject, Bullet, Missile, Laser, Punch };
enum class S35Kind : std::uint8_t { QuickPunch, HeavyPunch, HeldTool, ThrownObject };
enum class S4Weapon : std::uint8_t { Bullet, Missile, Laser, Punch };

struct AttackId
{
    Origin Source = Origin::S35;
    Guid Session; // New world/session; never a per-contact GUID.
    Guid NativeGuid;
    std::int64_t NativeSerial = 0;
    bool Valid() const
    {
        return Session.Valid() && ((Source == Origin::S35 && NativeGuid.Valid() && NativeSerial == 0)
            || (Source == Origin::S4 && !NativeGuid.Valid() && NativeSerial > 0));
    }
    bool operator==(const AttackId& R) const
    {
        return Source == R.Source && Session == R.Session && NativeGuid == R.NativeGuid && NativeSerial == R.NativeSerial;
    }
};
struct Vector
{
    double X = 0, Y = 0, Z = 0;
    bool Valid() const { return std::isfinite(X) && std::isfinite(Y) && std::isfinite(Z); }
    bool operator==(const Vector& R) const { return X == R.X && Y == R.Y && Z == R.Z; }
    bool Zero() const { return X == 0 && Y == 0 && Z == 0; }
};
// Conversion requires the receiving body's actual mass. Never infer it from a car profile.
inline bool ImpulseToDeltaVelocity(Vector ImpulseKgCmS, double MassKg, Vector& OutCmS)
{
    if (!ImpulseKgCmS.Valid() || !std::isfinite(MassKg) || MassKg <= 0) return false;
    const Vector Value{ImpulseKgCmS.X / MassKg, ImpulseKgCmS.Y / MassKg, ImpulseKgCmS.Z / MassKg};
    if (!Value.Valid()) return false;
    OutCmS = Value; return true;
}
inline bool DeltaVelocityToImpulse(Vector DeltaCmS, double MassKg, Vector& OutKgCmS)
{
    if (!DeltaCmS.Valid() || !std::isfinite(MassKg) || MassKg <= 0) return false;
    const Vector Value{DeltaCmS.X * MassKg, DeltaCmS.Y * MassKg, DeltaCmS.Z * MassKg};
    if (!Value.Valid()) return false;
    OutKgCmS = Value; return true;
}
inline bool ValidDeliveryTime(double IssuedAt, double Now)
{
    return std::isfinite(Now) && std::isfinite(IssuedAt) && Now >= 0 && IssuedAt >= 0
        && IssuedAt <= Now + 0.01 && Now - IssuedAt <= 2.0;
}
struct Envelope
{
    AttackId Attack;
    Guid TargetId; // Target LIFE, shared by all its parts. Health restore does not renew it.
    Guid InstigatorId; // Zero explicitly means unattributed (legal for S4, not S35).
    Guid PartId; // Stable local part identity, optional; never part of damage dedup key.
    DamageKind Kind = DamageKind::QuickPunch;
    double RequestedHealthDelta = 0; // Input request <= 0; NOT actual HP loss.
    Vector Impulse; // kg*cm/s. S4 missile delta-V is represented only in Outcome.
    Vector Contact; // cm
    Vector Direction; // Unitless source direction, separate from measured relative velocity.
    Vector RelativeVelocity; // cm/s; zero + false means not supplied by S4.
    bool HasRelativeVelocity = false;
    bool PhysicsResolved = false; // Chaos already applied the collision response.
    bool Airborne = false;
    double AttackAgeSeconds = 0;
    bool HasAttackAge = false; // S4 has contact issuance, not attack/launch start time.
    double ContactWorldSeconds = 0; // World clock, not relative age or wall clock.
    std::string DamageResponseProfile; // Explicit consumer binding; not inferred from material.
    std::string SurfaceMaterial;
    bool Valid() const
    {
        const bool KindMatches = Attack.Source == Origin::S35
            ? static_cast<unsigned>(Kind) <= static_cast<unsigned>(DamageKind::ThrownObject)
            : Kind == DamageKind::Bullet || Kind == DamageKind::Missile || Kind == DamageKind::Laser || Kind == DamageKind::Punch;
        return Attack.Valid() && TargetId.Valid() && KindMatches
            && (Attack.Source != Origin::S35 || InstigatorId.Valid())
            && std::isfinite(RequestedHealthDelta) && RequestedHealthDelta <= 0
            && std::isfinite(AttackAgeSeconds) && AttackAgeSeconds >= 0
            && std::isfinite(ContactWorldSeconds) && ContactWorldSeconds >= 0
            && Impulse.Valid() && Contact.Valid() && Direction.Valid() && RelativeVelocity.Valid()
            && !DamageResponseProfile.empty();
    }
};
inline bool SetS35Request(S35Kind Kind, double NegativeDelta, Envelope& Out)
{
    DamageKind Mapped;
    switch (Kind)
    {
    case S35Kind::QuickPunch: Mapped = DamageKind::QuickPunch; break;
    case S35Kind::HeavyPunch: Mapped = DamageKind::HeavyPunch; break;
    case S35Kind::HeldTool: Mapped = DamageKind::HeldTool; break;
    case S35Kind::ThrownObject: Mapped = DamageKind::ThrownObject; break;
    default: return false;
    }
    if (!std::isfinite(NegativeDelta) || NegativeDelta > 0) return false;
    Out.Kind = Mapped; Out.RequestedHealthDelta = NegativeDelta; return true;
}
inline bool SetS4Request(S4Weapon Weapon, double PositiveAmount, Envelope& Out)
{
    DamageKind Mapped;
    switch (Weapon)
    {
    case S4Weapon::Bullet: Mapped = DamageKind::Bullet; break;
    case S4Weapon::Missile: Mapped = DamageKind::Missile; break;
    case S4Weapon::Laser: Mapped = DamageKind::Laser; break;
    case S4Weapon::Punch: Mapped = DamageKind::Punch; break;
    default: return false;
    }
    if (!std::isfinite(PositiveAmount) || PositiveAmount < 0) return false;
    Out.Kind = Mapped; Out.RequestedHealthDelta = -PositiveAmount; return true;
}
inline bool ToS4Weapon(DamageKind Kind, S4Weapon& Out)
{
    switch (Kind)
    {
    case DamageKind::QuickPunch: case DamageKind::HeavyPunch: case DamageKind::HeldTool:
    case DamageKind::ThrownObject: case DamageKind::Punch: Out = S4Weapon::Punch; return true;
    case DamageKind::Bullet: Out = S4Weapon::Bullet; return true;
    case DamageKind::Missile: Out = S4Weapon::Missile; return true;
    case DamageKind::Laser: Out = S4Weapon::Laser; return true;
    default: return false;
    }
}
enum class ImpulseDisposition { None, AlreadyResolved, RequestedPhysical, NotSimulating, InvalidBodyMass };
struct PhysicalPlan
{
    ImpulseDisposition Disposition = ImpulseDisposition::None;
    Vector Impulse;
};
// Same physical clamp used by the optional UE bridge and native tests. This plans
// an additional effect; it does not simulate motion or change a body's velocity.
inline PhysicalPlan PlanPhysicalImpulse(const Envelope& E, bool Simulating, double MassKg, double MaxDeltaSpeedCmS)
{
    if (E.PhysicsResolved) return {ImpulseDisposition::AlreadyResolved, {}};
    if (E.Impulse.Zero()) return {};
    if (!Simulating) return {ImpulseDisposition::NotSimulating, {}};
    if (!E.Impulse.Valid() || !std::isfinite(MassKg) || MassKg <= 0
        || !std::isfinite(MaxDeltaSpeedCmS) || MaxDeltaSpeedCmS < 0
        || !std::isfinite(MassKg * MaxDeltaSpeedCmS)) return {ImpulseDisposition::InvalidBodyMass, {}};
    const double MaxComponent = std::max({std::abs(E.Impulse.X), std::abs(E.Impulse.Y), std::abs(E.Impulse.Z)});
    const Vector Scaled{E.Impulse.X / MaxComponent, E.Impulse.Y / MaxComponent, E.Impulse.Z / MaxComponent};
    const double ScaledNorm = std::sqrt(Scaled.X * Scaled.X + Scaled.Y * Scaled.Y + Scaled.Z * Scaled.Z);
    const double ComponentLimit = MassKg * MaxDeltaSpeedCmS / ScaledNorm;
    if (MaxComponent <= ComponentLimit) return {ImpulseDisposition::RequestedPhysical, E.Impulse};
    // Avoid overflow in |J| even for finite components near DBL_MAX.
    return {ImpulseDisposition::RequestedPhysical, {Scaled.X * ComponentLimit, Scaled.Y * ComponentLimit, Scaled.Z * ComponentLimit}};
}
struct Outcome
{
    bool Accepted = false;
    double RequestedHealthDelta = 0; // AFTER receiver immunity/cap policy; structural resistance only.
    double HealthDelta = 0; // Actual stored HP difference, not another damage command.
    Vector Impulse; // Additional physical impulse requested by the adapter, kg*cm/s.
    Vector DeltaVelocity; // S4 CMC requested velocity change, cm/s. Never automatically multiplied.
    ImpulseDisposition PhysicalResponse = ImpulseDisposition::None;
    bool Valid() const
    {
        return std::isfinite(RequestedHealthDelta) && RequestedHealthDelta <= 0
            // Float HP rounding can make actual loss slightly exceed the request.
            && std::isfinite(HealthDelta) && HealthDelta <= 0
            && Impulse.Valid() && DeltaVelocity.Valid();
    }
};
struct S4Packet { std::int64_t Serial = 0; double IssuedAt = 0; };
enum class Status { Accepted, Invalid, Duplicate, Capacity, Busy, TransportInvalid, ReceiverRejected, OutcomeInvalid };
struct RouteResult { Status State = Status::Invalid; Outcome Applied; S4Packet Packet; };

// One router per target life, game-thread only. One mutating receiver callback.
// Entries never evict: replay protection survives RestoreHealth and timestamp refresh.
// All copies/moves disabled: accidental copies would fork the replay ledger.
template<std::size_t Capacity = 4096> class TargetRouter
{
    struct Entry { Envelope Request; S4Packet Packet; bool Done = false; };
    Guid Session, Target;
    std::vector<Entry> Entries;
    bool Dispatching = false;
    double LastNow = -1;
public:
    TargetRouter(Guid InSession, Guid InTarget) : Session(InSession), Target(InTarget) {}
    TargetRouter(const TargetRouter&) = delete;
    TargetRouter& operator=(const TargetRouter&) = delete;
    std::size_t Size() const { return Entries.size(); }

    // Allocator must be UIBS4ResponseComponent::MakeHit (global serial allocator), once
    // per Attack/Target life at first CONTACT. It must not generate its own counter.
    // S4-origin inputs retain their original serial and IssuedAt; never allocate anew.
    // Retry uses the frozen first request+packet. A rejected retry cannot change damage.
    template<class Allocate, class Consume>
    RouteResult Route(const Envelope& Input, double Now, Allocate&& Allocator, Consume&& Receiver)
    {
        if (!Input.Valid() || !(Input.Attack.Session == Session) || !(Input.TargetId == Target)
            || !std::isfinite(Now) || Now < 0 || Now < LastNow || Input.ContactWorldSeconds > Now + 0.01) return {};
        if (Dispatching) return {Status::Busy, {}, {}};
        LastNow = Now;
        auto It = std::find_if(Entries.begin(), Entries.end(), [&](const Entry& E) { return E.Request.Attack == Input.Attack; });
        if (It != Entries.end() && It->Done) return {Status::Duplicate, {}, It->Packet};
        // Guard allocation too, including a re-entrant producer callback.
        Dispatching = true;
        struct Guard { bool& Flag; ~Guard() { Flag = false; } } Scope{Dispatching};
        if (It == Entries.end())
        {
            if (Entries.size() >= Capacity) return {Status::Capacity, {}, {}};
            S4Packet Packet;
            if (Input.Attack.Source == Origin::S4) Packet = {Input.Attack.NativeSerial, Input.ContactWorldSeconds};
            else
            {
                // Do not launder an old captured contact into a fresh S4 packet.
                if (!ValidDeliveryTime(Input.ContactWorldSeconds, Now)) return {Status::TransportInvalid, {}, {}};
                Packet = Allocator(Input);
            }
            Entries.push_back({Input, Packet, false});
            It = Entries.end() - 1;
        }
        if (It->Packet.Serial <= 0 || !ValidDeliveryTime(It->Packet.IssuedAt, Now))
            return {Status::TransportInvalid, {}, It->Packet};
        // Seal before invoking user code: an exception/uncertain mutation fails closed.
        It->Done = true;
        const Outcome Applied = Receiver(It->Request, It->Packet);
        if (!Applied.Accepted)
        {
            It->Done = false; // Consumer promises false means NO mutation.
            return {Status::ReceiverRejected, {}, It->Packet};
        }
        if (!Applied.Valid()) return {Status::OutcomeInvalid, {}, It->Packet};
        return {Status::Accepted, Applied, It->Packet};
    }
};
}
