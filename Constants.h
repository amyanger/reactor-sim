#ifndef CONSTANTS_H
#define CONSTANTS_H

// Physics constants
const double K_EFF_BASE = 1.05;
const double ROD_ABSORPTION = 1.1;
const double MIN_K_EFF = 0.7;
const double NEUTRON_TO_POWER = 0.1;
const double POWER_TO_HEAT = 0.01;
const double FUEL_DEPLETION_RATE = 0.1;

// Safety thresholds
const double SCRAM_TEMP = 1000.0;
const double SCRAM_NEUTRONS = 2000.0;
const double MELTDOWN_TEMP = 2000.0;
const double MIN_REACTOR_TEMP = 100.0;
const double CRITICAL_COOLANT = 20.0;

// Operational parameters
const double COOLANT_LOSS_RATE = 0.3;
const double PASSIVE_COOLING = 0.5;
const double LOW_COOLANT_HEATING = 5.0;

#endif