#pragma once
#include <algorithm>
#include <cmath>

namespace IBS35
{
// Shared by the timed component and its small native regression test.
struct PunchCombo
{
    int Index = 0;
    double Remaining = 0;
    int Begin() { Index = Remaining > 0 ? Index % 3 + 1 : 1; Remaining = 0; return Index; }
    void Complete() { Remaining = 0.8; }
    void Reset() { Index = 0; Remaining = 0; }
    void Tick(double Seconds)
    {
        if (std::isfinite(Seconds) && Seconds > 0) Remaining = std::max(0.0, Remaining - Seconds);
    }
};
inline double PunchCharge(double Charge) { return std::clamp(Charge, 0.0, 1.0); }
inline double ChargedDamageScale(double Charge) { return 0.65 + 0.7 * PunchCharge(Charge); }
inline double ChargedImpulseScale(double Charge) { return 0.7 + 0.7 * PunchCharge(Charge); }
}
