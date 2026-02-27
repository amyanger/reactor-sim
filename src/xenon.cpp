#include "xenon.h"

#include <sstream>
#include <algorithm>

void XenonSystem::update(ReactorState& state) {
    // Xenon builds up based on power level
    double powerFactor = state.power / 100.0;
    state.xenonLevel += powerFactor * state.currentDifficulty.xenonBuildupRate;

    // Xenon decays naturally
    state.xenonLevel = std::max(0.0, state.xenonLevel - RC::XENON_DECAY_RATE);

    // Cap xenon level
    state.xenonLevel = std::min(RC::MAX_XENON, state.xenonLevel);

    // High xenon warning
    if (state.xenonLevel > 70.0) {
        std::ostringstream oss;
        oss << Color::MAGENTA << Color::BOLD
            << "\xe2\x98\xa2 XENON POISONING: High Xe-135 levels affecting reactivity!"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
    }

    // Track successful xenon management
    if (state.xenonLevel > 50.0 && state.xenonLevel < 80.0) {
        state.xenonHandledCount++;
    }
}
