#pragma once

#include "reactor_state.h"

#include <string>

class SafetySystem {
public:
    // Check safety limits: triggers SCRAM, or game over on meltdown
    static void check(ReactorState& state);

    // Insert all rods, trip the turbine, and record the reason
    static void scram(ReactorState& state, const std::string& reason);

    // Operator reset after a SCRAM. Refused while trip conditions persist or after game over.
    static bool reset(ReactorState& state);
};
