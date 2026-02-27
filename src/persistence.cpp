#include "persistence.h"

#include <fstream>
#include <string>
#include <cstdio>

bool PersistenceSystem::saveGame(const ReactorState& state) {
    std::ofstream file(RC::SAVE_FILE);
    if (!file.is_open()) return false;

    file << state.currentDifficulty.name << "\n";
    file << state.neutrons << " " << state.controlRods << " " << state.temperature << "\n";
    file << state.coolant << " " << state.power << " " << state.fuel << "\n";
    file << state.xenonLevel << " " << state.xenonHandledCount << "\n";
    file << state.turbineRPM << " " << state.steamPressure << " " << state.electricityOutput << "\n";
    file << state.totalElectricityGenerated << " " << state.turbineOnline << " " << state.maxTurbineTurns << "\n";
    file << state.eccsAvailable << " " << state.eccsCooldownTimer << "\n";
    file << state.score << " " << state.turns << " " << state.scramCount << "\n";
    file << state.eventsExperienced << " " << state.turnsWithoutScram << " " << state.scramRecoveries << "\n";

    file.close();
    return true;
}

bool PersistenceSystem::loadGame(ReactorState& state) {
    std::ifstream file(RC::SAVE_FILE);
    if (!file.is_open()) return false;

    std::string diffName;
    file >> diffName;

    file >> state.neutrons >> state.controlRods >> state.temperature;
    file >> state.coolant >> state.power >> state.fuel;
    file >> state.xenonLevel >> state.xenonHandledCount;
    file >> state.turbineRPM >> state.steamPressure >> state.electricityOutput;
    file >> state.totalElectricityGenerated >> state.turbineOnline >> state.maxTurbineTurns;
    file >> state.eccsAvailable >> state.eccsCooldownTimer;
    file >> state.score >> state.turns >> state.scramCount;
    file >> state.eventsExperienced >> state.turnsWithoutScram >> state.scramRecoveries;

    file.close();
    state.running = true;
    return true;
}

void PersistenceSystem::deleteSave() {
    std::remove(RC::SAVE_FILE);
}

void PersistenceSystem::loadHighScore(ReactorState& state) {
    std::string filename = std::string(RC::HIGH_SCORE_FILE) + "_" + state.currentDifficulty.name;
    std::ifstream file(filename);
    if (file.is_open()) {
        file >> state.highScore;
        file.close();
    }
}

void PersistenceSystem::saveHighScore(const ReactorState& state) {
    if (state.score > state.highScore) {
        std::string filename = std::string(RC::HIGH_SCORE_FILE) + "_" + state.currentDifficulty.name;
        std::ofstream file(filename);
        if (file.is_open()) {
            file << state.score;
            file.close();
        }
    }
}

void PersistenceSystem::loadAchievements(ReactorState& state) {
    std::ifstream file(RC::ACHIEVEMENTS_FILE);
    if (file.is_open()) {
        int achievementId;
        while (file >> achievementId) {
            if (achievementId >= 0 && achievementId < static_cast<int>(Achievement::ACHIEVEMENT_COUNT)) {
                state.unlockedAchievements.insert(static_cast<Achievement>(achievementId));
            }
        }
        file.close();
    }
}

void PersistenceSystem::saveAchievements(const ReactorState& state) {
    std::ofstream file(RC::ACHIEVEMENTS_FILE);
    if (file.is_open()) {
        for (const auto& ach : state.unlockedAchievements) {
            file << static_cast<int>(ach) << "\n";
        }
        file.close();
    }
}
