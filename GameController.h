#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "Reactor.h"
#include "PhysicsEngine.h"
#include "UI.h"
#include "RandomEvents.h"

class GameController {
public:
    GameController();
    void run();
    
private:
    Reactor reactor;
    PhysicsEngine physics;
    UI ui;
    RandomEvents events;
    
    bool processUserInput(const std::string& input);
    bool handleScramRecovery();
};

#endif