#include "doctest.h"
#include "test_helpers.h"

#include "grid.h"
#include "scoring.h"

using namespace testutil;

// Score change for one grid + scoring update at a given output, as a multiple of demand
static int scoreForSupply(double supplyFraction) {
    ReactorState probe = makeState();
    probe.electricityOutput = 0.0;
    GridSystem::update(probe);
    double demand = probe.gridDemand;  // same seed gives the same demand below

    ReactorState s = makeState();
    s.electricityOutput = supplyFraction * demand;
    int before = s.score;
    GridSystem::update(s);
    ScoringSystem::update(s);
    REQUIRE(s.gridDemand == doctest::Approx(demand));
    return s.score - before;
}

TEST_CASE("no electricity, no points") {
    ReactorState s = makeState();
    s.electricityOutput = 0.0;
    ScoringSystem::update(s);
    CHECK(s.score == 0);
}

TEST_CASE("an idle shut-down plant never gains score") {
    ReactorState s = makeState();
    s.score = 500;
    s.rodTarget = 1.0;
    s.turbineOnline = false;
    int prev = s.score;
    for (int t = 0; t < 100; ++t) {
        CorePhysics::advanceTurn(s);
        s.clearMessages();
        CHECK(s.score <= prev);
        prev = s.score;
    }
}

TEST_CASE("matching demand beats both undersupply and oversupply") {
    int matched = scoreForSupply(1.0);
    int under = scoreForSupply(0.3);
    int over = scoreForSupply(1.6);
    CHECK(matched > 0);
    CHECK(matched > under);
    CHECK(matched > over);
}

TEST_CASE("oversupply is penalized and recorded") {
    ReactorState s = makeState();
    s.electricityOutput = 5000.0;
    GridSystem::update(s);
    CHECK(s.demandPenalty > 0);
}

TEST_CASE("harder difficulties pay more for the same delivery") {
    ReactorState easy = makeState(Difficulty::EASY);
    ReactorState hard = makeState(Difficulty::NIGHTMARE);
    easy.electricityOutput = hard.electricityOutput = 400.0;
    easy.gridDemand = hard.gridDemand = 800.0;
    ScoringSystem::update(easy);
    ScoringSystem::update(hard);
    CHECK(hard.score > easy.score);
}

TEST_CASE("statistics track peaks and the running average") {
    ReactorState s = makeState();
    run(s, 10);
    CHECK(s.turns == 10);
    CHECK(s.peakTemperature >= s.temperature);
    CHECK(s.averageTemperature == doctest::Approx(s.temperatureSum / s.turns));
}
