#include "input.h"
#include "renderer.h"
#include "emergency.h"
#include "safety.h"
#include "persistence.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cmath>

std::string InputHandler::normalize(const std::string& input) {
    size_t start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = input.find_last_not_of(" \t\r\n");
    std::string out = input.substr(start, end - start + 1);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

bool InputHandler::parseControlRodInput(const std::string& input, double& out) {
    try {
        size_t used = 0;
        double value = std::stod(input, &used);
        std::string rest = input.substr(used);
        if (!(rest.empty() || rest == "%") || !std::isfinite(value)) return false;
        out = std::max(0.0, std::min(1.0, value / 100.0));
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

InputResult InputHandler::handleInput(ReactorState& state) {
    std::cout << Color::GREEN << "\nControl rods (0-100%, now "
              << static_cast<int>(std::lround(state.controlRods * 100)) << "%, target "
              << static_cast<int>(std::lround(state.rodTarget * 100)) << "%)"
              << Color::DIM << " [Enter = next turn, h = help]: " << Color::RESET;

    std::string raw;
    if (!std::getline(std::cin, raw)) {
        return InputResult::QUIT;
    }
    std::string input = normalize(raw);

    // Commands that never change the simulation work at any time
    if (input == "q" || input == "quit") return InputResult::QUIT;
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
    if (input == "save" || input == "s") {
        if (PersistenceSystem::saveGame(state)) {
            if (state.soundEnabled) Sound::beep();
            std::cout << Color::GREEN << "\xf0\x9f\x92\xbe Game saved successfully!" << Color::RESET << "\n";
        } else {
            std::cout << Color::RED << "Failed to save game." << Color::RESET << "\n";
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

    // Everything below changes the plant, so it is blocked while paused
    if (state.paused) {
        std::cout << Color::YELLOW << "\xe2\x8f\xb8 Simulation paused. Use 'p' to resume before making changes." << Color::RESET << "\n";
        return InputResult::CONTINUE;
    }

    if (input.empty()) return InputResult::ADVANCE_TURN;

    if (input == "r") {
        bool done = EmergencySystem::refillCoolant(state);
        Renderer::drainMessages(state);
        return done ? InputResult::ADVANCE_TURN : InputResult::CONTINUE;
    }
    if (input == "df") {
        bool done = EmergencySystem::refillDiesel(state);
        Renderer::drainMessages(state);
        return done ? InputResult::ADVANCE_TURN : InputResult::CONTINUE;
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
    if (input == "da") {
        state.dieselAutoStart = !state.dieselAutoStart;
        std::cout << (state.dieselAutoStart
            ? std::string(Color::GREEN) + "\xf0\x9f\x94\x8c Diesel auto-start ENABLED"
            : std::string(Color::YELLOW) + "\xf0\x9f\x94\x8c Diesel auto-start DISABLED")
            << Color::RESET << "\n";
        state.addLogEntry("ACTION", state.dieselAutoStart ? "Diesel auto-start enabled" : "Diesel auto-start disabled");
        return InputResult::CONTINUE;
    }
    if (input == "reset") {
        SafetySystem::reset(state);
        Renderer::drainMessages(state);
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

    double target = 0.0;
    if (!parseControlRodInput(input, target)) {
        std::cout << Color::YELLOW << "Unknown command '" << input << "'. Type 'h' for help." << Color::RESET << "\n";
        return InputResult::CONTINUE;
    }
    if (!state.running) {
        std::cout << Color::RED << "Reactor is tripped; rods are locked in. Type 'reset' first, or Enter to wait."
                  << Color::RESET << "\n";
        return InputResult::CONTINUE;
    }
    state.rodTarget = target;
    return InputResult::ADVANCE_TURN;
}
