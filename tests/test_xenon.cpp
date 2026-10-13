#include "doctest.h"
#include "test_helpers.h"

#include "xenon.h"

using namespace testutil;

// Drive only the iodine/xenon chain at a fixed power level
static void holdPower(ReactorState& s, double fraction, int turns) {
    for (int i = 0; i < turns; ++i) {
        s.power = fraction * RC::RATED_THERMAL_MW;
        XenonSystem::update(s);
        s.clearMessages();
    }
}

TEST_CASE("full-power xenon settles near 50%") {
    ReactorState s = makeState();
    holdPower(s, 1.0, 600);
    CHECK(s.xenonLevel == doctest::Approx(50.0).epsilon(0.1));
    double settled = s.xenonLevel;
    holdPower(s, 1.0, 50);
    CHECK(s.xenonLevel == doctest::Approx(settled).epsilon(0.01));
}

TEST_CASE("no power, no iodine: xenon stays at zero") {
    ReactorState s = makeState();
    holdPower(s, 0.0, 100);
    CHECK(s.xenonLevel == doctest::Approx(0.0));
    CHECK(s.iodineLevel == doctest::Approx(0.0));
}

TEST_CASE("iodine pit: xenon peaks after shutdown from full power, then decays") {
    ReactorState s = makeState();
    holdPower(s, 1.0, 600);
    double equilibrium = s.xenonLevel;

    double peak = equilibrium;
    int peakTurn = 0;
    for (int t = 1; t <= 300; ++t) {
        holdPower(s, 0.0, 1);
        if (s.xenonLevel > peak) {
            peak = s.xenonLevel;
            peakTurn = t;
        }
    }
    CHECK(peak > equilibrium * 1.3);
    CHECK(peakTurn > 5);
    CHECK(peakTurn < 150);
    // Long after shutdown it has decayed well below the peak
    CHECK(s.xenonLevel < 0.5 * peak);
}

TEST_CASE("xenon stays within its bounds") {
    ReactorState s = makeState();
    s.iodineLevel = 1e4;
    holdPower(s, 0.0, 50);
    CHECK(s.xenonLevel <= RC::MAX_XENON);
    CHECK(s.xenonLevel >= 0.0);
}

TEST_CASE("a transient counts as handled only once xenon falls back while running") {
    ReactorState s = makeState();
    s.xenonLevel = RC::XENON_HIGH + 5.0;
    s.power = 0.0;
    XenonSystem::update(s);
    CHECK(s.xenonTransientActive);
    CHECK(s.xenonHandledCount == 0);

    s.xenonLevel = RC::XENON_RECOVERED - 5.0;
    s.running = false;
    XenonSystem::update(s);
    CHECK(s.xenonHandledCount == 0);

    s.running = true;
    XenonSystem::update(s);
    CHECK(s.xenonHandledCount == 1);
    CHECK_FALSE(s.xenonTransientActive);
}
