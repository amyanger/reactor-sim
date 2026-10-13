#pragma once

namespace RC {
    // Time scale: one turn is 6 minutes of plant time, so 10 turns = 1 hour.
    static constexpr double HOURS_PER_TURN = 0.1;
    static constexpr int    TURNS_PER_HOUR = 10;

    // Initial values (plant starts near equilibrium at 50% power, turbine online)
    static constexpr double INITIAL_NEUTRONS     = 500.0;
    static constexpr double INITIAL_TEMPERATURE  = 300.0;
    static constexpr double INITIAL_COOLANT      = 100.0;
    static constexpr double INITIAL_FUEL         = 100.0;
    static constexpr double INITIAL_CONTROL_RODS = 0.62;

    // Safety thresholds
    static constexpr double SCRAM_NEUTRONS   = 1300.0;  // 130% overpower trip
    static constexpr double CRITICAL_COOLANT = 20.0;
    static constexpr double RESET_TEMP_FRACTION = 0.9;  // trip can't be reset above 90% of SCRAM temp

    // Neutronics. Reactivity is in dollars ($), so prompt critical is rho >= 1.
    static constexpr double NEUTRONS_NOMINAL     = 1000.0;  // 100% rated power
    static constexpr double NEUTRON_TO_POWER     = 3.0;     // MW thermal per neutron unit
    static constexpr double RATED_THERMAL_MW     = NEUTRONS_NOMINAL * NEUTRON_TO_POWER;
    static constexpr double EXCESS_REACTIVITY    = 4.0;     // fresh, cold, xenon-free core
    static constexpr double ROD_WORTH            = 4.8;     // total worth of a full rod insertion
    static constexpr double ROD_SPEED            = 0.05;    // max rod travel per turn (fraction)
    static constexpr double TEMP_COEFFICIENT     = 0.002;   // $ lost per degree C above reference
    static constexpr double TEMP_REFERENCE       = 30.0;    // cold core temperature (C)
    static constexpr double BURNUP_WORTH         = 2.0;     // $ lost when fuel reaches 0%
    static constexpr double KINETICS_RATE_POS    = 0.3;     // per-turn growth scale for rho > 0
    static constexpr double KINETICS_RATE_NEG    = 1.5;     // per-turn decay scale for rho < 0
    static constexpr double PROMPT_CRITICAL_GROWTH = 1.5;   // multiplier per substep when rho >= 1
    static constexpr double NEUTRON_SOURCE       = 0.5;     // startup source, per turn
    static constexpr double MAX_NEUTRONS         = 1.0e6;
    static constexpr int    PHYSICS_SUBSTEPS     = 10;

    // Heat balance: T += (fission + decay - removed) / capacity
    static constexpr double HEAT_CAPACITY        = 4.0;     // MW-turns per degree C
    static constexpr double HEAT_TRANSFER        = 5.6;     // MW per degree C with full pumps and coolant
    static constexpr double HEAT_SINK_TEMP       = 30.0;    // ultimate heat sink (C)
    static constexpr double FULL_COOLANT_LEVEL   = 60.0;    // coolant % above which cooling is unaffected
    static constexpr double MIN_COOLANT_FACTOR   = 0.03;    // air/structure cooling of a dry core
    static constexpr double NATURAL_CIRCULATION  = 0.3;     // cooling fraction with no pump power
    static constexpr double DIESEL_PUMP_FACTOR   = 0.8;     // cooling fraction on diesel-powered pumps
    static constexpr double WEATHER_COOLING_WEIGHT = 0.5;   // how strongly weather scales cooling
    static constexpr double DECAY_HEAT_FRACTION  = 0.065;   // decay heat share of fission power
    static constexpr double DECAY_HEAT_RATE      = 0.05;    // per-turn approach/decay rate
    static constexpr double BOILOFF_DIVISOR      = 50.0;    // coolant % lost per turn = (T - scramT) / this

    // Turbine
    static constexpr double OPTIMAL_STEAM_TEMP = 550.0;
    static constexpr double MIN_TURBINE_TEMP   = 200.0;
    static constexpr double MAX_TURBINE_RPM    = 3600.0;
    static constexpr double TURBINE_SPINUP_RPM = 600.0;     // per turn
    static constexpr double TURBINE_SPINDOWN_RPM = 300.0;   // per turn
    static constexpr double HOUSE_POWER_RPM    = 1800.0;    // turbine powers the pumps above this
    static constexpr double PEAK_EFFICIENCY    = 0.36;      // thermal-to-electric at optimal temp
    static constexpr double MIN_EFFICIENCY     = 0.10;
    static constexpr double EFFICIENCY_WIDTH   = 500.0;     // C from optimum where efficiency hits 0
    static constexpr double ELECTRICIAN_FRACTION = 0.9;     // of max output, for the achievement

    // Steam pressure: target = PRESSURE_PER_DEGREE * (T - MIN_TURBINE_TEMP)
    static constexpr double PRESSURE_PER_DEGREE    = 0.2;
    static constexpr double PRESSURE_OFFLINE_FACTOR = 1.3;  // no steam draw with the turbine down
    static constexpr double PRESSURE_SMOOTHING     = 0.5;
    static constexpr double MAX_STEAM_PRESSURE = 150.0;
    static constexpr double CRITICAL_PRESSURE  = 130.0;     // relief valve opens
    static constexpr double RELIEF_CLOSE_FRACTION = 0.8;
    static constexpr double RELIEF_PRESSURE_DROP  = 15.0;   // bar per turn while venting
    static constexpr double RELIEF_HEAT_REMOVAL   = 300.0;  // MW removed by venting steam
    static constexpr double RELIEF_COOLANT_LOSS   = 1.0;    // coolant % per turn while venting
    static constexpr double RUPTURE_PRESSURE   = 160.0;
    static constexpr double RUPTURE_COOLANT_LOSS = 25.0;
    static constexpr double RUPTURE_HEAT       = 50.0;

    // Emergency Cooling System
    static constexpr double ECCS_COOLANT_BOOST  = 50.0;
    static constexpr double ECCS_TEMP_REDUCTION = 100.0;
    static constexpr int    ECCS_COOLDOWN       = 10;

    // Coolant makeup ('r' command, costs a turn)
    static constexpr double COOLANT_REFILL_AMOUNT = 40.0;

    // Diesel generator (powers coolant pumps when the turbine can't)
    static constexpr double DIESEL_FUEL_CAPACITY    = 100.0;
    static constexpr double DIESEL_FUEL_CONSUMPTION = 2.0;
    static constexpr double DIESEL_LOW_FUEL         = 20.0;
    static constexpr double DIESEL_NEEDED_HEAT_MW   = 20.0;   // core heat that warrants pump power
    static constexpr double DIESEL_NEEDED_TEMP      = 100.0;

    // Radiation (mSv/h)
    static constexpr double BACKGROUND_RADIATION = 0.1;
    static constexpr double FULL_POWER_RADIATION = 5.0;
    static constexpr double MAX_SAFE_RADIATION   = 20.0;
    static constexpr double WARNING_RADIATION    = 100.0;
    static constexpr double DANGER_RADIATION     = 500.0;
    static constexpr double LETHAL_RADIATION     = 2000.0;
    static constexpr double BREACH_RADIATION_MULTIPLIER = 3.0;

    // Containment
    static constexpr double MAX_CONTAINMENT       = 100.0;
    static constexpr double CONTAINMENT_WARNING   = 70.0;
    static constexpr double CONTAINMENT_CRITICAL  = 40.0;
    static constexpr double CONTAINMENT_RECOVERY  = 0.1;   // per turn with no stress
    static constexpr double CONTAINMENT_TEMP_STRESS_START = 0.8;  // fraction of SCRAM temp

    // Xenon/iodine chain, normalized so full-power equilibrium xenon is ~50%
    static constexpr double MAX_XENON          = 100.0;
    static constexpr double IODINE_DECAY       = 0.034657;  // ln2 / 20 turns
    static constexpr double XENON_DECAY        = 0.024755;  // ln2 / 28 turns
    static constexpr double XENON_BURNOUT      = 4.0 * XENON_DECAY;  // at full power
    static constexpr double XENON_TOTAL_YIELD  = 50.0 * (XENON_DECAY + XENON_BURNOUT);
    static constexpr double XENON_DIRECT_SHARE = 0.06;      // share of yield born as xenon
    static constexpr double XENON_WORTH_PER_PERCENT = 0.016;  // $ per xenon %
    static constexpr double XENON_HIGH         = 70.0;
    static constexpr double XENON_RECOVERED    = 50.0;

    // Scoring
    static constexpr int POINTS_PER_100MW      = 5;    // per 100 MW delivered to the grid, per turn
    static constexpr int GRID_MATCH_BONUS      = 10;   // supply within 95-110% of demand
    static constexpr double GRID_OVERSUPPLY    = 110.0;
    static constexpr double GRID_UNDERSUPPLY   = 50.0;
    static constexpr double GRID_DEMAND_SMOOTHING = 0.3;  // fraction of the gap to target closed per turn
    static constexpr int SCRAM_PENALTY         = 1000;
    static constexpr int REFILL_PENALTY        = 50;
    static constexpr int DIESEL_REFILL_PENALTY = 25;
    static constexpr int ECCS_PENALTY          = 100;
    static constexpr int BREACH_PENALTY        = 25;   // per turn while breached
    static constexpr double RECOVERY_ELECTRIC_MW = 300.0;  // output that counts as recovered after a SCRAM

    // File paths
    static constexpr const char* HIGH_SCORE_FILE    = ".reactor_highscore";
    static constexpr const char* ACHIEVEMENTS_FILE  = ".reactor_achievements";
    static constexpr const char* SAVE_FILE          = ".reactor_save";

    // Misc
    static constexpr int MAX_LOG_ENTRIES = 100;
}
