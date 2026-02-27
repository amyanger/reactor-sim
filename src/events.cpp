#include "events.h"

#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

void RandomEventSystem::process(ReactorState& state) {
    std::uniform_int_distribution<int> eventDist(
        0, static_cast<int>(state.currentDifficulty.eventChance) - 1);

    if (eventDist(state.rng) != 0) return;

    state.eventsExperienced++;

    std::uniform_int_distribution<int> eventTypeDist(0, 99);
    int roll = eventTypeDist(state.rng);

    if (roll < 18) {
        double leak = 10.0 + (state.rng() % 10);
        state.coolant = std::max(0.0, state.coolant - leak);
        std::ostringstream oss;
        oss << Color::YELLOW << Color::BOLD
            << "\xe2\x9a\xa0 COOLANT LEAK: Lost " << std::fixed << std::setprecision(1) << leak << "% coolant!"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("WARNING", "Coolant leak detected - " + std::to_string(static_cast<int>(leak)) + "% lost");

    } else if (roll < 32) {
        double surge = 30.0 + (state.rng() % 40);
        state.temperature += surge;
        std::ostringstream oss;
        oss << Color::RED << Color::BOLD
            << "\xe2\x9a\xa1 POWER SURGE: Temperature +" << std::fixed << std::setprecision(1) << surge << "\xc2\xb0" << "C!"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("WARNING", "Power surge - temperature spike");

    } else if (roll < 42) {
        state.coolant = std::max(0.0, state.coolant - 15.0);
        state.temperature += 20.0;
        std::ostringstream oss;
        oss << Color::RED << Color::BOLD
            << "\xf0\x9f\x94\xa7 PUMP FAILURE: -15% coolant, +20\xc2\xb0" << "C!"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("WARNING", "Coolant pump failure");

    } else if (roll < 52) {
        state.xenonLevel = std::min(RC::MAX_XENON, state.xenonLevel + 20.0);
        std::ostringstream oss;
        oss << Color::MAGENTA << Color::BOLD
            << "\xe2\x98\xa2 XENON SPIKE: Xe-135 levels surged! +20%"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Xenon-135 spike detected");

    } else if (roll < 62) {
        if (state.turbineOnline) {
            state.turbineRPM = std::max(0.0, state.turbineRPM - 500.0);
            std::ostringstream oss;
            oss << Color::YELLOW << Color::BOLD
                << "\xf0\x9f\x92\xa8 STEAM LEAK: Turbine -500 RPM"
                << Color::RESET << "\n";
            state.addMessage(oss.str());
            state.addLogEntry("WARNING", "Steam leak in turbine hall");
        } else {
            state.temperature += 15.0;
            std::ostringstream oss;
            oss << Color::YELLOW << Color::BOLD
                << "\xf0\x9f\x92\xa8 STEAM LEAK: +15\xc2\xb0" << "C"
                << Color::RESET << "\n";
            state.addMessage(oss.str());
            state.addLogEntry("WARNING", "Steam leak in reactor building");
        }

    } else if (roll < 70) {
        if (state.turbineOnline) {
            state.turbineOnline = false;
            state.turbineRPM *= 0.5;
            std::ostringstream oss;
            oss << Color::RED << Color::BOLD
                << "\xe2\x9a\x99 TURBINE TRIP: Emergency shutdown!"
                << Color::RESET << "\n";
            state.addMessage(oss.str());
            state.addLogEntry("WARNING", "Turbine trip - emergency shutdown");
        }

    } else if (roll < 80) {
        double bonus = 50.0 + (state.rng() % 50);
        state.score += static_cast<int>(bonus);
        std::ostringstream oss;
        oss << Color::GREEN << Color::BOLD
            << "\xe2\x9c\xa8 EFFICIENCY BOOST: +" << static_cast<int>(bonus) << " points!"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Efficiency improvement bonus");

    } else if (roll < 90) {
        double bonus = 10.0 + (state.rng() % 15);
        state.coolant = std::min(100.0, state.coolant + bonus);
        std::ostringstream oss;
        oss << Color::GREEN << Color::BOLD
            << "\xf0\x9f\x92\xa7 COOLANT DELIVERY: +" << std::fixed << std::setprecision(1) << bonus << "% coolant!"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Coolant delivery received");

    } else {
        state.temperature = std::max(RC::INITIAL_TEMPERATURE, state.temperature - 30.0);
        state.xenonLevel = std::max(0.0, state.xenonLevel - 10.0);
        std::ostringstream oss;
        oss << Color::GREEN << Color::BOLD
            << "\xf0\x9f\x91\xb7 MAINTENANCE CREW: -30\xc2\xb0" << "C, -10% xenon"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
        state.addLogEntry("EVENT", "Maintenance crew performed repairs");
    }
}
