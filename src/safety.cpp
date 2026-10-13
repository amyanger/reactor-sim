#include "safety.h"
#include "physics.h"

#include <sstream>
#include <string>
#include <algorithm>

void SafetySystem::scram(ReactorState& state, const std::string& reason) {
    {
        std::ostringstream oss;
        oss << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
            << "*** AUTO SCRAM! Emergency shutdown: " << reason << " ***"
            << Color::RESET << "\n";
        state.addAlertMessage(oss.str());
    }

    // Rods drop in at once: apply the prompt jump for the inserted reactivity
    double rhoBefore = std::min(0.99, CorePhysics::reactivity(state));
    state.controlRods = 1.0;
    state.rodTarget = 1.0;
    double rhoAfter = CorePhysics::reactivity(state);
    state.neutrons *= (1.0 - rhoBefore) / (1.0 - rhoAfter);
    state.reactivity = rhoAfter;

    state.turbineOnline = false;
    state.running = false;
    state.awaitingRecovery = false;
    state.scramCount++;
    state.turnsWithoutScram = 0;
    state.score = std::max(0, state.score - RC::SCRAM_PENALTY);
    {
        std::ostringstream oss;
        oss << Color::RED << "Score penalty: -" << RC::SCRAM_PENALTY << " points. "
            << "Decay heat still needs cooling. Type 'reset' once conditions clear." << Color::RESET << "\n";
        state.addMessage(oss.str());
    }
    state.addLogEntry("CRITICAL", "AUTO SCRAM triggered - " + reason);
}

void SafetySystem::check(ReactorState& state) {
    if (state.gameOver) return;
    const DifficultySettings& diff = state.currentDifficulty;

    if (state.running) {
        // Determine the reason before anything changes state
        std::string reason;
        if (state.promptCritical) reason = "prompt criticality";
        else if (state.temperature > diff.scramTemperature) reason = "temperature exceeded limit";
        else if (state.neutrons > RC::SCRAM_NEUTRONS) reason = "neutron flux exceeded limit";
        if (!reason.empty()) scram(state, reason);
    }
    state.promptCritical = false;

    if (state.temperature > diff.meltdownTemperature) {
        {
            std::ostringstream oss;
            oss << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                << "!!! MELTDOWN !!! Core has gone critical. Game Over."
                << Color::RESET << "\n";
            state.addAlertMessage(oss.str());
        }
        state.addLogEntry("CRITICAL", "MELTDOWN - Core destruction");
        state.running = false;
        state.gameOver = true;
        state.gameOverReason = "Core meltdown";
        return;
    }

    // A SCRAM recovery counts once the plant is back on the grid
    if (state.awaitingRecovery && state.running && state.electricityOutput >= RC::RECOVERY_ELECTRIC_MW) {
        state.awaitingRecovery = false;
        state.scramRecoveries++;
        std::ostringstream oss;
        oss << Color::GREEN << "\xe2\x9c\x93 Power restored after SCRAM." << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Power restored after SCRAM");
    }
}

bool SafetySystem::reset(ReactorState& state) {
    std::ostringstream oss;
    if (state.gameOver) {
        oss << Color::RED << "\xe2\x9c\x97 " << state.gameOverReason << ". The reactor cannot be restarted." << Color::RESET << "\n";
        state.addMessage(oss.str());
        return false;
    }
    if (state.running) {
        oss << Color::YELLOW << "Reactor is not tripped; nothing to reset." << Color::RESET << "\n";
        state.addMessage(oss.str());
        return false;
    }
    double resetLimit = state.currentDifficulty.scramTemperature * RC::RESET_TEMP_FRACTION;
    if (state.temperature > resetLimit) {
        oss << Color::RED << "\xe2\x9c\x97 Trip conditions still present: core at "
            << static_cast<int>(state.temperature) << "\xc2\xb0""C (must be below "
            << static_cast<int>(resetLimit) << "\xc2\xb0""C). Keep cooling." << Color::RESET << "\n";
        state.addMessage(oss.str());
        return false;
    }

    state.running = true;
    state.rodTarget = 1.0;
    state.awaitingRecovery = true;
    oss << Color::GREEN << "Trip reset. Rods are fully inserted; withdraw slowly and bring the turbine back with 't'."
        << Color::RESET << "\n";
    state.addMessage(oss.str());
    state.addLogEntry("ACTION", "SCRAM reset by operator");
    return true;
}
