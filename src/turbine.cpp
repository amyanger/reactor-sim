#include "turbine.h"

#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <string>

double TurbineSystem::efficiency(double temperature) {
    if (temperature < RC::MIN_TURBINE_TEMP) return 0.0;
    double offset = (temperature - RC::OPTIMAL_STEAM_TEMP) / RC::EFFICIENCY_WIDTH;
    double eta = RC::PEAK_EFFICIENCY * (1.0 - offset * offset);
    return std::max(RC::MIN_EFFICIENCY, std::min(RC::PEAK_EFFICIENCY, eta));
}

double TurbineSystem::maxOutput(const DifficultySettings& diff) {
    return RC::RATED_THERMAL_MW * RC::PEAK_EFFICIENCY * diff.turbineEfficiency;
}

void TurbineSystem::update(ReactorState& state) {
    // Steam pressure follows core temperature; with the turbine down nothing draws steam off
    if (state.temperature > RC::MIN_TURBINE_TEMP) {
        double targetPressure = RC::PRESSURE_PER_DEGREE * (state.temperature - RC::MIN_TURBINE_TEMP);
        if (!state.turbineOnline) targetPressure *= RC::PRESSURE_OFFLINE_FACTOR;
        state.steamPressure += (targetPressure - state.steamPressure) * RC::PRESSURE_SMOOTHING;
    } else {
        state.steamPressure = std::max(0.0, state.steamPressure - 5.0);
    }

    if (state.steamPressure > RC::CRITICAL_PRESSURE && !state.pressureReliefOpen) {
        state.pressureReliefOpen = true;
        {
            std::ostringstream oss;
            oss << Color::YELLOW << Color::BOLD
                << "\xf0\x9f\x94\xa7 PRESSURE RELIEF VALVE opened at "
                << std::fixed << std::setprecision(1) << state.steamPressure << " bar! Venting steam and coolant."
                << Color::RESET << "\n";
            state.addMessage(oss.str());
        }
        state.addLogEntry("WARNING", "Pressure relief valve opened");
        state.pressureWarnings++;
    }

    if (state.pressureReliefOpen) {
        state.steamPressure = std::max(0.0, state.steamPressure - RC::RELIEF_PRESSURE_DROP);
        if (state.steamPressure < RC::CRITICAL_PRESSURE * RC::RELIEF_CLOSE_FRACTION) {
            state.pressureReliefOpen = false;
            {
                std::ostringstream oss;
                oss << Color::GREEN << "\xe2\x9c\x93 Pressure relief valve closed. Pressure stabilized." << Color::RESET << "\n";
                state.addMessage(oss.str());
            }
            state.addLogEntry("EVENT", "Pressure relief valve closed");
        }
    }

    if (state.steamPressure > RC::RUPTURE_PRESSURE) {
        {
            std::ostringstream oss;
            oss << Color::BG_RED << Color::WHITE << Color::BOLD
                << " \xf0\x9f\x92\xa5 STEAM PIPE RUPTURE! Critical pressure exceeded! "
                << Color::RESET << "\n";
            state.addAlertMessage(oss.str());
        }
        state.addLogEntry("CRITICAL", "Steam pipe rupture - pressure exceeded " + std::to_string(static_cast<int>(RC::RUPTURE_PRESSURE)) + " bar");
        state.coolant = std::max(0.0, state.coolant - RC::RUPTURE_COOLANT_LOSS);
        state.temperature += RC::RUPTURE_HEAT;
        state.turbineOnline = false;
        state.steamPressure = 50.0;
    }

    if (!state.turbineOnline) {
        state.turbineRPM = std::max(0.0, state.turbineRPM - RC::TURBINE_SPINDOWN_RPM);
        state.electricityOutput = 0.0;
        state.maxTurbineTurns = 0;
        return;
    }

    if (state.temperature < RC::MIN_TURBINE_TEMP) {
        {
            std::ostringstream oss;
            oss << Color::YELLOW << "\xe2\x9a\xa0 Turbine cannot operate below " << RC::MIN_TURBINE_TEMP << "\xc2\xb0""C!" << Color::RESET << "\n";
            state.addMessage(oss.str());
        }
        state.turbineRPM = std::max(0.0, state.turbineRPM - RC::TURBINE_SPINDOWN_RPM);
        state.electricityOutput = 0.0;
        state.maxTurbineTurns = 0;
        return;
    }

    if (state.steamPressure > RC::CRITICAL_PRESSURE * 0.9) {
        std::ostringstream oss;
        oss << Color::RED << "\xe2\x9a\xa0 HIGH STEAM PRESSURE: " << std::fixed << std::setprecision(1) << state.steamPressure
            << " bar (relief at " << RC::CRITICAL_PRESSURE << ")" << Color::RESET << "\n";
        state.addMessage(oss.str());
    }

    state.turbineRPM = std::min(RC::MAX_TURBINE_RPM, state.turbineRPM + RC::TURBINE_SPINUP_RPM);

    // Electricity comes from the heat the coolant actually carried out of the core
    double rpmFraction = state.turbineRPM / RC::MAX_TURBINE_RPM;
    double output = state.heatRemoved * efficiency(state.temperature)
                  * state.currentDifficulty.turbineEfficiency * rpmFraction;
    state.electricityOutput = std::min(output, maxOutput(state.currentDifficulty));  // generator rating
    state.totalElectricityGenerated += state.electricityOutput * RC::HOURS_PER_TURN;

    if (state.electricityOutput >= RC::ELECTRICIAN_FRACTION * maxOutput(state.currentDifficulty)) {
        state.maxTurbineTurns++;
    } else {
        state.maxTurbineTurns = 0;
    }
}
