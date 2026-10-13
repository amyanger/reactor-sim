#pragma once

#include "reactor_state.h"

class CorePhysics {
public:
    // One turn of core neutronics, heat balance, fuel and coolant, then every subsystem
    static void update(ReactorState& state);

    // Full turn: physics, random events, safety checks. Pure state -> state (no I/O).
    // Returns true if a new achievement was unlocked, so the caller can persist it.
    static bool advanceTurn(ReactorState& state);

    // Net reactivity in dollars for the current rods, xenon, fuel and temperature
    static double reactivity(const ReactorState& state);

    // Rod worth inserted at a given insertion fraction (S-shaped integral worth curve)
    static double rodWorth(double insertion);

    // True when the turbine is spinning fast enough to power the coolant pumps
    static bool turbinePowersPumps(const ReactorState& state);
};
