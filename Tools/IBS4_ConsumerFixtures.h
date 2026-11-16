#pragma once
#include "IBS4Policy.h"
#include <limits>

// Exercises the real S4 policy used by ConsumeBoundHit, without mocking Unreal.
// UObject/delegate/CMC binding is covered by prepared UE tests, not by this file.
inline void RunS4ConsumerFixtures(void (*check)(bool, const char*))
{
    using namespace IBS4;
    Receiver hero;
    float health = 37; // invariance must not mean silently resetting H01 to 100
    uint64_t id = 1;
    for (int shot = 0; shot < 20; ++shot)
    {
        auto result = hero.receive(id, 0, 0, Weapon::Bullet, true, 999, 1000, 1.1, false, true);
        auto change = resolveHealth(health, result.healthDelta);
        check(result.accepted && change.valid && change.delta == 0 && change.next == health, "consumer bullet changes H01 HP");
        check(result.pushSpeed == 0, "consumer bullet interrupts movement");
        check(!hero.receive(id, 0, 0, Weapon::Bullet, true, 999, 1000, 1.1, false, true).accepted, "consumer duplicate bullet");
        health = change.next;
        ++id;
    }
    for (int shot = 0; shot < 20; ++shot)
    {
        const double time = 3 + shot * 2.0;
        auto result = hero.receive(id++, time, time, Weapon::Missile, true, 999, 1000, 1.1, false, true);
        check(result.accepted && result.healthDelta == 0 && result.pushSpeed == 1000, "consumer spaced missile recovery");
        for (int rapid = 1; rapid <= 10; ++rapid)
        {
            const double next = time + rapid * .1;
            auto chained = hero.receive(id++, next, next, Weapon::Missile, true, 999, 1000, 1.1, false, true);
            check(chained.accepted && chained.healthDelta == 0 && chained.pushSpeed == 0, "consumer cooldown slides or rapid missile pushes");
        }
    }
    for (const Weapon weapon : {Weapon::Punch, Weapon::Laser})
    {
        for (int shot = 0; shot < 20; ++shot)
        {
            auto result = hero.receive(id++, 50, 50, weapon, true, 60, 1000, 1.1, false, true);
            check(result.accepted && result.healthDelta == 0 && result.pushSpeed == 0, "bound H01 profile loses HP or misclassifies Punch as Missile");
        }
    }
    Receiver target;
    auto request = target.receive(1, 100, 100, Weapon::Punch, false, 60, 1000, 1.1, true, true);
    auto change = resolveHealth(10, request.healthDelta);
    check(request.accepted && request.healthDelta == -60 && change.next == 0 && change.delta == -10, "non-H01 requested versus actual damage");
    check(request.pushSpeed == 0, "resolved physical hit pushed twice");
    check(!target.receive(1, 100, 100, Weapon::Punch, false, 60, 1000, 1.1, false, true).accepted, "changing resolved flag replays damage");
    check(!target.receive(1, 104, 104, Weapon::Punch, false, 60, 1000, 1.1, false, true).accepted, "renewed time resurrects consumer serial");
    check(!target.receive(2, 104, 104, Weapon::Punch, false, -60, 1000, 1.1, false, true).accepted, "negative canonical delta passed as magnitude");
    check(target.receive(2, 104, 104, Weapon::Punch, false, 60, 1000, 1.1, false, true).accepted, "bad sign consumed consumer serial");

    Receiver shared;
    check(shared.receive(1, 0, 0, Weapon::Missile, true, 500, 1000, 1.1, true, true).pushSpeed == 0, "resolved missile double push");
    check(shared.receive(2, .1, .1, Weapon::Missile, true, 500).pushSpeed == 0, "standalone path bypasses consumer cooldown");
    check(shared.receive(3, 1.1, 1.1, Weapon::Missile, true, 500, 1000, 1.1, false, true).pushSpeed == 1000, "resolved contact permanently blocks recovery");
    check(!shared.receive(3, 1.1, 1.1, Weapon::Missile, true, 500).accepted, "standalone path bypasses consumer ledger");
    // Modes are inputs to one ledger, not separate receivers. No reset on rebind.
    check(shared.receive(4, 3, 3, Weapon::Punch, true, 60).healthDelta == -60, "standalone profile unexpectedly changed");
    check(shared.receive(5, 3, 3, Weapon::Punch, true, 60, 1000, 1.1, false, true).healthDelta == 0, "bound protected profile missing");

    Receiver deadlines;
    check(!deadlines.receive(1, .1, 100, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "relative age mistaken for absolute contact time");
    check(!deadlines.receive(1, 98, 100.0001, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "expired contact accepted");
    check(deadlines.receive(1, 98, 100, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "TTL boundary or invalid request consumed ID");
    check(!deadlines.receive(2, 101.0101, 101, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "future tolerance exceeded");
    check(deadlines.receive(2, 101.01, 101, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "future tolerance boundary");
    check(!deadlines.receive(3, -1, 0, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "negative consumer clock");
    check(!deadlines.receive(3, 102, 102, static_cast<Weapon>(255), false, 10, 1000, 1.1, false, true).accepted, "unknown consumer weapon");
    check(deadlines.receive(3, 102, 102, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "invalid clock/enum consumed ID");

    Receiver units;
    auto velocity = units.receive(1, 0, 0, Weapon::Missile, true, 0, 100, 1.1, false, true);
    check(velocity.accepted && velocity.pushSpeed == 100, "cm/s became kg*cm/s");
    check(units.receive(2, 2, 2, Weapon::Missile, true, 0, 120000, 1.1, false, true).pushSpeed == 1200, "consumer push cap missing");
    check(!units.receive(3, 4, 4, Weapon::Missile, true, 0, std::numeric_limits<double>::quiet_NaN(), 1.1, false, true).accepted, "consumer NaN push accepted");
    check(units.receive(3, 4, 4, Weapon::Missile, true, 0, 1000, 1.1, false, true).pushSpeed == 1000, "invalid push consumed ID/cooldown");

    Receiver saturated;
    for (uint64_t serial = 1; serial <= 256; ++serial)
        check(saturated.receive(serial, 0, 0, Weapon::Bullet, true, 10, 1000, 1.1, false, true).accepted, "consumer ledger capacity");
    check(!saturated.receive(257, 0, 0, Weapon::Missile, true, 10, 1000, 1.1, false, true).accepted, "consumer ledger overflow fails open");
    check(saturated.receive(258, 3, 3, Weapon::Missile, true, 10, 1000, 1.1, false, true).pushSpeed == 1000, "saturation consumed cooldown or never recovered");
    Receiver otherTarget;
    check(otherTarget.receive(1, 0, 0, Weapon::Punch, false, 10, 1000, 1.1, false, true).accepted, "target-local ledger leaks across recipients");
}
