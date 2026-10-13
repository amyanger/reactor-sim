#pragma once

#include <iostream>
#include <string>

// Sound effects using terminal bell
namespace Sound {
    inline void beep() { std::cout << '\a' << std::flush; }
    inline void alert() { for (int i = 0; i < 3; ++i) { std::cout << '\a' << std::flush; } }
}

// ANSI Color Codes — constexpr const char* to avoid ODR issues across TUs
namespace Color {
    constexpr const char* RESET      = "\033[0m";
    constexpr const char* RED        = "\033[31m";
    constexpr const char* GREEN      = "\033[32m";
    constexpr const char* YELLOW     = "\033[33m";
    constexpr const char* BLUE       = "\033[34m";
    constexpr const char* MAGENTA    = "\033[35m";
    constexpr const char* CYAN       = "\033[36m";
    constexpr const char* WHITE      = "\033[37m";
    constexpr const char* BOLD       = "\033[1m";
    constexpr const char* DIM        = "\033[2m";
    constexpr const char* BG_RED     = "\033[41m";
    constexpr const char* BG_GREEN   = "\033[42m";
    constexpr const char* BG_YELLOW  = "\033[43m";
    constexpr const char* BG_BLUE    = "\033[44m";
    constexpr const char* BG_MAGENTA = "\033[45m";
}

enum class Difficulty {
    EASY,
    NORMAL,
    HARD,
    NIGHTMARE
};

enum class Weather {
    CLEAR,
    CLOUDY,
    RAIN,
    STORM,
    HEATWAVE,
    COLD_SNAP
};

struct WeatherInfo {
    std::string name;
    std::string icon;
    double coolingModifier;
    double eventModifier;
    std::string description;
};

inline WeatherInfo getWeatherInfo(Weather w) {
    switch (w) {
        case Weather::CLEAR:     return {"Clear",     "\xe2\x98\x80\xef\xb8\x8f", 1.0, 1.0, "Optimal conditions"};
        case Weather::CLOUDY:    return {"Cloudy",    "\xe2\x98\x81\xef\xb8\x8f", 1.1, 1.0, "Slightly improved cooling"};
        case Weather::RAIN:      return {"Rain",      "\xf0\x9f\x8c\xa7\xef\xb8\x8f", 1.3, 0.9, "Enhanced cooling, fewer events"};
        case Weather::STORM:     return {"Storm",     "\xe2\x9b\x88\xef\xb8\x8f", 0.95, 1.5, "Risk of lightning damage"};
        case Weather::HEATWAVE:  return {"Heatwave",  "\xf0\x9f\x94\xa5", 0.6, 1.2, "Reduced cooling efficiency"};
        case Weather::COLD_SNAP: return {"Cold Snap", "\xe2\x9d\x84\xef\xb8\x8f", 1.5, 0.8, "Excellent cooling"};
        default:                 return {"Clear",     "\xe2\x98\x80\xef\xb8\x8f", 1.0, 1.0, "Optimal conditions"};
    }
}

enum class Achievement {
    FIRST_STEPS,
    SURVIVOR,
    VETERAN,
    POWER_PLAYER,
    ENERGY_BARON,
    COOL_UNDER_PRESSURE,
    PERFECT_RUN,
    XENON_MASTER,
    NIGHTMARE_SURVIVOR,
    ELECTRICIAN,
    GRID_HERO,
    WEATHER_WARRIOR,
    DIESEL_DEPENDENT,
    PRESSURE_PERFECT,
    RADIATION_SAFE,
    MARATHON_RUNNER,
    ACHIEVEMENT_COUNT
};

struct AchievementInfo {
    std::string name;
    std::string description;
    std::string icon;
};

// ODR-safe via inline function + static local (C++11 compatible)
inline const AchievementInfo* getAchievementInfoTable() {
    static const AchievementInfo table[] = {
        {"First Steps",         "Complete 10 turns",              "\xf0\x9f\x8e\xaf"},
        {"Survivor",            "Complete 50 turns",              "\xf0\x9f\x8f\x85"},
        {"Veteran Operator",    "Complete 100 turns",             "\xf0\x9f\x8e\x96"},
        {"Power Player",        "Generate 5,000 MW\xc2\xb7h",    "\xe2\x9a\xa1"},
        {"Energy Baron",        "Generate 25,000 MW\xc2\xb7h",   "\xf0\x9f\x92\x8e"},
        {"Cool Under Pressure", "Restore power after 3 SCRAMs",   "\xe2\x9d\x84\xef\xb8\x8f"},
        {"Perfect Run",         "50 turns without SCRAM",         "\xe2\x9c\xa8"},
        {"Xenon Master",        "Ride out 5 xenon transients",    "\xe2\x98\xa2\xef\xb8\x8f"},
        {"Nightmare Survivor",  "25 turns on Nightmare",          "\xf0\x9f\x91\xbb"},
        {"Electrician",         "90%+ turbine output for 10 turns","\xf0\x9f\x94\x8c"},
        {"Grid Hero",           "Match demand (\xc2\xb1" "10%) 20 turns", "\xf0\x9f\x94\x8b"},
        {"Weather Warrior",     "Survive 5 storms",               "\xe2\x9b\x88\xef\xb8\x8f"},
        {"Diesel Dependent",    "50+ turns on diesel power",       "\xe2\x9b\xbd"},
        {"Pressure Perfect",    "50 turns generating, no relief", "\xf0\x9f\x94\xa7"},
        {"Radiation Safe",      "Low radiation for 100 turns",    "\xf0\x9f\x9b\xa1\xef\xb8\x8f"},
        {"Marathon Runner",     "Complete 500 turns",             "\xf0\x9f\x8f\x83"}
    };
    return table;
}

struct DifficultySettings {
    std::string name;
    double fuelDepletionRate;
    double coolantLossRate;
    double eventChance;
    double scramTemperature;
    double meltdownTemperature;
    int scoreMultiplier;
    double turbineEfficiency;
    double xenonBuildupRate;
};

inline DifficultySettings getDifficultySettings(Difficulty diff) {
    // name, fuel burn (%/turn at full power), coolant leak (%/turn), event 1-in-N,
    // SCRAM temp, meltdown temp, score multiplier, turbine efficiency, xenon yield scale
    switch (diff) {
        case Difficulty::EASY:
            return {"Easy",      0.05, 0.15, 15.0, 1000.0, 1600.0, 1, 0.95, 0.75};
        case Difficulty::NORMAL:
            return {"Normal",    0.1,  0.3,  10.0,  900.0, 1400.0, 2, 0.90, 1.0};
        case Difficulty::HARD:
            return {"Hard",      0.15, 0.5,   7.0,  850.0, 1250.0, 3, 0.85, 1.25};
        case Difficulty::NIGHTMARE:
            return {"Nightmare", 0.2,  0.7,   5.0,  800.0, 1100.0, 5, 0.75, 1.5};
        default:
            return {"Normal",    0.1,  0.3,  10.0,  900.0, 1400.0, 2, 0.90, 1.0};
    }
}
