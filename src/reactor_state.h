#pragma once

#include "types.h"
#include "constants.h"

#include <vector>
#include <set>
#include <string>
#include <random>

struct GameMessage {
    std::string text;  // Pre-formatted with ANSI codes
    bool triggerSound;
    bool triggerAlert;

    GameMessage(const std::string& t, bool sound = false, bool alert = false)
        : text(t), triggerSound(sound), triggerAlert(alert) {}
};

struct LogEntry {
    int turn;
    std::string type;   // ACTION, EVENT, WARNING, CRITICAL
    std::string message;
};

struct ReactorState {
    // Difficulty
    DifficultySettings currentDifficulty;

    // Core state
    double neutrons;
    double controlRods;      // actual insertion, 0..1
    double rodTarget;        // operator setpoint; rods travel toward it at ROD_SPEED
    double reactivity;       // net reactivity in dollars at end of last turn
    double temperature;
    double coolant;
    double power;            // fission power, MW thermal
    double decayHeat;        // MW thermal, persists after shutdown
    double heatRemoved;      // MW thermal carried away by the coolant last turn
    double fuel;
    bool running;            // false while SCRAMmed (rods locked in)
    bool promptCritical;     // set by physics this turn, consumed by the safety check

    // Game over (meltdown or containment failure): cannot be reset
    bool gameOver;
    std::string gameOverReason;

    // Xenon poisoning
    double xenonLevel;
    double iodineLevel;
    int xenonHandledCount;
    bool xenonTransientActive;

    // Turbine system
    double turbineRPM;
    double steamPressure;
    double electricityOutput;
    double totalElectricityGenerated;
    bool turbineOnline;
    int maxTurbineTurns;
    bool pressureReliefOpen;
    int pressureWarnings;

    // Emergency Cooling System
    bool eccsAvailable;
    int eccsCooldownTimer;

    // Diesel Generator System
    double dieselFuel;
    bool dieselRunning;
    bool dieselAutoStart;
    bool dieselStartedByAuto;
    int dieselRuntime;

    // Radiation monitoring
    double radiationLevel;
    double totalRadiationExposure;
    int radiationAlarms;

    // Weather system
    Weather currentWeather;
    int weatherDuration;

    // Power grid demand system
    double gridDemand;
    double demandSatisfaction;
    int demandBonus;
    int demandPenalty;

    // Tips system
    bool tipsEnabled;
    int lastTipTurn;

    // Achievement tracking
    int highSatisfactionTurns;
    int stormsSurvived;
    int turnsWithoutPressureRelief;
    int safeRadiationTurns;

    // Containment system
    double containmentIntegrity;
    bool containmentBreach;

    // Scoring system
    int score;
    int turns;
    int scramCount;
    int highScore;
    int eventsExperienced;
    int turnsWithoutScram;
    int scramRecoveries;
    bool awaitingRecovery;   // reset after a SCRAM, power not yet restored

    // Statistics tracking
    double peakTemperature;
    double peakPower;
    double peakElectricity;
    double totalPowerGenerated;
    double averageTemperature;
    double temperatureSum;
    int criticalEvents;
    double lowestCoolant;
    double highestXenon;

    // Operator event log
    std::vector<LogEntry> operatorLog;

    // Achievements
    std::set<Achievement> unlockedAchievements;
    std::set<Achievement> sessionAchievements;

    // Modern random number generation
    std::mt19937 rng;

    // Sound/UI flags
    bool soundEnabled;
    bool paused;

    // Message queue — subsystems push here, renderer drains
    std::vector<GameMessage> messages;

    // Pass a fixed seed for reproducible runs (tests, replays)
    explicit ReactorState(Difficulty diff, unsigned int seed = std::random_device{}())
        : currentDifficulty(getDifficultySettings(diff)),
          neutrons(RC::INITIAL_NEUTRONS),
          controlRods(RC::INITIAL_CONTROL_RODS),
          rodTarget(RC::INITIAL_CONTROL_RODS),
          reactivity(0.0),
          temperature(RC::INITIAL_TEMPERATURE),
          coolant(RC::INITIAL_COOLANT),
          power(RC::INITIAL_NEUTRONS * RC::NEUTRON_TO_POWER),
          decayHeat(0.0),
          heatRemoved(0.0),
          fuel(RC::INITIAL_FUEL),
          running(true),
          promptCritical(false),
          gameOver(false),
          xenonLevel(0.0),
          iodineLevel(0.0),
          xenonHandledCount(0),
          xenonTransientActive(false),
          turbineRPM(RC::MAX_TURBINE_RPM),
          steamPressure(0.0),
          electricityOutput(0.0),
          totalElectricityGenerated(0.0),
          turbineOnline(true),
          maxTurbineTurns(0),
          pressureReliefOpen(false),
          pressureWarnings(0),
          eccsAvailable(true),
          eccsCooldownTimer(0),
          dieselFuel(RC::DIESEL_FUEL_CAPACITY),
          dieselRunning(false),
          dieselAutoStart(true),
          dieselStartedByAuto(false),
          dieselRuntime(0),
          radiationLevel(RC::BACKGROUND_RADIATION),
          totalRadiationExposure(0.0),
          radiationAlarms(0),
          currentWeather(Weather::CLEAR),
          weatherDuration(10),
          gridDemand(500.0),
          demandSatisfaction(0.0),
          demandBonus(0),
          demandPenalty(0),
          tipsEnabled(true),
          lastTipTurn(-10),
          highSatisfactionTurns(0),
          stormsSurvived(0),
          turnsWithoutPressureRelief(0),
          safeRadiationTurns(0),
          containmentIntegrity(RC::MAX_CONTAINMENT),
          containmentBreach(false),
          score(0),
          turns(0),
          scramCount(0),
          highScore(0),
          eventsExperienced(0),
          turnsWithoutScram(0),
          scramRecoveries(0),
          awaitingRecovery(false),
          peakTemperature(RC::INITIAL_TEMPERATURE),
          peakPower(0.0),
          peakElectricity(0.0),
          totalPowerGenerated(0.0),
          averageTemperature(0.0),
          temperatureSum(0.0),
          criticalEvents(0),
          lowestCoolant(RC::INITIAL_COOLANT),
          highestXenon(0.0),
          rng(seed),
          soundEnabled(true),
          paused(false) {}

    // Message helpers
    void addMessage(const std::string& text) {
        messages.emplace_back(text);
    }

    void addSoundMessage(const std::string& text) {
        messages.emplace_back(text, true, false);
    }

    void addAlertMessage(const std::string& text) {
        messages.emplace_back(text, false, true);
    }

    void clearMessages() {
        messages.clear();
    }

    // Log helper
    void addLogEntry(const std::string& type, const std::string& message) {
        LogEntry entry{turns, type, message};
        operatorLog.push_back(entry);
        if (static_cast<int>(operatorLog.size()) > RC::MAX_LOG_ENTRIES) {
            operatorLog.erase(operatorLog.begin());
        }
    }
};
