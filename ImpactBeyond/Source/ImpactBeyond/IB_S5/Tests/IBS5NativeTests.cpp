#if defined(IBS5_NATIVE_TEST_MAIN)
#include "../IBS5Policy.h"
#include <cstdlib>
#include <iostream>
#include <limits>

static int Checks = 0;
static void Check(bool Passed)
{
    ++Checks;
    if (!Passed) { std::cerr << "Failure " << Checks << '\n'; std::exit(1); }
}
int main()
{
    for (double Minutes : {18.0, 36.0, 72.0})
    {
        IBS5::Clock Clock;
        Clock.SetDuration(Minutes);
        for (int Cycle = 0; Cycle < 3; ++Cycle)
        {
            for (int Step = 0; Step < 3600; ++Step) Clock.Tick(Minutes * 60.0 / 3600.0);
            Check(std::abs(Clock.Phase - 0.5) < 1e-10);
        }
        Clock.Paused = true;
        const double Before = Clock.Phase;
        Clock.Tick(1000.0);
        Check(Clock.Phase == Before);
        Clock.Tick(std::numeric_limits<double>::quiet_NaN());
        Check(Clock.Phase == Before);
        for (int Save = 0; Save < 10; ++Save)
        {
            IBS5::Clock Restored;
            Restored.Restore(Clock.Phase, Clock.DayMinutes, Clock.Paused);
            Check(Restored.Phase == Clock.Phase && Restored.DayMinutes == Minutes && Restored.Paused);
        }
    }
    IBS5::Clock Clock;
    Clock.Restore(std::numeric_limits<double>::quiet_NaN(), 0.0, false);
    Check(Clock.Phase == 0.5 && Clock.DayMinutes == 36.0);
    for (double Hour : {0.0, 6.0, 12.0, 18.0, 24.0})
    {
        Clock.Phase = IBS5::Wrap01(Hour / 24.0);
        Check(std::abs(Clock.LocalHour(0.0) - std::fmod(Hour, 24.0)) < 1e-10);
        Check(std::abs(Clock.LocalHour(3.141592653589793) - Clock.LocalHour(-3.141592653589793)) < 1e-10);
    }
    IBS5::Incident Incident;
    // The same six-hour adjustment must apply in every region, including midnight wrap.
    for (double Longitude : {0.0, 1.5707963267948966, -1.5707963267948966, 3.141592653589793})
    {
        Clock.Restore(0.85, 36.0, true);
        const double Before = Clock.LocalHour(Longitude);
        Clock.AdvanceHours(6.0);
        Check(std::abs(Clock.LocalHour(Longitude) - std::fmod(Before + 6.0, 24.0)) < 1e-10);
        Clock.AdvanceHours(-6.0);
        Check(std::abs(Clock.LocalHour(Longitude) - Before) < 1e-10);
        const double Phase = Clock.Phase;
        Clock.AdvanceHours(std::numeric_limits<double>::quiet_NaN());
        Check(Clock.Phase == Phase && Clock.Paused);
    }
    Check(!Incident.Begin(true));
    for (int Index = 0; Index < 20; ++Index)
    {
        Incident.Tick(120.0);
        Check(!Incident.Begin(false));
        Check(Incident.Begin(true));
        Check(!Incident.Begin(true));
        Check(!Incident.Resolve(true, 601.0));
        Check(!Incident.Resolve(false, 10.0));
        Check(!Incident.Resolve(true, std::numeric_limits<double>::quiet_NaN()));
        if (Index % 2 == 0)
        {
            Check(Incident.Resolve(true, 500.0));
            Check(!Incident.Resolve(true, 500.0));
            Check(Incident.Stage == IBS5::IncidentStage::Resolved);
        }
        else
        {
            Incident.Tick(90.0);
            Check(Incident.Stage == IBS5::IncidentStage::Ignored);
            Check(!Incident.Resolve(true, 0.0));
        }
        Check(!Incident.Begin(true));
    }
    Check(Incident.Sequence == 20);
    Check(IBS5::WitnessedDisturbance(false, true, 5).Reputation == 0.0);
    Check(IBS5::WitnessedDisturbance(false, true, 5).Heat == 0.0);
    Check(IBS5::WitnessedDisturbance(true, false, 5).Heat == 0.0);
    Check(IBS5::WitnessedDisturbance(true, true, 5).Reputation == -8.0);
    Check(IBS5::WitnessedDisturbance(true, true, 5).Heat == 40.0);
    Check(IBS5::ClampReputation(150.0) == 100.0);
    Check(IBS5::ClampReputation(-150.0) == -100.0);
    Check(IBS5::AlertLevel(0) == 0 && IBS5::AlertLevel(100) == 5);
    std::cout << Checks << " S5 native checks PASS\n";
}
#endif
