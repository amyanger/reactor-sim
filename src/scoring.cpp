#include "scoring.h"

#include <algorithm>

void ScoringSystem::update(ReactorState& state) {
    // Track peak values
    if (state.temperature > state.peakTemperature) state.peakTemperature = state.temperature;
    if (state.power > state.peakPower) state.peakPower = state.power;
    if (state.electricityOutput > state.peakElectricity) state.peakElectricity = state.electricityOutput;
    if (state.coolant < state.lowestCoolant) state.lowestCoolant = state.coolant;
    if (state.xenonLevel > state.highestXenon) state.highestXenon = state.xenonLevel;

    // Track totals (called after turns is incremented, so turns counts this sample)
    state.totalPowerGenerated += state.power;
    state.temperatureSum += state.temperature;
    if (state.turns > 0) {
        state.averageTemperature = state.temperatureSum / state.turns;
    }

    if (state.temperature > state.currentDifficulty.scramTemperature * 0.9 ||
        state.coolant < RC::CRITICAL_COOLANT * 1.5 ||
        state.xenonLevel > RC::MAX_XENON * 0.8) {
        state.criticalEvents++;
    }

    // Points come only from electricity actually delivered to the grid
    double delivered = std::min(state.electricityOutput, state.gridDemand);
    state.score += static_cast<int>(delivered / 100.0 * RC::POINTS_PER_100MW * state.currentDifficulty.scoreMultiplier);
}
