#include "turbine.h"

#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <string>

void TurbineSystem::update(ReactorState& state) {
    // Calculate steam pressure based on temperature (more realistic model)
    if (state.temperature > RC::MIN_TURBINE_TEMP) {
        double targetPressure = ((state.temperature - RC::MIN_TURBINE_TEMP) / (state.currentDifficulty.meltdownTemperature - RC::MIN_TURBINE_TEMP)) * RC::MAX_STEAM_PRESSURE;
        state.steamPressure = state.steamPressure * 0.7 + targetPressure * 0.3;  // Gradual pressure change
    } else {
        state.steamPressure = std::max(0.0, state.steamPressure - 5.0);
    }

    // Pressure relief valve logic
    if (state.steamPressure > RC::CRITICAL_PRESSURE && !state.pressureReliefOpen) {
        state.pressureReliefOpen = true;
        {
            std::ostringstream oss;
            oss << Color::YELLOW << Color::BOLD
                << "\xf0\x9f\x94\xa7 PRESSURE RELIEF VALVE opened at "
                << std::fixed << std::setprecision(1) << state.steamPressure << " bar!"
                << Color::RESET << "\n";
            state.addMessage(oss.str());
        }
        state.addLogEntry("WARNING", "Pressure relief valve opened");
        state.pressureWarnings++;
    }

    if (state.pressureReliefOpen) {
        state.steamPressure = std::max(0.0, state.steamPressure - 10.0);
        if (state.steamPressure < RC::CRITICAL_PRESSURE * 0.8) {
            state.pressureReliefOpen = false;
            {
                std::ostringstream oss;
                oss << Color::GREEN << "\xe2\x9c\x93 Pressure relief valve closed. Pressure stabilized." << Color::RESET << "\n";
                state.addMessage(oss.str());
            }
            state.addLogEntry("EVENT", "Pressure relief valve closed");
        }
    }

    // Check for pipe rupture
    if (state.steamPressure > RC::RUPTURE_PRESSURE) {
        {
            std::ostringstream oss;
            oss << Color::BG_RED << Color::WHITE << Color::BOLD
                << " \xf0\x9f\x92\xa5 STEAM PIPE RUPTURE! Critical pressure exceeded! "
                << Color::RESET << "\n";
            state.addAlertMessage(oss.str());
        }
        state.addLogEntry("CRITICAL", "Steam pipe rupture - pressure exceeded " + std::to_string(static_cast<int>(RC::RUPTURE_PRESSURE)) + " bar");
        state.coolant = std::max(0.0, state.coolant - 25.0);
        state.temperature += 50.0;
        state.turbineOnline = false;
        state.steamPressure = 50.0;
    }

    if (!state.turbineOnline) {
        state.turbineRPM = std::max(0.0, state.turbineRPM - 100.0);
        state.electricityOutput = 0.0;
        return;
    }

    if (state.temperature < RC::MIN_TURBINE_TEMP) {
        {
            std::ostringstream oss;
            oss << Color::YELLOW << "\xe2\x9a\xa0 Turbine cannot operate below " << RC::MIN_TURBINE_TEMP << "\xc2\xb0""C!" << Color::RESET << "\n";
            state.addMessage(oss.str());
        }
        state.turbineRPM = std::max(0.0, state.turbineRPM - 50.0);
        state.electricityOutput = 0.0;
        return;
    }

    // Pressure warning
    if (state.steamPressure > RC::CRITICAL_PRESSURE * 0.9) {
        std::ostringstream oss;
        oss << Color::RED << "\xe2\x9a\xa0 HIGH STEAM PRESSURE: " << std::fixed << std::setprecision(1) << state.steamPressure
            << " bar (max " << RC::MAX_STEAM_PRESSURE << ")" << Color::RESET << "\n";
        state.addMessage(oss.str());
    }

    double pressureRatio = std::min(1.0, state.steamPressure / RC::MAX_STEAM_PRESSURE);
    double targetRPM = pressureRatio * RC::MAX_TURBINE_RPM;

    if (state.turbineRPM < targetRPM) {
        state.turbineRPM = std::min(targetRPM, state.turbineRPM + 200.0);
    } else {
        state.turbineRPM = std::max(targetRPM, state.turbineRPM - 200.0);
    }

    double tempEfficiency = 1.0 - std::abs(state.temperature - RC::OPTIMAL_STEAM_TEMP) / 1000.0;
    tempEfficiency = std::max(0.3, std::min(1.0, tempEfficiency));

    state.electricityOutput = (state.turbineRPM / RC::MAX_TURBINE_RPM) * 1000.0 * tempEfficiency * state.currentDifficulty.turbineEfficiency;
    state.totalElectricityGenerated += state.electricityOutput / 60.0;

    // Track max turbine output for achievement
    if (state.electricityOutput > 900.0) {
        state.maxTurbineTurns++;
    } else {
        state.maxTurbineTurns = 0;
    }
}
