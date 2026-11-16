#pragma once
#include "../ThreatModel.h"
namespace IBS7T {
// RBT03 technical PLACEHOLDER: 5-6 m design target, no mesh or final art supplied.
class HeavyRobot final : public Threat { public: explicit HeavyRobot(uint64_t id) : Threat(id,false) {} };
}
