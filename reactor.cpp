#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <string>
#include <stdexcept>

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

class ReactorSimulator {
private:
    static constexpr double INITIAL_NEUTRONS = 1000.0;
    static constexpr double INITIAL_TEMPERATURE = 300.0;
    static constexpr double INITIAL_COOLANT = 100.0;
    static constexpr double INITIAL_FUEL = 100.0;
    static constexpr double INITIAL_CONTROL_RODS = 0.5;

    static constexpr double SCRAM_TEMPERATURE = 1000.0;
    static constexpr double SCRAM_NEUTRONS = 2000.0;
    static constexpr double MELTDOWN_TEMPERATURE = 2000.0;
    static constexpr double CRITICAL_COOLANT = 20.0;

    static constexpr double FUEL_DEPLETION_RATE = 0.1;
    static constexpr double COOLANT_LOSS_RATE = 0.3;
    static constexpr double NATURAL_COOLING_RATE = 0.5;
    static constexpr double POWER_TO_HEAT_RATIO = 0.01;
    static constexpr double NEUTRON_TO_POWER_RATIO = 0.1;

    double neutrons;
    double controlRods;
    double temperature;
    double coolant;
    double power;
    double fuel;
    bool running;

public:
    ReactorSimulator() :
        neutrons(INITIAL_NEUTRONS),
        controlRods(INITIAL_CONTROL_RODS),
        temperature(INITIAL_TEMPERATURE),
        coolant(INITIAL_COOLANT),
        power(0.0),
        fuel(INITIAL_FUEL),
        running(true) {
        std::srand(std::time(nullptr));
    }

private:
    std::string getBarColor(double value, double max, bool inverse = false) const {
        double ratio = value / max;
        if (inverse) ratio = 1.0 - ratio;

        if (ratio < 0.3) return Color::GREEN;
        if (ratio < 0.6) return Color::YELLOW;
        return Color::RED;
    }

    void printBar(const std::string& label, double value, double max, int width = 20, bool inverse = false) const {
        int bars = static_cast<int>((value / max) * width);
        bars = std::max(0, std::min(width, bars)); // Clamp to valid range

        std::string color = getBarColor(value, max, inverse);

        std::cout << std::left << std::setw(8) << label << "[";
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
        std::cout << "\n" << Color::BOLD << Color::CYAN << "=== Reactor Dashboard ===" << Color::RESET << "\n";
        printBar("Temp", temperature, MELTDOWN_TEMPERATURE, 20, false);
        printBar("Coolant", coolant, 100.0, 20, true);
        printBar("Fuel", fuel, 100.0, 20, true);
        std::cout << std::endl;
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
        fuel = std::max(0.0, fuel - FUEL_DEPLETION_RATE);

        temperature += power * POWER_TO_HEAT_RATIO;
        coolant = std::max(0.0, coolant - COOLANT_LOSS_RATE);
        temperature = std::max(0.0, temperature - NATURAL_COOLING_RATE);

        if (coolant < CRITICAL_COOLANT) {
            std::cout << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "!!! WARNING: Coolant is critically low! !!!"
                      << Color::RESET << "\n";
            temperature += 5.0;
        }
    }

    void processRandomEvents() {
        if (std::rand() % 10 == 0) {
            if (std::rand() % 2 == 0 && coolant > 10.0) {
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
        if ((temperature > SCRAM_TEMPERATURE || neutrons > SCRAM_NEUTRONS) && running) {
            std::cout << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                      << "*** AUTO SCRAM! Emergency shutdown! ***"
                      << Color::RESET << "\n";
            controlRods = 1.0;
            neutrons *= 0.05;
            temperature = std::max(0.0, temperature - 200);
            running = false;
        }

        if (temperature > MELTDOWN_TEMPERATURE) {
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

public:
    void run() {
        std::cout << Color::BOLD << Color::CYAN
                  << "Welcome to the C++ Nuclear Reactor Simulator v0.2\n"
                  << Color::RESET;
        std::cout << Color::DIM << "Try not to melt the core. Type 'q' to quit.\n"
                  << Color::RESET;

        while (running) {
            displayDashboard();
            displayStatus();

            std::cout << Color::GREEN << "Set control rod level (0-100%, current "
                      << static_cast<int>(controlRods * 100)
                      << "%, or 'r' to refill coolant): " << Color::RESET;

            std::string input;
            if (!std::getline(std::cin, input)) {
                break;
            }

            if (input == "q") break;

            if (input == "r") {
                coolant = INITIAL_COOLANT;
                std::cout << Color::GREEN << "Coolant refilled!" << Color::RESET << "\n";
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

        std::cout << "\n" << Color::MAGENTA << Color::BOLD
                  << "Reactor simulation ended. Stay radioactive! ☢️\n"
                  << Color::RESET;
    }
};

int main() {
    ReactorSimulator simulator;
    simulator.run();
    return 0;
}
