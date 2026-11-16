#if defined(IBS7T_STANDALONE_TESTS)
#include "../ThreatModel.h"
#include "IB_Integration/ImpactContract/ImpactContract.h"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
int Checks = 0;
void Require(bool Condition, const char* Description)
{
    ++Checks;
    if (!Condition)
    {
        std::cerr << "FAIL " << Description << '\n';
        std::exit(1);
    }
}
struct CharacterPort final : IBS7T::DamagePort
{
    int Calls = 0;
    bool apply(const IBS7T::HitRequest&) override { ++Calls; return true; }
};
}

int main()
{
    for (const bool Kaiju : {false, true})
    {
        IBS7T::Threat Threat(100, Kaiju);
        IBS7T::DebrisPool Debris;
        const auto Part = Kaiju ? IBS7T::PartId::Horn : IBS7T::PartId::Hammer;
        Require(Threat.damagePart(Threat.epoch(), 1, Part, 60), "expose prepared part");
        Require(Threat.detach(Part, Debris), "detach prepared part");
        Require(Threat.damageBody(Threat.epoch(), 2, 80), "body damage");
        const auto Saved = Threat.snapshot();
        const auto PriorEpoch = Threat.epoch();
        Require(Threat.validSnapshot(Saved), "valid saved state");
        auto Trial = Threat;
        auto TrialDebris = Debris;
        Require(Trial.restore(Saved, TrialDebris), "preflight may restore private copies");
        Require(Threat.epoch() == PriorEpoch && Debris.count() == 1 && Threat.health() == 420,
            "successful preflight leaves live generation, debris and health untouched");
        Require(Debris.hold(100, Threat.epoch(), Part, true), "hold detached piece");
        Require(!Threat.restore(Saved, Debris), "restore cannot erase held piece");
        Trial = Threat; TrialDebris = Debris;
        Require(!Trial.restore(Saved, TrialDebris), "copied preflight preserves held-piece rejection");
        Require(Threat.epoch() == PriorEpoch && Debris.count() == 1,
            "rejected preflight cannot mutate live state");
        Require(Threat.health() == 420 && Threat.epoch() == PriorEpoch, "failed restore is atomic");
        Require(Debris.hold(100, Threat.epoch(), Part, false), "release piece");
        Require(Threat.restore(Saved, Debris), "restore accepted");
        Require(Threat.epoch() > PriorEpoch, "restore renews target life");
        Require(Threat.health() == 420 && !Threat.attached(Part), "health and detachment retained");
        Require(Debris.count() == 1, "single recreated debris record");
        Require(!Threat.damageBody(PriorEpoch, 3, 20), "old-life delivery rejected");
        for (const double InvalidHealth : {-1.0, 501.0, std::numeric_limits<double>::quiet_NaN()})
        {
            auto Invalid = Saved;
            Invalid.health = InvalidHealth;
            Require(!Threat.restore(Invalid, Debris), "invalid body health rejected");
            Require(Threat.health() == 420, "invalid snapshot preserves health");
        }
        auto Invalid = Saved;
        Invalid.kaiju = !Kaiju;
        Require(!Threat.restore(Invalid, Debris), "species mismatch rejected");
        Invalid = Saved;
        Invalid.parts[0].health = 90;
        Require(!Threat.restore(Invalid, Debris), "detached healthy part rejected");
        Invalid = Saved;
        Invalid.parts[0].state = static_cast<IBS7T::PartState>(99);
        Require(!Threat.restore(Invalid, Debris), "unknown part state rejected");
        Require(Threat.reset(Debris, true), "reset restores intact threat");
        Require(Threat.health() == 500 && Threat.attached(Part) && Debris.count() == 0, "reset clears detached state");
        CharacterPort Port;
        for (int Step = 0; Step < 100 && Threat.phase() != IBS7T::State::Attack; ++Step)
            Threat.tick(0.1, true, true);
        Require(Threat.hit(200, 100, true, true, IBS7T::Patch::Character, Port), "swept hero contact accepted");
        Require(!Threat.hit(200, 100, true, true, IBS7T::Patch::Character, Port), "hero contact deduplicated");
        Require(Port.Calls == 1, "one hero mutation");
    }

    // The real canonical router deduplicates across body/part contacts of one target life.
    const IBImpact::Guid Session{1, 2, 3, 4}, Life{5, 6, 7, 8};
    IBImpact::TargetRouter<> Router(Session, Life);
    IBS7T::Threat Threat(300, false);
    IBImpact::Envelope Request;
    Request.Attack = {IBImpact::Origin::S35, Session, {9, 10, 11, 12}, 0};
    Request.TargetId = Life;
    Request.InstigatorId = {13, 14, 15, 16};
    Request.PartId = {0, 0, 1, 1};
    Request.ContactWorldSeconds = 1;
    Request.DamageResponseProfile = "S7PreparedThreat";
    Require(IBImpact::SetS35Request(IBImpact::S35Kind::HeavyPunch, -60, Request), "S35 normalization");
    int Allocations = 0, Mutations = 0;
    auto Allocate = [&](const IBImpact::Envelope&) { ++Allocations; return IBImpact::S4Packet{1, 1}; };
    auto Consume = [&](const IBImpact::Envelope& Envelope, const IBImpact::S4Packet& Packet)
    {
        IBImpact::Outcome Outcome;
        const double Before = Threat.part(IBS7T::PartId::Hammer)->health;
        Outcome.Accepted = Threat.damagePart(Threat.epoch(), Packet.Serial, IBS7T::PartId::Hammer, -Envelope.RequestedHealthDelta);
        if (Outcome.Accepted) ++Mutations;
        Outcome.RequestedHealthDelta = Envelope.RequestedHealthDelta;
        Outcome.HealthDelta = Threat.part(IBS7T::PartId::Hammer)->health - Before;
        return Outcome;
    };
    Require(Router.Route(Request, 1, Allocate, Consume).State == IBImpact::Status::Accepted, "canonical ingress accepted");
    Request.PartId = {0, 0, 1, 6};
    Require(Router.Route(Request, 1.1, Allocate, Consume).State == IBImpact::Status::Duplicate, "body retry remains duplicate");
    Require(Allocations == 1 && Mutations == 1, "one allocation and mutation");
    Require(Threat.health() == 500 && Threat.part(IBS7T::PartId::Hammer)->health == 40, "single health owner");
    std::cout << "PASS S7 persistence/canonical ingress " << Checks << " checks\n";
}
#endif
