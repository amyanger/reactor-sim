#include "Reactor.h"
#include "Constants.h"
#include <algorithm>

Reactor::Reactor() 
    : neutrons(1000.0),
      controlRods(0.5),
      temperature(300.0),
      coolant(100.0),
      power(0.0),
      fuel(100.0),
      running(true) {
}

void Reactor::setControlRods(double level) {
    controlRods = std::min(1.0, std::max(0.0, level));
}

void Reactor::refillCoolant() {
    coolant = 100.0;
}

void Reactor::consumeFuel(double amount) {
    fuel = std::max(0.0, fuel - amount);
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
    coolant = std::min(100.0, std::max(0.0, coolant + delta));
}

void Reactor::setPower(double p) {
    power = p;
}

void Reactor::scram() {
    controlRods = 1.0;
    neutrons *= 0.05;
    updateTemperature(-200);
    running = false;
}

void Reactor::reset() {
    neutrons = 1000.0;
    controlRods = 0.5;
    temperature = 300.0;
    coolant = 100.0;
    power = 0.0;
    fuel = 100.0;
    running = true;
}

bool Reactor::checkMeltdown() const {
    return temperature > MELTDOWN_TEMP;
}