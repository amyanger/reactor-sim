#include "RandomEvents.h"
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

        if (disasterType == 0 && reactor.getCoolant() > 10.0) {
            applyCoolantLeak(reactor);
        } else {
            applyPowerSurge(reactor);
        }
    }
}

void RandomEvents::applyCoolantLeak(Reactor& reactor) {
    reactor.updateCoolant(-10.0);
    std::cout << "!!! RANDOM EVENT: Coolant Leak! Lost 10% coolant! !!!\n";
}

void RandomEvents::applyPowerSurge(Reactor& reactor) {
    reactor.updateTemperature(50.0);
    std::cout << "!!! RANDOM EVENT: Power Surge! Temperature increased by 50C! !!!\n";
}