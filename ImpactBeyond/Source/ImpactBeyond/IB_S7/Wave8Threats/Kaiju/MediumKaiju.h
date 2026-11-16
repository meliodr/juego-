#pragma once
#include "../ThreatModel.h"
namespace IBS7T {
// KAI01 technical PLACEHOLDER: 10-12 m design target, no creature asset supplied.
class MediumKaiju final : public Threat { public: explicit MediumKaiju(uint64_t id) : Threat(id,true) {} };
}
