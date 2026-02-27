#pragma once

#include "reactor_state.h"

class RandomEventSystem {
public:
    // Process random events for the current turn; mutates state directly
    static void process(ReactorState& state);
};
