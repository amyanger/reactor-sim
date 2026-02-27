#pragma once

#include "reactor_state.h"

#include <string>

class Renderer {
public:
    static void displayDashboard(const ReactorState& state);
    static void displayScore(const ReactorState& state);
    static void displayStatus(const ReactorState& state);
    static void displayHelp(const ReactorState& state);
    static void displayAchievements(const ReactorState& state);
    static void displayLog(const ReactorState& state);
    static void displayStatistics(const ReactorState& state);
    static void displayFinalScore(ReactorState& state);
    static void displayContextualTip(ReactorState& state);
    static void displayBanner(const ReactorState& state);
    static void drainMessages(ReactorState& state);

private:
    static std::string getBarColor(double value, double max, bool inverse = false);
    static void printBar(const std::string& label, double value, double max, int width = 18, bool inverse = false);
};
