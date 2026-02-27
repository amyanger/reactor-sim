#pragma once

#include "reactor_state.h"

class ReactorSimulator {
public:
    explicit ReactorSimulator(Difficulty diff);
    void run();

private:
    ReactorState state;
};
