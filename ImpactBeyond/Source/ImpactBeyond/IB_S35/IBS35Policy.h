#pragma once

// Small engine-independent rules shared by runtime and the lightweight executable tests.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace IBS35
{
enum class HoldState : std::uint8_t { Free, Held, Charging, Thrown };

inline bool ValidDamageRequest(int Kind, int LastSupportedKind, double RequestedDelta,
                               double AttackSeconds, double Health, double InitialHealth)
{
    return Kind >= 0 && Kind <= LastSupportedKind && std::isfinite(RequestedDelta) && RequestedDelta <= 0
        && std::isfinite(AttackSeconds) && AttackSeconds >= 0 && std::isfinite(Health)
        && std::isfinite(InitialHealth) && InitialHealth >= 0 && Health >= 0 && Health <= InitialHealth;
}

// Called only after validation; preserves requested damage separately in the UE envelope.
template<class Scalar> inline Scalar AppliedDamageDelta(Scalar Health, Scalar RequestedDelta, Scalar InitialHealth)
{
    // Keep the same scalar precision as stored health: sub-ULP damage must report zero loss.
    const Scalar Updated = std::clamp(Scalar(Health + RequestedDelta), Scalar(0), InitialHealth);
    return Updated - Health;
}

struct Lease
{
    HoldState State = HoldState::Free;
    std::uint64_t Carrier = 0;
    bool Claim(std::uint64_t Who)
    {
        if (!Who || Carrier || (State != HoldState::Free && State != HoldState::Thrown)) return false;
        Carrier = Who; State = HoldState::Held; return true;
    }
    bool BeginCharge(std::uint64_t Who)
    {
        if (!Who || Carrier != Who || State != HoldState::Held) return false;
        State = HoldState::Charging; return true;
    }
    bool CancelCharge(std::uint64_t Who)
    {
        if (!Who || Carrier != Who || State != HoldState::Charging) return false;
        State = HoldState::Held; return true;
    }
    bool Release(std::uint64_t Who, bool Launch)
    {
        if (!Who || Carrier != Who || (Launch && State != HoldState::Charging)) return false;
        Carrier = 0; State = Launch ? HoldState::Thrown : HoldState::Free; return true;
    }
    void Reset() { Carrier = 0; State = HoldState::Free; }
};

inline double ThrowSpeed(double MassKg, double Charge, double MinSpeed, double MaxSpeed)
{
    if (!std::isfinite(MassKg) || !std::isfinite(Charge) || !std::isfinite(MinSpeed)
        || !std::isfinite(MaxSpeed) || MassKg <= 0 || MinSpeed <= 0 || MaxSpeed < MinSpeed) return 0;
    const double Weight = std::clamp(std::sqrt(1200.0 / MassKg), 0.35, 1.25);
    return (MinSpeed + (MaxSpeed - MinSpeed) * std::clamp(Charge, 0.0, 1.0)) * Weight;
}

struct AttackTiming
{
    double Windup, Active, Recovery;
    bool Valid() const
    {
        return std::isfinite(Windup) && std::isfinite(Active) && std::isfinite(Recovery)
            && Windup >= 0 && Active > 0 && Recovery >= 0;
    }
    bool CrossesContact(double Before, double After) const
    {
        return Valid() && std::isfinite(Before) && std::isfinite(After)
            && After > Before && Before < Windup + Active && After >= Windup;
    }
    double Alpha(double Time) const { return std::clamp((Time - Windup) / Active, 0.0, 1.0); }
    double Duration() const { return Windup + Active + Recovery; }
};

// Never evict old IDs: a replay must not damage again. At capacity fail closed until an
// explicit target-life reset. The receiver renews TargetId on that reset.
template<class Key, std::size_t Capacity = 4096> class HitLedger
{
    std::vector<Key> Seen;
public:
    bool Accept(const Key& Id)
    {
        if (Seen.size() >= Capacity || std::find(Seen.begin(), Seen.end(), Id) != Seen.end()) return false;
        Seen.push_back(Id); return true;
    }
    void Reset() { Seen.clear(); }
    std::size_t Size() const { return Seen.size(); }
};
}
