#ifndef UI_H
#define UI_H

#include "Reactor.h"
#include <string>

class UI {
public:
    void displayWelcome() const;
    void displayDashboard(const Reactor& reactor) const;
    void displayStatus(const Reactor& reactor) const;
    void displayMeltdown() const;
    
    std::string getUserInput() const;
    double parseControlRodInput(const std::string& input) const;
    
private:
    void printBar(const std::string& label, double value, double max, int width = 20) const;
};

#endif