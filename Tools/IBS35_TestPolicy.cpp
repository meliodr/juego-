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
    for (int Category = 0; Category < 5; ++Category)
        for (int Cycle = 0; Cycle < 30; ++Cycle)
        {
            IBS35::Lease Object;
            Check(!Object.Claim(0), "invalid owner");
            Check(Object.Claim(1), "claim free");
            Check(!Object.Claim(1) && !Object.Claim(2), "exclusive ownership");
            Check(!Object.Release(2,false), "foreign drop rejected");
            Check(!Object.Release(1,true), "launch without charge rejected");
            Check(Object.BeginCharge(1), "charge");
            Check(Object.CancelCharge(1), "cancel");
            Check(Object.State == IBS35::HoldState::Held && Object.Carrier == 1, "cancel keeps hold");
            Check(!Object.Release(1,true), "cancel then release never launches");
            Check(Object.Release(1,false), "drop");
            Check(Object.State == IBS35::HoldState::Free && Object.Carrier == 0, "drop clears owner");
            Check(Object.Claim(2) && Object.BeginCharge(2) && Object.Release(2,true), "explicit throw");
            Check(Object.State == IBS35::HoldState::Thrown && Object.Carrier == 0, "throw clears owner");
            Check(Object.Claim(1), "catch thrown object");
            Object.Reset();
            Check(Object.State == IBS35::HoldState::Free && Object.Carrier == 0, "reset held");
        }
    IBS35::HitLedger<int> A, B;
    for (int Attack = 1; Attack <= 30; ++Attack)
    {
        Check(A.Accept(Attack), "first target contact");
        for (int Contact = 0; Contact < 8; ++Contact) Check(!A.Accept(Attack), "multi-contact dedup");
        Check(B.Accept(Attack), "same attack different target");
    }
    Check(!A.Accept(1), "delayed replay");
    IBS35::HitLedger<int,2> Small;
    Check(Small.Accept(1) && Small.Accept(2), "capacity fill");
    Check(!Small.Accept(3) && !Small.Accept(1), "capacity fails closed without eviction");
    Small.Reset(); Check(Small.Accept(3), "explicit life reset");
    for (int Reset = 0; Reset < 20; ++Reset)
    {
        IBS35::Lease Object;
        Object.Claim(1); Object.BeginCharge(1); Object.Reset();
        Check(!Object.Release(1,true), "no delayed launch after reset");
    }
    for (const IBS35::AttackTiming T : {IBS35::AttackTiming{.09,.08,.18}, IBS35::AttackTiming{.28,.14,.35}})
    {
        Check(T.Valid(), "valid timing");
        Check(!T.CrossesContact(0,T.Windup/2), "windup has no hit");
        Check(T.CrossesContact(0, T.Duration()+1), "hitch cannot skip contact");
        Check(!T.CrossesContact(T.Windup+T.Active,T.Duration()), "recovery has no hit");
        Check(T.Alpha(0)==0 && T.Alpha(T.Duration())==1, "bounded curve");
        Check(!T.CrossesContact(1,.5), "backwards time rejected");
    }
    Check(!IBS35::AttackTiming{.1,0,.2}.Valid(), "zero active invalid");
    const double NaN = std::numeric_limits<double>::quiet_NaN();
    Check(!IBS35::AttackTiming{NaN,.1,.2}.Valid(), "nan timing invalid");
    Check(IBS35::ThrowSpeed(2200,1,1300,6800) < IBS35::ThrowSpeed(85,1,1300,6800), "mass changes throw");
    Check(IBS35::ThrowSpeed(1200,0,1300,6800) == 1300, "minimum throw");
    Check(IBS35::ThrowSpeed(1200,1,1300,6800) == 6800, "maximum throw");
    Check(IBS35::ThrowSpeed(1200,2,1300,6800) == 6800, "charge clamp");
    Check(IBS35::ThrowSpeed(0,1,1300,6800) == 0 && IBS35::ThrowSpeed(NaN,1,1300,6800) == 0, "invalid mass");
    Check(IBS35::ThrowSpeed(1200,NaN,1300,6800) == 0, "invalid charge");
    Check(IBS35::ThrowSpeed(1200,1,6800,1300) == 0, "invalid speed range");
    for (int Kind = 0; Kind <= 3; ++Kind)
        Check(IBS35::ValidDamageRequest(Kind,3,-20,.1,100,100), "supported kind accepted");
    Check(!IBS35::ValidDamageRequest(4,3,-20,.1,100,100), "unknown kind rejected");
    Check(!IBS35::ValidDamageRequest(-1,3,-20,.1,100,100), "negative kind rejected");
    Check(!IBS35::ValidDamageRequest(0,3,-20,-.1,100,100), "negative attack age rejected");
    Check(!IBS35::ValidDamageRequest(0,3,20,.1,100,100), "healing request rejected");
    Check(!IBS35::ValidDamageRequest(0,3,NaN,.1,100,100), "nan damage rejected");
    Check(!IBS35::ValidDamageRequest(0,3,-20,NaN,100,100), "nan age rejected");
    Check(!IBS35::ValidDamageRequest(0,3,-20,.1,NaN,100), "nan health rejected");
    Check(!IBS35::ValidDamageRequest(0,3,-20,.1,100,NaN), "nan initial health rejected");
    Check(!IBS35::ValidDamageRequest(0,3,-20,.1,110,100), "invalid current health rejected");
    Check(IBS35::ValidDamageRequest(0,3,0,0,0,0), "zero health impulse-only request accepted");
    Check(IBS35::AppliedDamageDelta(10,-60,100) == -10, "overkill reports actual health delta");
    Check(IBS35::AppliedDamageDelta(0,-60,100) == 0, "depleted health reports zero delta");
    Check(IBS35::AppliedDamageDelta(100,-20,100) == -20, "ordinary damage delta");
    Check(IBS35::AppliedDamageDelta(100.f,-0.0000001f,100.f) == 0, "reported delta uses stored float precision");
    std::cout << "PASS IBS35 policy: " << Checks << " assertions; 150 lease cycles, 30 dedup attacks, 20 resets.\n"
              << "This executable does NOT test Unreal collision, physics, animation, DX12 or FPS.\n";
}
