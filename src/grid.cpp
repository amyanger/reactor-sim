#include "grid.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

void GridSystem::update(ReactorState& state) {
    // Demand fluctuates over time
    std::uniform_int_distribution<int> fluctDist(-50, 50);
    double fluctuation = fluctDist(state.rng);

    // Base demand varies by time of day simulation (every 10 turns is an "hour")
    int hourOfDay = (state.turns / 10) % 24;
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
    if (state.currentWeather == Weather::HEATWAVE) {
        baseDemand *= 1.3;  // AC usage
    } else if (state.currentWeather == Weather::COLD_SNAP) {
        baseDemand *= 1.2;  // Heating
    }

    state.gridDemand = std::max(200.0, std::min(1000.0, baseDemand + fluctuation));

    // Calculate satisfaction
    double effectiveOutput = state.electricityOutput;
    if (state.dieselRunning) {
        effectiveOutput += RC::DIESEL_POWER_OUTPUT;
    }

    state.demandSatisfaction = std::min(100.0, (effectiveOutput / state.gridDemand) * 100.0);

    // Bonus/penalty system
    if (state.demandSatisfaction >= 95.0) {
        int bonus = static_cast<int>((state.demandSatisfaction - 90.0) * 2);
        state.demandBonus += bonus;
        state.score += bonus;
    } else if (state.demandSatisfaction < 50.0) {
        int penalty = static_cast<int>((50.0 - state.demandSatisfaction) / 5);
        state.demandPenalty += penalty;
        // Don't subtract from score for demand issues, just don't give bonus
    }

    // Warnings
    if (state.demandSatisfaction < 30.0) {
        std::ostringstream oss;
        oss << Color::RED << Color::BOLD
            << "\xe2\x9a\xa0 GRID ALERT: Power output critically below demand! ("
            << std::fixed << std::setprecision(0) << state.demandSatisfaction << "%)"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
    } else if (state.demandSatisfaction < 60.0) {
        std::ostringstream oss;
        oss << Color::YELLOW
            << "\xe2\x9a\xa0 Low grid satisfaction: " << std::fixed << std::setprecision(0) << state.demandSatisfaction << "%"
            << Color::RESET << "\n";
        state.addMessage(oss.str());
    }
}
