#include "containment.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

void ContainmentSystem::update(ReactorState& state) {
    // Containment degrades under stress
    double stressFactor = 0.0;

    // High temperature stresses containment
    if (state.temperature > state.currentDifficulty.scramTemperature * 0.7) {
        stressFactor += (state.temperature - state.currentDifficulty.scramTemperature * 0.7) / 500.0;
    }

    // High steam pressure stresses containment
    if (state.steamPressure > RC::MAX_STEAM_PRESSURE * 0.8) {
        stressFactor += (state.steamPressure - RC::MAX_STEAM_PRESSURE * 0.8) / 100.0;
    }

    // High radiation indicates containment stress
    if (state.radiationLevel > RC::WARNING_RADIATION) {
        stressFactor += (state.radiationLevel - RC::WARNING_RADIATION) / 1000.0;
    }

    // Apply degradation
    if (stressFactor > 0) {
        state.containmentIntegrity = std::max(0.0, state.containmentIntegrity - stressFactor * 0.1);
    } else {
        // Slow natural recovery when not under stress
        state.containmentIntegrity = std::min(RC::MAX_CONTAINMENT, state.containmentIntegrity + 0.05);
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

        // Breach increases radiation significantly
        state.radiationLevel *= 2.0;
    } else if (state.containmentIntegrity < RC::CONTAINMENT_WARNING) {
        std::ostringstream oss;
        oss << Color::RED << "\xe2\x9a\xa0 CONTAINMENT WARNING: Integrity at "
            << std::fixed << std::setprecision(1) << state.containmentIntegrity << "%"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
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
