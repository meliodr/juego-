#pragma once
// Engine-independent laboratory components. All world effects require explicit ports.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>

namespace IBS6H {
using Id = std::uint64_t;
struct Vec {
    double x=0, y=0, z=0;
    Vec operator+(Vec b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec operator*(double n) const { return {x*n,y*n,z*n}; }
    double length() const { return std::hypot(x,y,z); }
    bool finite() const { return std::isfinite(x)&&std::isfinite(y)&&std::isfinite(z); }
};
enum class Hero { H01, H02, H03 };
enum class Effect { Shield, Construct };
enum class Result { Ok, Invalid, Busy, Cooldown, Energy, Blocked, Unavailable, Paused };
struct Body {
    Vec position, velocity;
    Vec up{0,0,1};
    double radius=42, halfHeight=96; // cm; capsule aligned to local radial up
    bool airborne=false, carrying=false;
    Id region=0;
};
struct BlinkRequest { Body origin; Vec destination; };
struct Descriptor { Hero hero; const char* label; const char* avatar; };
inline constexpr std::array<Descriptor,3> Descriptors{{
    {Hero::H01,"H01 / host unchanged","HOST"},
    {Hero::H02,"H02 / energy constructor","PLACEHOLDER - Manny pending"},
    {Hero::H03,"H03 / Blink","PLACEHOLDER - Manny pending"}
}};

// One owner-thread, synchronous, non-throwing ports. No default successful mocks.
// No call may retain references to arguments. World owns engine actors and GUID mapping.
struct WorldPort {
    virtual ~WorldPort()=default;
    virtual Body body() const=0;
    // Includes start overlap, full oriented capsule sweep (not point/ray samples),
    // exit overlap, terrain/support/fall checks, entire path loaded/region ready.
    virtual bool safeBlink(const BlinkRequest&) const=0;
    // Atomically repeat ALL safeBlink checks including current origin/velocity/up,
    // streaming readiness and dynamic blockers, then move. False must have no effect.
    // Preserve velocity, rotation and camera; resolve RegionId from destination.
    virtual bool commitBlink(const BlinkRequest&)=0;
    // Idempotent create by id; reject occupied spawn/full swept volume; enable a
    // blocking physical collider BEFORE success. One collider/owner per effect.
    virtual bool createEffect(Id, Effect)=0;
    virtual void removeEffect(Id)=0; // idempotent, synchronous collision teardown
    virtual bool effectExists(Id) const=0;
    // Uses existing S3.5 grip, never creates another physics handle.
    virtual bool grabWithConstruct(Id construct, Id target)=0;
    virtual void dropConstruct(Id construct)=0;
    // Host validates current hitbox/range, target/AttackId dedup, applies S3.5/S4
    // damage exactly once. False means no damage. VFX never produce damage.
    virtual bool shortAttack(Id attack, Hero source, Id target, double rangeCm)=0;
    // Session-global namespace; nonzero, never recycled on region/reset/hero change.
    virtual Id newId()=0;
};

class Heroes {
    struct Lease { Id id=0; double left=0; };
    WorldPort& world;
    Hero selected=Hero::H01;
    std::array<double,3> energy{{100,100,100}};
    Lease shield, construct;
    double shieldCooldown=0, constructCooldown=0, blinkCooldown=0;
    double shieldHealth=0, attackWindow=0;
    Id blinkAttack=0, constructAttack=0;
    bool suspended=false;
    std::set<Id> absorbed;
    static bool positive(double x) { return std::isfinite(x)&&x>0; }
    static bool valid(Hero h) { return h==Hero::H01||h==Hero::H02||h==Hero::H03; }
    static std::size_t index(Hero h) { return static_cast<std::size_t>(h); }
    void clear(Lease& lease, bool grip) {
        if(lease.id) { if(grip) world.dropConstruct(lease.id); world.removeEffect(lease.id); }
        lease={};
    }
    void expire(Lease& lease, double dt, bool grip) {
        if(lease.id && (!world.effectExists(lease.id) || (lease.left-=dt)<=0)) clear(lease,grip);
    }
    Result ready(Hero hero) const {
        if(suspended) return Result::Paused;
        return selected==hero ? Result::Ok : Result::Unavailable;
    }
public:
    explicit Heroes(WorldPort& port):world(port) {}
    Heroes(const Heroes&)=delete;
    Heroes& operator=(const Heroes&)=delete;
    ~Heroes() { clear(construct,true); clear(shield,false); }
    Hero active() const { return selected; }
    double power(Hero h) const { return valid(h)?energy[index(h)]:0; }
    double blinkRemaining() const { return blinkCooldown; }
    Id shieldId() const { return shield.id; }
    Id constructId() const { return construct.id; }
    bool paused() const { return suspended; }
    // Logical UI suspend only. The host owns real pause/S8, input focus and clock.
    void pause(bool value) { suspended=value; }
    Result select(Hero hero) {
        if(!valid(hero)) return Result::Invalid;
        if(hero==selected) return Result::Ok;
        if(world.body().carrying || shield.id || construct.id || attackWindow>0) return Result::Busy;
        selected=hero; // No pawn spawn, movement, region or camera operation.
        return Result::Ok;
    }
    void tick(double dt) {
        if(!positive(dt)||suspended) return;
        for(auto& e:energy) e=std::min(100.0,e+dt*5);
        shieldCooldown=std::max(0.0,shieldCooldown-dt);
        constructCooldown=std::max(0.0,constructCooldown-dt);
        blinkCooldown=std::max(0.0,blinkCooldown-dt);
        attackWindow=std::max(0.0,attackWindow-dt);
        if(attackWindow==0) blinkAttack=0;
        expire(shield,dt,false); expire(construct,dt,true);
        if(!shield.id) { shieldHealth=0; absorbed.clear(); }
        if(!construct.id) constructAttack=0;
    }
    Result summon(Effect effect) {
        if(effect!=Effect::Shield&&effect!=Effect::Construct) return Result::Invalid;
        if(auto r=ready(Hero::H02); r!=Result::Ok) return r;
        const bool isShield=effect==Effect::Shield;
        auto& lease=isShield?shield:construct;
        auto& cooldown=isShield?shieldCooldown:constructCooldown;
        if(lease.id) return Result::Busy;
        if(cooldown>0) return Result::Cooldown;
        const double cost=isShield?25:35;
        if(energy[1]<cost) return Result::Energy;
        const Id id=world.newId();
        if(!id || !world.createEffect(id,effect)) return Result::Blocked;
        lease={id,isShield?3.0:5.0}; cooldown=isShield?5.0:6.0; energy[1]-=cost;
        if(isShield) { shieldHealth=100; absorbed.clear(); }
        else constructAttack=world.newId();
        return Result::Ok;
    }
    // Returns absorbed amount only; caller applies remainder once via canonical receiver.
    double absorb(Id attack,double damage) {
        if(suspended||!shield.id||!world.effectExists(shield.id)||!attack||!positive(damage)) return 0;
        if(absorbed.find(attack)!=absorbed.end()) return 0;
        // At most 100 unique positive hits while active. Retire safely at the bound.
        if(absorbed.size()>=100) { clear(shield,false); return 0; }
        absorbed.insert(attack);
        const double amount=std::min(damage,shieldHealth); shieldHealth-=amount;
        if(shieldHealth<=0) clear(shield,false);
        return amount;
    }
    Result grab(Id target) {
        if(auto r=ready(Hero::H02); r!=Result::Ok) return r;
        if(!target) return Result::Invalid;
        if(!construct.id||!world.effectExists(construct.id)) return Result::Unavailable;
        return world.grabWithConstruct(construct.id,target)?Result::Ok:Result::Blocked;
    }
    Result strike(Id target) {
        if(suspended) return Result::Paused;
        if(!target) return Result::Invalid;
        Id* attack=nullptr;
        if(selected==Hero::H02 && construct.id && world.effectExists(construct.id)) attack=&constructAttack;
        if(selected==Hero::H03 && attackWindow>0) attack=&blinkAttack;
        if(!attack||!*attack) return Result::Unavailable;
        if(!world.shortAttack(*attack,selected,target,250)) return Result::Blocked;
        *attack=0; // one successful attack per construct/blink, including repeated input
        return Result::Ok;
    }
    Result blink(Vec direction,double meters=30) {
        if(auto r=ready(Hero::H03); r!=Result::Ok) return r;
        const double n=direction.length();
        if(!direction.finite()||!positive(n)||!std::isfinite(meters)||meters<25||meters>40) return Result::Invalid;
        if(blinkCooldown>0) return Result::Cooldown;
        if(energy[2]<20) return Result::Energy;
        const Body b=world.body();
        if(b.carrying) return Result::Busy; // no unvalidated cargo teleport
        if(!b.position.finite()||!b.velocity.finite()||!b.up.finite()||
           std::abs(b.up.length()-1)>1e-5||!positive(b.radius)||
           !positive(b.halfHeight)||b.halfHeight<b.radius) return Result::Invalid;
        direction=direction*(1/n);
        // Search backwards along the requested segment, never past a blocking wall.
        // Every candidate needs a full capsule sweep, including entry and exit.
        for(double cm=meters*100; cm>=2500; cm-=100) {
            BlinkRequest request{b,b.position+direction*cm};
            if(!request.destination.finite()||!world.safeBlink(request)) continue;
            if(!world.commitBlink(request)) return Result::Blocked;
            energy[2]-=20; blinkCooldown=2; attackWindow=0.75; blinkAttack=world.newId();
            return Result::Ok;
        }
        return Result::Blocked;
    }
    // Explicit host-owned region reset, not a user-facing cooldown bypass.
    void reset() {
        clear(construct,true); clear(shield,false);
        energy={{100,100,100}}; shieldCooldown=constructCooldown=blinkCooldown=0;
        shieldHealth=attackWindow=0; blinkAttack=constructAttack=0; absorbed.clear(); suspended=false;
    }
};

enum class Control { Previous, Next, Confirm, Back, H01Card, H02Card, H03Card };
// Shared virtual controls: keyboard arrows/Enter/Escape, gamepad D-pad/Cross/Circle,
// mouse cards/confirm. Host supplies edges; no global input binding or physical claim.
class Selector {
    Heroes& heroes;
    bool opened=false, previousPause=false;
    Hero highlighted=Hero::H01;
    std::array<bool,7> held{};
    void close() { opened=false; heroes.pause(previousPause); held.fill(false); }
public:
    explicit Selector(Heroes& h):heroes(h) {}
    ~Selector() { if(opened) close(); }
    Selector(const Selector&)=delete;
    Selector& operator=(const Selector&)=delete;
    void open() {
        if(opened) return;
        opened=true; highlighted=heroes.active(); previousPause=heroes.paused(); heroes.pause(true); held.fill(false);
    }
    bool isOpen() const { return opened; }
    Hero focus() const { return highlighted; }
    Result input(Control control,bool down) {
        const auto i=static_cast<std::size_t>(control);
        if(i>=held.size()) return Result::Invalid;
        if(!opened) return Result::Unavailable;
        const bool edge=down&&!held[i]; held[i]=down;
        if(!edge) return Result::Ok;
        const int h=static_cast<int>(highlighted);
        switch(control) {
        case Control::Previous: highlighted=static_cast<Hero>((h+2)%3); break;
        case Control::Next: highlighted=static_cast<Hero>((h+1)%3); break;
        case Control::H01Card: highlighted=Hero::H01; break;
        case Control::H02Card: highlighted=Hero::H02; break;
        case Control::H03Card: highlighted=Hero::H03; break;
        case Control::Back: close(); break;
        case Control::Confirm: {
            const auto result=heroes.select(highlighted);
            if(result==Result::Ok) close();
            return result;
        }}
        return Result::Ok;
    }
};
}
