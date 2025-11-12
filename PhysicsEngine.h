#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "Reactor.h"

struct PhysicsStatus {
    bool scramTriggered;
    bool lowCoolantWarning;
};

class PhysicsEngine {
public:
    PhysicsStatus updateReactor(Reactor& reactor);

private:
    double calculateKeff(double controlRods) const;
    double calculatePower(double neutrons) const;
    double calculateHeatGeneration(double power) const;
    void applyFuelBurnup(Reactor& reactor) const;
    void applyCooling(Reactor& reactor) const;
    bool checkScramConditions(const Reactor& reactor) const;
};

#endif