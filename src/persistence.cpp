#include "persistence.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace {

const char* const SAVE_HEADER = "REACTOR_SAVE v2";
constexpr int INT_LO = std::numeric_limits<int>::min();
constexpr int INT_HI = std::numeric_limits<int>::max();

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r");
    return s.substr(start, end - start + 1);
}

std::string singleLine(std::string s) {
    for (char& c : s) {
        if (c == '\n' || c == '\r') c = ' ';
    }
    return s;
}

struct Writer {
    std::ostringstream out;

    Writer() { out.precision(std::numeric_limits<double>::max_digits10); }

    void real(const char* key, double v, double, double) { out << key << '=' << v << '\n'; }
    void integer(const char* key, int v, int, int) { out << key << '=' << v << '\n'; }
    void flag(const char* key, bool v) { out << key << '=' << (v ? 1 : 0) << '\n'; }
    void text(const char* key, const std::string& v) { out << key << '=' << singleLine(v) << '\n'; }
};

struct Reader {
    const std::map<std::string, std::string>& kv;
    std::string err;

    bool ok() const { return err.empty(); }

    const std::string* find(const char* key) {
        if (!ok()) return nullptr;
        auto it = kv.find(key);
        if (it == kv.end()) {
            err = std::string("missing field '") + key + "'";
            return nullptr;
        }
        return &it->second;
    }

    void invalid(const char* key, const std::string& value) {
        err = std::string("invalid value for '") + key + "': " + value;
    }

    void real(const char* key, double& v, double lo, double hi) {
        const std::string* s = find(key);
        if (!s) return;
        try {
            size_t used = 0;
            double parsed = std::stod(*s, &used);
            if (used != s->size() || !std::isfinite(parsed) || parsed < lo || parsed > hi) {
                invalid(key, *s);
                return;
            }
            v = parsed;
        } catch (const std::exception&) {
            invalid(key, *s);
        }
    }

    void integer(const char* key, int& v, int lo, int hi) {
        const std::string* s = find(key);
        if (!s) return;
        try {
            size_t used = 0;
            long long parsed = std::stoll(*s, &used);
            if (used != s->size() || parsed < lo || parsed > hi) {
                invalid(key, *s);
                return;
            }
            v = static_cast<int>(parsed);
        } catch (const std::exception&) {
            invalid(key, *s);
        }
    }

    void flag(const char* key, bool& v) {
        const std::string* s = find(key);
        if (!s) return;
        if (*s == "1") v = true;
        else if (*s == "0") v = false;
        else invalid(key, *s);
    }

    void text(const char* key, std::string& v) {
        const std::string* s = find(key);
        if (s) v = *s;
    }
};

// One field list drives both directions so save and load can't drift apart.
// S is ReactorState or const ReactorState.
template <typename S, typename V>
void visitFields(S& s, V& v) {
    v.real("neutrons", s.neutrons, 0.0, RC::MAX_NEUTRONS);
    v.real("controlRods", s.controlRods, 0.0, 1.0);
    v.real("rodTarget", s.rodTarget, 0.0, 1.0);
    v.real("reactivity", s.reactivity, -1e3, 1e3);
    v.real("temperature", s.temperature, 0.0, 1e4);
    v.real("coolant", s.coolant, 0.0, 100.0);
    v.real("power", s.power, 0.0, RC::MAX_NEUTRONS * RC::NEUTRON_TO_POWER);
    v.real("decayHeat", s.decayHeat, 0.0, 1e7);
    v.real("heatRemoved", s.heatRemoved, 0.0, 1e7);
    v.real("fuel", s.fuel, 0.0, 100.0);
    v.flag("running", s.running);
    v.flag("gameOver", s.gameOver);
    v.text("gameOverReason", s.gameOverReason);

    v.real("xenonLevel", s.xenonLevel, 0.0, 1e4);
    v.real("iodineLevel", s.iodineLevel, 0.0, 1e4);
    v.integer("xenonHandledCount", s.xenonHandledCount, 0, INT_HI);
    v.flag("xenonTransientActive", s.xenonTransientActive);

    v.real("turbineRPM", s.turbineRPM, 0.0, RC::MAX_TURBINE_RPM);
    v.real("steamPressure", s.steamPressure, 0.0, 1e4);
    v.real("electricityOutput", s.electricityOutput, 0.0, 1e6);
    v.real("totalElectricityGenerated", s.totalElectricityGenerated, 0.0, 1e15);
    v.flag("turbineOnline", s.turbineOnline);
    v.integer("maxTurbineTurns", s.maxTurbineTurns, 0, INT_HI);
    v.flag("pressureReliefOpen", s.pressureReliefOpen);
    v.integer("pressureWarnings", s.pressureWarnings, 0, INT_HI);

    v.flag("eccsAvailable", s.eccsAvailable);
    v.integer("eccsCooldownTimer", s.eccsCooldownTimer, 0, INT_HI);

    v.real("dieselFuel", s.dieselFuel, 0.0, RC::DIESEL_FUEL_CAPACITY);
    v.flag("dieselRunning", s.dieselRunning);
    v.flag("dieselAutoStart", s.dieselAutoStart);
    v.flag("dieselStartedByAuto", s.dieselStartedByAuto);
    v.integer("dieselRuntime", s.dieselRuntime, 0, INT_HI);

    v.real("radiationLevel", s.radiationLevel, 0.0, 1e9);
    v.real("totalRadiationExposure", s.totalRadiationExposure, 0.0, 1e15);
    v.integer("radiationAlarms", s.radiationAlarms, 0, INT_HI);

    v.integer("weatherDuration", s.weatherDuration, 0, INT_HI);

    v.real("gridDemand", s.gridDemand, 0.0, 1e6);
    v.real("demandSatisfaction", s.demandSatisfaction, 0.0, 1e6);
    v.integer("demandBonus", s.demandBonus, INT_LO, INT_HI);
    v.integer("demandPenalty", s.demandPenalty, INT_LO, INT_HI);

    v.integer("lastTipTurn", s.lastTipTurn, INT_LO, INT_HI);

    v.integer("highSatisfactionTurns", s.highSatisfactionTurns, 0, INT_HI);
    v.integer("stormsSurvived", s.stormsSurvived, 0, INT_HI);
    v.integer("turnsWithoutPressureRelief", s.turnsWithoutPressureRelief, 0, INT_HI);
    v.integer("safeRadiationTurns", s.safeRadiationTurns, 0, INT_HI);

    v.real("containmentIntegrity", s.containmentIntegrity, 0.0, RC::MAX_CONTAINMENT);
    v.flag("containmentBreach", s.containmentBreach);

    v.integer("score", s.score, INT_LO, INT_HI);
    v.integer("turns", s.turns, 0, INT_HI);
    v.integer("scramCount", s.scramCount, 0, INT_HI);
    v.integer("eventsExperienced", s.eventsExperienced, 0, INT_HI);
    v.integer("turnsWithoutScram", s.turnsWithoutScram, 0, INT_HI);
    v.integer("scramRecoveries", s.scramRecoveries, 0, INT_HI);
    v.flag("awaitingRecovery", s.awaitingRecovery);

    v.real("peakTemperature", s.peakTemperature, 0.0, 1e5);
    v.real("peakPower", s.peakPower, 0.0, 1e7);
    v.real("peakElectricity", s.peakElectricity, 0.0, 1e6);
    v.real("totalPowerGenerated", s.totalPowerGenerated, 0.0, 1e15);
    v.real("averageTemperature", s.averageTemperature, 0.0, 1e5);
    v.real("temperatureSum", s.temperatureSum, 0.0, 1e15);
    v.integer("criticalEvents", s.criticalEvents, 0, INT_HI);
    v.real("lowestCoolant", s.lowestCoolant, 0.0, 100.0);
    v.real("highestXenon", s.highestXenon, 0.0, 1e4);
}

bool parseDifficulty(const std::string& name, Difficulty& out) {
    for (Difficulty d : {Difficulty::EASY, Difficulty::NORMAL, Difficulty::HARD, Difficulty::NIGHTMARE}) {
        if (getDifficultySettings(d).name == name) {
            out = d;
            return true;
        }
    }
    return false;
}

bool parseLogEntry(const std::string& value, LogEntry& entry) {
    size_t first = value.find('|');
    if (first == std::string::npos) return false;
    size_t second = value.find('|', first + 1);
    if (second == std::string::npos) return false;
    try {
        size_t used = 0;
        std::string turnText = value.substr(0, first);
        long long turn = std::stoll(turnText, &used);
        if (used != turnText.size() || turn < 0 || turn > INT_HI) return false;
        entry.turn = static_cast<int>(turn);
    } catch (const std::exception&) {
        return false;
    }
    entry.type = value.substr(first + 1, second - first - 1);
    entry.message = value.substr(second + 1);
    return true;
}

// Write to <path>.tmp and rename over the target, so a crash mid-write
// never leaves a truncated file behind.
bool writeFileAtomic(const std::string& path, const std::string& content, std::string& err) {
    std::string tmp = path + ".tmp";
    {
        std::ofstream file(tmp, std::ios::binary | std::ios::trunc);
        if (!file) {
            err = "cannot open " + tmp + " for writing";
            return false;
        }
        file << content;
        file.flush();
        if (!file) {
            err = "write to " + tmp + " failed";
            file.close();
            std::remove(tmp.c_str());
            return false;
        }
    }
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        err = "cannot replace " + path;
        std::remove(tmp.c_str());
        return false;
    }
    return true;
}

bool readFile(const std::string& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return !file.bad();
}

} // namespace

std::string PersistenceSystem::dataPath(const std::string& fileName) {
    namespace fs = std::filesystem;
    fs::path dir;
    const char* xdg = std::getenv("XDG_DATA_HOME");
    const char* home = std::getenv("HOME");
    if (xdg && *xdg == '/') {
        dir = fs::path(xdg) / "reactor-sim";
    } else if (home && *home) {
        dir = fs::path(home) / ".local" / "share" / "reactor-sim";
    } else {
        dir = ".reactor-sim";
    }
    std::error_code ec;
    fs::create_directories(dir, ec);  // a failure surfaces as a write error later
    return (dir / fileName).string();
}

std::string PersistenceSystem::serialize(const ReactorState& state) {
    Writer w;
    w.out << SAVE_HEADER << '\n';
    w.text("difficulty", state.currentDifficulty.name);
    visitFields(state, w);
    w.integer("weather", static_cast<int>(state.currentWeather), 0, 0);
    w.out << "rng=" << state.rng << '\n';
    for (const auto& entry : state.operatorLog) {
        std::string type = singleLine(entry.type);
        for (char& c : type) {
            if (c == '|') c = '/';
        }
        w.out << "log=" << entry.turn << '|' << type << '|' << singleLine(entry.message) << '\n';
    }
    return w.out.str();
}

bool PersistenceSystem::deserialize(const std::string& text, ReactorState& state, std::string& err) {
    std::map<std::string, std::string> kv;
    std::vector<std::string> logLines;
    std::istringstream in(text);
    std::string line;
    bool sawHeader = false;
    int lineNo = 0;

    while (std::getline(in, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (!sawHeader) {
            if (line != SAVE_HEADER) {
                err = "unsupported save format (expected '" + std::string(SAVE_HEADER) + "')";
                return false;
            }
            sawHeader = true;
            continue;
        }
        size_t eq = line.find('=');
        if (eq == std::string::npos || eq == 0) {
            err = "malformed line " + std::to_string(lineNo);
            return false;
        }
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        if (key == "log") logLines.push_back(value);
        else kv[key] = value;  // unknown keys are ignored
    }
    if (!sawHeader) {
        err = "save file is empty";
        return false;
    }

    // Start from the live state so session-only fields (preferences, high
    // score, unlocked achievements) carry over, then overwrite the rest.
    ReactorState tmp = state;

    auto diffIt = kv.find("difficulty");
    Difficulty diff;
    if (diffIt == kv.end()) {
        err = "missing field 'difficulty'";
        return false;
    }
    if (!parseDifficulty(diffIt->second, diff)) {
        err = "unknown difficulty '" + diffIt->second + "'";
        return false;
    }
    tmp.currentDifficulty = getDifficultySettings(diff);

    Reader r{kv, ""};
    visitFields(tmp, r);
    int weather = 0;
    r.integer("weather", weather, 0, static_cast<int>(Weather::COLD_SNAP));
    if (!r.ok()) {
        err = r.err;
        return false;
    }
    tmp.currentWeather = static_cast<Weather>(weather);

    auto rngIt = kv.find("rng");
    if (rngIt == kv.end()) {
        err = "missing field 'rng'";
        return false;
    }
    std::istringstream rngIn(rngIt->second);
    rngIn >> tmp.rng;
    if (rngIn.fail() || !(rngIn >> std::ws).eof()) {
        err = "invalid value for 'rng'";
        return false;
    }

    tmp.operatorLog.clear();
    for (const auto& value : logLines) {
        LogEntry entry;
        if (!parseLogEntry(value, entry)) {
            err = "invalid log entry: " + value;
            return false;
        }
        tmp.operatorLog.push_back(entry);
    }
    if (static_cast<int>(tmp.operatorLog.size()) > RC::MAX_LOG_ENTRIES) {
        tmp.operatorLog.erase(tmp.operatorLog.begin(),
                              tmp.operatorLog.end() - RC::MAX_LOG_ENTRIES);
    }

    tmp.promptCritical = false;
    tmp.clearMessages();
    state = std::move(tmp);
    return true;
}

bool PersistenceSystem::saveGame(const ReactorState& state, std::string& err) {
    return writeFileAtomic(dataPath(RC::SAVE_FILE), serialize(state), err);
}

bool PersistenceSystem::loadGame(ReactorState& state, std::string& err) {
    std::string path = dataPath(RC::SAVE_FILE);
    std::string text;
    if (!readFile(path, text)) {
        err = "no save file found at " + path;
        return false;
    }
    if (!deserialize(text, state, err)) return false;
    // The save may be for a different difficulty, which has its own high score
    loadHighScore(state);
    return true;
}

void PersistenceSystem::deleteSave() {
    std::remove(dataPath(RC::SAVE_FILE).c_str());
}

void PersistenceSystem::loadHighScore(ReactorState& state) {
    state.highScore = 0;
    std::string text;
    if (!readFile(dataPath(std::string(RC::HIGH_SCORE_FILE) + "_" + state.currentDifficulty.name), text)) return;
    std::istringstream in(text);
    int value = 0;
    if (in >> value && value > 0) state.highScore = value;
}

void PersistenceSystem::saveHighScore(const ReactorState& state) {
    if (state.score <= state.highScore) return;
    std::string err;
    writeFileAtomic(dataPath(std::string(RC::HIGH_SCORE_FILE) + "_" + state.currentDifficulty.name),
                    std::to_string(state.score) + "\n", err);
}

void PersistenceSystem::loadAchievements(ReactorState& state) {
    std::string text;
    if (!readFile(dataPath(RC::ACHIEVEMENTS_FILE), text)) return;
    std::istringstream in(text);
    int achievementId;
    while (in >> achievementId) {
        if (achievementId >= 0 && achievementId < static_cast<int>(Achievement::ACHIEVEMENT_COUNT)) {
            state.unlockedAchievements.insert(static_cast<Achievement>(achievementId));
        }
    }
}

void PersistenceSystem::saveAchievements(const ReactorState& state) {
    std::ostringstream out;
    for (const auto& ach : state.unlockedAchievements) {
        out << static_cast<int>(ach) << "\n";
    }
    std::string err;
    writeFileAtomic(dataPath(RC::ACHIEVEMENTS_FILE), out.str(), err);
}
