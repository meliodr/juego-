// Test-only include supplied by IBS35_CompareS4.py. No runtime dependency on S4.
#include "IBS4Policy.h"
#include "../ImpactBeyond/Source/ImpactBeyond/IB_S35/IBS35Policy.h"
#include <iostream>
#include <limits>
#include <cstdlib>

int Checks = 0;
void Check(bool Value, const char* Label)
{
    ++Checks;
    if (!Value) { std::cerr << "FAIL " << Label << '\n'; std::exit(1); }
}

int main()
{
    // The runner verifies the inspected UE enum declaration: HeavyPunch has ordinal 1.
    static_assert(static_cast<int>(IBS4::Weapon::Missile) == 1);
    IBS4::Receiver Naive, Explicit;
    const auto WrongKind = Naive.receive(1,100,100,static_cast<IBS4::Weapon>(1),true,60);
    const auto RightKind = Explicit.receive(1,100,100,IBS4::Weapon::Punch,true,60);
    Check(WrongKind.accepted && WrongKind.healthDelta == 0 && WrongKind.pushSpeed == 1000,
        "numeric heavy-punch cast accidentally grants missile immunity and push");
    Check(RightKind.accepted && RightKind.healthDelta == -60 && RightKind.pushSpeed == 0,
        "explicit punch map has different H01 response");

    IBS4::Receiver Sign;
    Check(!Sign.receive(1,100,100,IBS4::Weapon::Punch,false,-60).accepted, "S4 rejects S35 negative delta as amount");
    Check(Sign.receive(1,100,100,IBS4::Weapon::Punch,false,60).accepted, "negating requested delta is needed");
    Check(!IBS35::ValidDamageRequest(1,3,60,.1,100,100), "S35 rejects positive S4 amount as delta");
    Check(IBS35::AppliedDamageDelta(10,-60,100) == -10 && RightKind.healthDelta == -60,
        "S35 actual delta differs from S4 requested delta at low health");
    Check(IBS35::AppliedDamageDelta(0,-60,100) == 0, "actual zero must not erase structural request of 60");

    IBS4::Receiver Clock;
    Check(!Clock.receive(1,.1,100,IBS4::Weapon::Punch,false,20).accepted, "relative age cannot become absolute IssuedAt");
    Check(Clock.receive(1,99.9,100,IBS4::Weapon::Punch,false,20).accepted, "same world clock accepted");
    Check(!Clock.receive(2,100.02,100,IBS4::Weapon::Punch,false,20).accepted, "future timestamp rejected");
    Check(IBS35::ValidDamageRequest(3,3,-20,4,100,100), "S35 permits late projectile contact");
    Check(!Clock.receive(3,96,100,IBS4::Weapon::Punch,false,20).accepted, "S4 rejects four-second-old attack");

    IBS4::Receiver Replay;
    IBS35::HitLedger<std::uint64_t> Persistent;
    Check(Replay.receive(7,0,0,IBS4::Weapon::Punch,false,20).accepted && Persistent.Accept(7), "first common ID");
    Check(!Replay.receive(7,0,1,IBS4::Weapon::Punch,false,20).accepted && !Persistent.Accept(7), "live duplicates rejected");
    Check(!Replay.receive(7,0,3,IBS4::Weapon::Punch,false,20).accepted, "original timestamp expires");
    Check(Replay.receive(7,3,3,IBS4::Weapon::Punch,false,20).accepted && !Persistent.Accept(7),
        "refreshing timestamp resurrects an S4 ID but cannot resurrect S35 ID");
    Persistent.Reset();
    Check(Persistent.Accept(7), "S35 ledger reset requires separate renewed target-life validation");

    IBS4::Receiver Capacity;
    IBS35::HitLedger<std::uint64_t> Larger;
    for (std::uint64_t Id = 1; Id <= 256; ++Id)
        Check(Capacity.receive(Id,10,10,IBS4::Weapon::Punch,false,1).accepted && Larger.Accept(Id), "shared first 256 contacts");
    Check(!Capacity.receive(257,10,10,IBS4::Weapon::Punch,false,1).accepted && Larger.Accept(257), "ledger capacities differ");
    Check(Capacity.receive(257,12.02,12.02,IBS4::Weapon::Punch,false,1).accepted, "S4 reuses expired slots");

    // S35 impulse units kg*cm/s; S4 push parameter is a delta-speed in cm/s, applied mass-independently by CMC.
    constexpr double PhysicalImpulse = 120000, Mass = 1200;
    IBS4::Receiver RawImpulse, Converted;
    const auto WrongUnits = RawImpulse.receive(1,20,20,IBS4::Weapon::Missile,true,0,PhysicalImpulse);
    const auto CorrectUnits = Converted.receive(1,20,20,IBS4::Weapon::Missile,true,0,PhysicalImpulse/Mass);
    Check(WrongUnits.pushSpeed == 1200 && CorrectUnits.pushSpeed == 100, "unconverted impulse produces 12x delta-speed");
    Check(!Converted.receive(2,20.1,20.1,IBS4::Weapon::Missile,true,0,100).pushSpeed, "missile cooldown must survive bridge");
    IBS4::Receiver Protected;
    Check(Protected.receive(1,30,30,IBS4::Weapon::Bullet,true,10).healthDelta == 0, "H01 bullet policy must survive bridge");
    Check(Protected.receive(2,30,30,IBS4::Weapon::Missile,true,30).healthDelta == 0, "H01 missile policy must survive bridge");
    Check(Protected.receive(3,30,30,IBS4::Weapon::Laser,true,30).healthDelta == -30, "H01 is not immune to all weapons");
    std::cout << "PASS IBS35/S4 contract comparison: " << Checks << " assertions over actual native policy headers.\n"
              << "Expected incompatibilities were reproduced; this is NOT a passing Unreal integration.\n";
}
