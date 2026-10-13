#pragma once

#include "reactor_state.h"

class TurbineSystem {
public:
    static void update(ReactorState& state);

    // Thermal-to-electric efficiency at a given steam temperature
    static double efficiency(double temperature);

    // Best possible electric output (MW) for a difficulty
    static double maxOutput(const DifficultySettings& diff);
};
