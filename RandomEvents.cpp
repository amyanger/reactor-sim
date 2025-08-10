#include "RandomEvents.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

RandomEvents::RandomEvents() {
    std::srand(std::time(nullptr));
}

void RandomEvents::checkAndApply(Reactor& reactor) {
    int eventRoll = std::rand() % 10;
    if (eventRoll == 0) {  // 10% chance
        int disasterType = std::rand() % 2;
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