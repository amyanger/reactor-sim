#include "safety.h"

#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>

void SafetySystem::check(ReactorState& state) {
    if ((state.temperature > state.currentDifficulty.scramTemperature ||
         state.neutrons > RC::SCRAM_NEUTRONS) && state.running) {
        {
            std::ostringstream oss;
            oss << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                << "*** AUTO SCRAM! Emergency shutdown! ***"
                << Color::RESET << "\n";
            state.addAlertMessage(oss.str());
        }
        state.controlRods = 1.0;
        state.neutrons *= 0.05;
        state.temperature = std::max(0.0, state.temperature - 200);
        state.turbineOnline = false;
        state.running = false;
        state.scramCount++;
        state.turnsWithoutScram = 0;
        state.score = std::max(0, state.score - RC::SCRAM_PENALTY);
        {
            std::ostringstream oss;
            oss << Color::RED << "Score penalty: -" << RC::SCRAM_PENALTY << " points" << Color::RESET << "\n";
            state.addMessage(oss.str());
        }
        std::string reason = state.temperature > state.currentDifficulty.scramTemperature
            ? "temperature exceeded limit" : "neutron flux exceeded limit";
        state.addLogEntry("CRITICAL", "AUTO SCRAM triggered - " + reason);
    }

    if (state.temperature > state.currentDifficulty.meltdownTemperature) {
        {
            std::ostringstream oss;
            oss << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                << "!!! MELTDOWN !!! Core has gone critical. Game Over."
                << Color::RESET << "\n";
            state.addAlertMessage(oss.str());
        }
        state.addLogEntry("CRITICAL", "MELTDOWN - Core destruction");
        state.running = false;
    }
}

bool SafetySystem::handleScramReset(ReactorState& state) {
    std::cout << Color::YELLOW << "Type 'reset' to restart reactor, or 'q' to quit: "
              << Color::RESET;
    std::string input;
    if (!std::getline(std::cin, input)) {
        return false;
    }

    if (input == "reset") {
        std::cout << Color::GREEN << "Reactor restart initiated..." << Color::RESET << "\n";
        state.running = true;
        state.temperature = RC::INITIAL_TEMPERATURE;
        state.controlRods = 1.0;
        state.scramRecoveries++;
        return true;
    }
    return false;
}
