#include "PhysicsEngine.h"
#include "Constants.h"
#include <iostream>

void PhysicsEngine::updateReactor(Reactor& reactor) {
    if (!reactor.isRunning()) return;
    
    // Calculate neutron multiplication
    double k_eff = calculateKeff(reactor.getControlRods());
    reactor.updateNeutrons(k_eff);
    
    // Apply fuel effects
    double fuel_eff = reactor.getFuel() / 100.0;
    reactor.updateNeutrons(fuel_eff);
    applyFuelBurnup(reactor);
    
    // Calculate power and heat
    double power = calculatePower(reactor.getNeutrons());
    reactor.setPower(power);
    
    double heat = calculateHeatGeneration(power);
    reactor.updateTemperature(heat);
    
    // Apply cooling
    applyCooling(reactor);
    
    // Check for auto-SCRAM
    if (checkScramConditions(reactor)) {
        std::cout << "\n*** AUTO SCRAM! Emergency shutdown! ***\n";
        reactor.scram();
    }
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
        std::cout << "!!! WARNING: Coolant is critically low! !!!\n";
        reactor.updateTemperature(LOW_COOLANT_HEATING);
    }
}

bool PhysicsEngine::checkScramConditions(const Reactor& reactor) const {
    return reactor.getTemperature() > SCRAM_TEMP || 
           reactor.getNeutrons() > SCRAM_NEUTRONS;
}