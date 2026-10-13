#include "doctest.h"
#include "test_helpers.h"

#include "input.h"

using namespace testutil;

TEST_CASE("normalizeCommand trims whitespace and lowercases") {
    CHECK(normalizeCommand(" Q ") == "q");
    CHECK(normalizeCommand("\tRESET\r\n") == "reset");
    CHECK(normalizeCommand("   ") == "");
    CHECK(normalizeCommand("") == "");
    CHECK(normalizeCommand("Df") == "df");
}

TEST_CASE("parseRodPercent accepts plain and percent numbers") {
    REQUIRE(parseRodPercent("60").has_value());
    CHECK(*parseRodPercent("60") == doctest::Approx(0.6));
    CHECK(*parseRodPercent("60%") == doctest::Approx(0.6));
    CHECK(*parseRodPercent("0") == doctest::Approx(0.0));
    CHECK(*parseRodPercent("100") == doctest::Approx(1.0));
    CHECK(*parseRodPercent("12.5") == doctest::Approx(0.125));
}

TEST_CASE("parseRodPercent clamps out-of-range values") {
    CHECK(*parseRodPercent("150") == doctest::Approx(1.0));
    CHECK(*parseRodPercent("-20") == doctest::Approx(0.0));
}

TEST_CASE("parseRodPercent rejects junk and non-finite values") {
    CHECK_FALSE(parseRodPercent("nan").has_value());
    CHECK_FALSE(parseRodPercent("inf").has_value());
    CHECK_FALSE(parseRodPercent("-inf").has_value());
    CHECK_FALSE(parseRodPercent("5x").has_value());
    CHECK_FALSE(parseRodPercent("60%%").has_value());
    CHECK_FALSE(parseRodPercent("abc").has_value());
    CHECK_FALSE(parseRodPercent("").has_value());
    CHECK_FALSE(parseRodPercent("1e999").has_value());
}

TEST_CASE("applyCommand handles quit and view commands regardless of case") {
    ReactorState s = makeState();
    CHECK(InputHandler::applyCommand(s, " Q ") == InputResult::QUIT);
    CHECK(InputHandler::applyCommand(s, "quit") == InputResult::QUIT);
    CHECK(InputHandler::applyCommand(s, "H") == InputResult::SHOW_HELP);
    CHECK(InputHandler::applyCommand(s, "stats") == InputResult::SHOW_STATS);
    CHECK(InputHandler::applyCommand(s, "log") == InputResult::SHOW_LOG);
    CHECK(InputHandler::applyCommand(s, "a") == InputResult::SHOW_ACHIEVEMENTS);
}

TEST_CASE("a rod setting sets the target and advances the turn") {
    ReactorState s = makeState();
    CHECK(InputHandler::applyCommand(s, "40%") == InputResult::ADVANCE_TURN);
    CHECK(s.rodTarget == doctest::Approx(0.4));
    CHECK(InputHandler::applyCommand(s, "") == InputResult::ADVANCE_TURN);
}

TEST_CASE("unknown input does not advance the turn") {
    ReactorState s = makeState();
    double target = s.rodTarget;
    CHECK(InputHandler::applyCommand(s, "5x") == InputResult::CONTINUE);
    CHECK(InputHandler::applyCommand(s, "nan") == InputResult::CONTINUE);
    CHECK(s.rodTarget == doctest::Approx(target));
    CHECK_FALSE(s.messages.empty());
}

TEST_CASE("pause blocks every plant-changing command") {
    ReactorState s = makeState();
    CHECK(InputHandler::applyCommand(s, "p") == InputResult::CONTINUE);
    REQUIRE(s.paused);

    ReactorState before = s;
    for (const char* cmd : {"", "40", "r", "df", "t", "e", "d", "da", "reset"}) {
        CAPTURE(cmd);
        CHECK(InputHandler::applyCommand(s, cmd) == InputResult::CONTINUE);
    }
    CHECK(s.rodTarget == doctest::Approx(before.rodTarget));
    CHECK(s.coolant == doctest::Approx(before.coolant));
    CHECK(s.turbineOnline == before.turbineOnline);
    CHECK(s.eccsAvailable == before.eccsAvailable);
    CHECK(s.dieselRunning == before.dieselRunning);
    CHECK(s.dieselAutoStart == before.dieselAutoStart);
    CHECK(s.score == before.score);

    // View commands still work while paused, and 'p' resumes
    CHECK(InputHandler::applyCommand(s, "stats") == InputResult::SHOW_STATS);
    InputHandler::applyCommand(s, "pause");
    CHECK_FALSE(s.paused);
    CHECK(InputHandler::applyCommand(s, "40") == InputResult::ADVANCE_TURN);
}

TEST_CASE("rods cannot be moved while tripped") {
    ReactorState s = makeState();
    SafetySystem::scram(s, "test");
    CHECK(InputHandler::applyCommand(s, "20") == InputResult::CONTINUE);
    CHECK(s.rodTarget == doctest::Approx(1.0));
}

TEST_CASE("coolant refill costs a turn and points") {
    ReactorState s = makeState();
    s.coolant = 30.0;
    s.score = 500;
    CHECK(InputHandler::applyCommand(s, "r") == InputResult::ADVANCE_TURN);
    CHECK(s.coolant > 30.0);
    CHECK(s.score < 500);
}
