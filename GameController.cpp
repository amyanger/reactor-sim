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
                physics.updateReactor(reactor);
                events.checkAndApply(reactor);
                
                if (reactor.checkMeltdown()) {
                    ui.displayMeltdown();
                    break;
                }
            }
        } else {
            handleScramRecovery();
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

void GameController::handleScramRecovery() {
    std::cout << "Type 'reset' to attempt reactor restart, or 'q' to quit: ";
    std::string input;
    std::getline(std::cin, input);
    
    if (input == "reset") {
        std::cout << "Reactor restart attempt...\n";
        reactor.reset();
    } else if (input == "q") {
        std::exit(0);
    }
}