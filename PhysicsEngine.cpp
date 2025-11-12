#include "PhysicsEngine.h"
#include "Constants.h"
#include <algorithm>

// Maximum neutron count to prevent overflow
const double MAX_NEUTRONS = 10000.0;
// Minimum fuel efficiency to prevent complete neutron death
const double MIN_FUEL_EFF = 0.1;

PhysicsStatus PhysicsEngine::updateReactor(Reactor& reactor) {
    PhysicsStatus status = {false, false};

    if (!reactor.isRunning()) return status;

    // Calculate neutron multiplication with upper bound check
    double k_eff = calculateKeff(reactor.getControlRods());
    reactor.updateNeutrons(k_eff);

    // Clamp neutrons to prevent overflow
    double current_neutrons = reactor.getNeutrons();
    if (current_neutrons > MAX_NEUTRONS) {
        current_neutrons = MAX_NEUTRONS;
        reactor.updateNeutrons(MAX_NEUTRONS / reactor.getNeutrons());
    }

    // Apply fuel effects with minimum efficiency to prevent complete shutdown
    double fuel_eff = reactor.getFuel() / 100.0;
    fuel_eff = std::max(MIN_FUEL_EFF, fuel_eff);
    reactor.updateNeutrons(fuel_eff);
    applyFuelBurnup(reactor);

    // Calculate power and heat
    double power = calculatePower(reactor.getNeutrons());
    reactor.setPower(power);

    double heat = calculateHeatGeneration(power);
    reactor.updateTemperature(heat);

    // Apply cooling and check for warnings
    status.lowCoolantWarning = (reactor.getCoolant() < CRITICAL_COOLANT);
    applyCooling(reactor);

    // Check for auto-SCRAM
    if (checkScramConditions(reactor)) {
        reactor.scram();
        status.scramTriggered = true;
    }

    return status;
}

double PhysicsEngine::calculateKeff(double controlRods) const {
    double k_eff = K_EFF_BASE - controlRods * ROD_ABSORPTION;
    return (k_eff < MIN_K_EFF) ? MIN_K_EFF : k_eff;
}

double PhysicsEngine::calculatePower(double neutrons) const {
    return neutrons * NEUTRON_TO_POWER;
}

double PhysicsEngine::calculateHeatGeneration(double power) const {
    return power * POWER_TO_HEAT;
}

void PhysicsEngine::applyFuelBurnup(Reactor& reactor) const {
    reactor.consumeFuel(FUEL_DEPLETION_RATE);
}

void PhysicsEngine::applyCooling(Reactor& reactor) const {
    reactor.updateCoolant(-COOLANT_LOSS_RATE);
    reactor.updateTemperature(-PASSIVE_COOLING);

    if (reactor.getCoolant() < CRITICAL_COOLANT) {
        reactor.updateTemperature(LOW_COOLANT_HEATING);
    }
}

bool PhysicsEngine::checkScramConditions(const Reactor& reactor) const {
    return reactor.getTemperature() > SCRAM_TEMP || 
           reactor.getNeutrons() > SCRAM_NEUTRONS;
}