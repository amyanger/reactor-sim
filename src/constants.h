#pragma once

namespace RC {
    // Initial values
    static constexpr double INITIAL_NEUTRONS     = 1000.0;
    static constexpr double INITIAL_TEMPERATURE  = 300.0;
    static constexpr double INITIAL_COOLANT      = 100.0;
    static constexpr double INITIAL_FUEL         = 100.0;
    static constexpr double INITIAL_CONTROL_RODS = 0.5;

    // Safety thresholds
    static constexpr double SCRAM_NEUTRONS  = 2000.0;
    static constexpr double CRITICAL_COOLANT = 20.0;

    // Physics
    static constexpr double NATURAL_COOLING_RATE   = 0.5;
    static constexpr double POWER_TO_HEAT_RATIO    = 0.01;
    static constexpr double NEUTRON_TO_POWER_RATIO = 0.1;

    // Turbine
    static constexpr double OPTIMAL_STEAM_TEMP = 500.0;
    static constexpr double MIN_TURBINE_TEMP   = 200.0;
    static constexpr double MAX_TURBINE_RPM    = 3600.0;

    // Steam pressure
    static constexpr double MAX_STEAM_PRESSURE = 150.0;
    static constexpr double CRITICAL_PRESSURE  = 130.0;
    static constexpr double RUPTURE_PRESSURE   = 160.0;

    // Emergency Cooling System
    static constexpr double ECCS_COOLANT_BOOST  = 50.0;
    static constexpr double ECCS_TEMP_REDUCTION = 100.0;
    static constexpr int    ECCS_COOLDOWN       = 10;

    // Diesel generator
    static constexpr double DIESEL_FUEL_CAPACITY    = 100.0;
    static constexpr double DIESEL_FUEL_CONSUMPTION = 2.0;
    static constexpr double DIESEL_POWER_OUTPUT     = 50.0;

    // Radiation (mSv/h)
    static constexpr double BACKGROUND_RADIATION = 0.1;
    static constexpr double MAX_SAFE_RADIATION   = 20.0;
    static constexpr double WARNING_RADIATION    = 100.0;
    static constexpr double DANGER_RADIATION     = 500.0;
    static constexpr double LETHAL_RADIATION     = 2000.0;

    // Containment
    static constexpr double MAX_CONTAINMENT       = 100.0;
    static constexpr double CONTAINMENT_WARNING   = 70.0;
    static constexpr double CONTAINMENT_CRITICAL  = 40.0;

    // Xenon
    static constexpr double MAX_XENON        = 100.0;
    static constexpr double XENON_DECAY_RATE = 2.0;

    // Scoring
    static constexpr int POINTS_PER_TURN       = 10;
    static constexpr int POINTS_PER_POWER_UNIT = 1;
    static constexpr int POINTS_PER_MW         = 5;
    static constexpr int SCRAM_PENALTY         = 500;
    static constexpr int REFILL_PENALTY        = 50;
    static constexpr int ECCS_PENALTY          = 100;

    // File paths
    static constexpr const char* HIGH_SCORE_FILE    = ".reactor_highscore";
    static constexpr const char* ACHIEVEMENTS_FILE  = ".reactor_achievements";
    static constexpr const char* SAVE_FILE          = ".reactor_save";

    // Misc
    static constexpr int MAX_LOG_ENTRIES = 100;
}
