#include "IBS6Heroes.h"
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>
using namespace IBS6H;
static int checks=0;
static void check(bool ok,const char* label) {
    ++checks; if(!ok) throw std::runtime_error(label);
}
struct Box { Vec min,max; };
// Independent analytic slab intersection, continuous along the full segment.
// Conservative sphere bounds the capsule in this mock (not Unreal collision QA).
static bool intersects(Vec a,Vec b,Box box,double radius) {
    double lo=0,hi=1;
    const double start[]={a.x,a.y,a.z}, end[]={b.x,b.y,b.z};
    const double mn[]={box.min.x-radius,box.min.y-radius,box.min.z-radius};
    const double mx[]={box.max.x+radius,box.max.y+radius,box.max.z+radius};
    for(int i=0;i<3;++i) {
        const double d=end[i]-start[i];
        if(std::abs(d)<1e-12) { if(start[i]<mn[i]||start[i]>mx[i]) return false; }
        else {
            double t1=(mn[i]-start[i])/d,t2=(mx[i]-start[i])/d;
            if(t1>t2) std::swap(t1,t2);
            lo=std::max(lo,t1); hi=std::min(hi,t2); if(lo>hi) return false;
        }
    }
    return true;
}
struct MockWorld final:WorldPort {
    Body state{{0,0,1000},{120,30,8},{0,0,1},42,96,true,false,7};
    std::vector<Box> walls;
    std::map<Id,Effect> effects;
    std::set<Id> damageIds;
    Id serial=0, heldBy=0, heldTarget=0;
    int moves=0, damage=0, drops=0;
    double loadedUntil=1e9, maxAttackRange=250;
    bool spawnBlocked=false, commitRace=false, targetValid=true;
    Body body() const override { return state; }
    bool safeBlink(const BlinkRequest& r) const override {
        if(r.destination.x>loadedUntil || r.origin.position.x>loadedUntil) return false;
        if(!r.origin.airborne && r.destination.z<r.origin.halfHeight) return false;
        for(auto box:walls) if(intersects(r.origin.position,r.destination,box,r.origin.halfHeight)) return false;
        return true;
    }
    bool commitBlink(const BlinkRequest& r) override {
        if(commitRace || !safeBlink(r)) return false;
        if((state.position+r.origin.position*-1).length()>0) return false;
        state.position=r.destination; ++moves; return true;
    }
    bool createEffect(Id id,Effect type) override {
        if(spawnBlocked || effects.count(id)) return false;
        effects.emplace(id,type); return true;
    }
    void removeEffect(Id id) override { effects.erase(id); }
    bool effectExists(Id id) const override { return effects.count(id)!=0; }
    bool grabWithConstruct(Id id,Id target) override {
        if(!effectExists(id)||!targetValid||heldTarget) return false;
        heldBy=id; heldTarget=target; state.carrying=true; return true;
    }
    void dropConstruct(Id id) override {
        if(heldBy==id) { heldBy=heldTarget=0; state.carrying=false; ++drops; }
    }
    bool shortAttack(Id attack,Hero,Id target,double range) override {
        if(!target||!targetValid||range>maxAttackRange||!damageIds.insert(attack).second) return false;
        ++damage; return true;
    }
    Id newId() override { return ++serial; }
};
static void h02() {
    MockWorld w; Heroes h(w); check(h.select(Hero::H02)==Result::Ok,"H02 selection");
    for(int i=0;i<30;++i) {
        check(h.summon(Effect::Construct)==Result::Ok,"construct spawn");
        check(w.effects.size()==1,"one construct collider");
        check(h.summon(Effect::Construct)==Result::Busy,"no duplicate construct");
        check(h.grab(11)==Result::Ok,"canonical grip mock");
        check(h.select(Hero::H03)==Result::Busy,"cannot switch holding construct");
        check(h.strike(20)==Result::Ok,"construct damage");
        check(h.strike(20)==Result::Unavailable,"construct no repeat damage");
        h.tick(4.99); check(w.effects.size()==1,"construct duration");
        h.tick(.02); check(w.effects.empty()&&!w.state.carrying,"expiry releases grip and collider");
        check(h.summon(Effect::Construct)==Result::Cooldown,"construct cooldown");
        h.tick(10);
    }
    check(w.drops==30&&w.damage==30,"30 grip/damage cycles");
    for(int i=0;i<20;++i) {
        check(h.summon(Effect::Shield)==Result::Ok,"shield spawn");
        check(h.summon(Effect::Shield)==Result::Busy,"no duplicate shield");
        check(h.absorb(w.newId(),25)==25,"shield absorption");
        const Id hit=w.newId(); check(h.absorb(hit,25)==25,"second shield hit");
        check(h.absorb(hit,25)==0,"no repeat absorption");
        h.tick(2.99); check(w.effects.size()==1,"shield duration");
        h.tick(.02); check(w.effects.empty(),"shield collider expiry");
        h.tick(10);
    }
    w.spawnBlocked=true; const double energy=h.power(Hero::H02);
    check(h.summon(Effect::Construct)==Result::Blocked,"occupied construct spawn");
    check(h.power(Hero::H02)==energy,"failed creation no charge");
    w.spawnBlocked=false;
    for(int i=0;i<10;++i) {
        check(h.summon(Effect::Shield)==Result::Ok,"reset shield");
        check(h.summon(Effect::Construct)==Result::Ok,"reset construct");
        check(h.grab(11)==Result::Ok,"reset carrying");
        h.reset(); h.reset(); check(w.effects.empty()&&!w.state.carrying,"idempotent reset");
    }
    check(h.summon(Effect::Shield)==Result::Ok,"shield exhaustion spawn");
    check(h.absorb(w.newId(),150)==100&&w.effects.empty(),"shield capacity");
    h.reset(); check(h.summon(Effect::Construct)==Result::Ok,"external destroy setup");
    w.effects.clear(); check(h.strike(2)==Result::Unavailable,"no ghost construct attack");
    h.tick(.01); check(h.constructId()==0,"external destruction reconciled");
    std::cout<<"PASS H02: 30 construct/grip/impact cycles, 20 shields, 10 active resets\n";
}
static void h03() {
    MockWorld w; Heroes h(w); h.select(Hero::H03);
    for(int i=0;i<50;++i) {
        w.state.position={0,0,1000};
        check(h.blink({1,0,0},25+(i%16))==Result::Ok,"safe blink");
        check(w.state.position.x>=2500&&w.state.position.x<=4000,"distance limits");
        check(w.state.velocity.x==120&&w.state.region==7,"preserved mock velocity/region");
        check(h.blink({1,0,0})==Result::Cooldown,"blink cooldown");
        check(h.strike(20)==Result::Ok,"post arrival attack");
        check(h.strike(20)==Result::Unavailable,"post blink attack once");
        h.tick(10);
    }
    const int moves=w.moves;
    for(int i=0;i<20;++i) {
        w.state.position={0,0,1000};
        const double x=i<10?1200.0:2450.0;
        w.walls={{{x,-200,0},{x+.01,200,2000}}};
        const double energy=h.power(Hero::H03);
        check(h.blink({1,0,0})==Result::Blocked,"thin wall or occupied destination rejects full path");
        check(h.power(Hero::H03)==energy&&h.blinkRemaining()==0,"failure consumes no state");
        check(h.strike(2)==Result::Unavailable,"no attack after rejected blink");
    }
    check(w.moves==moves,"20 blocked no moves"); w.walls.clear();
    w.walls={{{3100,-200,0},{3200,200,2000}}};
    check(h.blink({1,0,0},40)==Result::Ok&&w.state.position.x==3000,"safe retreat before wall");
    h.tick(10); w.walls.clear(); w.state.position={0,0,1000};
    w.walls={{{-1,-100,0},{1,100,2000}}};
    check(h.blink({1,0,0})==Result::Blocked,"entry overlap"); w.walls.clear();
    w.commitRace=true; check(h.blink({1,0,0})==Result::Blocked,"commit dynamic race");
    check(h.blinkRemaining()==0,"race no cooldown"); w.commitRace=false;
    w.loadedUntil=2000; check(h.blink({1,0,0})==Result::Blocked,"unloaded path"); w.loadedUntil=1e9;
    w.state.carrying=true; check(h.blink({1,0,0})==Result::Busy,"cargo denied"); w.state.carrying=false;
    w.state.airborne=false; check(h.blink({0,0,-1})==Result::Blocked,"underground denied");
    w.state.airborne=true; w.state.up={0,1,0};
    check(h.blink({1,0,0})==Result::Ok,"radial up carried to mock");
    h.tick(1); check(h.strike(2)==Result::Unavailable,"attack window expires"); h.tick(10);
    check(h.blink({0,0,0})==Result::Invalid,"zero direction");
    check(h.blink({1,0,0},24)==Result::Invalid,"minimum distance");
    check(h.blink({1,0,0},41)==Result::Invalid,"maximum distance");
    check(h.blink({std::numeric_limits<double>::quiet_NaN(),0,0})==Result::Invalid,"nan direction");
    w.state.up={}; check(h.blink({1,0,0})==Result::Invalid,"invalid gravity");
    std::cout<<"PASS H03: 50 safe blinks, 20 blocked thin-wall/occupied paths; race/cargo/streaming mock checks\n";
    std::cout<<"SKIPPED: 10 real S3 cross-cell jumps (no integrated region adapter)\n";
}
static void selector() {
    MockWorld w; Heroes h(w); Selector ui(h);
    for(int i=0;i<30;++i) {
        ui.open(); check(h.paused(),"selector suspends abilities");
        check(h.blink({1,0,0})==Result::Paused,"no ability leaks");
        // Alternates virtual gamepad navigation and mouse card activation.
        const Hero expected=static_cast<Hero>((static_cast<int>(h.active())+1)%3);
        if(i%2==0) {
            ui.input(Control::Next,true); ui.input(Control::Next,true);
            check(ui.focus()==expected,"held gamepad input edge once");
        } else ui.input(static_cast<Control>(4+static_cast<int>(expected)),true);
        check(ui.input(Control::Confirm,true)==Result::Ok,"confirm selection");
        check(h.active()==expected&&!ui.isOpen()&&!h.paused(),"selection/resume");
        check(w.moves==0&&w.effects.empty(),"selection no world/camera side effects");
    }
    h.select(Hero::H03); check(h.blink({1,0,0})==Result::Ok,"independent state setup");
    h.tick(.8); const double saved=h.power(Hero::H03), remaining=h.blinkRemaining();
    h.select(Hero::H01); h.select(Hero::H02);
    check(h.power(Hero::H02)==100&&h.power(Hero::H03)==saved,"hero energy isolated");
    h.select(Hero::H03); check(h.blinkRemaining()==remaining,"selection no cooldown reset");
    for(int i=0;i<10;++i) {
        h.pause(true); ui.open(); ui.input(Control::Next,true); ui.input(Control::Back,true);
        check(h.active()==Hero::H03&&h.paused(),"cancel restores previous pause and hero");
        h.tick(100); check(h.power(Hero::H03)==saved,"paused clock frozen"); h.pause(false);
    }
    w.state.carrying=true; ui.open(); ui.input(Control::Next,true);
    check(ui.input(Control::Confirm,true)==Result::Busy&&ui.isOpen(),"carrying selection blocked");
    ui.input(Control::Back,true); w.state.carrying=false;
    check(h.select(static_cast<Hero>(99))==Result::Invalid,"invalid hero");
    ui.open(); check(ui.input(static_cast<Control>(99),true)==Result::Invalid,"invalid input");
    ui.input(Control::Back,true);
    std::cout<<"PASS selector: 30 virtual mouse/gamepad changes, 10 pause/return cycles, separate state\n";
}
int main() {
    try { h02(); h03(); selector(); std::cout<<"PASS total assertions="<<checks<<"\n"; }
    catch(const std::exception& e) { std::cerr<<"FAIL: "<<e.what()<<" after "<<checks<<" assertions\n"; return 1; }
}
