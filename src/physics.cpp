#include "physics.h"
#include "xenon.h"
#include "turbine.h"
#include "emergency.h"
#include "radiation.h"
#include "containment.h"
#include "weather.h"
#include "grid.h"
#include "scoring.h"
#include "events.h"
#include "safety.h"
#include "achievements.h"

#include <sstream>
#include <algorithm>
#include <cmath>

double CorePhysics::rodWorth(double insertion) {
    const double twoPi = 2.0 * 3.14159265358979323846;
    double x = std::max(0.0, std::min(1.0, insertion));
    return RC::ROD_WORTH * (x - std::sin(twoPi * x) / twoPi);
}

double CorePhysics::reactivity(const ReactorState& state) {
    return RC::EXCESS_REACTIVITY
         - rodWorth(state.controlRods)
         - state.xenonLevel * RC::XENON_WORTH_PER_PERCENT
         - RC::BURNUP_WORTH * (1.0 - state.fuel / 100.0)
         - RC::TEMP_COEFFICIENT * (state.temperature - RC::TEMP_REFERENCE);
}

bool CorePhysics::turbinePowersPumps(const ReactorState& state) {
    return state.turbineOnline && state.turbineRPM >= RC::HOUSE_POWER_RPM
        && state.temperature >= RC::MIN_TURBINE_TEMP;
}

// Effective heat transfer coefficient (MW per degree C) for this turn
static double heatTransfer(const ReactorState& state, bool pumpsOnTurbine) {
    double pumpFactor = RC::NATURAL_CIRCULATION;
    if (pumpsOnTurbine) pumpFactor = 1.0;
    else if (state.dieselRunning) pumpFactor = RC::DIESEL_PUMP_FACTOR;

    double coolantFactor = std::max(RC::MIN_COOLANT_FACTOR, std::min(1.0, state.coolant / RC::FULL_COOLANT_LEVEL));
    double weather = getWeatherInfo(state.currentWeather).coolingModifier;
    double weatherFactor = 1.0 - RC::WEATHER_COOLING_WEIGHT + RC::WEATHER_COOLING_WEIGHT * weather;

    return RC::HEAT_TRANSFER * pumpFactor * coolantFactor * weatherFactor;
}

void CorePhysics::update(ReactorState& state) {
    const DifficultySettings& diff = state.currentDifficulty;

    // A tripped reactor keeps its rods fully inserted
    if (!state.running) {
        state.controlRods = 1.0;
        state.rodTarget = 1.0;
    }
    state.rodTarget = std::max(0.0, std::min(1.0, state.rodTarget));

    // Diesels start within seconds, so pump power is settled before the heat balance
    EmergencySystem::updateDiesel(state);

    const int steps = RC::PHYSICS_SUBSTEPS;
    const double dt = 1.0 / steps;
    double rodDelta = std::max(-RC::ROD_SPEED, std::min(RC::ROD_SPEED, state.rodTarget - state.controlRods));
    bool pumpsOnTurbine = turbinePowersPumps(state);

    double rhoPrev = std::min(0.99, reactivity(state));
    double fissionSum = 0.0;
    double removedSum = 0.0;
    state.promptCritical = false;

    // Substeps let temperature feedback act within the turn instead of a turn late
    for (int i = 0; i < steps; ++i) {
        state.controlRods = std::max(0.0, std::min(1.0, state.controlRods + rodDelta * dt));
        double rho = reactivity(state);

        if (rho >= 1.0) {
            state.promptCritical = true;
            state.neutrons *= RC::PROMPT_CRITICAL_GROWTH;
        } else {
            // Prompt jump on a reactivity change, then the stable period from one delayed group
            state.neutrons *= (1.0 - rhoPrev) / (1.0 - rho);
            double rate = rho >= 0.0 ? RC::KINETICS_RATE_POS : RC::KINETICS_RATE_NEG;
            state.neutrons = state.neutrons * std::exp(rate * dt * rho / (1.0 - rho)) + RC::NEUTRON_SOURCE * dt;
        }
        state.neutrons = std::max(0.0, std::min(RC::MAX_NEUTRONS, state.neutrons));
        rhoPrev = std::min(0.99, rho);

        double fission = state.neutrons * RC::NEUTRON_TO_POWER;
        // Heat carried to the steam plant; vented relief steam never reaches the turbine
        double removed = heatTransfer(state, pumpsOnTurbine) * (state.temperature - RC::HEAT_SINK_TEMP);
        // and venting only removes heat while there is water left to boil
        double vented = state.pressureReliefOpen
            ? RC::RELIEF_HEAT_REMOVAL * std::min(1.0, state.coolant / RC::FULL_COOLANT_LEVEL) : 0.0;

        state.temperature += (fission + state.decayHeat - removed - vented) / RC::HEAT_CAPACITY * dt;
        state.temperature = std::max(0.0, state.temperature);

        fissionSum += fission * dt;
        removedSum += removed * dt;
    }

    state.reactivity = reactivity(state);
    state.power = fissionSum;
    state.heatRemoved = std::max(0.0, removedSum);

    // Decay heat tracks a share of fission power and dies away after shutdown
    state.decayHeat += (RC::DECAY_HEAT_FRACTION * state.power - state.decayHeat) * RC::DECAY_HEAT_RATE;

    // Burnup is proportional to power produced
    state.fuel = std::max(0.0, state.fuel - diff.fuelDepletionRate * state.power / RC::RATED_THERMAL_MW);

    // Coolant: steady leakage, plus boil-off when the core runs past the SCRAM limit
    double coolantLoss = diff.coolantLossRate;
    if (state.temperature > diff.scramTemperature) {
        coolantLoss += (state.temperature - diff.scramTemperature) / RC::BOILOFF_DIVISOR;
    }
    if (state.pressureReliefOpen) coolantLoss += RC::RELIEF_COOLANT_LOSS;
    state.coolant = std::max(0.0, state.coolant - coolantLoss);

    if (state.coolant < RC::CRITICAL_COOLANT) {
        std::ostringstream oss;
        oss << Color::BG_RED << Color::WHITE << Color::BOLD
            << "!!! WARNING: Coolant is critically low! !!!"
            << Color::RESET << "\n";
        state.addSoundMessage(oss.str());
    }

    XenonSystem::update(state);
    TurbineSystem::update(state);
    EmergencySystem::updateECCS(state);
    RadiationSystem::update(state);
    ContainmentSystem::update(state);
    WeatherSystem::update(state);
    GridSystem::update(state);

    state.turns++;
    if (state.running) state.turnsWithoutScram++;
    ScoringSystem::update(state);
}

bool CorePhysics::advanceTurn(ReactorState& state) {
    update(state);
    RandomEventSystem::process(state);
    SafetySystem::check(state);
    return AchievementSystem::check(state);
}
