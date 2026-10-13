#include "xenon.h"

#include <sstream>
#include <algorithm>

void XenonSystem::update(ReactorState& state) {
    // Normalized I-135 -> Xe-135 chain. Xenon is burned out by neutron flux, so it
    // peaks a few hours after a shutdown or power cut (the "iodine pit").
    double phi = state.power / RC::RATED_THERMAL_MW;
    double yieldScale = state.currentDifficulty.xenonBuildupRate;
    double iodineYield = RC::XENON_TOTAL_YIELD * (1.0 - RC::XENON_DIRECT_SHARE) * yieldScale;
    double xenonYield  = RC::XENON_TOTAL_YIELD * RC::XENON_DIRECT_SHARE * yieldScale;

    double iodineDecay = RC::IODINE_DECAY * state.iodineLevel;
    state.iodineLevel = std::max(0.0, state.iodineLevel + iodineYield * phi - iodineDecay);

    state.xenonLevel += xenonYield * phi + iodineDecay
                      - RC::XENON_DECAY * state.xenonLevel
                      - RC::XENON_BURNOUT * phi * state.xenonLevel;
    state.xenonLevel = std::max(0.0, std::min(RC::MAX_XENON, state.xenonLevel));

    if (state.xenonLevel > RC::XENON_HIGH) {
        std::ostringstream oss;
        oss << Color::MAGENTA << Color::BOLD
            << "\xe2\x98\xa2 XENON POISONING: High Xe-135 levels affecting reactivity!"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.xenonTransientActive = true;
    }

    // A transient counts as handled once xenon falls back with the reactor still running
    if (state.xenonTransientActive && state.running && state.xenonLevel < RC::XENON_RECOVERED) {
        state.xenonTransientActive = false;
        state.xenonHandledCount++;
        std::ostringstream oss;
        oss << Color::GREEN << "\xe2\x9c\x93 Xenon transient handled." << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Xenon transient ridden out");
    }
}
