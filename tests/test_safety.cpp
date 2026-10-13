#include "doctest.h"
#include "test_helpers.h"

#include "safety.h"

#include <string>

using namespace testutil;

static bool lastLogContains(const ReactorState& s, const std::string& text) {
    return !s.operatorLog.empty() && s.operatorLog.back().message.find(text) != std::string::npos;
}

TEST_CASE("over-temperature triggers a SCRAM logged as a temperature trip") {
    ReactorState s = makeState();
    s.temperature = s.currentDifficulty.scramTemperature + 1.0;
    SafetySystem::check(s);

    CHECK_FALSE(s.running);
    CHECK_FALSE(s.gameOver);
    CHECK(s.controlRods == doctest::Approx(1.0));
    CHECK(s.scramCount == 1);
    CHECK(lastLogContains(s, "temperature exceeded limit"));
    // The trip does not instantly remove heat
    CHECK(s.temperature == doctest::Approx(s.currentDifficulty.scramTemperature + 1.0));
}

TEST_CASE("overpower triggers a SCRAM logged as a flux trip") {
    ReactorState s = makeState();
    s.neutrons = RC::SCRAM_NEUTRONS + 1.0;
    SafetySystem::check(s);
    CHECK_FALSE(s.running);
    CHECK(lastLogContains(s, "neutron flux exceeded limit"));
}

TEST_CASE("prompt criticality triggers a SCRAM") {
    ReactorState s = makeState();
    s.promptCritical = true;
    SafetySystem::check(s);
    CHECK_FALSE(s.running);
    CHECK(lastLogContains(s, "prompt criticality"));
    CHECK_FALSE(s.promptCritical);
}

TEST_CASE("normal operation does not trip") {
    ReactorState s = makeState();
    run(s, 20);
    CHECK(s.running);
    CHECK(s.scramCount == 0);
}

TEST_CASE("SCRAM costs points but never drives the score negative") {
    ReactorState s = makeState();
    s.score = 100;
    SafetySystem::scram(s, "test");
    CHECK(s.score == 0);
}

TEST_CASE("meltdown ends the game and reset refuses to revive it") {
    ReactorState s = makeState();
    s.temperature = s.currentDifficulty.meltdownTemperature + 1.0;
    SafetySystem::check(s);

    CHECK(s.gameOver);
    CHECK_FALSE(s.running);
    CHECK(s.gameOverReason == "Core meltdown");

    // Cool it right down: still no restart
    s.temperature = 50.0;
    CHECK_FALSE(SafetySystem::reset(s));
    CHECK_FALSE(s.running);
    CHECK(s.gameOver);
}

TEST_CASE("reset is refused while the core is still hot, allowed once cool") {
    ReactorState s = makeState();
    SafetySystem::scram(s, "test");

    s.temperature = s.currentDifficulty.scramTemperature * RC::RESET_TEMP_FRACTION + 10.0;
    CHECK_FALSE(SafetySystem::reset(s));
    CHECK_FALSE(s.running);

    s.temperature = 300.0;
    CHECK(SafetySystem::reset(s));
    CHECK(s.running);
    CHECK(s.awaitingRecovery);
    // Rods stay in after reset; the operator withdraws them deliberately
    CHECK(s.rodTarget == doctest::Approx(1.0));
}

TEST_CASE("reset on a running reactor does nothing") {
    ReactorState s = makeState();
    CHECK_FALSE(SafetySystem::reset(s));
    CHECK(s.running);
}

TEST_CASE("a tripped reactor keeps its rods inserted") {
    ReactorState s = makeState();
    SafetySystem::scram(s, "test");
    s.rodTarget = 0.0;
    step(s);
    CHECK(s.controlRods == doctest::Approx(1.0));
}

TEST_CASE("pulling every rod at power trips the reactor rather than running away") {
    ReactorState s = makeState();
    s.rodTarget = 0.0;
    bool tripped = false;
    for (int t = 0; t < 60 && !tripped; ++t) {
        step(s);
        tripped = !s.running;
    }
    CHECK(tripped);
    CHECK(s.scramCount >= 1);
}
