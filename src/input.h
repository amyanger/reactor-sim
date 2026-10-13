#pragma once

#include "reactor_state.h"

#include <optional>
#include <string>

enum class InputResult {
    CONTINUE,
    ADVANCE_TURN,
    QUIT,
    // View-only commands; the caller renders the matching screen
    SHOW_HELP,
    SHOW_ACHIEVEMENTS,
    SHOW_STATS,
    SHOW_LOG
};

// Trim whitespace and lowercase a command
std::string normalizeCommand(const std::string& input);

// Parse "0".."100" (optional trailing '%') into a 0..1 rod fraction.
// Returns nullopt for anything that isn't a finite number.
std::optional<double> parseRodPercent(const std::string& input);

class InputHandler {
public:
    // Prompt, read a line from stdin, apply it, and render any output
    static InputResult handleInput(ReactorState& state);

    // Apply one command to the state. No console I/O: feedback goes to
    // state.messages and view commands come back as SHOW_* results.
    static InputResult applyCommand(ReactorState& state, const std::string& rawInput);
};
