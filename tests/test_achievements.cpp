#include "doctest.h"
#include "test_helpers.h"

#include "achievements.h"

using namespace testutil;

static bool has(const ReactorState& s, Achievement a) {
    return s.unlockedAchievements.count(a) > 0;
}

TEST_CASE("turn milestones unlock once and report it") {
    ReactorState s = makeState();
    s.turns = 10;
    CHECK(AchievementSystem::check(s));
    CHECK(has(s, Achievement::FIRST_STEPS));
    CHECK(s.sessionAchievements.count(Achievement::FIRST_STEPS) == 1);
    s.clearMessages();
    // Already unlocked: nothing new
    CHECK_FALSE(AchievementSystem::check(s));
}

TEST_CASE("an idle plant does not earn Pressure Perfect") {
    ReactorState s = makeState();
    s.turns = 100;
    s.electricityOutput = 0.0;
    for (int i = 0; i < 60; ++i) AchievementSystem::check(s);
    CHECK_FALSE(has(s, Achievement::PRESSURE_PERFECT));

    s.electricityOutput = 500.0;
    for (int i = 0; i < 60; ++i) AchievementSystem::check(s);
    CHECK(has(s, Achievement::PRESSURE_PERFECT));
}

TEST_CASE("Grid Hero needs supply close to demand, not just above it") {
    ReactorState over = makeState();
    over.gridDemand = 500.0;
    over.electricityOutput = 900.0;
    for (int i = 0; i < 30; ++i) AchievementSystem::check(over);
    CHECK_FALSE(has(over, Achievement::GRID_HERO));

    ReactorState matched = makeState();
    matched.gridDemand = 500.0;
    matched.electricityOutput = 500.0;
    for (int i = 0; i < 30; ++i) AchievementSystem::check(matched);
    CHECK(has(matched, Achievement::GRID_HERO));
}

TEST_CASE("Electrician unlocks after sustained near-rated output") {
    ReactorState s = makeState();
    s.maxTurbineTurns = 10;
    AchievementSystem::check(s);
    CHECK(has(s, Achievement::ELECTRICIAN));
}

TEST_CASE("a steady plant generates power and earns early achievements") {
    ReactorState s = makeState();
    s.rodTarget = 0.5;
    for (int t = 0; t < 60; ++t) {
        CorePhysics::advanceTurn(s);
        s.clearMessages();
    }
    CHECK(s.totalElectricityGenerated > 0.0);
    CHECK(has(s, Achievement::FIRST_STEPS));
}
