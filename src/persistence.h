#pragma once

#include "reactor_state.h"

class PersistenceSystem {
public:
    static bool saveGame(const ReactorState& state);
    static bool loadGame(ReactorState& state);
    static void deleteSave();

    static void loadHighScore(ReactorState& state);
    static void saveHighScore(const ReactorState& state);

    static void loadAchievements(ReactorState& state);
    static void saveAchievements(const ReactorState& state);
};
