#pragma once

#include "reactor_state.h"

class SafetySystem {
public:
    // Check safety limits: triggers SCRAM or meltdown if thresholds exceeded
    static void check(ReactorState& state);

    // Interactive SCRAM reset prompt (uses cout/cin directly); returns true if reset
    static bool handleScramReset(ReactorState& state);
};
