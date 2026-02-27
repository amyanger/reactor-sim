#include "input.h"
#include "renderer.h"
#include "emergency.h"
#include "persistence.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>

double InputHandler::parseControlRodInput(const std::string& input, double current) {
    try {
        double value = std::stod(input) / 100.0;
        return std::max(0.0, std::min(1.0, value));
    } catch (const std::exception&) {
        return current;
    }
}

InputResult InputHandler::handleInput(ReactorState& state) {
    std::cout << Color::GREEN << "\nControl rods (0-100%, current "
              << static_cast<int>(state.controlRods * 100)
              << "%): " << Color::RESET;

    std::string input;
    if (!std::getline(std::cin, input)) {
        return InputResult::QUIT;
    }

    if (input == "q") return InputResult::QUIT;

    if (input == "h" || input == "help") {
        Renderer::displayHelp(state);
        return InputResult::CONTINUE;
    }
    if (input == "a") {
        Renderer::displayAchievements(state);
        return InputResult::CONTINUE;
    }
    if (input == "stats") {
        Renderer::displayStatistics(state);
        return InputResult::CONTINUE;
    }
    if (input == "log") {
        Renderer::displayLog(state);
        return InputResult::CONTINUE;
    }

    if (input == "r") {
        state.coolant = RC::INITIAL_COOLANT;
        state.score = std::max(0, state.score - RC::REFILL_PENALTY);
        std::cout << Color::GREEN << "Coolant refilled! " << Color::RESET
                  << Color::RED << "(-" << RC::REFILL_PENALTY << " pts)" << Color::RESET << "\n";
        state.addLogEntry("ACTION", "Coolant system refilled to 100%");
        return InputResult::CONTINUE;
    }

    if (input == "t") {
        state.turbineOnline = !state.turbineOnline;
        std::cout << (state.turbineOnline
            ? std::string(Color::GREEN) + "Turbine starting..."
            : std::string(Color::YELLOW) + "Turbine stopping...")
            << Color::RESET << "\n";
        state.addLogEntry("ACTION", state.turbineOnline ? "Turbine brought online" : "Turbine taken offline");
        return InputResult::CONTINUE;
    }

    if (input == "e") {
        EmergencySystem::activateECCS(state);
        Renderer::drainMessages(state);
        return InputResult::CONTINUE;
    }

    if (input == "d") {
        EmergencySystem::toggleDiesel(state);
        Renderer::drainMessages(state);
        return InputResult::CONTINUE;
    }

    if (input == "df") {
        EmergencySystem::refillDiesel(state);
        Renderer::drainMessages(state);
        return InputResult::CONTINUE;
    }

    if (input == "da") {
        state.dieselAutoStart = !state.dieselAutoStart;
        std::cout << (state.dieselAutoStart
            ? std::string(Color::GREEN) + "\xf0\x9f\x94\x8c Diesel auto-start ENABLED"
            : std::string(Color::YELLOW) + "\xf0\x9f\x94\x8c Diesel auto-start DISABLED")
            << Color::RESET << "\n";
        state.addLogEntry("ACTION", state.dieselAutoStart ? "Diesel auto-start enabled" : "Diesel auto-start disabled");
        return InputResult::CONTINUE;
    }

    if (input == "save" || input == "s") {
        if (PersistenceSystem::saveGame(state)) {
            if (state.soundEnabled) Sound::beep();
            std::cout << Color::GREEN << "\xf0\x9f\x92\xbe Game saved successfully!" << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "Failed to save game." << Color::RESET << "\n";
        }
        return InputResult::CONTINUE;
    }

    if (input == "load" || input == "l") {
        if (PersistenceSystem::loadGame(state)) {
            if (state.soundEnabled) Sound::beep();
            std::cout << Color::GREEN << "\xf0\x9f\x92\xbe Game loaded successfully!" << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "No save file found." << Color::RESET << "\n";
        }
        return InputResult::CONTINUE;
    }

    if (input == "sound") {
        state.soundEnabled = !state.soundEnabled;
        std::cout << (state.soundEnabled
            ? std::string(Color::GREEN) + "\xf0\x9f\x94\x8a Sound enabled"
            : std::string(Color::YELLOW) + "\xf0\x9f\x94\x87 Sound disabled")
            << Color::RESET << "\n";
        return InputResult::CONTINUE;
    }

    if (input == "tips") {
        state.tipsEnabled = !state.tipsEnabled;
        std::cout << (state.tipsEnabled
            ? std::string(Color::GREEN) + "\xf0\x9f\x92\xa1 Tips enabled"
            : std::string(Color::YELLOW) + "\xf0\x9f\x92\xa1 Tips disabled")
            << Color::RESET << "\n";
        return InputResult::CONTINUE;
    }

    if (input == "p" || input == "pause") {
        state.paused = !state.paused;
        if (state.paused) {
            std::cout << Color::BG_YELLOW << Color::WHITE << Color::BOLD
                      << " \xe2\x8f\xb8 SIMULATION PAUSED " << Color::RESET << "\n";
            std::cout << Color::DIM << "Use 'p' or 'pause' to resume. You can still view stats, log, and achievements."
                      << Color::RESET << "\n";
            state.addLogEntry("ACTION", "Simulation paused by operator");
        } else {
            std::cout << Color::GREEN << Color::BOLD << "\xe2\x96\xb6 SIMULATION RESUMED" << Color::RESET << "\n";
            state.addLogEntry("ACTION", "Simulation resumed");
        }
        return InputResult::CONTINUE;
    }

    if (state.paused) {
        std::cout << Color::YELLOW << "\xe2\x8f\xb8 Simulation paused. Use 'p' to resume before making changes." << Color::RESET << "\n";
        return InputResult::CONTINUE;
    }

    state.controlRods = parseControlRodInput(input, state.controlRods);
    return InputResult::ADVANCE_TURN;
}
