#include "achievements.h"

#include <sstream>

void AchievementSystem::unlock(ReactorState& state, Achievement ach) {
    if (state.unlockedAchievements.find(ach) == state.unlockedAchievements.end()) {
        state.unlockedAchievements.insert(ach);
        state.sessionAchievements.insert(ach);
        const auto& info = getAchievementInfoTable()[static_cast<int>(ach)];
        std::ostringstream oss;
        oss << "\n" << Color::BG_MAGENTA << Color::WHITE << Color::BOLD
            << " " << info.icon << " ACHIEVEMENT UNLOCKED: " << info.name << "! "
            << Color::RESET << "\n";
        oss << Color::MAGENTA << "   " << info.description << Color::RESET << "\n\n";
        state.addMessage(oss.str());
    }
}

bool AchievementSystem::check(ReactorState& state) {
    size_t before = state.unlockedAchievements.size();

    // Turn-based achievements
    if (state.turns >= 10) unlock(state, Achievement::FIRST_STEPS);
    if (state.turns >= 50) unlock(state, Achievement::SURVIVOR);
    if (state.turns >= 100) unlock(state, Achievement::VETERAN);
    if (state.turns >= 500) unlock(state, Achievement::MARATHON_RUNNER);

    // Power generation achievements
    if (state.totalElectricityGenerated >= 100) unlock(state, Achievement::POWER_PLAYER);
    if (state.totalElectricityGenerated >= 500) unlock(state, Achievement::ENERGY_BARON);

    // SCRAM-related achievements
    if (state.scramRecoveries >= 3) unlock(state, Achievement::COOL_UNDER_PRESSURE);
    if (state.turnsWithoutScram >= 50) unlock(state, Achievement::PERFECT_RUN);

    // Xenon achievement
    if (state.xenonHandledCount >= 5) unlock(state, Achievement::XENON_MASTER);

    // Nightmare achievement
    if (state.currentDifficulty.name == "Nightmare" && state.turns >= 25) {
        unlock(state, Achievement::NIGHTMARE_SURVIVOR);
    }

    // Electrician achievement
    if (state.maxTurbineTurns >= 10) unlock(state, Achievement::ELECTRICIAN);

    // Grid satisfaction tracking
    if (state.demandSatisfaction >= 95.0) {
        state.highSatisfactionTurns++;
        if (state.highSatisfactionTurns >= 20) unlock(state, Achievement::GRID_HERO);
    } else {
        state.highSatisfactionTurns = 0;
    }

    // Storm survival tracking
    if (state.stormsSurvived >= 5) unlock(state, Achievement::WEATHER_WARRIOR);

    // Diesel achievement
    if (state.dieselRuntime >= 50) unlock(state, Achievement::DIESEL_DEPENDENT);

    // Pressure relief tracking
    if (!state.pressureReliefOpen) {
        state.turnsWithoutPressureRelief++;
        if (state.turnsWithoutPressureRelief >= 50 && state.turns >= 50) {
            unlock(state, Achievement::PRESSURE_PERFECT);
        }
    } else {
        state.turnsWithoutPressureRelief = 0;
    }

    // Radiation safety tracking
    if (state.radiationLevel < RC::WARNING_RADIATION) {
        state.safeRadiationTurns++;
        if (state.safeRadiationTurns >= 100) unlock(state, Achievement::RADIATION_SAFE);
    } else {
        state.safeRadiationTurns = 0;
    }

    return state.unlockedAchievements.size() > before;
}
