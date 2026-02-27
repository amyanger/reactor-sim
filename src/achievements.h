#pragma once

#include "reactor_state.h"

class AchievementSystem {
public:
    // Check all achievement conditions; returns true if any new ones unlocked
    static bool check(ReactorState& state);

    // Unlock a specific achievement (no-op if already unlocked)
    static void unlock(ReactorState& state, Achievement ach);
};
