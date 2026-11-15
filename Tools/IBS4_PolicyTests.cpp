#include "IBS4Policy.h"
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
            auto r = hero.receive(100 + i, now, now, Weapon::Missile, true, 900, 1000);
            health += r.healthDelta;
            check(r.accepted && r.pushSpeed == 1000 && health == 100, "spaced missile policy failed");
            auto rapid = hero.receive(200 + i, now + .1, now + .1, Weapon::Missile, true, 900);
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
        std::cout << "PASS native policy: " << Checks << " checks; 20 bullets, 20 spaced+rapid missiles, 60 material samples, 20 crater layouts, 10 alert sequences.\n";
        std::cout << "NOT RUN: Unreal compilation, collision/physics, rendered FX/audio, gamepad, DX12 performance.\n";
        return 0;
    }
    catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
