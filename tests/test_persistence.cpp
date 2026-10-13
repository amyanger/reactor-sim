#include "doctest.h"
#include "test_helpers.h"

#include "persistence.h"

#include <cstdlib>
#include <string>

using namespace testutil;

static ReactorState playedState() {
    ReactorState s = makeState(Difficulty::HARD);
    s.rodTarget = 0.5;
    for (int t = 0; t < 30; ++t) {
        CorePhysics::advanceTurn(s);
        s.clearMessages();
    }
    s.score = 1234;
    s.addLogEntry("ACTION", "test entry with | pipe");
    return s;
}

// Replace the value of one key=value line in a save
static std::string withValue(std::string text, const std::string& key, const std::string& value) {
    size_t pos = text.find("\n" + key + "=");
    REQUIRE(pos != std::string::npos);
    size_t start = pos + key.size() + 2;
    size_t end = text.find('\n', start);
    return text.replace(start, end - start, value);
}

static void checkRejected(const std::string& text) {
    ReactorState target = makeState(Difficulty::EASY);
    std::string before = PersistenceSystem::serialize(target);
    std::string err;
    CHECK_FALSE(PersistenceSystem::deserialize(text, target, err));
    CHECK_FALSE(err.empty());
    CHECK(PersistenceSystem::serialize(target) == before);
    CHECK(target.currentDifficulty.name == "Easy");
}

TEST_CASE("serialize/deserialize round-trips the full game state") {
    ReactorState original = playedState();
    std::string text = PersistenceSystem::serialize(original);
    CHECK(text.rfind("REACTOR_SAVE v2", 0) == 0);

    ReactorState loaded = makeState(Difficulty::EASY, 7);
    std::string err;
    REQUIRE_MESSAGE(PersistenceSystem::deserialize(text, loaded, err), err);

    CHECK(PersistenceSystem::serialize(loaded) == text);
    CHECK(loaded.currentDifficulty.name == original.currentDifficulty.name);
    CHECK(loaded.turns == original.turns);
    CHECK(loaded.score == 1234);
    CHECK(loaded.temperature == original.temperature);
    CHECK(loaded.neutrons == original.neutrons);
    CHECK(loaded.xenonLevel == original.xenonLevel);
    CHECK(loaded.iodineLevel == original.iodineLevel);
    CHECK(loaded.decayHeat == original.decayHeat);
    CHECK(loaded.rodTarget == original.rodTarget);
    CHECK(loaded.operatorLog.size() == original.operatorLog.size());
}

TEST_CASE("a loaded game continues exactly like the original") {
    ReactorState original = playedState();
    ReactorState loaded = makeState(Difficulty::EASY, 7);
    std::string err;
    REQUIRE(PersistenceSystem::deserialize(PersistenceSystem::serialize(original), loaded, err));

    for (int t = 0; t < 25; ++t) {
        CorePhysics::advanceTurn(original);
        CorePhysics::advanceTurn(loaded);
        original.clearMessages();
        loaded.clearMessages();
    }
    CHECK(PersistenceSystem::serialize(loaded) == PersistenceSystem::serialize(original));
}

TEST_CASE("game-over and tripped states survive a save") {
    ReactorState s = makeState();
    s.gameOver = true;
    s.running = false;
    s.gameOverReason = "Core meltdown";

    ReactorState loaded = makeState();
    std::string err;
    REQUIRE(PersistenceSystem::deserialize(PersistenceSystem::serialize(s), loaded, err));
    CHECK(loaded.gameOver);
    CHECK_FALSE(loaded.running);
    CHECK(loaded.gameOverReason == "Core meltdown");
    CHECK_FALSE(SafetySystem::reset(loaded));
}

TEST_CASE("bad saves are rejected and leave the state untouched") {
    std::string good = PersistenceSystem::serialize(playedState());

    SUBCASE("empty") { checkRejected(""); }
    SUBCASE("truncated") { checkRejected(good.substr(0, good.size() / 2)); }
    SUBCASE("wrong version") { checkRejected("REACTOR_SAVE v9" + good.substr(good.find('\n'))); }
    SUBCASE("old format") { checkRejected("Normal\n500\n0.5\n300\n"); }
    SUBCASE("nan value") { checkRejected(withValue(good, "temperature", "nan")); }
    SUBCASE("infinite value") { checkRejected(withValue(good, "neutrons", "inf")); }
    SUBCASE("out of range") { checkRejected(withValue(good, "coolant", "250")); }
    SUBCASE("not a number") { checkRejected(withValue(good, "fuel", "lots")); }
    SUBCASE("unknown difficulty") { checkRejected(withValue(good, "difficulty", "Impossible")); }
    SUBCASE("bad flag") { checkRejected(withValue(good, "running", "yes")); }
}

TEST_CASE("unknown keys and comments are tolerated") {
    std::string text = PersistenceSystem::serialize(playedState());
    text += "# a comment\nsomeFutureField=1\n\n";
    ReactorState loaded = makeState();
    std::string err;
    CHECK_MESSAGE(PersistenceSystem::deserialize(text, loaded, err), err);
}

TEST_CASE("saveGame and loadGame use the data directory") {
    const char* xdg = std::getenv("XDG_DATA_HOME");
    REQUIRE(xdg != nullptr);
    CHECK(PersistenceSystem::dataPath("save").rfind(xdg, 0) == 0);

    ReactorState original = playedState();
    std::string err;
    REQUIRE_MESSAGE(PersistenceSystem::saveGame(original, err), err);

    ReactorState loaded = makeState(Difficulty::EASY);
    REQUIRE_MESSAGE(PersistenceSystem::loadGame(loaded, err), err);
    CHECK(PersistenceSystem::serialize(loaded) == PersistenceSystem::serialize(original));

    PersistenceSystem::deleteSave();
    ReactorState fresh = makeState();
    CHECK_FALSE(PersistenceSystem::loadGame(fresh, err));
}
