#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <random>
#include <chrono>
#include <fstream>
#include <algorithm>

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
}

enum class Difficulty {
    EASY,
    NORMAL,
    HARD,
    NIGHTMARE
};

struct DifficultySettings {
    std::string name;
    double fuelDepletionRate;
    double coolantLossRate;
    double eventChance;         // 1 in X chance per turn
    double scramTemperature;
    double meltdownTemperature;
    int scoreMultiplier;
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

    // Scoring constants
    static constexpr int POINTS_PER_TURN = 10;
    static constexpr int POINTS_PER_POWER_UNIT = 1;
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

    // Scoring system
    int score;
    int turns;
    int scramCount;
    int highScore;

    // Modern random number generation
    std::mt19937 rng;

    static constexpr const char* HIGH_SCORE_FILE = ".reactor_highscore";

    static DifficultySettings getDifficultySettings(Difficulty diff) {
        switch (diff) {
            case Difficulty::EASY:
                return {"Easy", 0.05, 0.15, 15.0, 1200.0, 2500.0, 1};
            case Difficulty::NORMAL:
                return {"Normal", 0.1, 0.3, 10.0, 1000.0, 2000.0, 2};
            case Difficulty::HARD:
                return {"Hard", 0.15, 0.5, 7.0, 800.0, 1500.0, 3};
            case Difficulty::NIGHTMARE:
                return {"Nightmare", 0.2, 0.7, 5.0, 600.0, 1200.0, 5};
            default:
                return {"Normal", 0.1, 0.3, 10.0, 1000.0, 2000.0, 2};
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
        score(0),
        turns(0),
        scramCount(0),
        highScore(0),
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
        std::cout << Color::CYAN << "║" << Color::RESET << "   h      : Display this help screen"
                  << std::setw(22) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   q      : Quit the simulation"
                  << std::setw(27) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " REACTOR PHYSICS:" << Color::RESET
                  << std::setw(42) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Control rods absorb neutrons and slow reactions"
                  << std::setw(6) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Higher rod % = safer but less power/points"
                  << std::setw(10) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Lower rod % = more power but risk of SCRAM"
                  << std::setw(10) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Coolant prevents overheating - keep it above 20%"
                  << std::setw(5) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Fuel depletes over time, reducing efficiency"
                  << std::setw(9) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " SAFETY LIMITS (" << currentDifficulty.name << " mode):" << Color::RESET
                  << std::setw(34 - static_cast<int>(currentDifficulty.name.length())) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • SCRAM triggers at: " << currentDifficulty.scramTemperature << "°C or "
                  << SCRAM_NEUTRONS << " neutrons" << std::setw(7) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • MELTDOWN at: " << currentDifficulty.meltdownTemperature << "°C"
                  << std::setw(31) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • SCRAM penalty: -" << SCRAM_PENALTY << " points"
                  << std::setw(28) << "" << Color::CYAN << "║" << Color::RESET << "\n";

        std::cout << Color::CYAN << "╠═══════════════════════════════════════════════════════════╣" << Color::RESET << "\n";

        std::cout << Color::CYAN << "║" << Color::RESET << Color::BOLD << " SCORING:" << Color::RESET
                  << std::setw(50) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • +" << POINTS_PER_TURN << " points per turn survived"
                  << std::setw(30) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • +" << POINTS_PER_POWER_UNIT << " point per unit of power generated"
                  << std::setw(21) << "" << Color::CYAN << "║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║" << Color::RESET << "   • Score multiplier: x" << currentDifficulty.scoreMultiplier
                  << std::setw(34) << "" << Color::CYAN << "║" << Color::RESET << "\n";

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

        std::cout << std::left << std::setw(10) << label << "[";
        std::cout << color;
        for (int i = 0; i < width; ++i) {
            std::cout << (i < bars ? "█" : " ");
        }
        std::cout << Color::RESET << "]  ";

        std::cout << color << std::fixed << std::setprecision(1) << value << Color::RESET;
        if (label == "Temp") std::cout << "°C";
        else if (label == "Coolant" || label == "Fuel") std::cout << "%";
        std::cout << std::endl;
    }

    void displayDashboard() const {
        std::cout << "\n" << Color::BOLD << Color::CYAN << "╔══════════════════════════════════════╗" << Color::RESET << "\n";
        std::cout << Color::BOLD << Color::CYAN << "║   REACTOR DASHBOARD [" << currentDifficulty.name << "]"
                  << std::setw(16 - static_cast<int>(currentDifficulty.name.length())) << "" << "║" << Color::RESET << "\n";
        std::cout << Color::BOLD << Color::CYAN << "╚══════════════════════════════════════╝" << Color::RESET << "\n\n";
        printBar("Temp", temperature, currentDifficulty.meltdownTemperature, 20, false);
        printBar("Coolant", coolant, 100.0, 20, true);
        printBar("Fuel", fuel, 100.0, 20, true);
        std::cout << std::endl;
    }

    void displayScore() const {
        std::cout << Color::YELLOW << "╭─────────────────────────────────────╮" << Color::RESET << "\n";
        std::cout << Color::YELLOW << "│ " << Color::BOLD << "SCORE: " << Color::RESET
                  << std::setw(8) << score
                  << Color::YELLOW << " │ " << Color::BOLD << "HIGH: " << Color::RESET
                  << std::setw(8) << highScore
                  << Color::YELLOW << " │ " << Color::BOLD << "TURN: " << Color::RESET
                  << std::setw(4) << turns
                  << Color::YELLOW << " │" << Color::RESET << "\n";
        std::cout << Color::YELLOW << "╰─────────────────────────────────────╯" << Color::RESET << "\n";
    }

    void displayStatus() const {
        std::cout << "\n" << Color::DIM << "Neutrons: " << Color::RESET
                  << std::fixed << std::setprecision(2) << neutrons
                  << Color::DIM << " | Control Rods: " << Color::RESET
                  << static_cast<int>(controlRods * 100) << "% in"
                  << Color::DIM << " | Temp: " << Color::RESET << temperature << "°C"
                  << Color::DIM << " | Coolant: " << Color::RESET
                  << std::setprecision(1) << coolant << "%"
                  << Color::DIM << " | Fuel: " << Color::RESET
                  << std::setprecision(1) << fuel << "%\n";
    }

    double parseControlRodInput(const std::string& input) const {
        try {
            double value = std::stod(input) / 100.0;
            return std::max(0.0, std::min(1.0, value));
        } catch (const std::exception&) {
            return controlRods;
        }
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

        // Update score with difficulty multiplier
        turns++;
        score += POINTS_PER_TURN * currentDifficulty.scoreMultiplier;
        score += static_cast<int>(power * POINTS_PER_POWER_UNIT * currentDifficulty.scoreMultiplier);
    }

    void processRandomEvents() {
        std::uniform_int_distribution<int> eventDist(0, static_cast<int>(currentDifficulty.eventChance) - 1);
        std::uniform_int_distribution<int> typeDist(0, 1);

        if (eventDist(rng) == 0) {
            if (typeDist(rng) == 0 && coolant > 10.0) {
                coolant = std::max(0.0, coolant - 10.0);
                std::cout << Color::YELLOW << Color::BOLD
                          << "!!! RANDOM EVENT: Coolant Leak! Lost 10% coolant! !!!"
                          << Color::RESET << "\n";
            } else {
                temperature += 50.0;
                std::cout << Color::RED << Color::BOLD
                          << "!!! RANDOM EVENT: Power Surge! Temperature increased by 50°C! !!!"
                          << Color::RESET << "\n";
            }
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
                  << "╔══════════════════════════════════════╗\n"
                  << "║           FINAL RESULTS              ║\n"
                  << "╠══════════════════════════════════════╣" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Difficulty: "
                  << std::setw(25) << currentDifficulty.name << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Turns Survived: "
                  << std::setw(21) << turns << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "SCRAMs Triggered: "
                  << std::setw(19) << scramCount << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "Final Score: "
                  << std::setw(24) << score << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::CYAN << "║ " << Color::RESET << "High Score (" << currentDifficulty.name << "): "
                  << std::setw(20 - static_cast<int>(currentDifficulty.name.length())) << highScore << Color::CYAN << " ║" << Color::RESET << "\n";
        std::cout << Color::BOLD << Color::CYAN
                  << "╚══════════════════════════════════════╝" << Color::RESET << "\n";

        if (score >= highScore && score > 0) {
            std::cout << Color::BOLD << Color::GREEN << "🏆 NEW HIGH SCORE! 🏆" << Color::RESET << "\n";
        }
    }

public:
    void run() {
        std::cout << Color::BOLD << Color::CYAN
                  << "╔══════════════════════════════════════════════╗\n"
                  << "║  C++ NUCLEAR REACTOR SIMULATOR v0.4          ║\n"
                  << "║  Difficulty: " << std::left << std::setw(32) << currentDifficulty.name << "║\n"
                  << "╚══════════════════════════════════════════════╝\n"
                  << Color::RESET;
        std::cout << Color::DIM << "Commands: 0-100, 'r' (refill), 'h' (help), 'q' (quit)\n"
                  << Color::RESET;

        while (running) {
            displayDashboard();
            displayScore();
            displayStatus();

            std::cout << Color::GREEN << "\nSet control rod level (0-100%, current "
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
