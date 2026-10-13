#include "reactor.h"
#include "renderer.h"
#include "input.h"
#include "physics.h"
#include "persistence.h"

#include <iostream>

ReactorSimulator::ReactorSimulator(Difficulty diff)
    : state(diff)
{
    PersistenceSystem::loadHighScore(state);
    PersistenceSystem::loadAchievements(state);
}

void ReactorSimulator::run() {
    Renderer::displayBanner(state);

    while (!state.gameOver) {
        Renderer::displayDashboard(state);
        Renderer::displayScore(state);
        Renderer::displayStatus(state);
        Renderer::displayContextualTip(state);

        InputResult result = InputHandler::handleInput(state);

        if (result == InputResult::QUIT) break;
        if (result == InputResult::CONTINUE) continue;

        if (CorePhysics::advanceTurn(state)) {
            PersistenceSystem::saveAchievements(state);
        }
        Renderer::drainMessages(state);
    }

    if (state.gameOver) {
        std::cout << "\n" << Color::BG_RED << Color::WHITE << Color::BOLD
                  << " GAME OVER: " << state.gameOverReason << " " << Color::RESET << "\n";
    }

    PersistenceSystem::saveHighScore(state);
    // Update highScore in state so displayFinalScore shows the correct value
    if (state.score > state.highScore) {
        state.highScore = state.score;
    }
    Renderer::displayFinalScore(state);
    std::cout << "\n" << Color::MAGENTA << Color::BOLD
              << "Reactor simulation ended. Stay radioactive! \xe2\x98\xa2\xef\xb8\x8f\n"
              << Color::RESET;
}
