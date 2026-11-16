// Optional integration fixture. ImpactContract.h is read from the S3.5 checkout;
// it is NOT copied into S4 and this executable is NOT the UE S4Bridge.
#include "ImpactContract.h"
#include "IBS4Policy.h"
#include <iostream>
#include <stdexcept>

namespace
{
int Checks = 0;
void Check(bool Condition, const char* Message)
{
    ++Checks;
    if (!Condition) throw std::runtime_error(Message);
}
const IBImpact::Guid Session{1, 2, 3, 4}, Life{5, 6, 7, 8};
IBImpact::Envelope Native(std::int64_t Serial, double Time, IBImpact::S4Weapon Weapon)
{
    IBImpact::Envelope E;
    E.Attack = {IBImpact::Origin::S4, Session, {}, Serial};
    E.TargetId = Life; E.ContactWorldSeconds = Time; E.DamageResponseProfile = "S4BoundH01";
    Check(IBImpact::SetS4Request(Weapon, 60, E), "canonical native request");
    return E;
}
IBImpact::Envelope Foreign(IBImpact::Guid Attack, double Time, IBImpact::S35Kind Kind)
{
    IBImpact::Envelope E;
    E.Attack = {IBImpact::Origin::S35, Session, Attack, 0};
    E.TargetId = Life; E.InstigatorId = {9, 10, 11, 12};
    E.ContactWorldSeconds = Time; E.DamageResponseProfile = "S4BoundH01";
    Check(IBImpact::SetS35Request(Kind, -60, E), "canonical foreign request");
    return E;
}
// The same policy/HP helpers as ConsumeBoundHit. No test double of the canonical
// translator or TargetRouter. UObject ownership/CMC remain UE-only pending tests.
struct ConsumerFixture
{
    IBImpact::TargetRouter<> Router{Session, Life};
    IBS4::Receiver Receiver;
    bool H01 = true;
    float Health = 37;
    std::int64_t Serial = 1000;
    int Allocations = 0, Accepted = 0;
    IBImpact::RouteResult Deliver(const IBImpact::Envelope& E, double Now)
    {
        return Router.Route(E, Now, [&](const IBImpact::Envelope& Request)
        {
            ++Allocations; return IBImpact::S4Packet{++Serial, Request.ContactWorldSeconds};
        }, [&](const IBImpact::Envelope& Request, const IBImpact::S4Packet& Packet)
        {
            IBImpact::Outcome Out;
            if (!IBS4::validHealth(Health)) return Out;
            IBImpact::S4Weapon Mapped;
            if (!IBImpact::ToS4Weapon(Request.Kind, Mapped)) return Out;
            IBS4::Weapon Weapon;
            // Test-only bridge between native enums, explicit like UE consumers.
            switch (Mapped)
            {
            case IBImpact::S4Weapon::Bullet: Weapon = IBS4::Weapon::Bullet; break;
            case IBImpact::S4Weapon::Missile: Weapon = IBS4::Weapon::Missile; break;
            case IBImpact::S4Weapon::Laser: Weapon = IBS4::Weapon::Laser; break;
            case IBImpact::S4Weapon::Punch: Weapon = IBS4::Weapon::Punch; break;
            default: return Out;
            }
            auto Result = Receiver.receive(static_cast<uint64_t>(Packet.Serial), Packet.IssuedAt, Now,
                Weapon, H01, -Request.RequestedHealthDelta, 1000, 1.1, Request.PhysicsResolved, true);
            if (!Result.accepted) return Out;
            const auto Change = IBS4::resolveHealth(Health, Result.healthDelta);
            Health = Change.next; ++Accepted;
            Out.Accepted = true; Out.RequestedHealthDelta = Result.healthDelta; Out.HealthDelta = Change.delta;
            Out.DeltaVelocity.X = Result.pushSpeed; // Scalar check only, not a CMC motion measurement.
            return Out;
        });
    }
};
}
int main()
{
    try
    {
        using namespace IBImpact;
        ConsumerFixture Hero;
        std::int64_t NativeId = 1;
        for (int Shot = 0; Shot < 20; ++Shot)
        {
            const double Time = Shot * 5.0;
            auto E = Native(NativeId++, Time, S4Weapon::Bullet);
            auto R = Hero.Deliver(E, Time);
            if (R.State != Status::Accepted || R.Applied.HealthDelta != 0 || !R.Applied.DeltaVelocity.Zero())
                std::cerr << "bullet fixture shot=" << Shot << " status=" << static_cast<int>(R.State)
                          << " healthDelta=" << R.Applied.HealthDelta << " packet=" << R.Packet.Serial << '\n';
            Check(R.State == Status::Accepted && R.Applied.HealthDelta == 0 && R.Applied.DeltaVelocity.Zero(), "canonical bullet changed H01");
            E.RequestedHealthDelta = -999; E.ContactWorldSeconds = Time + 4;
            Check(Hero.Deliver(E, Time + 4).State == Status::Duplicate, "canonical replay changed time/damage");
        }
        for (int Shot = 0; Shot < 20; ++Shot)
        {
            const double Time = 105 + Shot * 2.0;
            auto R = Hero.Deliver(Native(NativeId++, Time, S4Weapon::Missile), Time);
            Check(R.State == Status::Accepted && R.Applied.HealthDelta == 0 && R.Applied.DeltaVelocity.X == 1000, "canonical spaced missile");
            R = Hero.Deliver(Native(NativeId++, Time + .1, S4Weapon::Missile), Time + .1);
            Check(R.State == Status::Accepted && R.Applied.DeltaVelocity.Zero(), "canonical rapid missile ignores cooldown");
        }
        Check(Hero.Health == 37 && Hero.Allocations == 0, "native S4 ID reallocated or H01 HP changed");
        auto Heavy = Foreign({77, 0, 0, 1}, 150, S35Kind::HeavyPunch);
        auto R = Hero.Deliver(Heavy, 150);
        Check(R.State == Status::Accepted && R.Applied.HealthDelta == 0 && R.Applied.DeltaVelocity.Zero(), "HeavyPunch ordinal became Missile");
        Check(Hero.Allocations == 1, "S35 contact not allocated once");
        Heavy.ContactWorldSeconds = 160;
        Check(Hero.Deliver(Heavy, 160).State == Status::Duplicate && Hero.Allocations == 1, "GUID redelivery allocated a new S4 ID");
        ConsumerFixture Npc; Npc.H01 = false; Npc.Health = 10;
        Heavy.ContactWorldSeconds = 0;
        R = Npc.Deliver(Heavy, 0);
        Check(R.State == Status::Accepted && R.Applied.RequestedHealthDelta == -60 && R.Applied.HealthDelta == -10 && Npc.Health == 0, "canonical damage sign or overkill");
        Check(Npc.Deliver(Heavy, 0).State == Status::Duplicate && Npc.Accepted == 1, "canonical attack damaged twice");
        ConsumerFixture FullGuid;
        auto A = Foreign({1, 0, 0, 77}, 0, S35Kind::HeldTool);
        auto B = Foreign({2, 0, 0, 77}, 0, S35Kind::ThrownObject);
        Check(FullGuid.Deliver(A, 0).State == Status::Accepted && FullGuid.Deliver(B, 0).State == Status::Accepted
            && FullGuid.Allocations == 2, "GUIDs with same low word collided");
        ConsumerFixture Timing;
        A.ContactWorldSeconds = 1;
        Check(Timing.Deliver(A, 4).State == Status::TransportInvalid && Timing.Allocations == 0, "expired S35 contact got a fresh packet");
        A.ContactWorldSeconds = 4;
        Timing.Health = -1;
        Check(Timing.Deliver(A, 4).State == Status::ReceiverRejected, "invalid HP accepted");
        Timing.H01 = false; Timing.Health = 10;
        A.RequestedHealthDelta = -1; // Retry must use the frozen first request (-60).
        R = Timing.Deliver(A, 4.1);
        Check(R.State == Status::Accepted && R.Applied.RequestedHealthDelta == -60 && R.Applied.HealthDelta == -10
            && Timing.Allocations == 1, "rejected retry changed payload or reallocated packet");
        ConsumerFixture Physics;
        auto Missile = Native(1, 0, S4Weapon::Missile); Missile.PhysicsResolved = true;
        R = Physics.Deliver(Missile, 0);
        Check(R.State == Status::Accepted && R.Applied.DeltaVelocity.Zero(), "canonical resolved missile pushed again");
        R = Physics.Deliver(Native(2, .1, S4Weapon::Missile), .1);
        Check(R.State == Status::Accepted && R.Applied.DeltaVelocity.Zero(), "resolved contact failed to reserve cooldown");
        R = Physics.Deliver(Native(3, 1.1, S4Weapon::Missile), 1.1);
        Check(R.State == Status::Accepted && R.Applied.DeltaVelocity.X == 1000, "canonical cooldown never recovered");
        Vector Delta;
        Check(ImpulseToDeltaVelocity({120000, 0, 0}, 1200, Delta) && Delta.X == 100, "canonical physical impulse units");
        Check(!ImpulseToDeltaVelocity({120000, 0, 0}, 0, Delta), "zero receiving mass accepted");
        std::cout << "PASS canonical/S4 consumer: " << Checks << " checks using real external ImpactContract.h and IBS4Policy.h.\n";
        std::cout << "NOT RUN: ImpactContractUE/S4Bridge, UObject binding, CMC/Chaos, combined gameplay.\n";
        return 0;
    }
    catch (const std::exception& Error)
    {
        std::cerr << "FAIL canonical/S4 consumer: " << Error.what() << '\n';
        return 1;
    }
}
