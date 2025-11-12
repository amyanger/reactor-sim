#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============================================================================
// REACTOR PHYSICS CONSTANTS
// ============================================================================

// Base neutron multiplication factor (k-effective) with no control rods
const double K_EFF_BASE = 1.05;

// Neutron absorption factor per unit of control rod insertion
const double ROD_ABSORPTION = 1.1;

// Minimum k-effective (prevents reactor from going too subcritical)
const double MIN_K_EFF = 0.7;

// Conversion factor from neutron count to power output (MW)
const double NEUTRON_TO_POWER = 0.1;

// Conversion factor from power output to heat generation (°C per MW)
const double POWER_TO_HEAT = 0.01;

// Rate of fuel depletion per simulation tick (% per tick)
const double FUEL_DEPLETION_RATE = 0.1;

// Minimum fuel efficiency factor (prevents complete neutron death at 0% fuel)
const double MIN_FUEL_EFFICIENCY = 0.1;

// Maximum neutron count before clamping (prevents overflow)
const double MAX_NEUTRONS = 10000.0;

// ============================================================================
// REACTOR INITIAL STATE
// ============================================================================

// Initial neutron population at reactor startup
const double INITIAL_NEUTRONS = 1000.0;

// Initial control rod position (0.0 = fully out, 1.0 = fully in)
const double INITIAL_CONTROL_RODS = 0.5;

// Initial reactor temperature in degrees Celsius
const double INITIAL_TEMPERATURE = 300.0;

// Initial coolant level (percentage)
const double INITIAL_COOLANT = 100.0;

// Initial power output (MW)
const double INITIAL_POWER = 0.0;

// Initial fuel level (percentage)
const double INITIAL_FUEL = 100.0;

// ============================================================================
// SAFETY SYSTEM THRESHOLDS
// ============================================================================

// Temperature threshold for automatic SCRAM (emergency shutdown)
const double SCRAM_TEMP = 1000.0;

// Neutron count threshold for automatic SCRAM
const double SCRAM_NEUTRONS = 2000.0;

// Temperature threshold for core meltdown (game over)
const double MELTDOWN_TEMP = 2000.0;

// Minimum safe reactor temperature
const double MIN_REACTOR_TEMP = 100.0;

// Coolant level below which overheating begins
const double CRITICAL_COOLANT = 20.0;

// ============================================================================
// SCRAM (EMERGENCY SHUTDOWN) PARAMETERS
// ============================================================================

// Control rod position during SCRAM (fully inserted)
const double SCRAM_CONTROL_RODS = 1.0;

// Neutron reduction factor during SCRAM (rapid shutdown)
const double SCRAM_NEUTRON_FACTOR = 0.05;

// Temperature drop applied during SCRAM (emergency cooling)
const double SCRAM_COOLING_DELTA = -200.0;

// ============================================================================
// OPERATIONAL PARAMETERS
// ============================================================================

// Rate of coolant loss per simulation tick (% per tick)
const double COOLANT_LOSS_RATE = 0.3;

// Passive cooling rate per tick (°C per tick)
const double PASSIVE_COOLING = 0.5;

// Additional heating when coolant is critically low (°C per tick)
const double LOW_COOLANT_HEATING = 5.0;

// Full coolant level (100%)
const double MAX_COOLANT = 100.0;

// Fully inserted control rod position
const double MAX_CONTROL_RODS = 1.0;

// Fully withdrawn control rod position
const double MIN_CONTROL_RODS = 0.0;

// Maximum fuel level (100%)
const double MAX_FUEL = 100.0;

// Minimum fuel level (0%)
const double MIN_FUEL = 0.0;

// ============================================================================
// RANDOM EVENT PARAMETERS
// ============================================================================

// Minimum coolant level required to trigger coolant leak event
const double EVENT_MIN_COOLANT_FOR_LEAK = 10.0;

// Coolant loss from leak event (percentage points)
const double EVENT_COOLANT_LEAK_AMOUNT = 10.0;

// Temperature increase from power surge event (°C)
const double EVENT_POWER_SURGE_TEMP = 50.0;

#endif