#pragma once

#include "types.h"
#include "constants.h"

#include <vector>
#include <set>
#include <string>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

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
    double controlRods;
    double temperature;
    double coolant;
    double power;
    double fuel;
    bool running;

    // Xenon poisoning
    double xenonLevel;
    int xenonHandledCount;

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
    int dieselRuntime;

    // Radiation monitoring
    double radiationLevel;
    double totalRadiationExposure;
    int radiationAlarms;

    // Weather system
    Weather currentWeather;
    int weatherDuration;
    int weatherChangeCooldown;

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

    // Constructor
    ReactorState(Difficulty diff)
        : currentDifficulty(getDifficultySettings(diff)),
          neutrons(RC::INITIAL_NEUTRONS),
          controlRods(RC::INITIAL_CONTROL_RODS),
          temperature(RC::INITIAL_TEMPERATURE),
          coolant(RC::INITIAL_COOLANT),
          power(0.0),
          fuel(RC::INITIAL_FUEL),
          running(true),
          xenonLevel(0.0),
          xenonHandledCount(0),
          turbineRPM(0.0),
          steamPressure(0.0),
          electricityOutput(0.0),
          totalElectricityGenerated(0.0),
          turbineOnline(false),
          maxTurbineTurns(0),
          pressureReliefOpen(false),
          pressureWarnings(0),
          eccsAvailable(true),
          eccsCooldownTimer(0),
          dieselFuel(RC::DIESEL_FUEL_CAPACITY),
          dieselRunning(false),
          dieselAutoStart(true),
          dieselRuntime(0),
          radiationLevel(RC::BACKGROUND_RADIATION),
          totalRadiationExposure(0.0),
          radiationAlarms(0),
          currentWeather(Weather::CLEAR),
          weatherDuration(10),
          weatherChangeCooldown(0),
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
          peakTemperature(RC::INITIAL_TEMPERATURE),
          peakPower(0.0),
          peakElectricity(0.0),
          totalPowerGenerated(0.0),
          averageTemperature(0.0),
          temperatureSum(0.0),
          criticalEvents(0),
          lowestCoolant(RC::INITIAL_COOLANT),
          highestXenon(0.0),
          rng(std::chrono::steady_clock::now().time_since_epoch().count()),
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
