#include "doctest.h"
#include "test_helpers.h"

#include "physics.h"
#include "turbine.h"

using namespace testutil;

TEST_CASE("rod worth is an increasing S-curve from 0 to full worth") {
    CHECK(CorePhysics::rodWorth(0.0) == doctest::Approx(0.0));
    CHECK(CorePhysics::rodWorth(1.0) == doctest::Approx(RC::ROD_WORTH));
    double prev = -1.0;
    for (int i = 0; i <= 100; ++i) {
        double w = CorePhysics::rodWorth(i / 100.0);
        CHECK(w >= prev);
        prev = w;
    }
    // Middle of the travel is worth more per percent than the ends
    double mid = CorePhysics::rodWorth(0.55) - CorePhysics::rodWorth(0.45);
    double end = CorePhysics::rodWorth(0.10) - CorePhysics::rodWorth(0.0);
    CHECK(mid > end);
    // Out-of-range input is clamped
    CHECK(CorePhysics::rodWorth(-1.0) == doctest::Approx(0.0));
    CHECK(CorePhysics::rodWorth(2.0) == doctest::Approx(RC::ROD_WORTH));
}

TEST_CASE("reactivity falls with burnup, xenon, temperature and rod insertion") {
    ReactorState s = makeState();
    double base = CorePhysics::reactivity(s);

    ReactorState burned = s;
    burned.fuel = 50.0;
    CHECK(CorePhysics::reactivity(burned) < base);

    ReactorState poisoned = s;
    poisoned.xenonLevel = 60.0;
    CHECK(CorePhysics::reactivity(poisoned) < base);

    ReactorState hot = s;
    hot.temperature += 200.0;
    CHECK(CorePhysics::reactivity(hot) < base);

    ReactorState inserted = s;
    inserted.controlRods = 0.9;
    CHECK(CorePhysics::reactivity(inserted) < base);

    // Fully inserted rods hold a cold, fresh core subcritical
    ReactorState cold = s;
    cold.controlRods = 1.0;
    cold.temperature = RC::TEMP_REFERENCE;
    CHECK(CorePhysics::reactivity(cold) < 0.0);
}

TEST_CASE("core state stays finite and in range over long runs at any rod setting") {
    for (double rods : {0.0, 0.3, 0.45, 0.62, 0.8, 1.0}) {
        for (Difficulty d : {Difficulty::EASY, Difficulty::NORMAL, Difficulty::NIGHTMARE}) {
            ReactorState s = makeState(d);
            s.rodTarget = rods;
            for (int t = 0; t < 400 && !s.gameOver; ++t) {
                CorePhysics::advanceTurn(s);
                s.clearMessages();
                REQUIRE(finiteCore(s));
                CHECK(s.neutrons >= 0.0);
                CHECK(s.neutrons <= RC::MAX_NEUTRONS);
                CHECK(s.temperature >= 0.0);
                CHECK(s.coolant >= 0.0);
                CHECK(s.coolant <= 100.0);
                CHECK(s.fuel >= 0.0);
                CHECK(s.xenonLevel >= 0.0);
                CHECK(s.xenonLevel <= RC::MAX_XENON);
                CHECK(s.controlRods >= 0.0);
                CHECK(s.controlRods <= 1.0);
            }
        }
    }
}

TEST_CASE("rods move toward the target at limited speed") {
    ReactorState s = makeState();
    double start = s.controlRods;
    s.rodTarget = 0.0;
    step(s);
    CHECK(s.controlRods == doctest::Approx(start - RC::ROD_SPEED));
}

TEST_CASE("the initial rod position is close to equilibrium") {
    ReactorState s = makeState();
    double p0 = s.power;
    run(s, 10);
    CHECK(s.running);
    CHECK(s.power == doctest::Approx(p0).epsilon(0.25));
    CHECK(s.temperature == doctest::Approx(RC::INITIAL_TEMPERATURE).epsilon(0.25));
}

TEST_CASE("fully inserted rods shut the core down") {
    ReactorState s = makeState();
    s.rodTarget = 1.0;
    run(s, 40);
    CHECK(s.reactivity < 0.0);
    CHECK(s.power < 0.05 * RC::RATED_THERMAL_MW);
}

TEST_CASE("a shut-down core can be restarted from the neutron source") {
    ReactorState s = makeState();
    s.rodTarget = 1.0;
    run(s, 60);
    double shutdownPower = s.power;
    REQUIRE(shutdownPower < 0.05 * RC::RATED_THERMAL_MW);

    s.rodTarget = 0.5;
    double peak = 0.0;
    for (int t = 0; t < 120 && s.running; ++t) {
        step(s);
        if (s.power > peak) peak = s.power;
    }
    CHECK(peak > 10.0 * shutdownPower);
    CHECK(peak > 0.2 * RC::RATED_THERMAL_MW);
}

TEST_CASE("burnup forces the operator to withdraw rods to stay critical") {
    // Find the rod position that makes reactivity zero for a given fuel level
    auto criticalRods = [](double fuel) {
        ReactorState s = makeState();
        s.fuel = fuel;
        s.xenonLevel = 50.0;
        s.temperature = 550.0;
        double lo = 0.0, hi = 1.0;
        for (int i = 0; i < 60; ++i) {
            s.controlRods = 0.5 * (lo + hi);
            if (CorePhysics::reactivity(s) > 0.0) lo = s.controlRods;
            else hi = s.controlRods;
        }
        return s.controlRods;
    };
    CHECK(criticalRods(60.0) < criticalRods(100.0) - 0.02);
}

TEST_CASE("decay heat persists after a SCRAM and keeps the core warm") {
    ReactorState s = makeState();
    s.rodTarget = 0.55;
    run(s, 60);
    REQUIRE(s.running);
    REQUIRE(s.decayHeat > 0.0);

    SafetySystem::scram(s, "test");
    double tempAtTrip = s.temperature;
    double decayAtTrip = s.decayHeat;
    step(s);

    CHECK_FALSE(s.running);
    CHECK(s.decayHeat > 0.5 * decayAtTrip);
    // No instant temperature drop on SCRAM: the core cools gradually
    CHECK(s.temperature > tempAtTrip - 100.0);
    run(s, 50);
    CHECK(s.decayHeat < decayAtTrip);
    CHECK(s.decayHeat > 0.0);
}

TEST_CASE("turbine efficiency peaks at the optimal steam temperature") {
    double peak = TurbineSystem::efficiency(RC::OPTIMAL_STEAM_TEMP);
    CHECK(peak == doctest::Approx(RC::PEAK_EFFICIENCY));
    CHECK(TurbineSystem::efficiency(RC::MIN_TURBINE_TEMP - 1.0) == doctest::Approx(0.0));
    CHECK(TurbineSystem::efficiency(400.0) < peak);
    CHECK(TurbineSystem::efficiency(700.0) < peak);
}

TEST_CASE("the generator can reach the Electrician threshold on Normal") {
    DifficultySettings normal = getDifficultySettings(Difficulty::NORMAL);
    // Demand peaks at 1000 MW; rated output must make high-demand turns winnable
    CHECK(TurbineSystem::maxOutput(normal) > 900.0);
}
