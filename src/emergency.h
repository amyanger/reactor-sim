#pragma once

#include "reactor_state.h"

class EmergencySystem {
public:
    static void updateECCS(ReactorState& state);
    static void activateECCS(ReactorState& state);
    static void updateDiesel(ReactorState& state);
    static void toggleDiesel(ReactorState& state);
    static void refillDiesel(ReactorState& state);
};
