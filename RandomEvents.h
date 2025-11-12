#ifndef RANDOM_EVENTS_H
#define RANDOM_EVENTS_H

#include "Reactor.h"
#include <random>

class RandomEvents {
public:
    RandomEvents();
    void checkAndApply(Reactor& reactor);

private:
    void applyCoolantLeak(Reactor& reactor);
    void applyPowerSurge(Reactor& reactor);

    std::mt19937 rng;
};

#endif