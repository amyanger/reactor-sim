#pragma once

#include "reactor_state.h"

#include <string>

class PersistenceSystem {
public:
    // Pure serialization, no file access. deserialize() only modifies `state`
    // when the whole save parses and validates; otherwise it sets `err`.
    static std::string serialize(const ReactorState& state);
    static bool deserialize(const std::string& text, ReactorState& state, std::string& err);

    static bool saveGame(const ReactorState& state, std::string& err);
    static bool loadGame(ReactorState& state, std::string& err);
    static void deleteSave();

    static void loadHighScore(ReactorState& state);
    static void saveHighScore(const ReactorState& state);

    static void loadAchievements(ReactorState& state);
    static void saveAchievements(const ReactorState& state);

    // $XDG_DATA_HOME/reactor-sim, else ~/.local/share/reactor-sim, else ./.reactor-sim
    static std::string dataPath(const std::string& fileName);
};
