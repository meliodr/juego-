#include "IB_S7/Wave8Threats/Robot/HeavyRobot.h"
#include "IB_S7/Wave8Threats/Kaiju/MediumKaiju.h"
#include "IB_S7/Wave8Threats/Radial/PowerWheel.h"
#include "IB_S7/Wave8Threats/IB_S7T/ContactGate.h"
#include <iostream>
#include <limits>
#include <cstdlib>
using namespace IBS7T;
static int checks=0;
#define CHECK(x) do { ++checks; if(!(x)) { std::cerr << "FAIL line " << __LINE__ << ": " #x "\n"; std::exit(1); } } while(false)
struct MockDamage final:DamagePort {
    int calls=0; bool accept=true; HitRequest last{};
    bool apply(const HitRequest& request) override { if(!accept) return false; ++calls; last=request; return true; }
};
struct MockSelection final:PowerWheelPort {
    int calls=0; bool accept=true; uint32_t last=0;
    bool select(uint32_t id) override { if(!accept) return false; ++calls; last=id; return true; }
};
static void advanceToAttack(Threat& t) {
    for(int i=0;i<200 && t.phase()!=State::Attack;++i) t.tick(0.1,true,true);
    CHECK(t.phase()==State::Attack);
}
int main() {
    DebrisPool pool; HeavyRobot robot(1); MediumKaiju kaiju(2); MockDamage damage;
    const double nan=std::numeric_limits<double>::quiet_NaN();
    CHECK(validContact(450,450,false,true));
    CHECK(!validContact(450.01,450,false,true));
    CHECK(!validContact(475,450,false,true)); // sweep radius must not extend reach
    CHECK(!validContact(0,450,true,true));
    CHECK(!validContact(100,450,false,false));
    CHECK(!validContact(-1,450,false,true));
    CHECK(!validContact(nan,450,false,true));
    CHECK(!validContact(1,nan,false,true));
    CHECK(!validContact(1,0,false,true));
    CHECK(!validContact(1,std::numeric_limits<double>::infinity(),false,true));
    CHECK(!robot.damagePart(robot.epoch(),1,PartId::Hammer,nan));
    CHECK(!robot.damagePart(robot.epoch(),1,PartId::Tail,30));
    CHECK(!robot.detach(PartId::Hammer,pool));
    for(int cycle=0;cycle<20;++cycle) {
        for(PartId part:{PartId::Hammer,PartId::Claw}) {
            CHECK(robot.reset(pool,true));
            CHECK(robot.damagePart(robot.epoch(),1,part,60));
            CHECK(!robot.damagePart(robot.epoch(),1,part,60));
            CHECK(robot.detach(part,pool));
            CHECK(!robot.detach(part,pool));
            CHECK(!robot.attached(part));
            CHECK(!robot.enabled(part==PartId::Hammer?Move::Hammer:Move::Grab));
            CHECK(pool.count()==1);
            CHECK(pool.hold(1,robot.epoch(),part,true));
            CHECK(!pool.hold(1,robot.epoch(),part,true));
            CHECK(!robot.reset(pool,true)); // never erase held part
            CHECK(pool.hold(1,robot.epoch(),part,false));
            const auto old=robot.epoch();
            CHECK(!robot.reset(pool,false));
            CHECK(robot.reset(pool,true)); CHECK(pool.count()==0);
            CHECK(!robot.damagePart(old,2,part,100));
        }
    }
    std::cout << "PASS robot 20 disarms per module, duplicate rejection, 40 mock holds/reset guards\n";
    for(int i=0;i<20;++i) {
        CHECK(robot.reset(pool,true)); CHECK(robot.choose(i%2?Move::Grab:Move::Hammer));
        CHECK(!robot.hit(9,100,true,true,Patch::Building,damage));
        advanceToAttack(robot);
        CHECK(!robot.hit(9,100,false,true,Patch::Building,damage));
        CHECK(!robot.hit(9,100,true,false,Patch::Building,damage));
        CHECK(!robot.hit(9,100,true,true,Patch::Unprepared,damage));
        CHECK(!robot.hit(9,2000,true,true,Patch::Building,damage));
        CHECK(!robot.hit(9,nan,true,true,Patch::Building,damage));
        damage.accept=false; CHECK(!robot.hit(9,100,true,true,Patch::Building,damage));
        damage.accept=true; CHECK(robot.hit(9,100,true,true,Patch::Building,damage));
        CHECK(!robot.hit(9,100,true,true,Patch::Building,damage));
        const auto old=robot.attackId();
        while(robot.phase()==State::Attack) robot.tick(0.1,true,true);
        CHECK(!robot.hit(10,100,true,true,Patch::Building,damage));
        CHECK(!robot.choose(Move::Hammer));
        for(int step=0;step<5;++step) { robot.tick(0.1,true,true); CHECK(robot.attackId()==old); }
        advanceToAttack(robot); CHECK(robot.attackId()>old);
    }
    std::cout << "PASS robot 20 attacks/cooldowns, range/sweep/region/prepared gates (MOCK collision)\n";
    for(PartId part:{PartId::Horn,PartId::Tail,PartId::Shoulder}) {
        for(int i=0;i<20;++i) {
            CHECK(kaiju.reset(pool,true)); CHECK(kaiju.damagePart(kaiju.epoch(),1,part,60));
            CHECK(!kaiju.damagePart(kaiju.epoch(),1,part,60)); CHECK(kaiju.detach(part,pool));
            CHECK(!kaiju.attached(part));
            if(part==PartId::Horn) { CHECK(kaiju.enabled(Move::Charge)); CHECK(kaiju.spec().damage==45); }
            if(part==PartId::Tail) CHECK(!kaiju.enabled(Move::TailSweep));
            CHECK(!kaiju.damageBody(kaiju.epoch(),1,100)); // same AttackId cannot also damage body
            if(part==PartId::Shoulder) { CHECK(kaiju.damageBody(kaiju.epoch(),2,100)); CHECK(kaiju.health()==350); }
        }
    }
    for(int i=0;i<20;++i) {
        CHECK(kaiju.reset(pool,true)); CHECK(kaiju.choose(i%2?Move::TailSweep:Move::Charge));
        advanceToAttack(kaiju); CHECK(kaiju.hit(10,100,true,true,i%2?Patch::Terrain:Patch::Building,damage));
        CHECK(damage.last.move==(i%2?Move::TailSweep:Move::Charge));
        CHECK(!kaiju.hit(10,100,true,true,Patch::Terrain,damage));
    }
    // Interrupt a live attack by detachment. Bounded target ledger fails closed.
    CHECK(kaiju.reset(pool,true)); CHECK(kaiju.choose(Move::TailSweep)); advanceToAttack(kaiju);
    for(uint64_t id=10;id<26;++id) CHECK(kaiju.hit(id,10,true,true,Patch::Terrain,damage));
    CHECK(!kaiju.hit(26,10,true,true,Patch::Terrain,damage));
    CHECK(kaiju.damagePart(kaiju.epoch(),1,PartId::Tail,100)); CHECK(kaiju.detach(PartId::Tail,pool));
    CHECK(kaiju.phase()==State::Stagger); CHECK(!kaiju.hit(30,10,true,true,Patch::Terrain,damage));
    CHECK(kaiju.reset(pool,true)); CHECK(kaiju.damageBody(kaiju.epoch(),1,500));
    CHECK(kaiju.health()==0); CHECK(!kaiju.enabled(Move::Charge)); CHECK(kaiju.phase()==State::Retreat);
    for(int i=0;i<10;++i) { CHECK(kaiju.reset(pool,true)); CHECK(robot.reset(pool,true)); CHECK(pool.count()==0); }
    // Fill all five slots with one robot + one kaiju; budget exhaustion cannot retain a capability.
    uint64_t damageSerial=0;
    for(auto id:{PartId::Hammer,PartId::Claw}) { CHECK(robot.damagePart(robot.epoch(),++damageSerial,id,100)); CHECK(robot.detach(id,pool)); }
    for(auto id:{PartId::Horn,PartId::Tail,PartId::Shoulder}) { CHECK(kaiju.damagePart(kaiju.epoch(),++damageSerial,id,100)); CHECK(kaiju.detach(id,pool)); }
    CHECK(pool.count()==5); CHECK(!pool.add(3,1,PartId::Hammer));
    HeavyRobot fallback(3); CHECK(fallback.damagePart(1,1,PartId::Hammer,100)); CHECK(fallback.detach(PartId::Hammer,pool));
    CHECK(!fallback.enabled(Move::Hammer)); CHECK(pool.count()==5);
    CHECK(robot.reset(pool,true)); CHECK(kaiju.reset(pool,true));
    CHECK(robot.damagePart(robot.epoch(),1,PartId::Hammer,10));
    CHECK(!robot.damagePart(robot.epoch(),1,PartId::Claw,10));
    robot.tick(100,true,true); CHECK(robot.phase()==State::Notice);
    robot.tick(100,true,true); CHECK(robot.phase()==State::Notice);
    robot.tick(100,true,true); CHECK(robot.phase()==State::Telegraph);
    robot.tick(0.1,false,true); CHECK(robot.phase()==State::Recover);
    robot.tick(0.1,true,false); CHECK(robot.phase()==State::Retreat);
    std::cout << "PASS kaiju 20 impacts per zone, 20 mock prepared-patch attacks, 10 paired resets, fixed debris budget\n";
    PowerWheel wheel; MockSelection selection; std::array<Slot,8> slots{};
    for(unsigned i=0;i<8;++i) slots[i]={i+1,"Mock power","PLACEHOLDER_CIRCLE",0,true};
    CHECK(!wheel.configure(slots,5)); CHECK(wheel.configure(slots,8));
    CHECK(!wheel.open(false)); CHECK(!wheel.open(true,nan));
    for(bool stick:{false,true}) for(int i=0;i<30;++i) {
        CHECK(wheel.open(true,0.2)); CHECK(wheel.requestedTimeScale()==0.2);
        wheel.point(0,0,stick); CHECK(wheel.hovered()==-1); CHECK(!wheel.confirm(selection));
        CHECK(wheel.open(true)); wheel.point(1,0,stick); CHECK(wheel.hovered()==0);
        CHECK(wheel.confirm(selection)); CHECK(selection.last==1);
        const auto before=selection.calls; CHECK(wheel.open(true,0.2)); wheel.point(0,1,stick);
        wheel.cancel(); CHECK(!wheel.confirm(selection)); CHECK(selection.calls==before); CHECK(wheel.requestedTimeScale()==1);
    }
    CHECK(wheel.open(true)); wheel.point(1,0,true);
    wheel.point(std::cos(0.4),std::sin(0.4),true); CHECK(wheel.hovered()==0);
    wheel.point(std::cos(0.6),std::sin(0.6),true); CHECK(wheel.hovered()==1); wheel.cancel();
    CHECK(wheel.updateCooldown(1,3)); CHECK(wheel.open(true)); wheel.point(1,0,false); CHECK(!wheel.confirm(selection));
    CHECK(wheel.updateCooldown(1,0)); CHECK(!wheel.updateCooldown(1,-1));
    CHECK(wheel.open(true)); wheel.point(0,1,false); CHECK(wheel.confirm(selection)); CHECK(wheel.selected()==3);
    CHECK(wheel.selectPrevious(selection)); CHECK(wheel.selected()==1);
    selection.accept=false; CHECK(wheel.open(true)); wheel.point(0,1,false); CHECK(!wheel.confirm(selection)); CHECK(wheel.selected()==1); selection.accept=true;
    for(int i=0;i<10;++i) {
        const int before=selection.calls;
        CHECK(wheel.open(true,0.1)); wheel.point(1,0,true); wheel.contextChanged(); // simulated flight enter
        CHECK(!wheel.confirm(selection)); CHECK(wheel.requestedTimeScale()==1);
        CHECK(wheel.open(true,0.1)); wheel.contextChanged(); // simulated flight exit
        CHECK(!wheel.confirm(selection)); CHECK(selection.calls==before);
    }
    for(unsigned count:{6u,7u,8u}) {
        CHECK(wheel.configure(slots,count));
        for(unsigned i=0;i<count;++i) {
            const double angle=6.283185307179586*i/count;
            CHECK(wheel.open(true)); wheel.point(std::cos(angle),std::sin(angle),true);
            CHECK(wheel.hovered()==static_cast<int>(i)); CHECK(wheel.confirm(selection)); CHECK(selection.last==i+1);
        }
    }
    slots[0].available=false; CHECK(wheel.configure(slots,8)); CHECK(wheel.open(true)); wheel.point(1,0,false); CHECK(!wheel.confirm(selection));
    slots[1].ability=slots[0].ability; CHECK(!wheel.configure(slots,8)); CHECK(!wheel.open(true));
    std::cout << "PASS radial 30 selections AND 30 cancellations per simulated device, 10 flight roundtrips, slots 6/7/8\n";
    std::cout << "PASS " << checks << " native checks; NO Unreal, S4 collision, rendered UI or physical controller tested\n";
}
