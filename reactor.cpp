#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <random>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <vector>

// ANSI Color Codes for terminal output
namespace Color {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";

    // Background colors
    const std::string BG_RED = "\033[41m";
    const std::string BG_GREEN = "\033[42m";
    const std::string BG_YELLOW = "\033[43m";
    const std::string BG_BLUE = "\033[44m";
}

enum class Difficulty {
    EASY,
    NORMAL,
    HARD,
    NIGHTMARE
};

enum class EventType {
    // Negative events
    COOLANT_LEAK,
    POWER_SURGE,
    PUMP_FAILURE,
    SENSOR_MALFUNCTION,
    STEAM_LEAK,
    CONTROL_ROD_STUCK,
    TURBINE_TRIP,
    // Positive events
    EFFICIENCY_BOOST,
    COOLANT_BONUS,
    MAINTENANCE_CREW,
    // Neutral events
    INSPECTION,
    SHIFT_CHANGE
};

struct DifficultySettings {
    std::string name;
    double fuelDepletionRate;
    double coolantLossRate;
    double eventChance;
    double scramTemperature;
    double meltdownTemperature;
    int scoreMultiplier;
    double turbineEfficiency;
};

class ReactorSimulator {
private:
    static constexpr double INITIAL_NEUTRONS = 1000.0;
    static constexpr double INITIAL_TEMPERATURE = 300.0;
    static constexpr double INITIAL_COOLANT = 100.0;
    static constexpr double INITIAL_FUEL = 100.0;
    static constexpr double INITIAL_CONTROL_RODS = 0.5;

    static constexpr double SCRAM_NEUTRONS = 2000.0;
    static constexpr double CRITICAL_COOLANT = 20.0;

    static constexpr double NATURAL_COOLING_RATE = 0.5;
    static constexpr double POWER_TO_HEAT_RATIO = 0.01;
    static constexpr double NEUTRON_TO_POWER_RATIO = 0.1;

    // Turbine constants
    static constexpr double OPTIMAL_STEAM_TEMP = 500.0;
    static constexpr double MIN_TURBINE_TEMP = 200.0;
    static constexpr double MAX_TURBINE_RPM = 3600.0;

    // Scoring constants
    static constexpr int POINTS_PER_TURN = 10;
    static constexpr int POINTS_PER_POWER_UNIT = 1;
    static constexpr int POINTS_PER_MW = 5;
    static constexpr int SCRAM_PENALTY = 500;
    static constexpr int REFILL_PENALTY = 50;

    // Difficulty settings
    DifficultySettings currentDifficulty;

    double neutrons;
    double controlRods;
    double temperature;
    double coolant;
    double power;
    double fuel;
    bool running;

    // Turbine system
    double turbineRPM;
    double steamPressure;
    double electricityOutput;  // in MW
    double totalElectricityGenerated;
    bool turbineOnline;

    // Scoring system
    int score;
    int turns;
    int scramCount;
    int highScore;
    int eventsExperienced;

    // Modern random number generation
    std::mt19937 rng;

    static constexpr const char* HIGH_SCORE_FILE = ".reactor_highscore";

    static DifficultySettings getDifficultySettings(Difficulty diff) {
        switch (diff) {
            case Difficulty::EASY:
                return {"Easy", 0.05, 0.15, 15.0, 1200.0, 2500.0, 1, 0.95};
            case Difficulty::NORMAL:
                return {"Normal", 0.1, 0.3, 10.0, 1000.0, 2000.0, 2, 0.90};
            case Difficulty::HARD:
                return {"Hard", 0.15, 0.5, 7.0, 800.0, 1500.0, 3, 0.85};
            case Difficulty::NIGHTMARE:
                return {"Nightmare", 0.2, 0.7, 5.0, 600.0, 1200.0, 5, 0.75};
            default:
                return {"Normal", 0.1, 0.3, 10.0, 1000.0, 2000.0, 2, 0.90};
        }
    }

public:
    ReactorSimulator(Difficulty diff = Difficulty::NORMAL) :
        currentDifficulty(getDifficultySettings(diff)),
        neutrons(INITIAL_NEUTRONS),
        controlRods(INITIAL_CONTROL_RODS),
        temperature(INITIAL_TEMPERATURE),
        coolant(INITIAL_COOLANT),
        power(0.0),
        fuel(INITIAL_FUEL),
        running(true),
        turbineRPM(0.0),
        steamPressure(0.0),
        electricityOutput(0.0),
        totalElectricityGenerated(0.0),
        turbineOnline(false),
        score(0),
        turns(0),
        scramCount(0),
        highScore(0),
        eventsExperienced(0),
        rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
        loadHighScore();
    }

private:
    void loadHighScore() {
        std::string filename = std::string(HIGH_SCORE_FILE) + "_" + currentDifficulty.name;
        std::ifstream file(filename);
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }
    }

    void saveHighScore() {
        if (score > highScore) {
            highScore = score;
            std::string filename = std::string(HIGH_SCORE_FILE) + "_" + currentDifficulty.name;
            std::ofstream file(filename);
            if (file.is_open()) {
                file << highScore;
                file.close();
            }
        }
    }

    void displayHelp() const {
        std::cout << "\n" << Color::BOLD << Color::CYAN
                  << "╔═══════════════════════════════════════════════════════════╗\n"
                  << "║                    OPERATOR'S MANUAL                      ║\n"
                  << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " COMMANDS:" << Color::RESET
                  << std::setw(50) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   0-100  : Set control rod insertion percentage"
                  << std::setw(10) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   r      : Refill coolant (costs " << REFILL_PENALTY << " points)"
                  << std::setw(17) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   t      : Toggle turbine online/offline"
                  << std::setw(18) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   h      : Display this help screen"
                  << std::setw(22) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   q      : Quit the simulation"
                  << std::setw(27) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " TURBINE SYSTEM:" << Color::RESET
                  << std::setw(43) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Turbine generates electricity from steam"
                  << std::setw(13) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Needs temperature > 200°C to operate"
                  << std::setw(17) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Optimal steam temperature: 500°C"
                  << std::setw(21) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • More MW generated = more points!"
                  << std::setw(21) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " RANDOM EVENTS:" << Color::RESET
                  << std::setw(44) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Coolant Leak, Power Surge, Pump Failure"
                  << std::setw(14) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Sensor Malfunction, Steam Leak, Turbine Trip"
                  << std::setw(9) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Bonus events: Efficiency Boost, Maintenance Crew"
                  << std::setw(5) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " SAFETY LIMITS (" << currentDifficulty.name << " mode):" << Color::RESET
                  << std::setw(34 - static_cast<int>(currentDifficulty.name.length())) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • SCRAM triggers at: " << currentDifficulty.scramTemperature << "°C or "
                  << SCRAM_NEUTRONS << " neutrons" << std::setw(7) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • MELTDOWN at: " << currentDifficulty.meltdownTemperature << "°C"
                  << std::setw(31) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::BOLD << Color::CYAN
                  << "╚═══════════════════════════════════════════════════════════╝" << Color::RESET << "\n\n";

        std::cout << Color::DIM << "Press Enter to continue..." << Color::RESET;
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    std::string getBarColor(double value, double max, bool inverse = false) const {
        double ratio = value / max;
        if (inverse) ratio = 1.0 - ratio;

        if (ratio < 0.3) return Color::GREEN;
        if (ratio < 0.6) return Color::YELLOW;
        return Color::RED;
    }

    void printBar(const std::string& label, double value, double max, int width = 20, bool inverse = false) const {
        int bars = static_cast<int>((value / max) * width);
        bars = std::max(0, std::min(width, bars));

        std::string color = getBarColor(value, max, inverse);

        std::cout << std::left << std::setw(12) << label << "[";
        std::cout << color;
        for (int i = 0; i < width; ++i) {
            std::cout << (i < bars ? "█" : " ");
        }
        std::cout << Color::RESET << "]  ";

        std::cout << color << std::fixed << std::setprecision(1) << value << Color::RESET;
        if (label == "Temp" || label == "Steam") std::cout << "°C";
        else if (label == "Coolant" || label == "Fuel") std::cout << "%";
        else if (label == "Turbine") std::cout << " RPM";
        else if (label == "Output") std::cout << " MW";
        std::cout << std::endl;
    }

    void displayDashboard() const {
        std::cout << "\n" << Color::BOLD << Color::CYAN << "╔══════════════════════════════════════════════╗" << Color::RESET << "\n";
        std::cout << Color::BOLD << Color::CYAN << "║      REACTOR DASHBOARD [" << currentDifficulty.name << "]"
                  << std::setw(20 - static_cast<int>(currentDifficulty.name.length())) << "" << "║" << Color::RESET << "\n";
        std::cout << Color::BOLD << Color::CYAN << "╠══════════════════════════════════════════════╣" << Color::RESET << "\n";

        // Core section
        std::cout << Color::CYAN << "║ " << Color::BOLD << "REACTOR CORE" << Color::RESET
                  << std::setw(33) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Temp", temperature, currentDifficulty.meltdownTemperature, 18, false);

        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Coolant", coolant, 100.0, 18, true);

        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Fuel", fuel, 100.0, 18, true);

        // Turbine section
        std::cout << Color::CYAN << "╠══════════════════════════════════════════════╣" << Color::RESET << "\n";
        std::string turbineStatus = turbineOnline ? (Color::GREEN + "ONLINE" + Color::RESET) : (Color::RED + "OFFLINE" + Color::RESET);
        std::cout << Color::CYAN << "║ " << Color::BOLD << "TURBINE HALL" << Color::RESET
                  << " [" << turbineStatus << "]"
                  << std::setw(20) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Turbine", turbineRPM, MAX_TURBINE_RPM, 18, false);

        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Output", electricityOutput, 1000.0, 18, false);

        std::cout << Color::BOLD << Color::CYAN << "╚══════════════════════════════════════════════╝" << Color::RESET << "\n";
    }

    void displayScore() const {
        std::cout << Color::YELLOW << "╭───────────────────────────────────────────────╮" << Color::RESET << "\n";
        std::cout << Color::YELLOW << "│ " << Color::BOLD << "SCORE: " << Color::RESET
                  << std::setw(7) << score
                  << Color::YELLOW << " │ " << Color::BOLD << "HIGH: " << Color::RESET
                  << std::setw(7) << highScore
                  << Color::YELLOW << " │ " << Color::BOLD << "TURN: " << Color::RESET
                  << std::setw(4) << turns
                  << Color::YELLOW << " │ " << Color::BOLD << "MW·h: " << Color::RESET
                  << std::setw(5) << static_cast<int>(totalElectricityGenerated)
                  << Color::YELLOW << " │" << Color::RESET << "\n";
        std::cout << Color::YELLOW << "╰───────────────────────────────────────────────╯" << Color::RESET << "\n";
    }

    void displayStatus() const {
        std::cout << "\n" << Color::DIM << "Neutrons: " << Color::RESET
                  << std::fixed << std::setprecision(2) << neutrons
                  << Color::DIM << " | Rods: " << Color::RESET
                  << static_cast<int>(controlRods * 100) << "%"
                  << Color::DIM << " | Power: " << Color::RESET << std::setprecision(1) << power << " thermal"
                  << Color::DIM << " | Events: " << Color::RESET << eventsExperienced << "\n";
    }

    double parseControlRodInput(const std::string& input) const {
        try {
            double value = std::stod(input) / 100.0;
            return std::max(0.0, std::min(1.0, value));
        } catch (const std::exception&) {
            return controlRods;
        }
    }

    void updateTurbine() {
        if (!turbineOnline) {
            turbineRPM = std::max(0.0, turbineRPM - 100.0);  // Spin down
            electricityOutput = 0.0;
            return;
        }

        if (temperature < MIN_TURBINE_TEMP) {
            std::cout << Color::YELLOW << "⚠ Turbine cannot operate below " << MIN_TURBINE_TEMP << "°C!" << Color::RESET << "\n";
            turbineRPM = std::max(0.0, turbineRPM - 50.0);
            electricityOutput = 0.0;
            return;
        }

        // Calculate steam pressure based on temperature
        steamPressure = std::min(100.0, (temperature - MIN_TURBINE_TEMP) / 8.0);

        // Calculate target RPM based on steam pressure
        double targetRPM = (steamPressure / 100.0) * MAX_TURBINE_RPM;

        // Gradually adjust RPM
        if (turbineRPM < targetRPM) {
            turbineRPM = std::min(targetRPM, turbineRPM + 200.0);
        } else {
            turbineRPM = std::max(targetRPM, turbineRPM - 200.0);
        }

        // Calculate efficiency based on temperature (optimal at 500°C)
        double tempEfficiency = 1.0 - std::abs(temperature - OPTIMAL_STEAM_TEMP) / 1000.0;
        tempEfficiency = std::max(0.3, std::min(1.0, tempEfficiency));

        // Calculate electricity output
        electricityOutput = (turbineRPM / MAX_TURBINE_RPM) * 1000.0 * tempEfficiency * currentDifficulty.turbineEfficiency;
        totalElectricityGenerated += electricityOutput / 60.0;  // Convert to MW·h (assuming 1 turn = 1 minute)
    }

    void updatePhysics() {
        double k_eff = 1.05 - controlRods * 1.1;
        k_eff = std::max(0.7, k_eff);
        neutrons *= k_eff;

        power = neutrons * NEUTRON_TO_POWER_RATIO;

        double fuel_eff = fuel / 100.0;
        neutrons *= fuel_eff;
        fuel = std::max(0.0, fuel - currentDifficulty.fuelDepletionRate);

        temperature += power * POWER_TO_HEAT_RATIO;
        coolant = std::max(0.0, coolant - currentDifficulty.coolantLossRate);
        temperature = std::max(0.0, temperature - NATURAL_COOLING_RATE);

        if (coolant < CRITICAL_COOLANT) {
            std::cout << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "!!! WARNING: Coolant is critically low! !!!"
                      << Color::RESET << "\n";
            temperature += 5.0;
        }

        // Update turbine
        updateTurbine();

        // Update score with difficulty multiplier
        turns++;
        score += POINTS_PER_TURN * currentDifficulty.scoreMultiplier;
        score += static_cast<int>(power * POINTS_PER_POWER_UNIT * currentDifficulty.scoreMultiplier);
        score += static_cast<int>(electricityOutput / 100.0 * POINTS_PER_MW * currentDifficulty.scoreMultiplier);
    }

    void processRandomEvents() {
        std::uniform_int_distribution<int> eventDist(0, static_cast<int>(currentDifficulty.eventChance) - 1);

        if (eventDist(rng) != 0) return;

        eventsExperienced++;

        // Weight different events
        std::uniform_int_distribution<int> eventTypeDist(0, 99);
        int roll = eventTypeDist(rng);

        if (roll < 20) {
            // Coolant Leak (20%)
            double leak = 10.0 + (rng() % 10);
            coolant = std::max(0.0, coolant - leak);
            std::cout << Color::YELLOW << Color::BOLD
                      << "⚠ COOLANT LEAK: Lost " << std::fixed << std::setprecision(1) << leak << "% coolant!"
                      << Color::RESET << "\n";
        } else if (roll < 35) {
            // Power Surge (15%)
            double surge = 30.0 + (rng() % 40);
            temperature += surge;
            std::cout << Color::RED << Color::BOLD
                      << "⚡ POWER SURGE: Temperature increased by " << std::fixed << std::setprecision(1) << surge << "°C!"
                      << Color::RESET << "\n";
        } else if (roll < 45) {
            // Pump Failure (10%)
            coolant = std::max(0.0, coolant - 15.0);
            temperature += 20.0;
            std::cout << Color::RED << Color::BOLD
                      << "🔧 PUMP FAILURE: Coolant circulation disrupted! -15% coolant, +20°C!"
                      << Color::RESET << "\n";
        } else if (roll < 55) {
            // Sensor Malfunction (10%)
            std::cout << Color::MAGENTA << Color::BOLD
                      << "📡 SENSOR MALFUNCTION: Readings may be inaccurate for the next turn!"
                      << Color::RESET << "\n";
            // Could add visual glitches here
        } else if (roll < 65) {
            // Steam Leak (10%)
            if (turbineOnline) {
                turbineRPM = std::max(0.0, turbineRPM - 500.0);
                std::cout << Color::YELLOW << Color::BOLD
                          << "💨 STEAM LEAK: Turbine efficiency reduced! -500 RPM"
                          << Color::RESET << "\n";
            } else {
                temperature += 15.0;
                std::cout << Color::YELLOW << Color::BOLD
                          << "💨 STEAM LEAK: Steam escaped from pressure release valve! +15°C"
                          << Color::RESET << "\n";
            }
        } else if (roll < 72) {
            // Control Rod Stuck (7%)
            std::cout << Color::RED << Color::BOLD
                      << "🔴 CONTROL ROD STUCK: Manual adjustment required!"
                      << Color::RESET << "\n";
            // Rod position doesn't change next turn
        } else if (roll < 79) {
            // Turbine Trip (7%)
            if (turbineOnline) {
                turbineOnline = false;
                turbineRPM *= 0.5;
                std::cout << Color::RED << Color::BOLD
                          << "⚙ TURBINE TRIP: Emergency turbine shutdown! Use 't' to restart."
                          << Color::RESET << "\n";
            }
        } else if (roll < 87) {
            // Efficiency Boost (8%)
            double bonus = 50.0 + (rng() % 50);
            score += static_cast<int>(bonus);
            std::cout << Color::GREEN << Color::BOLD
                      << "✨ EFFICIENCY BOOST: Optimal conditions! +" << static_cast<int>(bonus) << " bonus points!"
                      << Color::RESET << "\n";
        } else if (roll < 94) {
            // Coolant Bonus (7%)
            double bonus = 10.0 + (rng() % 15);
            coolant = std::min(100.0, coolant + bonus);
            std::cout << Color::GREEN << Color::BOLD
                      << "💧 COOLANT DELIVERY: Emergency reserves delivered! +" << std::fixed << std::setprecision(1) << bonus << "% coolant!"
                      << Color::RESET << "\n";
        } else {
            // Maintenance Crew (6%)
            temperature = std::max(INITIAL_TEMPERATURE, temperature - 30.0);
            std::cout << Color::GREEN << Color::BOLD
                      << "👷 MAINTENANCE CREW: Quick repairs performed! -30°C temperature"
                      << Color::RESET << "\n";
        }
    }

    void checkSafetyLimits() {
        if ((temperature > currentDifficulty.scramTemperature || neutrons > SCRAM_NEUTRONS) && running) {
            std::cout << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "*** AUTO SCRAM! Emergency shutdown! ***"
                      << Color::RESET << "\n";
            controlRods = 1.0;
            neutrons *= 0.05;
            temperature = std::max(0.0, temperature - 200);
            turbineOnline = false;
            running = false;
            scramCount++;
            score = std::max(0, score - SCRAM_PENALTY);
            std::cout << Color::RED << "Score penalty: -" << SCRAM_PENALTY << " points" << Color::RESET << "\n";
        }

        if (temperature > currentDifficulty.meltdownTemperature) {
            std::cout << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "!!! MELTDOWN !!! Core has gone critical. You have failed as reactor operator."
                      << Color::RESET << "\n";
            running = false;
        }
    }

    bool handleScramReset() {
        std::cout << Color::YELLOW << "Type 'reset' to attempt reactor restart, or 'q' to quit: "
                  << Color::RESET;
        std::string input;
        if (!std::getline(std::cin, input)) {
            return false;
        }

        if (input == "reset") {
            std::cout << Color::GREEN << "Reactor restart attempt..." << Color::RESET << "\n";
            running = true;
            temperature = INITIAL_TEMPERATURE;
            controlRods = 1.0;
            return true;
        }
        return false;
    }

    void displayFinalScore() {
        saveHighScore();
        std::cout << "\n" << Color::BOLD << Color::CYAN
                  << "╔══════════════════════════════════════════════╗\n"
                  << "║              FINAL RESULTS                   ║\n"
                  << "╠══════════════════════════════════════════════╣" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Difficulty: "
                  << std::setw(32) << currentDifficulty.name << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Turns Survived: "
                  << std::setw(28) << turns << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "SCRAMs Triggered: "
                  << std::setw(26) << scramCount << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Events Experienced: "
                  << std::setw(24) << eventsExperienced << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Total Electricity (MW·h): "
                  << std::setw(18) << static_cast<int>(totalElectricityGenerated) << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Final Score: "
                  << std::setw(31) << score << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "High Score (" << currentDifficulty.name << "): "
                  << std::setw(27 - static_cast<int>(currentDifficulty.name.length())) << highScore << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::BOLD << Color::CYAN
                  << "╚══════════════════════════════════════════════╝" << Color::RESET << "\n";

        if (score >= highScore && score > 0) {
            std::cout << Color::BOLD << Color::GREEN << "🏆 NEW HIGH SCORE! 🏆" << Color::RESET << "\n";
        }
    }

public:
    void run() {
        std::cout << Color::BOLD << Color::CYAN
                  << "╔══════════════════════════════════════════════════════╗\n"
                  << "║      C++ NUCLEAR REACTOR SIMULATOR v0.5              ║\n"
                  << "║      Difficulty: " << std::left << std::setw(38) << currentDifficulty.name << "║\n"
                  << "╚══════════════════════════════════════════════════════╝\n"
                  << Color::RESET;
        std::cout << Color::DIM << "Commands: 0-100, 'r' (refill), 't' (turbine), 'h' (help), 'q' (quit)\n"
                  << Color::RESET;

        while (running) {
            displayDashboard();
            displayScore();
            displayStatus();

            std::cout << Color::GREEN << "\nControl rods (0-100%, current "
                      << static_cast<int>(controlRods * 100)
                      << "%): " << Color::RESET;

            std::string input;
            if (!std::getline(std::cin, input)) {
                break;
            }

            if (input == "q") break;

            if (input == "h" || input == "help") {
                displayHelp();
                continue;
            }

            if (input == "r") {
                coolant = INITIAL_COOLANT;
                score = std::max(0, score - REFILL_PENALTY);
                std::cout << Color::GREEN << "Coolant refilled! " << Color::RESET
                          << Color::RED << "(-" << REFILL_PENALTY << " points)" << Color::RESET << "\n";
                continue;
            }

            if (input == "t") {
                turbineOnline = !turbineOnline;
                if (turbineOnline) {
                    std::cout << Color::GREEN << "Turbine starting up..." << Color::RESET << "\n";
                } else {
                    std::cout << Color::YELLOW << "Turbine shutting down..." << Color::RESET << "\n";
                }
                continue;
            }

            controlRods = parseControlRodInput(input);

            updatePhysics();
            processRandomEvents();
            checkSafetyLimits();

            if (!running && !handleScramReset()) {
                break;
            }
        }

        displayFinalScore();
        std::cout << "\n" << Color::MAGENTA << Color::BOLD
                  << "Reactor simulation ended. Stay radioactive! ☢️\n"
                  << Color::RESET;
    }
};

Difficulty selectDifficulty() {
    std::cout << Color::BOLD << Color::CYAN
              << "\n╔══════════════════════════════════════╗\n"
              << "║        SELECT DIFFICULTY             ║\n"
              << "╠══════════════════════════════════════╣\n" << Color::RESET;
    std::cout << Color::CYAN << "║ " << Color::GREEN << "1. Easy     " << Color::RESET
              << "- Forgiving parameters      " << Color::CYAN << "║" << Color::RESET << "\n";
    std::cout << Color::CYAN << "║ " << Color::YELLOW << "2. Normal   " << Color::RESET
              << "- Standard experience       " << Color::CYAN << "║" << Color::RESET << "\n";
    std::cout << Color::CYAN << "║ " << Color::RED << "3. Hard     " << Color::RESET
              << "- For experienced operators " << Color::CYAN << "║" << Color::RESET << "\n";
    std::cout << Color::CYAN << "║ " << Color::MAGENTA << "4. Nightmare" << Color::RESET
              << "- Only the brave survive    " << Color::CYAN << "║" << Color::RESET << "\n";
    std::cout << Color::BOLD << Color::CYAN
              << "╚══════════════════════════════════════╝\n" << Color::RESET;
    std::cout << Color::DIM << "Enter choice (1-4): " << Color::RESET;

    std::string input;
    if (!std::getline(std::cin, input)) {
        return Difficulty::NORMAL;
    }

    try {
        int choice = std::stoi(input);
        switch (choice) {
            case 1: return Difficulty::EASY;
            case 2: return Difficulty::NORMAL;
            case 3: return Difficulty::HARD;
            case 4: return Difficulty::NIGHTMARE;
            default: return Difficulty::NORMAL;
        }
    } catch (const std::exception&) {
        return Difficulty::NORMAL;
    }
}

int main() {
    Difficulty diff = selectDifficulty();
    ReactorSimulator simulator(diff);
    simulator.run();
    return 0;
}
