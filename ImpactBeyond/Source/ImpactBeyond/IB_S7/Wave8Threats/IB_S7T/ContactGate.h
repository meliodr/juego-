#pragma once
#include <cmath>
namespace IBS7T {
// Shared native/UE boundary policy. The adapter supplies measured impact-point distance,
// not sweep-center travel (which would silently extend reach by the sweep radius).
inline bool validContact(double impactDistanceCm,double reachCm,bool startPenetrating,bool regionReady) {
    return regionReady && !startPenetrating && std::isfinite(impactDistanceCm) &&
        std::isfinite(reachCm) && impactDistanceCm>=0 && reachCm>0 && impactDistanceCm<=reachCm;
}
}
