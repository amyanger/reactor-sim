#include "RandomEvents.h"
#include "Constants.h"
#include <iostream>
#include <chrono>

RandomEvents::RandomEvents() {
    // Use high-resolution clock for better seed entropy
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(static_cast<unsigned int>(seed));
}

void RandomEvents::checkAndApply(Reactor& reactor) {
    std::uniform_int_distribution<int> eventDist(0, 9);
    int eventRoll = eventDist(rng);

    if (eventRoll == 0) {  // 10% chance
        std::uniform_int_distribution<int> disasterDist(0, 1);
        int disasterType = disasterDist(rng);

        if (disasterType == 0 && reactor.getCoolant() > EVENT_MIN_COOLANT_FOR_LEAK) {
            applyCoolantLeak(reactor);
        } else {
            applyPowerSurge(reactor);
        }
    }
}

void RandomEvents::applyCoolantLeak(Reactor& reactor) {
    reactor.updateCoolant(-EVENT_COOLANT_LEAK_AMOUNT);
    std::cout << "!!! RANDOM EVENT: Coolant Leak! Lost "
              << EVENT_COOLANT_LEAK_AMOUNT << "% coolant! !!!\n";
}

void RandomEvents::applyPowerSurge(Reactor& reactor) {
    reactor.updateTemperature(EVENT_POWER_SURGE_TEMP);
    std::cout << "!!! RANDOM EVENT: Power Surge! Temperature increased by "
              << EVENT_POWER_SURGE_TEMP << "C! !!!\n";
}