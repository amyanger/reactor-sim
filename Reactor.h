#ifndef REACTOR_H
#define REACTOR_H

class Reactor {
public:
    Reactor();
    
    // Getters
    double getNeutrons() const { return neutrons; }
    double getControlRods() const { return controlRods; }
    double getTemperature() const { return temperature; }
    double getCoolant() const { return coolant; }
    double getPower() const { return power; }
    double getFuel() const { return fuel; }
    bool isRunning() const { return running; }
    
    // Control operations
    void setControlRods(double level);
    void refillCoolant();
    void consumeFuel(double amount);
    
    // State updates
    void updateNeutrons(double factor);
    void updateTemperature(double delta);
    void updateCoolant(double delta);
    void setPower(double p);
    
    // Safety systems
    void scram();
    void reset();
    bool checkMeltdown() const;
    
private:
    double neutrons;
    double controlRods;  // 0.0 to 1.0
    double temperature;  // degrees C
    double coolant;      // percentage
    double power;        // current output
    double fuel;         // percentage
    bool running;
};

#endif