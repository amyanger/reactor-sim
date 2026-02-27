#include "emergency.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

void EmergencySystem::updateECCS(ReactorState& state) {
    if (!state.eccsAvailable && state.eccsCooldownTimer > 0) {
        state.eccsCooldownTimer--;
        if (state.eccsCooldownTimer == 0) {
            state.eccsAvailable = true;
            std::ostringstream oss;
            oss << Color::GREEN << "\xe2\x9c\x93 ECCS recharged and ready!" << Color::RESET << "\n";
            state.addMessage(oss.str());
        }
    }
}

void EmergencySystem::activateECCS(ReactorState& state) {
    if (!state.eccsAvailable) {
        std::ostringstream oss;
        oss << Color::RED << "\xe2\x9c\x97 ECCS on cooldown! " << state.eccsCooldownTimer << " turns remaining." << Color::RESET << "\n";
        state.addMessage(oss.str());
        return;
    }

    state.eccsAvailable = false;
    state.eccsCooldownTimer = RC::ECCS_COOLDOWN;

    state.coolant = std::min(100.0, state.coolant + RC::ECCS_COOLANT_BOOST);
    state.temperature = std::max(RC::INITIAL_TEMPERATURE, state.temperature - RC::ECCS_TEMP_REDUCTION);
    state.score = std::max(0, state.score - RC::ECCS_PENALTY);

    {
        std::ostringstream oss;
        oss << Color::BG_BLUE << Color::WHITE << Color::BOLD
            << " \xf0\x9f\x9a\xa8 ECCS ACTIVATED! +" << RC::ECCS_COOLANT_BOOST << "% coolant, -" << RC::ECCS_TEMP_REDUCTION << "\xc2\xb0""C "
            << Color::RESET << "\n";
        state.addMessage(oss.str());
    }
    {
        std::ostringstream oss;
        oss << Color::RED << "(-" << RC::ECCS_PENALTY << " points)" << Color::RESET << "\n";
        state.addMessage(oss.str());
    }
    state.addLogEntry("CRITICAL", "ECCS activated - emergency cooling");
}

void EmergencySystem::updateDiesel(ReactorState& state) {
    // Auto-start logic - starts when turbine output drops below 50 MW
    if (state.dieselAutoStart && !state.dieselRunning && state.electricityOutput < 50.0 && state.dieselFuel > 0) {
        state.dieselRunning = true;
        std::ostringstream oss;
        oss << Color::YELLOW << Color::BOLD
            << "\xf0\x9f\x94\x8c DIESEL GENERATOR auto-started! Low power detected."
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Diesel generator auto-started");
    }

    if (state.dieselRunning) {
        if (state.dieselFuel > 0) {
            state.dieselFuel = std::max(0.0, state.dieselFuel - RC::DIESEL_FUEL_CONSUMPTION);
            state.dieselRuntime++;

            // Diesel helps maintain basic cooling even during low power
            if (state.temperature > RC::INITIAL_TEMPERATURE) {
                state.temperature -= 2.0;
            }

            // Low fuel warning
            if (state.dieselFuel < 20.0 && state.dieselFuel > 0) {
                std::ostringstream oss;
                oss << Color::YELLOW << "\xe2\x9a\xa0 Diesel fuel low: "
                    << std::fixed << std::setprecision(1) << state.dieselFuel << "%" << Color::RESET << "\n";
                state.addMessage(oss.str());
            }
        } else {
            state.dieselRunning = false;
            std::ostringstream oss;
            oss << Color::RED << Color::BOLD
                << "\xe2\x9a\xa0 DIESEL GENERATOR stopped - OUT OF FUEL!"
                << Color::RESET << "\n";
            state.addMessage(oss.str());
            state.addLogEntry("WARNING", "Diesel generator stopped - fuel depleted");
        }
    }
}

void EmergencySystem::toggleDiesel(ReactorState& state) {
    if (!state.dieselRunning && state.dieselFuel <= 0) {
        std::ostringstream oss;
        oss << Color::RED << "\xe2\x9c\x97 Cannot start diesel generator - no fuel!" << Color::RESET << "\n";
        state.addMessage(oss.str());
        return;
    }

    state.dieselRunning = !state.dieselRunning;
    if (state.dieselRunning) {
        std::ostringstream oss;
        oss << Color::GREEN << "\xf0\x9f\x94\x8c Diesel generator started manually." << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("ACTION", "Diesel generator started manually");
    } else {
        std::ostringstream oss;
        oss << Color::YELLOW << "\xf0\x9f\x94\x8c Diesel generator stopped." << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("ACTION", "Diesel generator stopped");
    }
}

void EmergencySystem::refillDiesel(ReactorState& state) {
    if (state.dieselFuel >= RC::DIESEL_FUEL_CAPACITY) {
        std::ostringstream oss;
        oss << Color::YELLOW << "Diesel tank already full." << Color::RESET << "\n";
        state.addMessage(oss.str());
        return;
    }
    state.dieselFuel = RC::DIESEL_FUEL_CAPACITY;
    std::ostringstream oss;
    oss << Color::GREEN << "\xe2\x9b\xbd Diesel tank refilled!" << Color::RESET << "\n";
    state.addMessage(oss.str());
    state.addLogEntry("ACTION", "Diesel fuel tank refilled");
}
