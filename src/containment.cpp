#include "containment.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

void ContainmentSystem::update(ReactorState& state) {
    // Containment degrades under stress
    double stressFactor = 0.0;

    // Running hot stresses containment (integrity points lost per turn)
    double stressTemp = state.currentDifficulty.scramTemperature * RC::CONTAINMENT_TEMP_STRESS_START;
    if (state.temperature > stressTemp) {
        stressFactor += (state.temperature - stressTemp) / 100.0;
    }

    // Steam pressure above the relief setpoint
    if (state.steamPressure > RC::CRITICAL_PRESSURE) {
        stressFactor += (state.steamPressure - RC::CRITICAL_PRESSURE) / 10.0;
    }

    // High radiation indicates containment stress
    if (state.radiationLevel > RC::WARNING_RADIATION) {
        stressFactor += (state.radiationLevel - RC::WARNING_RADIATION) / 200.0;
    }

    if (stressFactor > 0) {
        state.containmentIntegrity = std::max(0.0, state.containmentIntegrity - stressFactor);
    } else {
        state.containmentIntegrity = std::min(RC::MAX_CONTAINMENT, state.containmentIntegrity + RC::CONTAINMENT_RECOVERY);
    }

    if (state.containmentIntegrity <= 0.0 && !state.gameOver) {
        std::ostringstream oss;
        oss << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
            << "!!! CONTAINMENT FAILURE !!! Radioactive release. Game Over."
            << Color::RESET << "\n";
        state.addAlertMessage(oss.str());
        state.addLogEntry("CRITICAL", "Containment failure - radioactive release");
        state.gameOver = true;
        state.gameOverReason = "Containment failure";
        state.running = false;
        return;
    }

    // Containment warnings
    if (state.containmentIntegrity < RC::CONTAINMENT_CRITICAL && !state.containmentBreach) {
        state.containmentBreach = true;
        {
            std::ostringstream oss;
            oss << Color::BG_RED << Color::WHITE << Color::BOLD
                << " \xe2\x9a\xa0 CONTAINMENT BREACH! Structural integrity critical! "
                << Color::RESET << "\n";
            state.addAlertMessage(oss.str());
        }
        state.addLogEntry("CRITICAL", "Containment breach detected");
    } else if (state.containmentIntegrity < RC::CONTAINMENT_WARNING) {
        std::ostringstream oss;
        oss << Color::RED << "\xe2\x9a\xa0 CONTAINMENT WARNING: Integrity at "
            << std::fixed << std::setprecision(1) << state.containmentIntegrity << "%"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
    }

    // A breach leaks radiation (see RadiationSystem) and costs points every turn
    if (state.containmentBreach) {
        state.score = std::max(0, state.score - RC::BREACH_PENALTY * state.currentDifficulty.scoreMultiplier);
    }

    // Recovery from breach
    if (state.containmentBreach && state.containmentIntegrity > RC::CONTAINMENT_WARNING) {
        state.containmentBreach = false;
        std::ostringstream oss;
        oss << Color::GREEN << "\xe2\x9c\x93 Containment integrity restored!" << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Containment integrity restored");
    }
}
