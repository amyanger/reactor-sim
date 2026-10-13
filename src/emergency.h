#pragma once

#include "reactor_state.h"

class EmergencySystem {
public:
    static void updateECCS(ReactorState& state);
    static void activateECCS(ReactorState& state);
    static void updateDiesel(ReactorState& state);
    static void toggleDiesel(ReactorState& state);
    // Refills cost points and a turn; they return false if nothing was done
    static bool refillDiesel(ReactorState& state);
    static bool refillCoolant(ReactorState& state);
};
