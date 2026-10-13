#include "input.h"
#include "renderer.h"
#include "emergency.h"
#include "safety.h"
#include "persistence.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cmath>

std::string normalizeCommand(const std::string& input) {
    size_t start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = input.find_last_not_of(" \t\r\n");
    std::string out = input.substr(start, end - start + 1);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

std::optional<double> parseRodPercent(const std::string& input) {
    try {
        size_t used = 0;
        double value = std::stod(input, &used);
        std::string rest = input.substr(used);
        if (!(rest.empty() || rest == "%") || !std::isfinite(value)) return std::nullopt;
        return std::max(0.0, std::min(1.0, value / 100.0));
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

namespace {

std::string colored(const char* color, const std::string& text) {
    return std::string(color) + text + Color::RESET + "\n";
}

} // namespace

InputResult InputHandler::applyCommand(ReactorState& state, const std::string& rawInput) {
    std::string input = normalizeCommand(rawInput);

    // Commands that never change the simulation work at any time
    if (input == "q" || input == "quit") return InputResult::QUIT;
    if (input == "h" || input == "help") return InputResult::SHOW_HELP;
    if (input == "a") return InputResult::SHOW_ACHIEVEMENTS;
    if (input == "stats") return InputResult::SHOW_STATS;
    if (input == "log") return InputResult::SHOW_LOG;
    if (input == "save" || input == "s") {
        std::string err;
        if (PersistenceSystem::saveGame(state, err)) {
            std::string msg = colored(Color::GREEN, "\xf0\x9f\x92\xbe Game saved successfully!");
            if (state.soundEnabled) state.addSoundMessage(msg);
            else state.addMessage(msg);
        } else {
            state.addMessage(colored(Color::RED, "Failed to save game: " + err));
        }
        return InputResult::CONTINUE;
    }
    if (input == "sound") {
        state.soundEnabled = !state.soundEnabled;
        state.addMessage(state.soundEnabled
            ? colored(Color::GREEN, "\xf0\x9f\x94\x8a Sound enabled")
            : colored(Color::YELLOW, "\xf0\x9f\x94\x87 Sound disabled"));
        return InputResult::CONTINUE;
    }
    if (input == "tips") {
        state.tipsEnabled = !state.tipsEnabled;
        state.addMessage(state.tipsEnabled
            ? colored(Color::GREEN, "\xf0\x9f\x92\xa1 Tips enabled")
            : colored(Color::YELLOW, "\xf0\x9f\x92\xa1 Tips disabled"));
        return InputResult::CONTINUE;
    }
    if (input == "p" || input == "pause") {
        state.paused = !state.paused;
        if (state.paused) {
            state.addMessage(std::string(Color::BG_YELLOW) + Color::WHITE + Color::BOLD
                             + " \xe2\x8f\xb8 SIMULATION PAUSED " + Color::RESET + "\n");
            state.addMessage(colored(Color::DIM,
                "Use 'p' or 'pause' to resume. You can still view stats, log, and achievements."));
            state.addLogEntry("ACTION", "Simulation paused by operator");
        } else {
            state.addMessage(std::string(Color::GREEN) + Color::BOLD
                             + "\xe2\x96\xb6 SIMULATION RESUMED" + Color::RESET + "\n");
            state.addLogEntry("ACTION", "Simulation resumed");
        }
        return InputResult::CONTINUE;
    }

    // Everything below changes the plant, so it is blocked while paused
    if (state.paused) {
        state.addMessage(colored(Color::YELLOW,
            "\xe2\x8f\xb8 Simulation paused. Use 'p' to resume before making changes."));
        return InputResult::CONTINUE;
    }

    if (input.empty()) return InputResult::ADVANCE_TURN;

    if (input == "r") {
        return EmergencySystem::refillCoolant(state) ? InputResult::ADVANCE_TURN : InputResult::CONTINUE;
    }
    if (input == "df") {
        return EmergencySystem::refillDiesel(state) ? InputResult::ADVANCE_TURN : InputResult::CONTINUE;
    }
    if (input == "t") {
        state.turbineOnline = !state.turbineOnline;
        state.addMessage(state.turbineOnline
            ? colored(Color::GREEN, "Turbine starting...")
            : colored(Color::YELLOW, "Turbine stopping..."));
        state.addLogEntry("ACTION", state.turbineOnline ? "Turbine brought online" : "Turbine taken offline");
        return InputResult::CONTINUE;
    }
    if (input == "e") {
        EmergencySystem::activateECCS(state);
        return InputResult::CONTINUE;
    }
    if (input == "d") {
        EmergencySystem::toggleDiesel(state);
        return InputResult::CONTINUE;
    }
    if (input == "da") {
        state.dieselAutoStart = !state.dieselAutoStart;
        state.addMessage(state.dieselAutoStart
            ? colored(Color::GREEN, "\xf0\x9f\x94\x8c Diesel auto-start ENABLED")
            : colored(Color::YELLOW, "\xf0\x9f\x94\x8c Diesel auto-start DISABLED"));
        state.addLogEntry("ACTION", state.dieselAutoStart ? "Diesel auto-start enabled" : "Diesel auto-start disabled");
        return InputResult::CONTINUE;
    }
    if (input == "reset") {
        SafetySystem::reset(state);
        return InputResult::CONTINUE;
    }
    if (input == "load" || input == "l") {
        std::string err;
        if (PersistenceSystem::loadGame(state, err)) {
            std::string msg = colored(Color::GREEN, "\xf0\x9f\x92\xbe Game loaded successfully!");
            if (state.soundEnabled) state.addSoundMessage(msg);
            else state.addMessage(msg);
        } else {
            state.addMessage(colored(Color::RED, "Could not load game: " + err));
        }
        return InputResult::CONTINUE;
    }

    std::optional<double> target = parseRodPercent(input);
    if (!target) {
        state.addMessage(colored(Color::YELLOW, "Unknown command '" + input + "'. Type 'h' for help."));
        return InputResult::CONTINUE;
    }
    if (!state.running) {
        state.addMessage(colored(Color::RED,
            "Reactor is tripped; rods are locked in. Type 'reset' first, or Enter to wait."));
        return InputResult::CONTINUE;
    }
    state.rodTarget = *target;
    return InputResult::ADVANCE_TURN;
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

    InputResult result = applyCommand(state, raw);
    Renderer::drainMessages(state);

    switch (result) {
        case InputResult::SHOW_HELP:         Renderer::displayHelp(state);         return InputResult::CONTINUE;
        case InputResult::SHOW_ACHIEVEMENTS: Renderer::displayAchievements(state); return InputResult::CONTINUE;
        case InputResult::SHOW_STATS:        Renderer::displayStatistics(state);   return InputResult::CONTINUE;
        case InputResult::SHOW_LOG:          Renderer::displayLog(state);          return InputResult::CONTINUE;
        default:                             return result;
    }
}
