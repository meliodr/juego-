#include "IBS4Policy.h"
#include "IBS4_ConsumerFixtures.h"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

static int Checks = 0;
static void check(bool condition, const char* message)
{
    ++Checks;
    if (!condition) throw std::runtime_error(message);
}

int main()
{
    using namespace IBS4;
    try
    {
        Receiver hero;
        double health = 100;
        for (uint64_t i = 1; i <= 20; ++i)
        {
            auto r = hero.receive(i, 0, 0, Weapon::Bullet, true, 900);
            health += r.healthDelta;
            check(r.accepted && r.pushSpeed == 0 && health == 100, "human bullet changed H01 health/control");
            check(!hero.receive(i, 0, 0, Weapon::Bullet, true, 900).accepted, "duplicate bullet accepted");
        }
        for (uint64_t i = 1; i <= 20; ++i)
        {
            const double now = static_cast<double>(i) * 2;
            auto r = hero.receive(100 + 2 * i, now, now, Weapon::Missile, true, 900, 1000);
            health += r.healthDelta;
            check(r.accepted && r.pushSpeed == 1000 && health == 100, "spaced missile policy failed");
            auto rapid = hero.receive(101 + 2 * i, now + .1, now + .1, Weapon::Missile, true, 900);
            check(rapid.accepted && rapid.healthDelta == 0 && rapid.pushSpeed == 0, "rapid missile stunlock protection failed");
        }
        Receiver npc;
        check(npc.receive(1, 0, 0, Weapon::Bullet, false, 10).healthDelta == -10, "human immunity leaked to NPC");
        check(hero.receive(1000, 50, 50, Weapon::Laser, true, 3).healthDelta == -3, "H01 made immune to all weapons");
        for (const Surface material : {Surface::Metal, Surface::Concrete, Surface::Glass})
        {
            Receiver coupon;
            double sum = 0;
            for (uint64_t i = 1; i <= 20; ++i)
            {
                const double now = static_cast<double>(i) * .1;
                const double amount = laserDamage(material, .1);
                auto r = coupon.receive(i, now, now, Weapon::Laser, false, amount);
                check(r.accepted, "laser sample rejected");
                sum -= r.healthDelta;
                check(!coupon.receive(i, now, now, Weapon::Laser, false, amount).accepted, "double laser tick");
            }
            const double expected = material == Surface::Metal ? 60 : material == Surface::Concrete ? 120 : 320;
            check(std::abs(sum - expected) < .0001, "material rate mismatch");
            check(laserDamage(material, 5) == laserDamage(material, .1), "hitch caused laser damage burst");
        }
        for (int repeat = 0; repeat < 20; ++repeat)
        {
            for (int x = -2; x <= 2; ++x) for (int y = -2; y <= 2; ++y)
            {
                const double d = craterDepth(x, y);
                check(d >= 0 && d <= 60, "crater depth out of range");
                check(160 - d >= 100, "crater removed supporting floor");
                if (std::max(std::abs(x), std::abs(y)) == 2) check(d == 0, "outer ring not flush");
            }
        }
        for (int repeat = 0; repeat < 10; ++repeat)
        {
            Alert alert;
            check(alert.update(0, true, false) == Phase::Patrol, "seeing player triggered aggression");
            check(!alert.report(1, false, true, 0), "enemy incident blamed on H01");
            check(!alert.report(1, true, false, 0), "unverified report accepted");
            check(alert.report(1, true, true, 0), "verified cause rejected");
            check(!alert.report(1, true, true, .5), "same cause duplicated");
            check(alert.update(1.99, true, false) == Phase::Warning, "warning skipped");
            check(alert.update(2, true, false) == Phase::Aim, "aim transition missing");
            check(alert.update(3, true, false) == Phase::Fire, "fire transition missing");
            check(alert.update(3.1, false, false) == Phase::Warning, "fired through occlusion");
            check(alert.update(4, true, true) == Phase::Retreat, "retreat transition missing");
            check(alert.update(20, true, false) == Phase::Patrol, "alert did not expire");
        }
        Receiver saturated;
        for (uint64_t i = 1; i <= 256; ++i) check(saturated.receive(i, 0, 0, Weapon::Bullet, true, 10).accepted, "ledger capacity");
        check(!saturated.receive(257, 0, 0, Weapon::Bullet, true, 10).accepted, "full ledger failed open");
        check(!saturated.receive(1, 0, 0, Weapon::Bullet, true, 10).accepted, "evicted live ID");
        check(!saturated.receive(1, 0, 3, Weapon::Bullet, true, 10).accepted, "expired packet replayed");
        check(saturated.receive(258, 3, 3, Weapon::Bullet, true, 10).accepted, "expired slots not reusable");
        check(!saturated.receive(259, 100, 3, Weapon::Bullet, true, 10).accepted, "future packet accepted");
        check(!saturated.receive(260, 3, 3, Weapon::Bullet, true, -10).accepted, "negative damage accepted");
        check(!saturated.receive(261, 3, 3, Weapon::Missile, true, 10, std::numeric_limits<double>::infinity()).accepted, "infinite impulse accepted");
        check(!saturated.receive(262, 3, std::numeric_limits<double>::quiet_NaN(), Weapon::Bullet, true, 10).accepted, "NaN accepted");
        Receiver replay;
        check(replay.receive(1, 0, 0, Weapon::Missile, true, 10).accepted, "initial replay fixture rejected");
        check(!replay.receive(1, 3, 3, Weapon::Missile, true, 10).accepted, "expired AttackId resurrected by refreshed IssuedAt");
        Receiver clock;
        check(!clock.receive(1, -.5, -.5, Weapon::Bullet, true, 10).accepted, "negative world timestamp accepted");
        check(clock.receive(1, 0, 0, Weapon::Bullet, true, 10).accepted, "invalid clock consumed attack ID");
        check(!replay.receive(1, 4, 4, Weapon::Missile, true, 10).accepted, "retired ID resurrected twice");
        check(replay.receive(2, 4, 4, Weapon::Missile, true, 10).accepted, "fresh monotonic ID rejected after retirement");
        Receiver unordered;
        check(unordered.receive(3, 0, 0, Weapon::Punch, false, 10).accepted, "live unordered fixture");
        check(unordered.receive(2, 0, .1, Weapon::Punch, false, 10).accepted, "live in-window delivery rejected");
        check(!unordered.receive(1, 3, 3, Weapon::Punch, false, 10).accepted, "old external serial bypassed retired floor");
        Receiver kinds;
        check(!kinds.receive(1, 0, 0, static_cast<Weapon>(255), true, 10).accepted, "unknown enum accepted");
        auto punch = kinds.receive(1, 0, 0, Weapon::Punch, true, 60);
        check(punch.accepted && punch.healthDelta == -60 && punch.pushSpeed == 0, "Punch got human-Missile immunity or push");
        check(!kinds.receive(2, 0, 0, Weapon::Punch, true, -60).accepted, "S3.5 negative delta accepted as S4 magnitude");
        check(kinds.receive(2, 0, 0, Weapon::Punch, true, 60).accepted, "bad signed amount consumed ID");
        Receiver units;
        check(units.receive(1, 0, 0, Weapon::Missile, true, 10, 100).pushSpeed == 100, "cm/s magnitude changed by implicit mass conversion");
        check(units.receive(2, 2, 2, Weapon::Missile, true, 10, 120000).pushSpeed == 1200, "missile delta velocity cap lost");
        auto change = resolveHealth(10, punch.healthDelta);
        check(change.valid && change.next == 0 && change.delta == -10, "overkill reported as effective HP loss");
        check(punch.healthDelta == -60, "structural request erased by HP clamp");
        change = resolveHealth(0, -60);
        check(change.valid && change.next == 0 && change.delta == 0, "zero HP published fictitious loss");
        change = resolveHealth(100, -1e-8);
        check(change.valid && change.next == 100 && change.delta == 0, "sub-float damage published fictitious loss");
        check(!resolveHealth(-1, -10).valid, "negative health accepted");
        check(!resolveHealth(101, -10).valid, "health above cap accepted");
        check(!resolveHealth(std::numeric_limits<float>::quiet_NaN(), -10).valid, "NaN health accepted");
        check(!resolveHealth(50, 10).valid, "damage path accepted healing");
        check(!resolveHealth(50, -std::numeric_limits<double>::infinity()).valid, "infinite requested delta accepted");
        check(resolveHealth(100, hero.receive(2000, 60, 60, Weapon::Missile, true, 999).healthDelta).delta == 0,
            "resolved health broke H01 missile immunity");
        const auto overlaps = [](const RestoreBox& a, const RestoreBox& b)
        {
            return a.hx > 0 && a.hy > 0 && a.hz > 0 && b.hx > 0 && b.hy > 0 && b.hz > 0 &&
                std::abs(a.x - b.x) < a.hx + b.hx && std::abs(a.y - b.y) < a.hy + b.hy && std::abs(a.z - b.z) < a.hz + b.hz;
        };
        int risingTiles = 0;
        for (int x = -2; x <= 2; ++x) for (int y = -2; y <= 2; ++y)
        {
            const auto volume = craterRestoreVolume(x, y);
            check(volume.z + volume.hz == 0 && volume.z - volume.hz == -craterDepth(x, y), "restore volume differs from raised floor");
            if (volume.hz > 0) ++risingTiles;
            check(!overlaps(volume, {0, 0, 140, 38, 38, 90}), "hovering pawn falsely blocks crater restore");
            check(!overlaps(volume, {300, 0, 20, 20, 20, 20}), "outside prop falsely blocks restore");
        }
        check(risingTiles == 9, "unchanged outer ring queried as raised geometry");
        check(overlaps(craterRestoreVolume(0, 0), {0, 0, 30, 38, 38, 90}), "occupant on crater bottom not protected");
        check(overlaps(craterRestoreVolume(1, 0), {100, 0, -15, 5, 5, 5}), "kinematic blocker in rising ring omitted");
        check(!canRestore(true, false) && !canRestore(false, true) && !canRestore(true, true), "restore ignores occupant/held cannon");
        check(canRestore(false, false), "clear restore rejected");
        const auto body = robotBodyBox();
        const auto cannon = cannonAttachmentBox();
        check(!overlaps(body, cannon), "detached cannon begins embedded in torso");
        check((cannon.x - cannon.hx) - (body.x + body.hx) == 15, "cannon separation clearance changed");
        const double cargoSphere = std::sqrt(cannon.hx * cannon.hx + cannon.hy * cannon.hy + cannon.hz * cannon.hz) + 2;
        check(cannon.x - cargoSphere > body.x + body.hx, "conservative carry sphere starts embedded in torso");
        const int BeforeConsumer = Checks;
        RunS4ConsumerFixtures(check);
        std::cout << "PASS consumer fixtures: " << Checks - BeforeConsumer << " checks (real S4 policy, no UObject/CMC execution).\n";
        std::cout << "PASS native policy: " << Checks << " checks; 20 bullets, 20 spaced+rapid missiles, 60 material samples, 20 crater layouts, 10 alert sequences.\n";
        std::cout << "NOT RUN: Unreal compilation, collision/physics, rendered FX/audio, gamepad, DX12 performance.\n";
        return 0;
    }
    catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
