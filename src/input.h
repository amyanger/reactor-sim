#pragma once

#include "reactor_state.h"

#include <string>

enum class InputResult {
    CONTINUE,
    ADVANCE_TURN,
    QUIT
};

class InputHandler {
public:
    static InputResult handleInput(ReactorState& state);

    // Trim whitespace and lowercase a command
    static std::string normalize(const std::string& input);

    // Parse "0".."100" (optional trailing '%') into a 0..1 rod fraction.
    // Returns false for anything that isn't a finite number.
    static bool parseControlRodInput(const std::string& input, double& out);
};
