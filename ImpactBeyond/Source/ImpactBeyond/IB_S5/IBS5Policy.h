#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace IBS5
{
inline double Wrap01(double Value)
{
    if (!std::isfinite(Value)) return 0.5;
    return Value - std::floor(Value);
}

struct Clock
{
    double Phase = 0.5;
    double DayMinutes = 36.0;
    bool Paused = false;

    void SetDuration(double Minutes)
    {
        DayMinutes = Minutes == 18.0 || Minutes == 72.0 ? Minutes : 36.0;
    }
    void Restore(double SavedPhase, double SavedDuration, bool SavedPaused)
    {
        Phase = Wrap01(SavedPhase);
        SetDuration(SavedDuration);
        Paused = SavedPaused;
    }
    void Tick(double Seconds)
    {
        if (!Paused && std::isfinite(Seconds) && Seconds > 0.0)
            Phase = Wrap01(Phase + Seconds / (DayMinutes * 60.0));
    }
    void AdvanceHours(double Hours)
    {
        if (std::isfinite(Hours)) Phase = Wrap01(Phase + Hours / 24.0);
    }
    double LocalHour(double LongitudeRadians) const
    {
        const double Longitude = std::isfinite(LongitudeRadians) ? LongitudeRadians : 0.0;
        return Wrap01(Phase + Longitude / 6.28318530717958647692) * 24.0;
    }
};

enum class IncidentStage : std::uint8_t { Waiting, Active, Resolved, Ignored };

struct Incident
{
    IncidentStage Stage = IncidentStage::Waiting;
    double Remaining = 45.0;
    std::uint32_t Sequence = 0;
    bool PlayerCaused = false;
    bool Rescue = false;

    bool Begin(bool RegionReady)
    {
        if (!RegionReady || Stage == IncidentStage::Active || Remaining > 0.0) return false;
        ++Sequence;
        Rescue = (Sequence % 2) == 0;
        PlayerCaused = false;
        Stage = IncidentStage::Active;
        Remaining = 90.0;
        return true;
    }
    bool Resolve(bool ValidHelper, double DistanceCm)
    {
        if (Stage != IncidentStage::Active || !ValidHelper || !std::isfinite(DistanceCm)
            || DistanceCm > 600.0 || DistanceCm < 0.0) return false;
        Stage = IncidentStage::Resolved;
        Remaining = 120.0;
        return true;
    }
    void Tick(double Seconds)
    {
        if (!std::isfinite(Seconds) || Seconds <= 0.0) return;
        Remaining = std::max(0.0, Remaining - Seconds);
        if (Stage == IncidentStage::Active && Remaining == 0.0)
        {
            Stage = IncidentStage::Ignored;
            Remaining = 120.0;
        }
    }
};

inline double ClampReputation(double Value)
{
    return std::isfinite(Value) ? std::clamp(Value, -100.0, 100.0) : 0.0;
}
inline int AlertLevel(double Heat)
{
    if (Heat >= 80.0) return 5;
    if (Heat >= 60.0) return 4;
    if (Heat >= 40.0) return 3;
    if (Heat >= 25.0) return 2;
    if (Heat >= 10.0) return 1;
    return 0;
}

struct Consequence
{
    double Heat = 0.0;
    double Reputation = 0.0;
};
inline Consequence WitnessedDisturbance(bool PlayerCaused, bool Enabled, double Severity)
{
    if (!PlayerCaused || !Enabled || !std::isfinite(Severity) || Severity <= 0.0) return {};
    Severity = std::clamp(Severity, 0.0, 5.0);
    return {Severity * 8.0, -std::min(8.0, Severity * 2.0)};
}
}
