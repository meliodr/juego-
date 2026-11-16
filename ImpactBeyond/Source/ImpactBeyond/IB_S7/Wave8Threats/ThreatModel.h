#pragma once
// Native offline model. No UE actor, physics, renderer or shared gameplay dependency.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace IBS7T {
enum class State { Patrol, Notice, Telegraph, Attack, Recover, Stagger, Retreat };
enum class PartId { Hammer, Claw, Horn, Tail, Shoulder };
enum class Move { Hammer, Grab, Charge, TailSweep };
enum class PartState { Intact, Exposed, Detached };
enum class Patch { Building, Terrain, Character, Unprepared };
struct AttackSpec { Move move; double telegraph, active, recovery, rangeCm, damage; };
struct HitRequest {
    uint64_t actor, generation, attack, target;
    Move move;
    Patch patch;
    double damage, pushSpeedCmS;
};
// Adapter must validate collision/sweep, loaded region and attribution BEFORE hit().
// apply() is synchronous and transactional: false means no gameplay mutation.
struct DamagePort { virtual ~DamagePort() = default; virtual bool apply(const HitRequest&) = 0; };
struct Part { PartId id; double health = 100; PartState state = PartState::Intact; };
struct ThreatSnapshot {
    bool kaiju = false;
    double health = 500;
    std::array<Part, 3> parts{};
};
struct LoosePart { bool active = false, held = false; uint64_t actor = 0, generation = 0; PartId part = PartId::Hammer; };
class DebrisPool {
    std::array<LoosePart, 5> pieces{};
public:
    bool add(uint64_t actor, uint64_t generation, PartId part) {
        for (const auto& p : pieces) if (p.active && p.actor == actor && p.generation == generation && p.part == part) return false;
        for (auto& p : pieces) if (!p.active) { p = {true, false, actor, generation, part}; return true; }
        return false; // Functional detachment still occurs; cosmetic fallback is NOT grabbable.
    }
    bool hold(uint64_t actor, uint64_t generation, PartId part, bool value) {
        for (auto& p : pieces) if (p.active && p.actor == actor && p.generation == generation && p.part == part) {
            if (p.held == value) return false;
            p.held = value; return true;
        }
        return false;
    }
    bool releaseActor(uint64_t actor) {
        for (const auto& p : pieces) if (p.active && p.actor == actor && p.held) return false;
        for (auto& p : pieces) if (p.actor == actor) p = {};
        return true;
    }
    unsigned count() const { unsigned n = 0; for (const auto& p : pieces) n += p.active; return n; }
};
class Threat {
    uint64_t actor, generation = 1, serial = 0, lastDamageId = 0;
    bool kaiju;
    std::array<Part, 3> parts;
    double body = 500, elapsed = 0;
    State state = State::Patrol;
    Move selected;
    std::array<uint64_t, 16> targets{};
    void enter(State next) { state = next; elapsed = 0; }
public:
    // Unique nonzero actor IDs are allocated by the arena/integrator, never by mesh name.
    Threat(uint64_t id, bool isKaiju) : actor(id), kaiju(isKaiju),
        parts(isKaiju ? std::array<Part,3>{{{PartId::Horn},{PartId::Tail},{PartId::Shoulder}}}
                      : std::array<Part,3>{{{PartId::Hammer},{PartId::Claw},{PartId::Shoulder}}}),
        selected(isKaiju ? Move::Charge : Move::Hammer) {}
    State phase() const { return state; }
    uint64_t epoch() const { return generation; }
    uint64_t attackId() const { return serial; }
    double health() const { return body; }
    ThreatSnapshot snapshot() const { return {kaiju, body, parts}; }
    bool validSnapshot(const ThreatSnapshot& value) const {
        if (value.kaiju != kaiju || !std::isfinite(value.health) || value.health < 0 || value.health > 500) return false;
        for (unsigned i = 0; i < partCount(); ++i) {
            const auto& p = value.parts[i];
            if (p.id != parts[i].id || !std::isfinite(p.health) || p.health < 0 || p.health > 100
                || static_cast<unsigned>(p.state) > static_cast<unsigned>(PartState::Detached)
                || (p.state == PartState::Intact && p.health <= 50)
                || (p.state != PartState::Intact && p.health > 50)) return false;
        }
        return true;
    }
    bool restore(const ThreatSnapshot& value, DebrisPool& pool) {
        if (!validSnapshot(value) || !pool.releaseActor(actor)) return false;
        // Hydration creates a fresh target life so packets captured before unload cannot apply.
        ++generation; lastDamageId = 0; body = value.health; parts = value.parts;
        targets.fill(0); selected = kaiju ? Move::Charge : Move::Hammer;
        enter(body > 0 ? State::Patrol : State::Retreat);
        for (unsigned i = 0; i < partCount(); ++i)
            if (parts[i].state == PartState::Detached) pool.add(actor, generation, parts[i].id);
        return true;
    }
    unsigned partCount() const { return kaiju ? 3u : 2u; }
    const Part* part(PartId id) const {
        for (unsigned i=0; i<partCount(); ++i) if (parts[i].id == id) return &parts[i];
        return nullptr;
    }
    bool attached(PartId id) const { const auto* p=part(id); return p && p->state != PartState::Detached; }
    bool enabled(Move m) const {
        if (!actor || body <= 0) return false;
        if (kaiju) return m == Move::Charge || (m == Move::TailSweep && attached(PartId::Tail));
        return (m == Move::Hammer && attached(PartId::Hammer)) || (m == Move::Grab && attached(PartId::Claw));
    }
    AttackSpec spec() const {
        switch(selected) {
        case Move::Hammer: return {selected,1.2,0.3,1.5,450,80};
        case Move::Grab: return {selected,0.8,0.4,1.2,350,35};
        case Move::Charge: return {selected,1.5,0.5,2,900,attached(PartId::Horn)?100.0:45.0};
        default: return {selected,1.0,0.5,1.8,700,70};
        }
    }
    bool choose(Move m) {
        if ((state != State::Patrol && state != State::Notice) || !enabled(m)) return false;
        selected=m; return true;
    }
    // Monotonic per-target damage serials, scoped to generation. Reordered older messages fail closed.
    bool damagePart(uint64_t epochValue, uint64_t hitId, PartId id, double amount) {
        if (epochValue != generation || hitId <= lastDamageId || !std::isfinite(amount) || amount <= 0 || body <= 0) return false;
        for (unsigned i=0; i<partCount(); ++i) {
            auto& p=parts[i];
            if (p.id != id || p.state == PartState::Detached) continue;
            lastDamageId=hitId; p.health=std::max(0.0,p.health-std::min(amount,100.0));
            if (p.health <= 50) p.state=PartState::Exposed;
            return true;
        }
        return false;
    }
    bool detach(PartId id, DebrisPool& pool) {
        for (unsigned i=0; i<partCount(); ++i) {
            auto& p=parts[i];
            if (p.id != id || p.state != PartState::Exposed) continue;
            p.state=PartState::Detached; pool.add(actor,generation,id);
            enter(State::Stagger); targets.fill(0); return true;
        }
        return false;
    }
    bool damageBody(uint64_t epochValue, uint64_t hitId, double amount) {
        if (epochValue!=generation || hitId<=lastDamageId || !std::isfinite(amount) || amount<=0 || body<=0) return false;
        lastDamageId=hitId;
        body=std::max(0.0,body-std::min(amount,500.0)*(kaiju && !attached(PartId::Shoulder)?1.5:1.0));
        if (body<=0) enter(State::Retreat);
        return true;
    }
    void tick(double dt, bool visible, bool regionReady) {
        if (!std::isfinite(dt) || dt<=0) return;
        if (!regionReady || !actor || body<=0) { enter(State::Retreat); return; }
        elapsed+=std::min(dt,0.25); // bounded steps: hitches never skip telegraph into damage
        if (!visible && (state==State::Notice || state==State::Telegraph || state==State::Attack)) { enter(State::Recover); return; }
        switch(state) {
        case State::Patrol: if(visible) enter(State::Notice); break;
        case State::Notice:
            if (elapsed>=0.4) {
                if (!enabled(selected)) selected=kaiju?Move::Charge:(attached(PartId::Hammer)?Move::Hammer:Move::Grab);
                enter(enabled(selected)?State::Telegraph:State::Retreat);
            } break;
        case State::Telegraph: if(elapsed>=spec().telegraph) { ++serial; targets.fill(0); enter(State::Attack); } break;
        case State::Attack: if(elapsed>=spec().active || !enabled(selected)) enter(State::Recover); break;
        case State::Recover: if(elapsed>=spec().recovery) enter(State::Patrol); break;
        case State::Stagger: if(elapsed>=0.8) enter(State::Recover); break;
        case State::Retreat: if(elapsed>=1 && body>0) enter(State::Patrol); break;
        }
    }
    bool hit(uint64_t target, double distanceCm, bool sweptContact, bool regionReady, Patch patch, DamagePort& port) {
        if (state!=State::Attack || !enabled(selected) || !target || target==actor || !sweptContact || !regionReady ||
            (patch!=Patch::Building && patch!=Patch::Terrain && patch!=Patch::Character) || !std::isfinite(distanceCm) || distanceCm<0 || distanceCm>spec().rangeCm) return false;
        for (auto seen:targets) if(seen==target) return false;
        for (auto& seen:targets) if(!seen) {
            if(!port.apply({actor,generation,serial,target,selected,patch,spec().damage,600})) return false;
            seen=target; return true;
        }
        return false;
    }
    bool reset(DebrisPool& pool, bool restoreVolumeClear) {
        if (!restoreVolumeClear || !pool.releaseActor(actor)) return false;
        ++generation; lastDamageId=0; body=500; targets.fill(0);
        for(auto& p:parts) { p.health=100; p.state=PartState::Intact; }
        selected=kaiju?Move::Charge:Move::Hammer; enter(State::Patrol); return true;
    }
};
}
