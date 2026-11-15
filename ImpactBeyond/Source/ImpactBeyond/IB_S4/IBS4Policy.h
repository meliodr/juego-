#pragma once

// Dependency-free policy shared by the lab's UE adapter and native regression tests.
// This is deliberately not a replacement for the eventual S3.5 impact contract.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace IBS4
{
enum class Weapon : uint8_t { Bullet, Missile, Laser, Punch };
enum class Surface : uint8_t { Metal, Concrete, Glass };
enum class Phase : uint8_t { Patrol, Warning, Aim, Fire, Retreat };

struct Response
{
    bool accepted = false;
    // Policy-filtered request. The UE adapter separately resolves actual stored HP.
    double healthDelta = 0;
    // Delta-velocity magnitude in cm/s, not a physical impulse in kg*cm/s.
    double pushSpeed = 0;
};

inline bool validHealth(float health)
{
    return std::isfinite(health) && health >= 0 && health <= 100;
}
struct HealthChange { bool valid = false; float next = 0; float delta = 0; };
inline HealthChange resolveHealth(float health, double requestedDelta)
{
    if (!validHealth(health) || !std::isfinite(requestedDelta) || requestedDelta > 0) return {};
    const float next = std::clamp(health + static_cast<float>(std::max(requestedDelta, -1000.0)), 0.f, 100.f);
    return {true, next, next - health}; // delta reflects the precision of stored float HP
}

class Receiver
{
    struct Seen { uint64_t id = 0; double expiry = -1; };
    std::array<Seen, 256> seen{};
    double nextPush = 0;
    uint64_t retiredThrough = 0;
public:
    // Packets are valid for two seconds. A full live ledger fails closed rather than
    // evicting an ID and allowing duplicate damage. Each target owns one ledger.
    // MakeHit allocates monotonically increasing IDs at contact time. Retiring a
    // packet seals older serials permanently, even if a caller refreshes its timestamp.
    // Arbitrary external GUIDs/counters are NOT compatible with this lab contract.
    Response receive(uint64_t id, double issued, double now, Weapon weapon,
                     bool h01, double amount, double push = 1000, double cooldown = 1.1)
    {
        if (!id || static_cast<uint8_t>(weapon) > static_cast<uint8_t>(Weapon::Punch) ||
            !std::isfinite(now) || !std::isfinite(issued) ||
            !std::isfinite(amount) || !std::isfinite(push) || !std::isfinite(cooldown) ||
            now < 0 || issued < 0 || issued > now + 0.01 || now - issued > 2 || amount < 0 || cooldown < 0)
            return {};
        for (auto& item : seen)
        {
            if (item.id && item.expiry < now)
            {
                retiredThrough = std::max(retiredThrough, item.id);
                item = {};
            }
        }
        if (id <= retiredThrough) return {};
        Seen* free = nullptr;
        for (auto& item : seen)
        {
            if (item.expiry >= now && item.id == id) return {};
            if (item.expiry < now && !free) free = &item;
        }
        if (!free) return {};
        *free = {id, issued + 2.01};
        Response out{true, -std::min(amount, 1000.0), 0};
        if (h01 && (weapon == Weapon::Bullet || weapon == Weapon::Missile))
            out.healthDelta = 0;
        if (weapon == Weapon::Missile && now >= nextPush)
        {
            out.pushSpeed = std::clamp(push, 0.0, 1200.0);
            nextPush = now + std::max(cooldown, 0.5);
        }
        return out;
    }
};

inline double laserDamage(Surface surface, double step)
{
    const double rate = surface == Surface::Glass ? 160 : surface == Surface::Concrete ? 60 : 30;
    return rate * std::clamp(step, 0.0, 0.1);
}

// A deterministic prepared crater: a 5x5 tiled square with 30 cm steps and a
// 60 cm central depression. Its outer ring stays flush with the surrounding floor.
inline double craterDepth(int x, int y)
{
    const int ring = std::max(std::abs(x), std::abs(y));
    return ring >= 2 ? 0.0 : ring == 1 ? 30.0 : 60.0;
}

struct RestoreBox { double x, y, z, hx, hy, hz; };
inline RestoreBox robotBodyBox() { return {0, 0, 85, 50, 45, 65}; }
inline RestoreBox cannonAttachmentBox() { return {115, 0, 135, 50, 15, 15}; }
// Exact volume newly occupied when a broken crater tile rises to z=0. The outer
// ring contributes no restore volume. Used by runtime queries and native tests.
inline RestoreBox craterRestoreVolume(int x, int y)
{
    const double depth = craterDepth(x, y);
    return {x * 100.0, y * 100.0, -depth / 2, 50, 50, depth / 2};
}

inline bool canRestore(bool occupied, bool heldPart)
{
    return !occupied && !heldPart;
}

class Alert
{
public:
    Phase phase = Phase::Patrol;
    uint64_t cause = 0;
    double expires = 0;
    double changed = 0;
    bool report(uint64_t incident, bool attributedToTarget, bool verified, double now)
    {
        if (!incident || !attributedToTarget || !verified || !std::isfinite(now)) return false;
        if (incident == cause) return false;
        cause = incident;
        expires = now + 18;
        // A fresh report does not shorten the warning or restart a firing loop.
        if (phase == Phase::Patrol) { phase = Phase::Warning; changed = now; }
        return true;
    }
    Phase update(double now, bool visible, bool tooClose)
    {
        if (!cause || now >= expires) { phase = Phase::Patrol; cause = 0; return phase; }
        if (tooClose) { phase = Phase::Retreat; changed = now; return phase; }
        if (!visible)
        {
            if (phase == Phase::Fire || phase == Phase::Aim) { phase = Phase::Warning; changed = now; }
            return phase;
        }
        if (phase == Phase::Retreat && now - changed >= 1) { phase = Phase::Warning; changed = now; }
        else if (phase == Phase::Warning && now - changed >= 2) { phase = Phase::Aim; changed = now; }
        else if (phase == Phase::Aim && now - changed >= 1) { phase = Phase::Fire; changed = now; }
        return phase;
    }
};
}
