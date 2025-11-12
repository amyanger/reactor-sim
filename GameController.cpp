#include "GameController.h"
#include <iostream>

GameController::GameController() : reactor(), physics(), ui(), events() {
}

void GameController::run() {
    ui.displayWelcome();
    
    while (true) {
        if (reactor.isRunning()) {
            ui.displayDashboard(reactor);
            ui.displayStatus(reactor);
            
            std::string input = ui.getUserInput();
            if (!processUserInput(input)) {
                break;  // User quit
            }
            
            if (reactor.isRunning()) {
                PhysicsStatus status = physics.updateReactor(reactor);

                // Display physics warnings
                if (status.lowCoolantWarning) {
                    std::cout << "!!! WARNING: Coolant is critically low! !!!\n";
                }
                if (status.scramTriggered) {
                    std::cout << "\n*** AUTO SCRAM! Emergency shutdown! ***\n";
                }

                events.checkAndApply(reactor);

                if (reactor.checkMeltdown()) {
                    ui.displayMeltdown();
                    break;
                }
            }
        } else {
            if (!handleScramRecovery()) {
                break;  // User quit
            }
        }
    }
    
    std::cout << "\nReactor simulation ended. Stay radioactive! 💥\n";
}

bool GameController::processUserInput(const std::string& input) {
    if (input == "q") {
        return false;
    }
    
    if (input == "r") {
        reactor.refillCoolant();
        std::cout << "Coolant refilled!\n";
        return true;
    }
    
    try {
        double level = ui.parseControlRodInput(input);
        reactor.setControlRods(level);
    } catch (const std::exception& e) {
        std::cout << e.what() << " Control rods unchanged.\n";
    }
    
    return true;
}

bool GameController::handleScramRecovery() {
    std::cout << "Type 'reset' to attempt reactor restart, or 'q' to quit: ";
    std::string input;
    std::getline(std::cin, input);

    if (input == "reset") {
        std::cout << "Reactor restart attempt...\n";
        reactor.reset();
        return true;
    } else if (input == "q") {
        return false;
    }
    return true;
}