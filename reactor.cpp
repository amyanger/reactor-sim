#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <string> // For string operations (added for dashboard helper)

int main() {
    // Initialize reactor variables
    double neutrons = 1000.0;       // starting neutron population
    double controlRods = 0.5;       // 0.0 (fully out, max reaction) to 1.0 (fully in, shuts down)
    double temperature = 300.0;     // degrees C
    double coolant = 100.0;          // coolant percentage
    double power = 0.0;              // current power output
    double fuel = 100.0;             // fuel level (%), new variable
    bool running = true;             // reactor operational status

    // Helper function to print a horizontal bar for any stat (added for dashboard)
    auto printBar = [](const std::string& label, double value, double max, int width = 20) {
        int bars = static_cast<int>((value / max) * width);
        std::cout << std::left << std::setw(8) << label << "[";
        for (int i = 0; i < width; ++i) {
            if (i < bars)
                std::cout << "█";
            else
                std::cout << " ";
        }
        std::cout << "]  ";
        if (label == "Temp")
            std::cout << std::fixed << std::setprecision(1) << value << "°C";
        else if (label == "Coolant" || label == "Fuel")
            std::cout << std::fixed << std::setprecision(1) << value << "%";
        else
            std::cout << std::fixed << std::setprecision(1) << value;
        std::cout << std::endl;
    };

    std::srand(std::time(nullptr)); // Seed random number generator for random events.

    std::cout << "Welcome to the C++ Nuclear Reactor Simulator v0.1\n";
    std::cout << "Try not to melt the core. Type 'q' to quit.\n";

    // Main simulation loop: runs each tick while reactor is operational
    while (running) {
        // === ASCII Dashboard === (added)
        std::cout << "\n=== Reactor Dashboard ===\n";
        printBar("Temp", temperature, 2000.0);      // Assume 2000°C is max for bar
        printBar("Coolant", coolant, 100.0);
        printBar("Fuel", fuel, 100.0);
        std::cout << std::endl;

        // Show current reactor state with formatted output
        std::cout << "\nNeutrons: " << std::fixed << std::setprecision(2) << neutrons
                  << " | Control Rods: " << (int)(controlRods * 100) << "% in"
                  << " | Temp: " << temperature << "C"
                  << " | Coolant: " << std::setprecision(1) << coolant << "%"
                  << " | Fuel: " << std::setprecision(1) << fuel << "%\n";

        // User input section: adjust control rods or refill coolant
        std::cout << "Set control rod level (0-100%, current " << (int)(controlRods * 100) << "%, or 'r' to refill coolant): ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "q") break;        // Quit simulation
        if (input == "r") {
            coolant = 100.0;             // Refill coolant to full
            std::cout << "Coolant refilled!\n";
            continue;                   // Skip rest of loop to prompt again
        }
        // Parse control rod input and clamp between 0 and 1
        double newLevel = std::stod(input) / 100.0;
        if (newLevel < 0) newLevel = 0;
        if (newLevel > 1) newLevel = 1;
        controlRods = newLevel;

        // Physics calculation: neutron multiplication factor (k-eff)
        // Control rods absorb neutrons, reducing k-eff and neutron population growth
        double k_eff = 1.05 - controlRods * 1.1; // more rods = lower k_eff
        if (k_eff < 0.7) k_eff = 0.7;            // minimum k_eff to avoid negative growth

        // Core physics: basic neutron growth/decay due to k-eff
        neutrons *= k_eff;

        // Calculate power output based on neutron population
        power = neutrons * 0.1;

        // Fuel burnup: as fuel drops, neutron production drops
        double fuel_eff = fuel / 100.0;
        neutrons *= fuel_eff;          // fewer neutrons generated as fuel depletes
        // Deplete fuel a little each tick to simulate consumption
        fuel -= 0.1;
        if (fuel < 0) fuel = 0;

        // Temperature increase due to power output
        temperature += power * 0.01;  // core heats up from power produced

        // Passive cooling and coolant effects
        coolant -= 0.3;               // coolant loss per tick
        if (coolant < 0) coolant = 0;
        temperature -= 0.5;           // natural cooling each tick

        // If coolant is critically low, temperature rises faster (danger)
        if (coolant < 20.0) {
            std::cout << "!!! WARNING: Coolant is critically low! !!!\n";
            temperature += 5.0;       // extra heating due to poor cooling
        }

        // Random events: 10% chance per tick to simulate unexpected problems
        int eventRoll = std::rand() % 10; // random number from 0-9
        if (eventRoll == 0) { // 1 in 10 chance
            int disasterType = std::rand() % 2; // 0 or 1
            if (disasterType == 0 && coolant > 10.0) {
                coolant -= 10.0;       // coolant leak event
                std::cout << "!!! RANDOM EVENT: Coolant Leak! Lost 10% coolant! !!!\n";
                if (coolant < 0) coolant = 0;
            } else {
                temperature += 50.0;   // power surge event
                std::cout << "!!! RANDOM EVENT: Power Surge! Temperature increased by 50C! !!!\n";
            }
        }

        // Auto SCRAM: triggers if temperature or neutrons get too high
        if ((temperature > 1000 || neutrons > 2000) && running) {
            std::cout << "\n*** AUTO SCRAM! Emergency shutdown! ***\n";
            controlRods = 1.0;        // fully insert control rods
            neutrons *= 0.05;         // rapidly reduce neutron population
            temperature -= 200;       // cool core somewhat
            running = false;          // freeze operation, require manual reset
        }

        // Core meltdown (for fun): catastrophic failure if temperature too high
        if (temperature > 2000) {
            std::cout << "\n!!! MELTDOWN !!! Core has gone critical. You have failed as reactor operator.\n";
            running = false;          // end simulation on meltdown
        }

        // If SCRAM occurred, offer reset option to user
        if (!running) {
            std::cout << "Type 'reset' to attempt reactor restart, or 'q' to quit: ";
            std::getline(std::cin, input);
            if (input == "reset") {
                std::cout << "Reactor restart attempt...\n";
                running = true;        // resume simulation
                // Optionally reset state slightly for restart
                temperature = 300.0;
                controlRods = 1.0;
            } else {
                break;                 // exit simulation loop
            }
        }
    }

    // End of simulation message
    std::cout << "\nReactor simulation ended. Stay radioactive, Arjun. 💥\n";
    return 0;
}
