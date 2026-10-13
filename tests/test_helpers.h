#pragma once

#include "reactor_state.h"
#include "physics.h"
#include "safety.h"

#include <cmath>

namespace testutil {

constexpr unsigned int SEED = 42;

inline ReactorState makeState(Difficulty diff = Difficulty::NORMAL, unsigned int seed = SEED) {
    ReactorState s(diff, seed);
    s.tipsEnabled = false;
    s.soundEnabled = false;
    return s;
}

// Deterministic turn without random events: physics plus safety checks
inline void step(ReactorState& s) {
    CorePhysics::update(s);
    SafetySystem::check(s);
    s.clearMessages();
}

inline void run(ReactorState& s, int turns) {
    for (int i = 0; i < turns; ++i) step(s);
}

inline bool finiteCore(const ReactorState& s) {
    return std::isfinite(s.neutrons) && std::isfinite(s.temperature) && std::isfinite(s.power)
        && std::isfinite(s.xenonLevel) && std::isfinite(s.iodineLevel) && std::isfinite(s.decayHeat)
        && std::isfinite(s.coolant) && std::isfinite(s.fuel) && std::isfinite(s.steamPressure);
}

} // namespace testutil
