#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <string>
#include <stdexcept>

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
    void printBar(const std::string& label, double value, double max, int width = 20) const {
        int bars = static_cast<int>((value / max) * width);
        std::cout << std::left << std::setw(8) << label << "[";
        for (int i = 0; i < width; ++i) {
            std::cout << (i < bars ? "█" : " ");
        }
        std::cout << "]  ";
        
        std::cout << std::fixed << std::setprecision(1) << value;
        if (label == "Temp") std::cout << "°C";
        else if (label == "Coolant" || label == "Fuel") std::cout << "%";
        std::cout << std::endl;
    }

    void displayDashboard() const {
        std::cout << "\n=== Reactor Dashboard ===\n";
        printBar("Temp", temperature, MELTDOWN_TEMPERATURE);
        printBar("Coolant", coolant, 100.0);
        printBar("Fuel", fuel, 100.0);
        std::cout << std::endl;
    }

    void displayStatus() const {
        std::cout << "\nNeutrons: " << std::fixed << std::setprecision(2) << neutrons
                  << " | Control Rods: " << (int)(controlRods * 100) << "% in"
                  << " | Temp: " << temperature << "C"
                  << " | Coolant: " << std::setprecision(1) << coolant << "%"
                  << " | Fuel: " << std::setprecision(1) << fuel << "%\n";
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
        temperature -= NATURAL_COOLING_RATE;
        
        if (coolant < CRITICAL_COOLANT) {
            std::cout << "!!! WARNING: Coolant is critically low! !!!\n";
            temperature += 5.0;
        }
    }

    void processRandomEvents() {
        if (std::rand() % 10 == 0) {
            if (std::rand() % 2 == 0 && coolant > 10.0) {
                coolant = std::max(0.0, coolant - 10.0);
                std::cout << "!!! RANDOM EVENT: Coolant Leak! Lost 10% coolant! !!!\n";
            } else {
                temperature += 50.0;
                std::cout << "!!! RANDOM EVENT: Power Surge! Temperature increased by 50C! !!!\n";
            }
        }
    }

    void checkSafetyLimits() {
        if ((temperature > SCRAM_TEMPERATURE || neutrons > SCRAM_NEUTRONS) && running) {
            std::cout << "\n*** AUTO SCRAM! Emergency shutdown! ***\n";
            controlRods = 1.0;
            neutrons *= 0.05;
            temperature -= 200;
            running = false;
        }
        
        if (temperature > MELTDOWN_TEMPERATURE) {
            std::cout << "\n!!! MELTDOWN !!! Core has gone critical. You have failed as reactor operator.\n";
            running = false;
        }
    }

    bool handleScramReset() {
        std::cout << "Type 'reset' to attempt reactor restart, or 'q' to quit: ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input == "reset") {
            std::cout << "Reactor restart attempt...\n";
            running = true;
            temperature = INITIAL_TEMPERATURE;
            controlRods = 1.0;
            return true;
        }
        return false;
    }

public:
    void run() {
        std::cout << "Welcome to the C++ Nuclear Reactor Simulator v0.1\n";
        std::cout << "Try not to melt the core. Type 'q' to quit.\n";

        while (running) {
            displayDashboard();
            displayStatus();
            
            std::cout << "Set control rod level (0-100%, current " << (int)(controlRods * 100) 
                      << "%, or 'r' to refill coolant): ";
            
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "q") break;
            
            if (input == "r") {
                coolant = INITIAL_COOLANT;
                std::cout << "Coolant refilled!\n";
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
        
        std::cout << "\nReactor simulation ended. Stay radioactive, Arjun. 💥\n";
    }
};

int main() {
    ReactorSimulator simulator;
    simulator.run();
    return 0;
}
