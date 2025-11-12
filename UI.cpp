#include "UI.h"
#include "Constants.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

void UI::displayWelcome() const {
    std::cout << "Welcome to the C++ Nuclear Reactor Simulator v1.0\n";
    std::cout << "Try not to melt the core. Type 'q' to quit.\n";
}

void UI::displayDashboard(const Reactor& reactor) const {
    std::cout << "\n=== Reactor Dashboard ===\n";
    printBar("Temp", reactor.getTemperature(), MELTDOWN_TEMP);
    printBar("Coolant", reactor.getCoolant(), 100.0);
    printBar("Fuel", reactor.getFuel(), 100.0);
    std::cout << std::endl;
}

void UI::displayStatus(const Reactor& reactor) const {
    std::cout << "\nNeutrons: " << std::fixed << std::setprecision(2) << reactor.getNeutrons()
              << " | Control Rods: " << (int)(reactor.getControlRods() * 100) << "% in"
              << " | Temp: " << reactor.getTemperature() << "C"
              << " | Coolant: " << std::setprecision(1) << reactor.getCoolant() << "%"
              << " | Fuel: " << std::setprecision(1) << reactor.getFuel() << "%\n";
}

void UI::displayMeltdown() const {
    std::cout << "\n!!! MELTDOWN !!! Core has gone critical. You have failed as reactor operator.\n";
}

std::string UI::getUserInput() const {
    std::cout << "Set control rod level (0-100%, or 'r' to refill coolant): ";
    std::string input;
    std::getline(std::cin, input);
    return input;
}

double UI::parseControlRodInput(const std::string& input) const {
    try {
        double value = std::stod(input);
        if (value < 0.0 || value > 100.0) {
            throw std::out_of_range("Value must be between 0 and 100!");
        }
        return value / 100.0;
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument("Invalid input! Please enter a number between 0-100.");
    } catch (const std::out_of_range& e) {
        throw std::out_of_range("Number out of range! Please enter a value between 0-100.");
    }
}

void UI::printBar(const std::string& label, double value, double max, int width) const {
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
}