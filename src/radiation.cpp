#include "radiation.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

void RadiationSystem::update(ReactorState& state) {
    // Base radiation from power level
    double powerRadiation = (state.power / 100.0) * 5.0;

    // Additional radiation from high temperature (containment stress)
    double tempFactor = 0.0;
    if (state.temperature > state.currentDifficulty.scramTemperature * 0.8) {
        tempFactor = ((state.temperature - state.currentDifficulty.scramTemperature * 0.8) /
                     (state.currentDifficulty.meltdownTemperature - state.currentDifficulty.scramTemperature * 0.8)) * 50.0;
    }

    // Additional radiation from low coolant (exposed fuel)
    double coolantFactor = 0.0;
    if (state.coolant < 30.0) {
        coolantFactor = ((30.0 - state.coolant) / 30.0) * 100.0;
    }

    // Calculate target radiation
    double targetRadiation = RC::BACKGROUND_RADIATION + powerRadiation + tempFactor + coolantFactor;

    // Smooth transition
    state.radiationLevel = state.radiationLevel * 0.7 + targetRadiation * 0.3;

    // Track total exposure
    state.totalRadiationExposure += state.radiationLevel / 60.0;  // per turn

    // Radiation warnings
    if (state.radiationLevel > RC::DANGER_RADIATION) {
        std::ostringstream oss;
        oss << Color::BG_RED << Color::WHITE << Color::BOLD
            << " \xe2\x98\xa2 RADIATION CRITICAL: " << std::fixed << std::setprecision(1) << state.radiationLevel << " mSv/h - EVACUATE! "
            << Color::RESET << "\n";
        state.addAlertMessage(oss.str());
        state.radiationAlarms++;
        state.addLogEntry("CRITICAL", "Radiation level critical - evacuation recommended");
    } else if (state.radiationLevel > RC::WARNING_RADIATION) {
        std::ostringstream oss;
        oss << Color::RED << Color::BOLD
            << "\xe2\x98\xa2 HIGH RADIATION: " << std::fixed << std::setprecision(1) << state.radiationLevel << " mSv/h"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        if (state.radiationAlarms % 5 == 0) {  // Don't spam
            state.addLogEntry("WARNING", "Elevated radiation levels detected");
        }
        state.radiationAlarms++;
    } else if (state.radiationLevel > RC::MAX_SAFE_RADIATION) {
        std::ostringstream oss;
        oss << Color::YELLOW << "\xe2\x9a\xa0 Elevated radiation: "
            << std::fixed << std::setprecision(1) << state.radiationLevel << " mSv/h" << Color::RESET << "\n";
        state.addMessage(oss.str());
    }
}
