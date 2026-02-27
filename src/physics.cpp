#include "physics.h"
#include "xenon.h"
#include "turbine.h"
#include "emergency.h"
#include "radiation.h"
#include "containment.h"
#include "weather.h"
#include "grid.h"
#include "scoring.h"
#include "achievements.h"
#include "persistence.h"

#include <sstream>
#include <algorithm>

void CorePhysics::update(ReactorState& state) {
    // Apply xenon poisoning effect on reactivity
    double xenonFactor = 1.0 - (state.xenonLevel / RC::MAX_XENON) * 0.3;

    double k_eff = (1.05 - state.controlRods * 1.1) * xenonFactor;
    k_eff = std::max(0.7, k_eff);
    state.neutrons *= k_eff;

    state.power = state.neutrons * RC::NEUTRON_TO_POWER_RATIO;

    double fuel_eff = state.fuel / 100.0;
    state.neutrons *= fuel_eff;
    state.fuel = std::max(0.0, state.fuel - state.currentDifficulty.fuelDepletionRate);

    state.temperature += state.power * RC::POWER_TO_HEAT_RATIO;
    state.coolant = std::max(0.0, state.coolant - state.currentDifficulty.coolantLossRate);

    // Apply weather-modified cooling
    WeatherInfo weatherInfo = getWeatherInfo(state.currentWeather);
    double effectiveCooling = RC::NATURAL_COOLING_RATE * weatherInfo.coolingModifier;
    state.temperature = std::max(0.0, state.temperature - effectiveCooling);

    if (state.coolant < RC::CRITICAL_COOLANT) {
        {
            std::ostringstream oss;
            oss << Color::BG_RED << Color::WHITE << Color::BOLD
                << "!!! WARNING: Coolant is critically low! !!!"
                << Color::RESET << "\n";
            state.addSoundMessage(oss.str());
        }
        state.temperature += 5.0;
    }

    // Update subsystems
    XenonSystem::update(state);
    TurbineSystem::update(state);
    EmergencySystem::updateECCS(state);
    EmergencySystem::updateDiesel(state);
    RadiationSystem::update(state);
    ContainmentSystem::update(state);
    WeatherSystem::update(state);
    GridSystem::update(state);

    // Update statistics
    ScoringSystem::update(state);

    // Update score with difficulty multiplier
    state.turns++;
    state.turnsWithoutScram++;
    state.score += RC::POINTS_PER_TURN * state.currentDifficulty.scoreMultiplier;
    state.score += static_cast<int>(state.power * RC::POINTS_PER_POWER_UNIT * state.currentDifficulty.scoreMultiplier);
    state.score += static_cast<int>(state.electricityOutput / 100.0 * RC::POINTS_PER_MW * state.currentDifficulty.scoreMultiplier);

    // Check achievements
    if (AchievementSystem::check(state)) {
        PersistenceSystem::saveAchievements(state);
    }
}
