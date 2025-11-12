#include "Reactor.h"
#include "Constants.h"
#include <algorithm>

Reactor::Reactor()
    : neutrons(INITIAL_NEUTRONS),
      controlRods(INITIAL_CONTROL_RODS),
      temperature(INITIAL_TEMPERATURE),
      coolant(INITIAL_COOLANT),
      power(INITIAL_POWER),
      fuel(INITIAL_FUEL),
      running(true) {
}

void Reactor::setControlRods(double level) {
    controlRods = std::min(MAX_CONTROL_RODS, std::max(MIN_CONTROL_RODS, level));
}

void Reactor::refillCoolant() {
    coolant = MAX_COOLANT;
}

void Reactor::consumeFuel(double amount) {
    fuel = std::max(MIN_FUEL, fuel - amount);
}

void Reactor::updateNeutrons(double factor) {
    neutrons *= factor;
}

void Reactor::updateTemperature(double delta) {
    temperature += delta;
    if (temperature < MIN_REACTOR_TEMP) {
        temperature = MIN_REACTOR_TEMP;
    }
}

void Reactor::updateCoolant(double delta) {
    coolant = std::min(MAX_COOLANT, std::max(MIN_FUEL, coolant + delta));
}

void Reactor::setPower(double p) {
    power = p;
}

void Reactor::scram() {
    controlRods = SCRAM_CONTROL_RODS;
    neutrons *= SCRAM_NEUTRON_FACTOR;
    updateTemperature(SCRAM_COOLING_DELTA);
    running = false;
}

void Reactor::reset() {
    neutrons = INITIAL_NEUTRONS;
    controlRods = INITIAL_CONTROL_RODS;
    temperature = INITIAL_TEMPERATURE;
    coolant = INITIAL_COOLANT;
    power = INITIAL_POWER;
    fuel = INITIAL_FUEL;
    running = true;
}

bool Reactor::checkMeltdown() const {
    return temperature > MELTDOWN_TEMP;
}