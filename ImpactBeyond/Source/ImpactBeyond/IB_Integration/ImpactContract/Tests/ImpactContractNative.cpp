// Excluded from UBT's normal translation units. Explicit native runner defines this.
#if defined(IB_IMPACT_NATIVE_TEST)
#include "../ImpactContract.h"
#include "IBS4Policy.h" // Actual unmodified S4 checkout, supplied by runner include path.
#include "../../../IB_S35/IBS35Policy.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <type_traits>

using namespace IBImpact;
int Checks = 0;
void Check(bool Value, const char* Name)
{
    ++Checks;
    if (!Value) { std::cerr << "FAIL: " << Name << '\n'; std::exit(1); }
}
Guid G(std::uint32_t N) { return {0x12345678, 0x90abcdef, 0x11223344, N}; }
Envelope Hit(std::uint32_t N = 1, Guid Target = G(20), double At = 100)
{
    Envelope E;
    E.Attack = {Origin::S35, G(10), G(N), 0}; E.TargetId = Target; E.InstigatorId = G(30);
    E.PartId = G(40); E.ContactWorldSeconds = At; E.DamageResponseProfile = "S4.Prepared.Metal";
    E.Kind = DamageKind::HeavyPunch; E.RequestedHealthDelta = -60;
    return E;
}
IBS4::Weapon Native(S4Weapon W)
{
    switch (W)
    {
    case S4Weapon::Bullet: return IBS4::Weapon::Bullet;
    case S4Weapon::Missile: return IBS4::Weapon::Missile;
    case S4Weapon::Laser: return IBS4::Weapon::Laser;
    case S4Weapon::Punch: return IBS4::Weapon::Punch;
    }
    std::abort();
}
// Native harness only: real S4 policy + stored float health; no mocked physics/UE claims.
struct Consumer
{
    IBS4::Receiver Policy;
    float Health = 100;
    double Now = 100;
    int Calls = 0;
    bool H01 = false;
    bool ProtectAllH01 = false; // Native policy flag used by the real ConsumeBoundHit endpoint.
    Outcome operator()(const Envelope& E, const S4Packet& Packet)
    {
        ++Calls;
        S4Weapon W;
        if (!ToS4Weapon(E.Kind, W)) return {};
        const auto R = Policy.receive(static_cast<std::uint64_t>(Packet.Serial), Packet.IssuedAt, Now, Native(W), H01,
            -E.RequestedHealthDelta, 1000, 1.1, E.PhysicsResolved, ProtectAllH01);
        if (!R.accepted) return {};
        const auto Change = IBS4::resolveHealth(Health, R.healthDelta);
        Check(Change.valid, "S4 actual health result valid");
        Health = Change.next;
        Outcome Out; Out.Accepted = true; Out.RequestedHealthDelta = R.healthDelta; Out.HealthDelta = Change.delta;
        Out.DeltaVelocity = {R.pushSpeed, 0, 0}; // magnitude policy only; NOT a simulated CMC move.
        return Out;
    }
};
int main()
{
    static_assert(!std::is_copy_constructible_v<TargetRouter<>>);
    static_assert(!std::is_move_constructible_v<TargetRouter<>>);
    const double NaN = std::numeric_limits<double>::quiet_NaN();
    const double Inf = std::numeric_limits<double>::infinity();
    const auto Accept = [](const Envelope& E, const S4Packet&) { Outcome O; O.Accepted = true; O.RequestedHealthDelta = E.RequestedHealthDelta; return O; };
    std::int64_t Sequence = 0;
    int Allocations = 0;
    const auto Allocate = [&](const Envelope& E) { ++Allocations; return S4Packet{++Sequence, E.ContactWorldSeconds}; };

    for (auto K : {S35Kind::QuickPunch, S35Kind::HeavyPunch, S35Kind::HeldTool, S35Kind::ThrownObject})
    {
        auto E = Hit(); Check(SetS35Request(K, -60, E), "explicit S35 kind accepted");
        S4Weapon W; Check(ToS4Weapon(E.Kind, W) && W == S4Weapon::Punch, "all physical kinds explicitly become S4 Punch");
        TargetRouter<> Router(G(10), G(20)); Consumer C; C.H01 = true;
        const auto R = Router.Route(E, 100, Allocate, C);
        Check(R.State == Status::Accepted && C.Health == 40 && R.Applied.DeltaVelocity.Zero(), "heavy/tool never missile or bullet immunity");
        Check(R.Applied.RequestedHealthDelta == -60 && R.Applied.HealthDelta == -60, "damage sign preserved");
    }
    Envelope Bad = Hit();
    Check(!SetS35Request(static_cast<S35Kind>(255), -1, Bad), "unknown S35 enum rejected");
    Check(!SetS4Request(static_cast<S4Weapon>(255), 1, Bad), "unknown S4 enum rejected");
    S4Weapon Unknown;
    Check(!ToS4Weapon(static_cast<DamageKind>(255), Unknown), "unknown canonical enum rejected");
    for (double V : {1.0, NaN, Inf}) Check(!SetS35Request(S35Kind::QuickPunch, V, Bad), "invalid S35 delta");
    for (double V : {-1.0, NaN, Inf}) Check(!SetS4Request(S4Weapon::Bullet, V, Bad), "invalid S4 amount");

    for (auto W : {S4Weapon::Bullet, S4Weapon::Missile, S4Weapon::Laser, S4Weapon::Punch})
    {
        TargetRouter<> Router(G(10), G(20)); Consumer C; C.H01 = true;
        auto E = Hit(); E.Attack = {Origin::S4, G(10), {}, ++Sequence}; E.InstigatorId = {};
        Check(SetS4Request(W, 60, E), "native S4 enum normalizes");
        const int Before = Allocations;
        const auto R = Router.Route(E, 100, Allocate, C);
        const bool Immune = W == S4Weapon::Bullet || W == S4Weapon::Missile;
        Check(R.State == Status::Accepted && C.Health == (Immune ? 100.f : 40.f), "S4 H01 policy reused");
        Check(Allocations == Before && R.Packet.Serial == E.Attack.NativeSerial, "S4 preserves original serial without allocation");
        Check(R.Applied.DeltaVelocity.X == (W == S4Weapon::Missile ? 1000 : 0), "S4 push remains cm/s");
        Check(E.Impulse.Zero() && !E.InstigatorId.Valid(), "no invented impulse or player attribution");
    }
    for (float Health : {0.f, 10.f, 100.f})
    {
        TargetRouter<> Router(G(10), G(20)); Consumer C; C.Health = Health;
        auto E = Hit(); E.RequestedHealthDelta = -1500;
        const auto R = Router.Route(E, 100, Allocate, C);
        Check(R.Applied.RequestedHealthDelta == -1000 && R.Applied.HealthDelta == -Health, "S4 cap and actual HP kept separately");
        Check(R.Applied.HealthDelta == IBS35::AppliedDamageDelta(Health, -1000.f, 100.f), "actual delta agrees with S35 float policy");
    }
    for (double Amount : {1e-20, 5e-6, 0.0})
    {
        TargetRouter<> Router(G(10), G(20)); Consumer C; auto E = Hit(); E.RequestedHealthDelta = -Amount;
        const auto R = Router.Route(E, 100, Allocate, C);
        Check(R.State == Status::Accepted && R.Applied.HealthDelta == static_cast<double>(C.Health - 100.f), "sub-ULP/rounded loss is actual stored delta");
    }

    Vector Converted{9,9,9};
    Check(ImpulseToDeltaVelocity({120000, -60000, 30000}, 1200, Converted) && Converted == Vector{100,-50,25}, "J/m physical conversion");
    Check(DeltaVelocityToImpulse(Converted, 1200, Converted) && Converted == Vector{120000,-60000,30000}, "m*dv conversion");
    for (double Mass : {0.0, -1.0, NaN, Inf})
    {
        const Vector Before = Converted;
        Check(!ImpulseToDeltaVelocity({1,2,3}, Mass, Converted) && Converted == Before, "invalid mass leaves output intact");
        Check(!DeltaVelocityToImpulse({1,2,3}, Mass, Converted) && Converted == Before, "invalid mass inverse rejected");
    }
    Check(!ImpulseToDeltaVelocity({Inf,0,0},1,Converted), "infinite impulse rejected");
    Check(!DeltaVelocityToImpulse({1e308,0,0},1e308,Converted), "conversion overflow rejected");
    Check(!ImpulseToDeltaVelocity({1e308,0,0},1e-308,Converted), "division overflow rejected");
    {
        auto E = Hit(); E.Impulse = {120000,0,0};
        auto P = PlanPhysicalImpulse(E,true,1200,50);
        Check(P.Disposition == ImpulseDisposition::RequestedPhysical && P.Impulse == Vector{60000,0,0}, "real mass times max delta-speed caps physical impulse");
        P = PlanPhysicalImpulse(E,true,1200,2000);
        Check(P.Impulse == E.Impulse, "small physical impulse unchanged");
        Check(PlanPhysicalImpulse(E,true,1200,0).Impulse.Zero(), "zero velocity cap means zero additional impulse");
        E.PhysicsResolved = true;
        P = PlanPhysicalImpulse(E,true,1200,2000);
        Check(P.Disposition == ImpulseDisposition::AlreadyResolved && P.Impulse.Zero(), "Chaos-resolved collision never applies second impulse");
        E.PhysicsResolved = false;
        P = PlanPhysicalImpulse(E,false,0,2000);
        Check(P.Disposition == ImpulseDisposition::NotSimulating && P.Impulse.Zero(), "static body reports no physical application");
        for (double Mass : {0.0,-1.0,NaN,Inf})
            Check(PlanPhysicalImpulse(E,true,Mass,2000).Disposition == ImpulseDisposition::InvalidBodyMass, "invalid actual mass reported explicitly");
        for (double Limit : {-1.0,NaN,Inf})
            Check(PlanPhysicalImpulse(E,true,1200,Limit).Disposition == ImpulseDisposition::InvalidBodyMass, "invalid velocity limit reported");
        E.Impulse = {1.7e308,1.7e308,1.7e308};
        P = PlanPhysicalImpulse(E,true,1,100);
        Check(P.Impulse.Valid() && std::abs(std::hypot(P.Impulse.X,P.Impulse.Y,P.Impulse.Z)-100)<1e-10, "finite huge impulse clamps without length overflow");
        E.Impulse = {-3,4,0}; P = PlanPhysicalImpulse(E,true,1,1);
        Check(std::abs(P.Impulse.X+0.6)<1e-12 && std::abs(P.Impulse.Y-0.8)<1e-12, "physical clamp preserves direction");
        E.Impulse = {}; Check(PlanPhysicalImpulse(E,false,0,0).Disposition == ImpulseDisposition::None, "zero request needs no mass");
    }

    for (int Field = 0; Field < 15; ++Field)
    {
        auto E = Hit();
        switch (Field)
        {
        case 0: E.Attack.NativeGuid = {}; break;
        case 1: E.Attack.Session = {}; break;
        case 2: E.Attack.NativeSerial = 5; break;
        case 3: E.TargetId = {}; break;
        case 4: E.InstigatorId = {}; break;
        case 5: E.Kind = DamageKind::Missile; break;
        case 6: E.RequestedHealthDelta = 1; break;
        case 7: E.RequestedHealthDelta = NaN; break;
        case 8: E.AttackAgeSeconds = -1; break;
        case 9: E.ContactWorldSeconds = Inf; break;
        case 10: E.Impulse.X = NaN; break;
        case 11: E.Contact.Y = Inf; break;
        case 12: E.RelativeVelocity.Z = NaN; break;
        case 13: E.Direction.X = Inf; break;
        case 14: E.DamageResponseProfile.clear(); break;
        }
        TargetRouter<> Router(G(10), G(20)); Consumer C;
        const int Before = Allocations;
        Check(Router.Route(E,100,Allocate,C).State == Status::Invalid && C.Calls == 0 && Before == Allocations, "invalid envelope has no mutation or allocation");
    }
    {
        TargetRouter<> Router(G(10), G(20)); Consumer C; auto E = Hit();
        E.Attack.Session = G(11); Check(Router.Route(E,100,Allocate,C).State == Status::Invalid, "wrong world session rejected");
        E = Hit(); E.TargetId = G(21); Check(Router.Route(E,100,Allocate,C).State == Status::Invalid, "stale target life rejected");
        E = Hit(); E.ContactWorldSeconds = 100.02;
        Check(Router.Route(E,100,Allocate,C).State == Status::Invalid, "future contact rejected");
        E = Hit(); Check(Router.Route(E,NaN,Allocate,C).State == Status::Invalid, "NaN clock rejected");
    }
    {
        TargetRouter<> Router(G(10), G(20)); Consumer C; auto E = Hit();
        E.AttackAgeSeconds = 10; E.PhysicsResolved = true; E.Impulse = {5000,0,0};
        const auto R = Router.Route(E,100,Allocate,C);
        Check(R.State == Status::Accepted && R.Packet.IssuedAt == 100, "10-second projectile gets contact timestamp not launch timestamp");
        const int Before = Allocations;
        for (int I = 0; I < 30; ++I)
        {
            E.PartId = G(100 + I); E.ContactWorldSeconds = 100 + I; C.Now = 100 + I;
            Check(Router.Route(E,C.Now,Allocate,C).State == Status::Duplicate, "different part/refreshed timestamp cannot redamage same target life");
        }
        C.Health = 100; E.ContactWorldSeconds = C.Now;
        Check(Router.Route(E,C.Now,Allocate,C).State == Status::Duplicate && C.Health == 100, "RestoreHealth does not reopen ledger");
        Check(C.Calls == 1 && Allocations == Before, "duplicates never invoke receiver or allocate serial");
        Check(Router.Route(E,100,Allocate,C).State == Status::Invalid, "world clock rewind rejected");
        TargetRouter<> NewLife(G(10),G(21)); E.TargetId = G(21);
        Check(NewLife.Route(E,C.Now,Allocate,C).State == Status::Accepted, "explicit new target life gets independent delivery");
    }
    {
        TargetRouter<> A(G(10),G(20)), B(G(10),G(21)); Consumer CA, CB;
        auto E = Hit(); const auto RA = A.Route(E,100,Allocate,CA);
        E.TargetId = G(21); E.ContactWorldSeconds = 105; CB.Now = 105;
        const auto RB = B.Route(E,105,Allocate,CB);
        Check(RA.State == Status::Accepted && RB.State == Status::Accepted && RA.Packet.Serial != RB.Packet.Serial,
            "same canonical attack may contact new target later with distinct S4 transport serial");
    }
    {
        TargetRouter<> Router(G(10),G(20)); auto E = Hit(); int Attempts = 0;
        const auto Retry = [&](const Envelope& Frozen,const S4Packet&) { ++Attempts; Outcome O; O.Accepted = Attempts > 1; O.RequestedHealthDelta = Frozen.RequestedHealthDelta; return O; };
        const auto First = Router.Route(E,100,Allocate,Retry);
        E.RequestedHealthDelta = -999; E.ContactWorldSeconds = 101;
        const auto Second = Router.Route(E,101,Allocate,Retry);
        Check(First.State == Status::ReceiverRejected && Second.State == Status::Accepted && First.Packet.Serial == Second.Packet.Serial,
            "rejected retry reuses same transport serial");
        Check(Second.Packet.IssuedAt == 100 && Second.Applied.RequestedHealthDelta == -60, "retry freezes timestamp and request");
    }
    {
        TargetRouter<> Router(G(10),G(20)); auto E = Hit();
        const auto Reject = [](const Envelope&, const S4Packet&) { return Outcome{}; };
        const auto First = Router.Route(E,100,Allocate,Reject); const int Before = Allocations;
        E.ContactWorldSeconds = 103;
        Check(Router.Route(E,103,Allocate,Accept).State == Status::TransportInvalid && Allocations == Before, "expired rejection cannot mint fresh delivery");
        Check(First.Packet.IssuedAt == 100, "original time remains fixed");
        TargetRouter<> Stale(G(10),G(20)); E.ContactWorldSeconds = 100;
        Check(Stale.Route(E,103,Allocate,Accept).State == Status::TransportInvalid, "old captured contact not laundered");
    }
    for (double Offset : {0.0, 2.0, 2.0001})
    {
        TargetRouter<> Router(G(10),G(20)); Consumer C; C.Now = 100 + Offset;
        auto E = Hit(); E.Attack = {Origin::S4,G(10),{},++Sequence}; E.Kind = DamageKind::Punch;
        Check(Router.Route(E,C.Now,Allocate,C).State == (Offset <= 2 ? Status::Accepted : Status::TransportInvalid), "exact two-second delivery boundary");
    }
    Check(ValidDeliveryTime(100.01,100) && !ValidDeliveryTime(100.0101,100), "S4 clock tolerance boundary");
    {
        TargetRouter<> Router(G(10),G(20)); Consumer C;
        auto E = Hit(); E.Attack.NativeGuid = {0,0,0,1};
        Check(Router.Route(E,100,Allocate,C).State == Status::Accepted,"GUID identity one accepted");
        E.Attack = {Origin::S4,G(10),{},1}; E.Kind = DamageKind::Punch;
        // Use an isolated actual S4 receiver here: transport serials already consumed
        // elsewhere in this process must come from its single global allocator in UE.
        Consumer NativeC;
        Check(Router.Route(E,100,Allocate,NativeC).State == Status::Accepted,"native serial one never aliases GUID low word one");
        E.Attack.NativeSerial = -1; Check(!E.Valid(),"negative S4 int64 rejected before unsigned conversion");
        E.Attack.NativeSerial = 0; Check(!E.Valid(),"zero S4 serial rejected");
        E.Attack.NativeSerial = std::numeric_limits<std::int64_t>::max();
        TargetRouter<> MaxRouter(G(10),G(20)); Consumer MaxC;
        Check(MaxRouter.Route(E,100,Allocate,MaxC).State == Status::Accepted,"INT64_MAX preserves all 63 positive bits");
        E.Attack.NativeGuid = G(55); Check(!E.Valid(),"mixed GUID and serial invalid");
    }
    {
        TargetRouter<4> Router(G(10),G(20)); Consumer C;
        // Equal low word, distinct high bits: no truncated GUID collision.
        for (std::uint32_t I = 1; I <= 4; ++I)
        {
            auto E = Hit(); E.Attack.NativeGuid.A = I;
            Check(Router.Route(E,100,Allocate,C).State == Status::Accepted, "full GUID preserved");
        }
        Check(Router.Route(Hit(99),100,Allocate,C).State == Status::Capacity && C.Calls == 4, "ledger fails closed when full");
    }
    {
        TargetRouter<> Router(G(10),G(20)); auto E = Hit();
        const auto Reentrant = [&](const Envelope& R,const S4Packet& P)
        {
            Check(Router.Route(Hit(2),100,Allocate,Accept).State == Status::Busy, "reentrant mutation blocked");
            return Accept(R,P);
        };
        Check(Router.Route(E,100,Allocate,Reentrant).State == Status::Accepted, "outer mutation accepted once");
        E = Hit(3);
        const auto Throw = [](const Envelope&,const S4Packet&) -> Outcome { throw std::runtime_error("uncertain consumer"); };
        try { Router.Route(E,100,Allocate,Throw); Check(false,"exception expected"); } catch (const std::runtime_error&) {}
        Check(Router.Route(E,100,Allocate,Accept).State == Status::Duplicate, "uncertain mutation fails closed");
        E = Hit(4);
        const auto BadOutcome = [&](const Envelope&,const S4Packet&) { Outcome O; O.Accepted = true; O.HealthDelta = NaN; return O; };
        Check(Router.Route(E,100,Allocate,BadOutcome).State == Status::OutcomeInvalid, "invalid accepted outcome diagnosed");
        Check(Router.Route(E,100,Allocate,Accept).State == Status::Duplicate, "invalid accepted outcome cannot reapply");
    }
    {
        TargetRouter<> Router(G(10),G(20)); Consumer C;
        for (std::uint32_t I = 1; I <= 256; ++I) Check(Router.Route(Hit(I),100,Allocate,C).State == Status::Accepted, "S4 live capacity accepted");
        auto E = Hit(257); Check(Router.Route(E,100,Allocate,C).State == Status::ReceiverRejected, "real S4 256-live ledger fails closed");
        E.ContactWorldSeconds = 103; C.Now = 103;
        Check(Router.Route(E,103,Allocate,C).State == Status::TransportInvalid, "full-ledger retry expiration preserved");
        Check(Router.Route(Hit(258,G(20),103),103,Allocate,C).State == Status::Accepted, "new packet accepted after S4 retirement");
        auto Old = Hit(1,G(20),103);
        Check(Router.Route(Old,103,Allocate,C).State == Status::Duplicate, "canonical dedup survives native retirement");
    }
    // Bound-policy regression. Binding ownership/UObject reentry itself is covered
    // by the prepared UE bridge test, not claimed by this native fixture.
    for (auto W : {S4Weapon::Bullet,S4Weapon::Missile,S4Weapon::Laser,S4Weapon::Punch})
    {
        TargetRouter<> Router(G(10),G(20)); Consumer C; C.H01 = true; C.ProtectAllH01 = true; C.Health = 37;
        for (std::uint32_t I = 0; I < 20; ++I)
        {
            C.Now = 100 + I * 2;
            auto E = Hit(I+1,G(20),C.Now); E.Attack = {Origin::S4,G(10),{},++Sequence};
            Check(SetS4Request(W,60,E),"bound native weapon normalizes");
            const auto R = Router.Route(E,C.Now,Allocate,C);
            Check(R.State == Status::Accepted && C.Health == 37 && R.Applied.HealthDelta == 0
                && R.Applied.RequestedHealthDelta == 0,"bound H01 preserves non-default HP for every weapon");
            const int Before = C.Calls;
            Check(Router.Route(E,C.Now,Allocate,C).State == Status::Duplicate && Before == C.Calls,"bound native replay has no second consumption");
        }
    }
    {
        TargetRouter<> Router(G(10),G(20)); Consumer C; C.H01 = true; C.ProtectAllH01 = true; C.Health = 37;
        auto E = Hit(); E.PhysicsResolved = true; E.Impulse = {120000,0,0};
        auto R = Router.Route(E,100,Allocate,C);
        Check(R.State == Status::Accepted && C.Health == 37,"bound S35 HeavyPunch also preserves H01 HP");
        Check(PlanPhysicalImpulse(E,true,1200,2000).Impulse.Zero(),"resolved S35 collision requests no extra Chaos impulse");
        E.Attack = {Origin::S4,G(10),{},++Sequence}; E.Kind = DamageKind::Missile;
        R = Router.Route(E,100,Allocate,C);
        Check(R.State == Status::Accepted && R.Applied.DeltaVelocity.Zero(),"bound resolved missile requests no CMC delta-V");
        E.Attack.NativeSerial = ++Sequence; E.ContactWorldSeconds = C.Now = 100.1; E.PhysicsResolved = false;
        R = Router.Route(E,C.Now,Allocate,C);
        Check(R.State == Status::Accepted && R.Applied.DeltaVelocity.Zero(),"resolved missile reserves shared cooldown");
        E.Attack.NativeSerial = ++Sequence; E.ContactWorldSeconds = C.Now = 101.1;
        R = Router.Route(E,C.Now,Allocate,C);
        Check(R.State == Status::Accepted && R.Applied.DeltaVelocity.X == 1000,"resolved cooldown expires without permanent suppression");
        auto Other = Hit(500,G(20),C.Now);
        const auto Reentrant = [&](const Envelope& Frozen,const S4Packet& Packet)
        {
            Check(Router.Route(Hit(501,G(20),C.Now),C.Now,Allocate,C).State == Status::Busy,"bound-policy callback cannot reenter canonical router");
            return C(Frozen,Packet);
        };
        Check(Router.Route(Other,C.Now,Allocate,Reentrant).State == Status::Accepted && C.Health == 37,"bound-policy outer delivery completes once");
    }
    std::cout << "PASS " << Checks << " assertions; canonical adapter + actual S4 policy; no Unreal/physics execution\n";
}
#endif
