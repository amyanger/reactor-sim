#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <random>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>
#include <sstream>
#include <cmath>

// Sound effects using terminal bell
namespace Sound {
    inline void beep() { std::cout << '\a' << std::flush; }
    inline void alert() { for (int i = 0; i < 3; ++i) { std::cout << '\a' << std::flush; } }
}

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
    const std::string BG_MAGENTA = "\033[45m";
}

enum class Difficulty {
    EASY,
    NORMAL,
    HARD,
    NIGHTMARE
};

enum class Weather {
    CLEAR,
    CLOUDY,
    RAIN,
    STORM,
    HEATWAVE,
    COLD_SNAP
};

struct WeatherInfo {
    std::string name;
    std::string icon;
    double coolingModifier;    // Affects natural cooling rate
    double eventModifier;      // Affects random event chance
    std::string description;
};

// Achievement definitions
enum class Achievement {
    FIRST_STEPS,        // Complete 10 turns
    SURVIVOR,           // Complete 50 turns
    VETERAN,            // Complete 100 turns
    POWER_PLAYER,       // Generate 100 MW·h
    ENERGY_BARON,       // Generate 500 MW·h
    COOL_UNDER_PRESSURE,// Recover from 3 SCRAMs in one game
    PERFECT_RUN,        // Complete 50 turns without SCRAM
    XENON_MASTER,       // Successfully manage xenon poisoning 5 times
    NIGHTMARE_SURVIVOR, // Complete 25 turns on Nightmare
    ELECTRICIAN,        // Keep turbine at max output for 10 turns
    ACHIEVEMENT_COUNT
};

struct AchievementInfo {
    std::string name;
    std::string description;
    std::string icon;
};

const AchievementInfo ACHIEVEMENT_INFO[] = {
    {"First Steps", "Complete 10 turns", "🎯"},
    {"Survivor", "Complete 50 turns", "🏅"},
    {"Veteran Operator", "Complete 100 turns", "🎖"},
    {"Power Player", "Generate 100 MW·h", "⚡"},
    {"Energy Baron", "Generate 500 MW·h", "💎"},
    {"Cool Under Pressure", "Recover from 3 SCRAMs", "❄️"},
    {"Perfect Run", "50 turns without SCRAM", "✨"},
    {"Xenon Master", "Handle xenon poisoning 5 times", "☢️"},
    {"Nightmare Survivor", "25 turns on Nightmare", "👻"},
    {"Electrician", "Max turbine output for 10 turns", "🔌"}
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
    double xenonBuildupRate;
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

    // Steam pressure constants
    static constexpr double MAX_STEAM_PRESSURE = 150.0;  // bar
    static constexpr double CRITICAL_PRESSURE = 130.0;   // bar - warning threshold
    static constexpr double RUPTURE_PRESSURE = 160.0;    // bar - pipe rupture

    // Emergency cooling system
    static constexpr double ECCS_COOLANT_BOOST = 50.0;
    static constexpr double ECCS_TEMP_REDUCTION = 100.0;
    static constexpr int ECCS_COOLDOWN = 10;

    // Diesel generator constants
    static constexpr double DIESEL_FUEL_CAPACITY = 100.0;
    static constexpr double DIESEL_FUEL_CONSUMPTION = 2.0;  // per turn when running
    static constexpr double DIESEL_POWER_OUTPUT = 50.0;     // MW equivalent

    // Radiation constants (in mSv/h)
    static constexpr double BACKGROUND_RADIATION = 0.1;
    static constexpr double MAX_SAFE_RADIATION = 20.0;      // Normal operations
    static constexpr double WARNING_RADIATION = 100.0;       // Warning level
    static constexpr double DANGER_RADIATION = 500.0;        // Evacuation level
    static constexpr double LETHAL_RADIATION = 2000.0;       // Fatal dose

    // Xenon poisoning
    static constexpr double MAX_XENON = 100.0;
    static constexpr double XENON_DECAY_RATE = 2.0;

    // Scoring constants
    static constexpr int POINTS_PER_TURN = 10;
    static constexpr int POINTS_PER_POWER_UNIT = 1;
    static constexpr int POINTS_PER_MW = 5;
    static constexpr int SCRAM_PENALTY = 500;
    static constexpr int REFILL_PENALTY = 50;
    static constexpr int ECCS_PENALTY = 100;

    // Difficulty settings
    DifficultySettings currentDifficulty;

    double neutrons;
    double controlRods;
    double temperature;
    double coolant;
    double power;
    double fuel;
    bool running;

    // Xenon poisoning system
    double xenonLevel;
    int xenonHandledCount;

    // Turbine system
    double turbineRPM;
    double steamPressure;
    double electricityOutput;
    double totalElectricityGenerated;
    bool turbineOnline;
    int maxTurbineTurns;
    bool pressureReliefOpen;
    int pressureWarnings;

    // Emergency Cooling System
    bool eccsAvailable;
    int eccsCooldownTimer;

    // Diesel Generator System
    double dieselFuel;
    bool dieselRunning;
    bool dieselAutoStart;
    int dieselRuntime;

    // Radiation monitoring
    double radiationLevel;
    double totalRadiationExposure;
    int radiationAlarms;

    // Weather system
    Weather currentWeather;
    int weatherDuration;
    int weatherChangeCooldown;

    // Power grid demand system
    double gridDemand;           // Current demand in MW
    double demandSatisfaction;   // How well we're meeting demand (0-100%)
    int demandBonus;             // Bonus points for meeting demand
    int demandPenalty;           // Penalty for not meeting demand

    static WeatherInfo getWeatherInfo(Weather w) {
        switch (w) {
            case Weather::CLEAR:    return {"Clear", "☀️", 1.0, 1.0, "Optimal conditions"};
            case Weather::CLOUDY:   return {"Cloudy", "☁️", 1.1, 1.0, "Slightly improved cooling"};
            case Weather::RAIN:     return {"Rain", "🌧️", 1.3, 0.9, "Enhanced cooling, fewer events"};
            case Weather::STORM:    return {"Storm", "⛈️", 1.2, 1.5, "Risk of lightning damage"};
            case Weather::HEATWAVE: return {"Heatwave", "🔥", 0.6, 1.2, "Reduced cooling efficiency"};
            case Weather::COLD_SNAP:return {"Cold Snap", "❄️", 1.5, 0.8, "Excellent cooling"};
            default:                return {"Clear", "☀️", 1.0, 1.0, "Optimal conditions"};
        }
    }

    // Scoring system
    int score;
    int turns;
    int scramCount;
    int highScore;
    int eventsExperienced;
    int turnsWithoutScram;
    int scramRecoveries;

    // Statistics tracking
    double peakTemperature;
    double peakPower;
    double peakElectricity;
    double totalPowerGenerated;
    double averageTemperature;
    double temperatureSum;
    int criticalEvents;
    double lowestCoolant;
    double highestXenon;

    // Operator event log
    struct LogEntry {
        int turn;
        std::string type;  // ACTION, EVENT, WARNING, CRITICAL
        std::string message;
    };
    std::vector<LogEntry> operatorLog;
    static constexpr int MAX_LOG_ENTRIES = 100;

    // Achievements
    std::set<Achievement> unlockedAchievements;
    std::set<Achievement> sessionAchievements;

    // Modern random number generation
    std::mt19937 rng;

    static constexpr const char* HIGH_SCORE_FILE = ".reactor_highscore";
    static constexpr const char* ACHIEVEMENTS_FILE = ".reactor_achievements";
    static constexpr const char* SAVE_FILE = ".reactor_save";

    bool soundEnabled;
    bool paused;

    static DifficultySettings getDifficultySettings(Difficulty diff) {
        switch (diff) {
            case Difficulty::EASY:
                return {"Easy", 0.05, 0.15, 15.0, 1200.0, 2500.0, 1, 0.95, 0.5};
            case Difficulty::NORMAL:
                return {"Normal", 0.1, 0.3, 10.0, 1000.0, 2000.0, 2, 0.90, 1.0};
            case Difficulty::HARD:
                return {"Hard", 0.15, 0.5, 7.0, 800.0, 1500.0, 3, 0.85, 1.5};
            case Difficulty::NIGHTMARE:
                return {"Nightmare", 0.2, 0.7, 5.0, 600.0, 1200.0, 5, 0.75, 2.0};
            default:
                return {"Normal", 0.1, 0.3, 10.0, 1000.0, 2000.0, 2, 0.90, 1.0};
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
        xenonLevel(0.0),
        xenonHandledCount(0),
        turbineRPM(0.0),
        steamPressure(0.0),
        electricityOutput(0.0),
        totalElectricityGenerated(0.0),
        turbineOnline(false),
        maxTurbineTurns(0),
        pressureReliefOpen(false),
        pressureWarnings(0),
        eccsAvailable(true),
        eccsCooldownTimer(0),
        dieselFuel(DIESEL_FUEL_CAPACITY),
        dieselRunning(false),
        dieselAutoStart(true),
        dieselRuntime(0),
        radiationLevel(BACKGROUND_RADIATION),
        totalRadiationExposure(0.0),
        radiationAlarms(0),
        currentWeather(Weather::CLEAR),
        weatherDuration(10),
        weatherChangeCooldown(0),
        gridDemand(500.0),
        demandSatisfaction(0.0),
        demandBonus(0),
        demandPenalty(0),
        score(0),
        turns(0),
        scramCount(0),
        highScore(0),
        eventsExperienced(0),
        turnsWithoutScram(0),
        scramRecoveries(0),
        peakTemperature(INITIAL_TEMPERATURE),
        peakPower(0.0),
        peakElectricity(0.0),
        totalPowerGenerated(0.0),
        averageTemperature(0.0),
        temperatureSum(0.0),
        criticalEvents(0),
        lowestCoolant(INITIAL_COOLANT),
        highestXenon(0.0),
        rng(std::chrono::steady_clock::now().time_since_epoch().count()),
        soundEnabled(true),
        paused(false) {
        loadHighScore();
        loadAchievements();
    }

    void playSound() {
        if (soundEnabled) Sound::beep();
    }

    void playAlert() {
        if (soundEnabled) Sound::alert();
    }

    bool saveGame() {
        std::ofstream file(SAVE_FILE);
        if (!file.is_open()) return false;

        file << currentDifficulty.name << "\n";
        file << neutrons << " " << controlRods << " " << temperature << "\n";
        file << coolant << " " << power << " " << fuel << "\n";
        file << xenonLevel << " " << xenonHandledCount << "\n";
        file << turbineRPM << " " << steamPressure << " " << electricityOutput << "\n";
        file << totalElectricityGenerated << " " << turbineOnline << " " << maxTurbineTurns << "\n";
        file << eccsAvailable << " " << eccsCooldownTimer << "\n";
        file << score << " " << turns << " " << scramCount << "\n";
        file << eventsExperienced << " " << turnsWithoutScram << " " << scramRecoveries << "\n";

        file.close();
        return true;
    }

    bool loadGame() {
        std::ifstream file(SAVE_FILE);
        if (!file.is_open()) return false;

        std::string diffName;
        file >> diffName;

        file >> neutrons >> controlRods >> temperature;
        file >> coolant >> power >> fuel;
        file >> xenonLevel >> xenonHandledCount;
        file >> turbineRPM >> steamPressure >> electricityOutput;
        file >> totalElectricityGenerated >> turbineOnline >> maxTurbineTurns;
        file >> eccsAvailable >> eccsCooldownTimer;
        file >> score >> turns >> scramCount;
        file >> eventsExperienced >> turnsWithoutScram >> scramRecoveries;

        file.close();
        running = true;
        return true;
    }

    void deleteSave() {
        std::remove(SAVE_FILE);
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

    void loadAchievements() {
        std::ifstream file(ACHIEVEMENTS_FILE);
        if (file.is_open()) {
            int achievementId;
            while (file >> achievementId) {
                if (achievementId >= 0 && achievementId < static_cast<int>(Achievement::ACHIEVEMENT_COUNT)) {
                    unlockedAchievements.insert(static_cast<Achievement>(achievementId));
                }
            }
            file.close();
        }
    }

    void saveAchievements() {
        std::ofstream file(ACHIEVEMENTS_FILE);
        if (file.is_open()) {
            for (const auto& ach : unlockedAchievements) {
                file << static_cast<int>(ach) << "\n";
            }
            file.close();
        }
    }

    void unlockAchievement(Achievement ach) {
        if (unlockedAchievements.find(ach) == unlockedAchievements.end()) {
            unlockedAchievements.insert(ach);
            sessionAchievements.insert(ach);
            const auto& info = ACHIEVEMENT_INFO[static_cast<int>(ach)];
            std::cout << "\n" << Color::BG_MAGENTA << Color::WHITE << Color::BOLD
                      << " " << info.icon << " ACHIEVEMENT UNLOCKED: " << info.name << "! "
                      << Color::RESET << "\n";
            std::cout << Color::MAGENTA << "   " << info.description << Color::RESET << "\n\n";
            saveAchievements();
        }
    }

    void checkAchievements() {
        // Turn-based achievements
        if (turns >= 10) unlockAchievement(Achievement::FIRST_STEPS);
        if (turns >= 50) unlockAchievement(Achievement::SURVIVOR);
        if (turns >= 100) unlockAchievement(Achievement::VETERAN);

        // Power generation achievements
        if (totalElectricityGenerated >= 100) unlockAchievement(Achievement::POWER_PLAYER);
        if (totalElectricityGenerated >= 500) unlockAchievement(Achievement::ENERGY_BARON);

        // SCRAM-related achievements
        if (scramRecoveries >= 3) unlockAchievement(Achievement::COOL_UNDER_PRESSURE);
        if (turnsWithoutScram >= 50) unlockAchievement(Achievement::PERFECT_RUN);

        // Xenon achievement
        if (xenonHandledCount >= 5) unlockAchievement(Achievement::XENON_MASTER);

        // Nightmare achievement
        if (currentDifficulty.name == "Nightmare" && turns >= 25) {
            unlockAchievement(Achievement::NIGHTMARE_SURVIVOR);
        }

        // Electrician achievement
        if (maxTurbineTurns >= 10) unlockAchievement(Achievement::ELECTRICIAN);
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
        std::cout << Color::CYAN << "║" << Color::RESET << "   r      : Refill coolant (-" << REFILL_PENALTY << " points)"
                  << std::setw(22) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   t      : Toggle turbine online/offline"
                  << std::setw(18) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   e      : Activate ECCS (-" << ECCS_PENALTY << " pts, " << ECCS_COOLDOWN << " turn CD)"
                  << std::setw(12) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   d      : Toggle diesel generator"
                  << std::setw(23) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   df     : Refill diesel fuel"
                  << std::setw(28) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   da     : Toggle diesel auto-start"
                  << std::setw(22) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   a      : View achievements"
                  << std::setw(29) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   stats  : View session statistics"
                  << std::setw(23) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   log    : View operator event log"
                  << std::setw(23) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   p      : Pause/Resume simulation"
                  << std::setw(23) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   s/save : Save game"
                  << std::setw(37) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   l/load : Load saved game"
                  << std::setw(31) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   sound  : Toggle sound effects"
                  << std::setw(25) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   h      : Display this help screen"
                  << std::setw(22) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   q      : Quit the simulation"
                  << std::setw(27) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " XENON POISONING:" << Color::RESET
                  << std::setw(42) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Xenon-135 builds up during operation"
                  << std::setw(17) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • High xenon absorbs neutrons, reducing power"
                  << std::setw(10) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Reduce power gradually to manage xenon"
                  << std::setw(15) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " EMERGENCY CORE COOLING SYSTEM (ECCS):" << Color::RESET
                  << std::setw(20) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Instantly adds " << ECCS_COOLANT_BOOST << "% coolant"
                  << std::setw(27) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Reduces temperature by " << ECCS_TEMP_REDUCTION << "°C"
                  << std::setw(21) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • " << ECCS_COOLDOWN << " turn cooldown after use"
                  << std::setw(28) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " SAFETY LIMITS (" << currentDifficulty.name << " mode):" << Color::RESET
                  << std::setw(34 - static_cast<int>(currentDifficulty.name.length())) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • SCRAM at: " << currentDifficulty.scramTemperature << "°C or "
                  << SCRAM_NEUTRONS << " neutrons" << std::setw(15) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • MELTDOWN at: " << currentDifficulty.meltdownTemperature << "°C"
                  << std::setw(31) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::BOLD << Color::CYAN
                  << "╚═══════════════════════════════════════════════════════════╝" << Color::RESET << "\n\n";

        std::cout << Color::DIM << "Press Enter to continue..." << Color::RESET;
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    void displayAchievements() const {
        std::cout << "\n" << Color::BOLD << Color::MAGENTA
                  << "╔═══════════════════════════════════════════════════════════╗\n"
                  << "║                    ACHIEVEMENTS                           ║\n"
                  << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        for (int i = 0; i < static_cast<int>(Achievement::ACHIEVEMENT_COUNT); ++i) {
            Achievement ach = static_cast<Achievement>(i);
            const auto& info = ACHIEVEMENT_INFO[i];
            bool unlocked = unlockedAchievements.find(ach) != unlockedAchievements.end();

            std::cout << Color::MAGENTA << "║ " << Color::RESET;
            if (unlocked) {
                std::cout << Color::GREEN << info.icon << " " << std::left << std::setw(20) << info.name
                          << Color::RESET << Color::DIM << info.description << Color::RESET;
            } else {
                std::cout << Color::DIM << "🔒 " << std::left << std::setw(20) << "???"
                          << "???" << Color::RESET;
            }
            std::cout << std::setw(18 - static_cast<int>(info.description.length())) << "" << Color::MAGENTA << "║" << Color::RESET << "\n";
        }

        std::cout << Color::BOLD << Color::MAGENTA
                  << "╠═══════════════════════════════════════════════════════════╣\n"
                  << "║ Unlocked: " << unlockedAchievements.size() << "/" << static_cast<int>(Achievement::ACHIEVEMENT_COUNT)
                  << std::setw(45) << "" << "║\n"
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

    void printBar(const std::string& label, double value, double max, int width = 18, bool inverse = false) const {
        int bars = static_cast<int>((value / max) * width);
        bars = std::max(0, std::min(width, bars));

        std::string color = getBarColor(value, max, inverse);

        std::cout << std::left << std::setw(10) << label << "[";
        std::cout << color;
        for (int i = 0; i < width; ++i) {
            std::cout << (i < bars ? "█" : " ");
        }
        std::cout << Color::RESET << "] ";

        std::cout << color << std::fixed << std::setprecision(1) << value << Color::RESET;
        if (label == "Temp") std::cout << "°C";
        else if (label == "Coolant" || label == "Fuel" || label == "Xenon" || label == "Diesel") std::cout << "%";
        else if (label == "Turbine") std::cout << " RPM";
        else if (label == "Pressure") std::cout << " bar";
        else if (label == "Radiation") std::cout << " mSv/h";
        else if (label == "Output") std::cout << " MW";
        std::cout << "\n";
    }

    void displayDashboard() const {
        WeatherInfo weatherInfo = getWeatherInfo(currentWeather);
        std::cout << "\n" << Color::BOLD << Color::CYAN << "╔════════════════════════════════════════════════════╗" << Color::RESET << "\n";
        std::string pauseIndicator = paused ? (Color::BG_YELLOW + Color::WHITE + " PAUSED " + Color::RESET + Color::CYAN) : "";
        std::cout << Color::BOLD << Color::CYAN << "║   REACTOR DASHBOARD [" << currentDifficulty.name << "] "
                  << weatherInfo.icon << " " << weatherInfo.name
                  << std::setw(15 - static_cast<int>(currentDifficulty.name.length()) - static_cast<int>(weatherInfo.name.length()) - (paused ? 8 : 0)) << "" << pauseIndicator << "║" << Color::RESET << "\n";
        std::cout << Color::BOLD << Color::CYAN << "╠════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        // Core section
        std::cout << Color::CYAN << "║ " << Color::BOLD << "REACTOR CORE" << Color::RESET;
        std::string eccsStatus = eccsAvailable ? (Color::GREEN + "ECCS READY" + Color::RESET) : (Color::RED + "ECCS CD:" + std::to_string(eccsCooldownTimer) + Color::RESET);
        std::cout << std::setw(26) << "" << eccsStatus << Color::CYAN << "  ║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Temp", temperature, currentDifficulty.meltdownTemperature, 16, false);
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Coolant", coolant, 100.0, 16, true);
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Fuel", fuel, 100.0, 16, true);
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Xenon", xenonLevel, MAX_XENON, 16, false);
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Radiation", radiationLevel, DANGER_RADIATION, 16, false);

        // Turbine section
        std::cout << Color::CYAN << "╠════════════════════════════════════════════════════╣" << Color::RESET << "\n";
        std::string turbineStatus = turbineOnline ? (Color::GREEN + "ONLINE" + Color::RESET) : (Color::RED + "OFFLINE" + Color::RESET);
        std::cout << Color::CYAN << "║ " << Color::BOLD << "TURBINE HALL" << Color::RESET
                  << " [" << turbineStatus << "]"
                  << std::setw(27) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Turbine", turbineRPM, MAX_TURBINE_RPM, 16, false);
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Pressure", steamPressure, MAX_STEAM_PRESSURE, 16, false);
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Output", electricityOutput, 1000.0, 16, false);

        // Diesel generator section
        std::cout << Color::CYAN << "╠════════════════════════════════════════════════════╣" << Color::RESET << "\n";
        std::string dieselStatus = dieselRunning ? (Color::GREEN + "RUNNING" + Color::RESET) : (Color::DIM + "STANDBY" + Color::RESET);
        std::string autoStatus = dieselAutoStart ? "AUTO" : "MANUAL";
        std::cout << Color::CYAN << "║ " << Color::BOLD << "DIESEL GENERATOR" << Color::RESET
                  << " [" << dieselStatus << "] [" << autoStatus << "]"
                  << std::setw(15) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET;
        printBar("Diesel", dieselFuel, DIESEL_FUEL_CAPACITY, 16, true);

        std::cout << Color::BOLD << Color::CYAN << "╚════════════════════════════════════════════════════╝" << Color::RESET << "\n";
    }

    void displayScore() const {
        std::cout << Color::YELLOW << "╭───────────────────────────────────────────────────────────╮" << Color::RESET << "\n";
        std::cout << Color::YELLOW << "│ " << Color::BOLD << "SCORE:" << Color::RESET
                  << std::setw(7) << score
                  << Color::YELLOW << " │ " << Color::BOLD << "HIGH:" << Color::RESET
                  << std::setw(7) << highScore
                  << Color::YELLOW << " │ " << Color::BOLD << "TURN:" << Color::RESET
                  << std::setw(4) << turns
                  << Color::YELLOW << " │ " << Color::BOLD << "MW·h:" << Color::RESET
                  << std::setw(6) << static_cast<int>(totalElectricityGenerated)
                  << Color::YELLOW << " │" << Color::RESET << "\n";

        // Grid demand line
        std::string demandColor = demandSatisfaction >= 90 ? Color::GREEN :
                                 (demandSatisfaction >= 60 ? Color::YELLOW : Color::RED);
        std::cout << Color::YELLOW << "│ " << Color::BOLD << "GRID DEMAND:" << Color::RESET
                  << std::setw(4) << static_cast<int>(gridDemand) << " MW"
                  << Color::YELLOW << " │ " << Color::BOLD << "SATISFACTION:" << Color::RESET
                  << demandColor << std::setw(3) << static_cast<int>(demandSatisfaction) << "%" << Color::RESET
                  << Color::YELLOW << " │ " << Color::BOLD << "BONUS:" << Color::RESET
                  << std::setw(5) << demandBonus
                  << Color::YELLOW << " │" << Color::RESET << "\n";
        std::cout << Color::YELLOW << "╰───────────────────────────────────────────────────────────╯" << Color::RESET << "\n";
    }

    void displayStatus() const {
        std::cout << Color::DIM << "Neutrons: " << Color::RESET
                  << std::fixed << std::setprecision(0) << neutrons
                  << Color::DIM << " | Rods: " << Color::RESET
                  << static_cast<int>(controlRods * 100) << "%"
                  << Color::DIM << " | Power: " << Color::RESET << std::setprecision(1) << power
                  << Color::DIM << " | Events: " << Color::RESET << eventsExperienced
                  << Color::DIM << " | 🏆 " << Color::RESET << unlockedAchievements.size() << "/" << static_cast<int>(Achievement::ACHIEVEMENT_COUNT) << "\n";
    }

    double parseControlRodInput(const std::string& input) const {
        try {
            double value = std::stod(input) / 100.0;
            return std::max(0.0, std::min(1.0, value));
        } catch (const std::exception&) {
            return controlRods;
        }
    }

    void updateXenon() {
        // Xenon builds up based on power level
        double powerFactor = power / 100.0;
        xenonLevel += powerFactor * currentDifficulty.xenonBuildupRate;

        // Xenon decays naturally
        xenonLevel = std::max(0.0, xenonLevel - XENON_DECAY_RATE);

        // Cap xenon level
        xenonLevel = std::min(MAX_XENON, xenonLevel);

        // High xenon warning
        if (xenonLevel > 70.0) {
            std::cout << Color::MAGENTA << Color::BOLD
                      << "☢ XENON POISONING: High Xe-135 levels affecting reactivity!"
                      << Color::RESET << "\n";
        }

        // Track successful xenon management
        if (xenonLevel > 50.0 && xenonLevel < 80.0) {
            xenonHandledCount++;
        }
    }

    void updateTurbine() {
        // Calculate steam pressure based on temperature (more realistic model)
        if (temperature > MIN_TURBINE_TEMP) {
            double targetPressure = ((temperature - MIN_TURBINE_TEMP) / (currentDifficulty.meltdownTemperature - MIN_TURBINE_TEMP)) * MAX_STEAM_PRESSURE;
            steamPressure = steamPressure * 0.7 + targetPressure * 0.3;  // Gradual pressure change
        } else {
            steamPressure = std::max(0.0, steamPressure - 5.0);
        }

        // Pressure relief valve logic
        if (steamPressure > CRITICAL_PRESSURE && !pressureReliefOpen) {
            pressureReliefOpen = true;
            std::cout << Color::YELLOW << Color::BOLD
                      << "🔧 PRESSURE RELIEF VALVE opened at " << std::fixed << std::setprecision(1) << steamPressure << " bar!"
                      << Color::RESET << "\n";
            addLogEntry("WARNING", "Pressure relief valve opened");
            pressureWarnings++;
        }

        if (pressureReliefOpen) {
            steamPressure = std::max(0.0, steamPressure - 10.0);
            if (steamPressure < CRITICAL_PRESSURE * 0.8) {
                pressureReliefOpen = false;
                std::cout << Color::GREEN << "✓ Pressure relief valve closed. Pressure stabilized." << Color::RESET << "\n";
                addLogEntry("EVENT", "Pressure relief valve closed");
            }
        }

        // Check for pipe rupture
        if (steamPressure > RUPTURE_PRESSURE) {
            std::cout << Color::BG_RED << Color::WHITE << Color::BOLD
                      << " 💥 STEAM PIPE RUPTURE! Critical pressure exceeded! "
                      << Color::RESET << "\n";
            addLogEntry("CRITICAL", "Steam pipe rupture - pressure exceeded " + std::to_string(static_cast<int>(RUPTURE_PRESSURE)) + " bar");
            coolant = std::max(0.0, coolant - 25.0);
            temperature += 50.0;
            turbineOnline = false;
            steamPressure = 50.0;
            playAlert();
        }

        if (!turbineOnline) {
            turbineRPM = std::max(0.0, turbineRPM - 100.0);
            electricityOutput = 0.0;
            return;
        }

        if (temperature < MIN_TURBINE_TEMP) {
            std::cout << Color::YELLOW << "⚠ Turbine cannot operate below " << MIN_TURBINE_TEMP << "°C!" << Color::RESET << "\n";
            turbineRPM = std::max(0.0, turbineRPM - 50.0);
            electricityOutput = 0.0;
            return;
        }

        // Pressure warning
        if (steamPressure > CRITICAL_PRESSURE * 0.9) {
            std::cout << Color::RED << "⚠ HIGH STEAM PRESSURE: " << std::fixed << std::setprecision(1) << steamPressure
                      << " bar (max " << MAX_STEAM_PRESSURE << ")" << Color::RESET << "\n";
        }

        double pressureRatio = std::min(1.0, steamPressure / MAX_STEAM_PRESSURE);
        double targetRPM = pressureRatio * MAX_TURBINE_RPM;

        if (turbineRPM < targetRPM) {
            turbineRPM = std::min(targetRPM, turbineRPM + 200.0);
        } else {
            turbineRPM = std::max(targetRPM, turbineRPM - 200.0);
        }

        double tempEfficiency = 1.0 - std::abs(temperature - OPTIMAL_STEAM_TEMP) / 1000.0;
        tempEfficiency = std::max(0.3, std::min(1.0, tempEfficiency));

        electricityOutput = (turbineRPM / MAX_TURBINE_RPM) * 1000.0 * tempEfficiency * currentDifficulty.turbineEfficiency;
        totalElectricityGenerated += electricityOutput / 60.0;

        // Track max turbine output for achievement
        if (electricityOutput > 900.0) {
            maxTurbineTurns++;
        } else {
            maxTurbineTurns = 0;
        }
    }

    void updateECCS() {
        if (!eccsAvailable && eccsCooldownTimer > 0) {
            eccsCooldownTimer--;
            if (eccsCooldownTimer == 0) {
                eccsAvailable = true;
                std::cout << Color::GREEN << "✓ ECCS recharged and ready!" << Color::RESET << "\n";
            }
        }
    }

    void updateDieselGenerator() {
        // Auto-start logic - starts when turbine output drops below 50 MW
        if (dieselAutoStart && !dieselRunning && electricityOutput < 50.0 && dieselFuel > 0) {
            dieselRunning = true;
            std::cout << Color::YELLOW << Color::BOLD
                      << "🔌 DIESEL GENERATOR auto-started! Low power detected."
                      << Color::RESET << "\n";
            addLogEntry("EVENT", "Diesel generator auto-started");
        }

        if (dieselRunning) {
            if (dieselFuel > 0) {
                dieselFuel = std::max(0.0, dieselFuel - DIESEL_FUEL_CONSUMPTION);
                dieselRuntime++;

                // Diesel helps maintain basic cooling even during low power
                if (temperature > INITIAL_TEMPERATURE) {
                    temperature -= 2.0;
                }

                // Low fuel warning
                if (dieselFuel < 20.0 && dieselFuel > 0) {
                    std::cout << Color::YELLOW << "⚠ Diesel fuel low: "
                              << std::fixed << std::setprecision(1) << dieselFuel << "%" << Color::RESET << "\n";
                }
            } else {
                dieselRunning = false;
                std::cout << Color::RED << Color::BOLD
                          << "⚠ DIESEL GENERATOR stopped - OUT OF FUEL!"
                          << Color::RESET << "\n";
                addLogEntry("WARNING", "Diesel generator stopped - fuel depleted");
            }
        }
    }

    void toggleDieselGenerator() {
        if (!dieselRunning && dieselFuel <= 0) {
            std::cout << Color::RED << "✗ Cannot start diesel generator - no fuel!" << Color::RESET << "\n";
            return;
        }

        dieselRunning = !dieselRunning;
        if (dieselRunning) {
            std::cout << Color::GREEN << "🔌 Diesel generator started manually." << Color::RESET << "\n";
            addLogEntry("ACTION", "Diesel generator started manually");
        } else {
            std::cout << Color::YELLOW << "🔌 Diesel generator stopped." << Color::RESET << "\n";
            addLogEntry("ACTION", "Diesel generator stopped");
        }
    }

    void refillDiesel() {
        if (dieselFuel >= DIESEL_FUEL_CAPACITY) {
            std::cout << Color::YELLOW << "Diesel tank already full." << Color::RESET << "\n";
            return;
        }
        dieselFuel = DIESEL_FUEL_CAPACITY;
        std::cout << Color::GREEN << "⛽ Diesel tank refilled!" << Color::RESET << "\n";
        addLogEntry("ACTION", "Diesel fuel tank refilled");
    }

    void updateRadiation() {
        // Base radiation from power level
        double powerRadiation = (power / 100.0) * 5.0;

        // Additional radiation from high temperature (containment stress)
        double tempFactor = 0.0;
        if (temperature > currentDifficulty.scramTemperature * 0.8) {
            tempFactor = ((temperature - currentDifficulty.scramTemperature * 0.8) /
                         (currentDifficulty.meltdownTemperature - currentDifficulty.scramTemperature * 0.8)) * 50.0;
        }

        // Additional radiation from low coolant (exposed fuel)
        double coolantFactor = 0.0;
        if (coolant < 30.0) {
            coolantFactor = ((30.0 - coolant) / 30.0) * 100.0;
        }

        // Calculate target radiation
        double targetRadiation = BACKGROUND_RADIATION + powerRadiation + tempFactor + coolantFactor;

        // Smooth transition
        radiationLevel = radiationLevel * 0.7 + targetRadiation * 0.3;

        // Track total exposure
        totalRadiationExposure += radiationLevel / 60.0;  // per turn

        // Radiation warnings
        if (radiationLevel > DANGER_RADIATION) {
            std::cout << Color::BG_RED << Color::WHITE << Color::BOLD
                      << " ☢ RADIATION CRITICAL: " << std::fixed << std::setprecision(1) << radiationLevel << " mSv/h - EVACUATE! "
                      << Color::RESET << "\n";
            playAlert();
            radiationAlarms++;
            addLogEntry("CRITICAL", "Radiation level critical - evacuation recommended");
        } else if (radiationLevel > WARNING_RADIATION) {
            std::cout << Color::RED << Color::BOLD
                      << "☢ HIGH RADIATION: " << std::fixed << std::setprecision(1) << radiationLevel << " mSv/h"
                      << Color::RESET << "\n";
            if (radiationAlarms % 5 == 0) {  // Don't spam
                addLogEntry("WARNING", "Elevated radiation levels detected");
            }
            radiationAlarms++;
        } else if (radiationLevel > MAX_SAFE_RADIATION) {
            std::cout << Color::YELLOW << "⚠ Elevated radiation: "
                      << std::fixed << std::setprecision(1) << radiationLevel << " mSv/h" << Color::RESET << "\n";
        }
    }

    void updateWeather() {
        weatherDuration--;

        if (weatherDuration <= 0) {
            // Change weather
            std::uniform_int_distribution<int> weatherDist(0, 5);
            Weather newWeather = static_cast<Weather>(weatherDist(rng));

            if (newWeather != currentWeather) {
                WeatherInfo info = getWeatherInfo(newWeather);
                std::cout << Color::CYAN << "🌡️ Weather change: " << info.icon << " " << info.name
                          << Color::DIM << " - " << info.description << Color::RESET << "\n";
                addLogEntry("EVENT", "Weather changed to " + info.name);
                currentWeather = newWeather;
            }

            // Random duration between 5-20 turns
            std::uniform_int_distribution<int> durationDist(5, 20);
            weatherDuration = durationDist(rng);
        }

        // Storm can cause random equipment damage
        if (currentWeather == Weather::STORM) {
            std::uniform_int_distribution<int> stormDist(0, 20);
            if (stormDist(rng) == 0) {
                std::cout << Color::YELLOW << Color::BOLD
                          << "⚡ LIGHTNING STRIKE near the facility!"
                          << Color::RESET << "\n";
                addLogEntry("WARNING", "Lightning strike detected");

                // Random effect
                std::uniform_int_distribution<int> effectDist(0, 2);
                switch (effectDist(rng)) {
                    case 0:
                        std::cout << Color::YELLOW << "   Turbine RPM fluctuation" << Color::RESET << "\n";
                        if (turbineOnline) turbineRPM *= 0.9;
                        break;
                    case 1:
                        std::cout << Color::YELLOW << "   Minor sensor interference" << Color::RESET << "\n";
                        break;
                    case 2:
                        std::cout << Color::RED << "   External power grid disruption!" << Color::RESET << "\n";
                        if (!dieselRunning && dieselAutoStart && dieselFuel > 0) {
                            dieselRunning = true;
                            std::cout << Color::GREEN << "   Diesel generator auto-started." << Color::RESET << "\n";
                        }
                        break;
                }
            }
        }
    }

    void updateGridDemand() {
        // Demand fluctuates over time
        std::uniform_int_distribution<int> fluctDist(-50, 50);
        double fluctuation = fluctDist(rng);

        // Base demand varies by time of day simulation (every 10 turns is an "hour")
        int hourOfDay = (turns / 10) % 24;
        double baseDemand;
        if (hourOfDay >= 7 && hourOfDay <= 9) {
            baseDemand = 700.0;  // Morning peak
        } else if (hourOfDay >= 17 && hourOfDay <= 21) {
            baseDemand = 800.0;  // Evening peak
        } else if (hourOfDay >= 0 && hourOfDay <= 5) {
            baseDemand = 300.0;  // Night low
        } else {
            baseDemand = 500.0;  // Normal
        }

        // Weather affects demand
        WeatherInfo weatherInfo = getWeatherInfo(currentWeather);
        if (currentWeather == Weather::HEATWAVE) {
            baseDemand *= 1.3;  // AC usage
        } else if (currentWeather == Weather::COLD_SNAP) {
            baseDemand *= 1.2;  // Heating
        }

        gridDemand = std::max(200.0, std::min(1000.0, baseDemand + fluctuation));

        // Calculate satisfaction
        double effectiveOutput = electricityOutput;
        if (dieselRunning) {
            effectiveOutput += DIESEL_POWER_OUTPUT;
        }

        demandSatisfaction = std::min(100.0, (effectiveOutput / gridDemand) * 100.0);

        // Bonus/penalty system
        if (demandSatisfaction >= 95.0) {
            int bonus = static_cast<int>((demandSatisfaction - 90.0) * 2);
            demandBonus += bonus;
            score += bonus;
        } else if (demandSatisfaction < 50.0) {
            int penalty = static_cast<int>((50.0 - demandSatisfaction) / 5);
            demandPenalty += penalty;
            // Don't subtract from score for demand issues, just don't give bonus
        }

        // Warnings
        if (demandSatisfaction < 30.0) {
            std::cout << Color::RED << Color::BOLD
                      << "⚠ GRID ALERT: Power output critically below demand! ("
                      << std::fixed << std::setprecision(0) << demandSatisfaction << "%)"
                      << Color::RESET << "\n";
        } else if (demandSatisfaction < 60.0) {
            std::cout << Color::YELLOW
                      << "⚠ Low grid satisfaction: " << std::fixed << std::setprecision(0) << demandSatisfaction << "%"
                      << Color::RESET << "\n";
        }
    }

    void updateStatistics() {
        // Track peak values
        if (temperature > peakTemperature) peakTemperature = temperature;
        if (power > peakPower) peakPower = power;
        if (electricityOutput > peakElectricity) peakElectricity = electricityOutput;
        if (coolant < lowestCoolant) lowestCoolant = coolant;
        if (xenonLevel > highestXenon) highestXenon = xenonLevel;

        // Track totals
        totalPowerGenerated += power;
        temperatureSum += temperature;
        if (turns > 0) {
            averageTemperature = temperatureSum / turns;
        }

        // Count critical events
        if (temperature > currentDifficulty.scramTemperature * 0.9 ||
            coolant < CRITICAL_COOLANT * 1.5 ||
            xenonLevel > MAX_XENON * 0.8) {
            criticalEvents++;
        }
    }

    void addLogEntry(const std::string& type, const std::string& message) {
        LogEntry entry{turns, type, message};
        operatorLog.push_back(entry);
        if (operatorLog.size() > MAX_LOG_ENTRIES) {
            operatorLog.erase(operatorLog.begin());
        }
    }

    void displayLog() const {
        std::cout << "\n" << Color::BOLD << Color::WHITE
                  << "╔═══════════════════════════════════════════════════════════╗\n"
                  << "║                    OPERATOR LOG                           ║\n"
                  << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        if (operatorLog.empty()) {
            std::cout << Color::WHITE << "║ " << Color::DIM << "No log entries yet."
                      << std::setw(40) << "" << Color::WHITE << "║" << Color::RESET << "\n";
        } else {
            // Show last 15 entries
            int start = std::max(0, static_cast<int>(operatorLog.size()) - 15);
            for (size_t i = start; i < operatorLog.size(); ++i) {
                const auto& entry = operatorLog[i];
                std::string color;
                std::string prefix;
                if (entry.type == "CRITICAL") { color = Color::RED; prefix = "!!"; }
                else if (entry.type == "WARNING") { color = Color::YELLOW; prefix = "!!"; }
                else if (entry.type == "EVENT") { color = Color::CYAN; prefix = ">>"; }
                else { color = Color::GREEN; prefix = ">>"; }

                std::cout << Color::WHITE << "║ " << Color::DIM << "[T" << std::setw(3) << entry.turn << "] "
                          << Color::RESET << color << prefix << " " << entry.message << Color::RESET;

                int padding = 48 - static_cast<int>(entry.message.length());
                if (padding > 0) std::cout << std::setw(padding) << "";
                std::cout << Color::WHITE << "║" << Color::RESET << "\n";
            }
        }

        std::cout << Color::BOLD << Color::WHITE
                  << "╠═══════════════════════════════════════════════════════════╣\n"
                  << "║ Total Entries: " << std::setw(4) << operatorLog.size()
                  << std::setw(38) << "" << "║\n"
                  << "╚═══════════════════════════════════════════════════════════╝" << Color::RESET << "\n\n";

        std::cout << Color::DIM << "Press Enter to continue..." << Color::RESET;
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    void displayStatistics() const {
        std::cout << "\n" << Color::BOLD << Color::BLUE
                  << "╔═══════════════════════════════════════════════════════════╗\n"
                  << "║                  SESSION STATISTICS                       ║\n"
                  << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::BLUE << "║ " << Color::RESET << Color::BOLD << "TEMPERATURE:" << Color::RESET
                  << std::setw(47) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Peak Temperature: "
                  << std::fixed << std::setprecision(1) << std::setw(10) << peakTemperature << "°C"
                  << std::setw(27) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Average Temperature: "
                  << std::fixed << std::setprecision(1) << std::setw(7) << averageTemperature << "°C"
                  << std::setw(27) << "" << Color::BLUE << "║" << Color::RESET << "\n";

        std::cout << Color::BLUE << "║ " << Color::RESET << Color::BOLD << "POWER GENERATION:" << Color::RESET
                  << std::setw(42) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Peak Power: "
                  << std::fixed << std::setprecision(1) << std::setw(16) << peakPower << " units"
                  << std::setw(23) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Peak Electricity: "
                  << std::fixed << std::setprecision(1) << std::setw(10) << peakElectricity << " MW"
                  << std::setw(25) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Total Power Generated: "
                  << std::fixed << std::setprecision(0) << std::setw(5) << totalPowerGenerated << " units"
                  << std::setw(22) << "" << Color::BLUE << "║" << Color::RESET << "\n";

        std::cout << Color::BLUE << "║ " << Color::RESET << Color::BOLD << "SAFETY METRICS:" << Color::RESET
                  << std::setw(44) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Lowest Coolant: "
                  << std::fixed << std::setprecision(1) << std::setw(12) << lowestCoolant << "%"
                  << std::setw(26) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Highest Xenon: "
                  << std::fixed << std::setprecision(1) << std::setw(13) << highestXenon << "%"
                  << std::setw(26) << "" << Color::BLUE << "║" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << "  Critical Events: "
                  << std::setw(11) << criticalEvents
                  << std::setw(28) << "" << Color::BLUE << "║" << Color::RESET << "\n";

        // Calculate efficiency rating
        double efficiency = 0.0;
        if (turns > 0) {
            double tempEfficiency = 1.0 - (averageTemperature / currentDifficulty.meltdownTemperature);
            double safetyRating = 1.0 - (static_cast<double>(scramCount) / std::max(1, turns));
            double powerRating = totalElectricityGenerated / std::max(1, turns);
            efficiency = (tempEfficiency * 0.3 + safetyRating * 0.4 + std::min(1.0, powerRating / 100.0) * 0.3) * 100.0;
        }

        std::string efficiencyRating;
        std::string efficiencyColor;
        if (efficiency >= 80) { efficiencyRating = "EXCELLENT"; efficiencyColor = Color::GREEN; }
        else if (efficiency >= 60) { efficiencyRating = "GOOD"; efficiencyColor = Color::GREEN; }
        else if (efficiency >= 40) { efficiencyRating = "AVERAGE"; efficiencyColor = Color::YELLOW; }
        else if (efficiency >= 20) { efficiencyRating = "POOR"; efficiencyColor = Color::RED; }
        else { efficiencyRating = "CRITICAL"; efficiencyColor = Color::RED; }

        std::cout << Color::BLUE << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";
        std::cout << Color::BLUE << "║ " << Color::RESET << Color::BOLD << "OPERATOR EFFICIENCY: " << Color::RESET
                  << efficiencyColor << std::fixed << std::setprecision(1) << efficiency << "% - " << efficiencyRating
                  << Color::RESET << std::setw(25 - static_cast<int>(efficiencyRating.length())) << "" << Color::BLUE << "║" << Color::RESET << "\n";

        std::cout << Color::BOLD << Color::BLUE
                  << "╚═══════════════════════════════════════════════════════════╝" << Color::RESET << "\n\n";

        std::cout << Color::DIM << "Press Enter to continue..." << Color::RESET;
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    void activateECCS() {
        if (!eccsAvailable) {
            std::cout << Color::RED << "✗ ECCS on cooldown! " << eccsCooldownTimer << " turns remaining." << Color::RESET << "\n";
            return;
        }

        eccsAvailable = false;
        eccsCooldownTimer = ECCS_COOLDOWN;

        coolant = std::min(100.0, coolant + ECCS_COOLANT_BOOST);
        temperature = std::max(INITIAL_TEMPERATURE, temperature - ECCS_TEMP_REDUCTION);
        score = std::max(0, score - ECCS_PENALTY);

        std::cout << Color::BG_BLUE << Color::WHITE << Color::BOLD
                  << " 🚨 ECCS ACTIVATED! +" << ECCS_COOLANT_BOOST << "% coolant, -" << ECCS_TEMP_REDUCTION << "°C "
                  << Color::RESET << "\n";
        std::cout << Color::RED << "(-" << ECCS_PENALTY << " points)" << Color::RESET << "\n";
        addLogEntry("CRITICAL", "ECCS activated - emergency cooling");
    }

    void updatePhysics() {
        // Apply xenon poisoning effect on reactivity
        double xenonFactor = 1.0 - (xenonLevel / MAX_XENON) * 0.3;

        double k_eff = (1.05 - controlRods * 1.1) * xenonFactor;
        k_eff = std::max(0.7, k_eff);
        neutrons *= k_eff;

        power = neutrons * NEUTRON_TO_POWER_RATIO;

        double fuel_eff = fuel / 100.0;
        neutrons *= fuel_eff;
        fuel = std::max(0.0, fuel - currentDifficulty.fuelDepletionRate);

        temperature += power * POWER_TO_HEAT_RATIO;
        coolant = std::max(0.0, coolant - currentDifficulty.coolantLossRate);

        // Apply weather-modified cooling
        WeatherInfo weatherInfo = getWeatherInfo(currentWeather);
        double effectiveCooling = NATURAL_COOLING_RATE * weatherInfo.coolingModifier;
        temperature = std::max(0.0, temperature - effectiveCooling);

        if (coolant < CRITICAL_COOLANT) {
            playSound();
            std::cout << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "!!! WARNING: Coolant is critically low! !!!"
                      << Color::RESET << "\n";
            temperature += 5.0;
        }

        // Update subsystems
        updateXenon();
        updateTurbine();
        updateECCS();
        updateDieselGenerator();
        updateRadiation();
        updateWeather();
        updateGridDemand();

        // Update statistics
        updateStatistics();

        // Update score with difficulty multiplier
        turns++;
        turnsWithoutScram++;
        score += POINTS_PER_TURN * currentDifficulty.scoreMultiplier;
        score += static_cast<int>(power * POINTS_PER_POWER_UNIT * currentDifficulty.scoreMultiplier);
        score += static_cast<int>(electricityOutput / 100.0 * POINTS_PER_MW * currentDifficulty.scoreMultiplier);

        // Check achievements
        checkAchievements();
    }

    void processRandomEvents() {
        std::uniform_int_distribution<int> eventDist(0, static_cast<int>(currentDifficulty.eventChance) - 1);

        if (eventDist(rng) != 0) return;

        eventsExperienced++;

        std::uniform_int_distribution<int> eventTypeDist(0, 99);
        int roll = eventTypeDist(rng);

        if (roll < 18) {
            double leak = 10.0 + (rng() % 10);
            coolant = std::max(0.0, coolant - leak);
            std::cout << Color::YELLOW << Color::BOLD
                      << "⚠ COOLANT LEAK: Lost " << std::fixed << std::setprecision(1) << leak << "% coolant!"
                      << Color::RESET << "\n";
            addLogEntry("WARNING", "Coolant leak detected - " + std::to_string(static_cast<int>(leak)) + "% lost");
        } else if (roll < 32) {
            double surge = 30.0 + (rng() % 40);
            temperature += surge;
            std::cout << Color::RED << Color::BOLD
                      << "⚡ POWER SURGE: Temperature +" << std::fixed << std::setprecision(1) << surge << "°C!"
                      << Color::RESET << "\n";
            addLogEntry("WARNING", "Power surge - temperature spike");
        } else if (roll < 42) {
            coolant = std::max(0.0, coolant - 15.0);
            temperature += 20.0;
            std::cout << Color::RED << Color::BOLD
                      << "🔧 PUMP FAILURE: -15% coolant, +20°C!"
                      << Color::RESET << "\n";
            addLogEntry("WARNING", "Coolant pump failure");
        } else if (roll < 52) {
            xenonLevel = std::min(MAX_XENON, xenonLevel + 20.0);
            std::cout << Color::MAGENTA << Color::BOLD
                      << "☢ XENON SPIKE: Xe-135 levels surged! +20%"
                      << Color::RESET << "\n";
            addLogEntry("EVENT", "Xenon-135 spike detected");
        } else if (roll < 62) {
            if (turbineOnline) {
                turbineRPM = std::max(0.0, turbineRPM - 500.0);
                std::cout << Color::YELLOW << Color::BOLD
                          << "💨 STEAM LEAK: Turbine -500 RPM"
                          << Color::RESET << "\n";
                addLogEntry("WARNING", "Steam leak in turbine hall");
            } else {
                temperature += 15.0;
                std::cout << Color::YELLOW << Color::BOLD
                          << "💨 STEAM LEAK: +15°C"
                          << Color::RESET << "\n";
                addLogEntry("WARNING", "Steam leak in reactor building");
            }
        } else if (roll < 70) {
            if (turbineOnline) {
                turbineOnline = false;
                turbineRPM *= 0.5;
                std::cout << Color::RED << Color::BOLD
                          << "⚙ TURBINE TRIP: Emergency shutdown!"
                          << Color::RESET << "\n";
                addLogEntry("WARNING", "Turbine trip - emergency shutdown");
            }
        } else if (roll < 80) {
            double bonus = 50.0 + (rng() % 50);
            score += static_cast<int>(bonus);
            std::cout << Color::GREEN << Color::BOLD
                      << "✨ EFFICIENCY BOOST: +" << static_cast<int>(bonus) << " points!"
                      << Color::RESET << "\n";
            addLogEntry("EVENT", "Efficiency improvement bonus");
        } else if (roll < 90) {
            double bonus = 10.0 + (rng() % 15);
            coolant = std::min(100.0, coolant + bonus);
            std::cout << Color::GREEN << Color::BOLD
                      << "💧 COOLANT DELIVERY: +" << std::fixed << std::setprecision(1) << bonus << "% coolant!"
                      << Color::RESET << "\n";
            addLogEntry("EVENT", "Coolant delivery received");
        } else {
            temperature = std::max(INITIAL_TEMPERATURE, temperature - 30.0);
            xenonLevel = std::max(0.0, xenonLevel - 10.0);
            std::cout << Color::GREEN << Color::BOLD
                      << "👷 MAINTENANCE CREW: -30°C, -10% xenon"
                      << Color::RESET << "\n";
            addLogEntry("EVENT", "Maintenance crew performed repairs");
        }
    }

    void checkSafetyLimits() {
        if ((temperature > currentDifficulty.scramTemperature || neutrons > SCRAM_NEUTRONS) && running) {
            playAlert();
            std::cout << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "*** AUTO SCRAM! Emergency shutdown! ***"
                      << Color::RESET << "\n";
            controlRods = 1.0;
            neutrons *= 0.05;
            temperature = std::max(0.0, temperature - 200);
            turbineOnline = false;
            running = false;
            scramCount++;
            turnsWithoutScram = 0;
            score = std::max(0, score - SCRAM_PENALTY);
            std::cout << Color::RED << "Score penalty: -" << SCRAM_PENALTY << " points" << Color::RESET << "\n";
            std::string reason = temperature > currentDifficulty.scramTemperature ? "temperature exceeded limit" : "neutron flux exceeded limit";
            addLogEntry("CRITICAL", "AUTO SCRAM triggered - " + reason);
        }

        if (temperature > currentDifficulty.meltdownTemperature) {
            playAlert();
            std::cout << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "!!! MELTDOWN !!! Core has gone critical. Game Over."
                      << Color::RESET << "\n";
            addLogEntry("CRITICAL", "MELTDOWN - Core destruction");
            running = false;
        }
    }

    bool handleScramReset() {
        std::cout << Color::YELLOW << "Type 'reset' to restart reactor, or 'q' to quit: "
                  << Color::RESET;
        std::string input;
        if (!std::getline(std::cin, input)) {
            return false;
        }

        if (input == "reset") {
            std::cout << Color::GREEN << "Reactor restart initiated..." << Color::RESET << "\n";
            running = true;
            temperature = INITIAL_TEMPERATURE;
            controlRods = 1.0;
            scramRecoveries++;
            return true;
        }
        return false;
    }

    void displayFinalScore() {
        saveHighScore();
        std::cout << "\n" << Color::BOLD << Color::CYAN
                  << "╔════════════════════════════════════════════════════╗\n"
                  << "║                  FINAL RESULTS                     ║\n"
                  << "╠════════════════════════════════════════════════════╣" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Difficulty: "
                  << std::setw(38) << currentDifficulty.name << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Turns Survived: "
                  << std::setw(34) << turns << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "SCRAMs Triggered: "
                  << std::setw(32) << scramCount << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Events Experienced: "
                  << std::setw(30) << eventsExperienced << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Total Electricity (MW·h): "
                  << std::setw(24) << static_cast<int>(totalElectricityGenerated) << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Final Score: "
                  << std::setw(37) << score << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "High Score (" << currentDifficulty.name << "): "
                  << std::setw(33 - static_cast<int>(currentDifficulty.name.length())) << highScore << Color::CYAN << " ║" << Color::RESET << "\n";

        if (!sessionAchievements.empty()) {
            std::cout << Color::CYAN << "╠════════════════════════════════════════════════════╣" << Color::RESET << "\n";
            std::cout << Color::CYAN << "║ " << Color::BOLD << "ACHIEVEMENTS UNLOCKED THIS SESSION:" << Color::RESET
                      << std::setw(15) << "" << Color::CYAN << "║" << Color::RESET << "\n";
            for (const auto& ach : sessionAchievements) {
                const auto& info = ACHIEVEMENT_INFO[static_cast<int>(ach)];
                std::cout << Color::CYAN << "║   " << Color::GREEN << info.icon << " " << info.name << Color::RESET
                          << std::setw(37 - static_cast<int>(info.name.length())) << "" << Color::CYAN << "║" << Color::RESET << "\n";
            }
        }

        std::cout << Color::BOLD << Color::CYAN
                  << "╚════════════════════════════════════════════════════╝" << Color::RESET << "\n";

        if (score >= highScore && score > 0) {
            std::cout << Color::BOLD << Color::GREEN << "🏆 NEW HIGH SCORE! 🏆" << Color::RESET << "\n";
        }
    }

public:
    void run() {
        std::cout << Color::BOLD << Color::CYAN
                  << "╔════════════════════════════════════════════════════════════╗\n"
                  << "║         C++ NUCLEAR REACTOR SIMULATOR v0.8                 ║\n"
                  << "║         Difficulty: " << std::left << std::setw(40) << currentDifficulty.name << "║\n"
                  << "╚════════════════════════════════════════════════════════════╝\n"
                  << Color::RESET;
        std::cout << Color::DIM << "Commands: 0-100, r, t, e, s(ave), l(oad), a, stats, h(elp), q(uit)\n"
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
            if (input == "h" || input == "help") { displayHelp(); continue; }
            if (input == "a") { displayAchievements(); continue; }
            if (input == "stats") { displayStatistics(); continue; }
            if (input == "log") { displayLog(); continue; }

            if (input == "r") {
                coolant = INITIAL_COOLANT;
                score = std::max(0, score - REFILL_PENALTY);
                std::cout << Color::GREEN << "Coolant refilled! " << Color::RESET
                          << Color::RED << "(-" << REFILL_PENALTY << " pts)" << Color::RESET << "\n";
                addLogEntry("ACTION", "Coolant system refilled to 100%");
                continue;
            }

            if (input == "t") {
                turbineOnline = !turbineOnline;
                std::cout << (turbineOnline ? Color::GREEN + "Turbine starting..." : Color::YELLOW + "Turbine stopping...") << Color::RESET << "\n";
                addLogEntry("ACTION", turbineOnline ? "Turbine brought online" : "Turbine taken offline");
                continue;
            }

            if (input == "e") {
                activateECCS();
                continue;
            }

            if (input == "d") {
                toggleDieselGenerator();
                continue;
            }

            if (input == "df") {
                refillDiesel();
                continue;
            }

            if (input == "da") {
                dieselAutoStart = !dieselAutoStart;
                std::cout << (dieselAutoStart ? Color::GREEN + "🔌 Diesel auto-start ENABLED" : Color::YELLOW + "🔌 Diesel auto-start DISABLED") << Color::RESET << "\n";
                addLogEntry("ACTION", dieselAutoStart ? "Diesel auto-start enabled" : "Diesel auto-start disabled");
                continue;
            }

            if (input == "save" || input == "s") {
                if (saveGame()) {
                    playSound();
                    std::cout << Color::GREEN << "💾 Game saved successfully!" << Color::RESET << "\n";
                } else {
                    std::cout << Color::RED << "Failed to save game." << Color::RESET << "\n";
                }
                continue;
            }

            if (input == "load" || input == "l") {
                if (loadGame()) {
                    playSound();
                    std::cout << Color::GREEN << "💾 Game loaded successfully!" << Color::RESET << "\n";
                } else {
                    std::cout << Color::RED << "No save file found." << Color::RESET << "\n";
                }
                continue;
            }

            if (input == "sound") {
                soundEnabled = !soundEnabled;
                std::cout << (soundEnabled ? Color::GREEN + "🔊 Sound enabled" : Color::YELLOW + "🔇 Sound disabled") << Color::RESET << "\n";
                continue;
            }

            if (input == "p" || input == "pause") {
                paused = !paused;
                if (paused) {
                    std::cout << Color::BG_YELLOW << Color::WHITE << Color::BOLD
                              << " ⏸ SIMULATION PAUSED " << Color::RESET << "\n";
                    std::cout << Color::DIM << "Use 'p' or 'pause' to resume. You can still view stats, log, and achievements."
                              << Color::RESET << "\n";
                    addLogEntry("ACTION", "Simulation paused by operator");
                } else {
                    std::cout << Color::GREEN << Color::BOLD << "▶ SIMULATION RESUMED" << Color::RESET << "\n";
                    addLogEntry("ACTION", "Simulation resumed");
                }
                continue;
            }

            if (paused) {
                std::cout << Color::YELLOW << "⏸ Simulation paused. Use 'p' to resume before making changes." << Color::RESET << "\n";
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
