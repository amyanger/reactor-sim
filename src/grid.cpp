#include "grid.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

void GridSystem::update(ReactorState& state) {
    // Demand fluctuates over time
    std::uniform_int_distribution<int> fluctDist(-50, 50);
    double fluctuation = fluctDist(state.rng);

    int hourOfDay = (state.turns / RC::TURNS_PER_HOUR) % 24;
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

    // Demand drifts toward the time-of-day target instead of jumping every turn
    double targetDemand = std::max(200.0, std::min(1000.0, baseDemand + fluctuation));
    state.gridDemand += (targetDemand - state.gridDemand) * RC::GRID_DEMAND_SMOOTHING;

    // Supply as a percentage of demand; above 100% means overproduction
    double supply = state.electricityOutput / state.gridDemand * 100.0;
    state.demandSatisfaction = std::min(100.0, supply);

    int multiplier = state.currentDifficulty.scoreMultiplier;
    if (supply >= 95.0 && supply <= RC::GRID_OVERSUPPLY) {
        int bonus = RC::GRID_MATCH_BONUS * multiplier;
        state.demandBonus += bonus;
        state.score += bonus;
    } else {
        int penalty = 0;
        if (supply < RC::GRID_UNDERSUPPLY) {
            penalty = static_cast<int>((RC::GRID_UNDERSUPPLY - supply) / 5.0) * multiplier;
        } else if (supply > RC::GRID_OVERSUPPLY) {
            penalty = static_cast<int>((supply - RC::GRID_OVERSUPPLY) / 5.0) * multiplier;
            std::ostringstream oss;
            oss << Color::YELLOW << "\xe2\x9a\xa0 Grid oversupply: " << std::fixed << std::setprecision(0)
                << supply << "% of demand. Frequency rising, reduce output." << Color::RESET << "\n";
            state.addMessage(oss.str());
        }
        state.demandPenalty += penalty;
        state.score = std::max(0, state.score - penalty);
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
