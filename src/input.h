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

private:
    static double parseControlRodInput(const std::string& input, double current);
};
