☢️ C++ Nuclear Reactor Simulator

A feature-rich, interactive C++ nuclear reactor simulation that lets you try your hand as a reactor operator. Manage control rods, coolant, turbines, and emergency systems while dealing with random events, xenon poisoning, weather, and grid demand. Try not to melt the core!

---

## 🚀 Features

### Core Gameplay
- **Real-time command-line gameplay**: Control rods, coolant, turbines, and reactor safety
- **4 Difficulty levels**: Easy, Normal, Hard, and Nightmare with unique parameters
- **Scoring system**: Points for survival, power generation, and grid satisfaction with difficulty multipliers
- **High score tracking**: Persistent high scores per difficulty level

### Reactor Systems
- **Turbine Hall**: Generate electricity from steam with RPM and pressure simulation
- **Emergency Core Cooling System (ECCS)**: Emergency coolant injection with cooldown
- **Diesel Generator**: Backup power source with fuel management and auto-start
- **Xenon-135 Poisoning**: Realistic neutron absorption mechanics
- **Containment Integrity**: Structural stress, degradation, and breach detection
- **Radiation Monitoring**: 3-tier radiation warnings with exposure tracking
- **Fuel burnup & auto SCRAM**: Realistic fuel depletion and emergency shutdowns

### Environmental Systems
- **Dynamic Weather**: 6 weather types (Clear, Cloudy, Rain, Storm, Heatwave, Cold Snap) affecting cooling and events
- **Power Grid Demand**: Time-of-day demand simulation with satisfaction scoring
- **Steam Pressure**: Realistic pressure buildup with relief valve and pipe rupture mechanics

### Random Events (11 types)
- Coolant leaks, power surges, pump failures
- Steam leaks, turbine trips, xenon spikes
- Bonus events: efficiency boost, coolant delivery, maintenance crew

### Meta Features
- **16 Achievements**: Unlock achievements for various accomplishments
- **Save/Load**: Continue your game later
- **Operator Tips**: Contextual tips based on reactor state
- **Event Log**: Track all reactor events and operator actions
- **Statistics**: Detailed session stats tracking
- **Pause**: Pause simulation while reviewing data
- **Sound effects**: Terminal beep alerts for warnings and emergencies
- **Colorful ASCII dashboard**: Real-time reactor, turbine, grid, and weather status

---

## 🛠️ Getting Started

### 1. Clone This Repo
```bash
git clone https://github.com/amyanger/reactor-sim.git
cd reactor-sim
```

### 2. Compile the Code
```bash
# Using Make
make

# Or compile manually
g++ -std=c++11 -Wall -Wextra -Isrc src/*.cpp -o reactor

# Windows (MSYS2 MinGW)
PATH="/c/msys64/mingw64/bin:$PATH" make
```

### 3. Run the Simulator
```bash
./reactor
```

---

## 🎮 How to Play

### Commands
| Command | Action |
|---------|--------|
| `0-100` | Set control rod insertion percentage |
| `r` | Refill coolant (-50 points) |
| `t` | Toggle turbine online/offline |
| `e` | Activate ECCS emergency cooling |
| `d` | Toggle diesel generator |
| `df` | Refill diesel fuel |
| `da` | Toggle diesel auto-start |
| `p` / `pause` | Pause/resume simulation |
| `s` / `save` | Save game |
| `l` / `load` | Load saved game |
| `a` | View achievements |
| `stats` | View session statistics |
| `log` | View event log |
| `sound` | Toggle sound effects |
| `tips` | Toggle operator tips |
| `h` / `help` | Display help screen |
| `q` | Quit simulation |
| `reset` | Restart after SCRAM |

### Tips
- **Control Rods**: 0% = max power (risky!), 100% = safest (low power)
- **Turbine**: Keep temperature around 500°C for optimal electricity generation
- **ECCS**: Save it for emergencies — 10 turn cooldown after use
- **Xenon**: Watch for xenon buildup at high power levels
- **Grid**: Match power output to demand for bonus points
- **Weather**: Storms can cause lightning strikes; heatwaves reduce cooling

---

## 🏆 Achievements

| Achievement | Description |
|-------------|-------------|
| First Steps | Complete 10 turns |
| Survivor | Complete 50 turns |
| Veteran Operator | Complete 100 turns |
| Marathon Runner | Complete 500 turns |
| Power Player | Generate 100 MW·h |
| Energy Baron | Generate 500 MW·h |
| Cool Under Pressure | Recover from 3 SCRAMs |
| Perfect Run | 50 turns without SCRAM |
| Xenon Master | Handle xenon poisoning 5 times |
| Nightmare Survivor | 25 turns on Nightmare |
| Electrician | Max turbine output for 10 turns |
| Grid Hero | 95%+ grid satisfaction for 20 turns |
| Weather Warrior | Survive 5 storms |
| Diesel Dependent | 50+ turns on diesel power |
| Pressure Perfect | No relief valve use in 50 turns |
| Radiation Safe | Low radiation for 100 turns |

---

## 📊 Difficulty Levels

| Difficulty | Fuel Loss | Coolant Loss | Event Chance | SCRAM Temp | Score Multiplier |
|------------|-----------|--------------|--------------|------------|------------------|
| Easy | 0.05/turn | 0.15/turn | 1 in 15 | 1200°C | 1x |
| Normal | 0.10/turn | 0.30/turn | 1 in 10 | 1000°C | 2x |
| Hard | 0.15/turn | 0.50/turn | 1 in 7 | 800°C | 3x |
| Nightmare | 0.20/turn | 0.70/turn | 1 in 5 | 600°C | 5x |

---

## 📁 Project Structure

```
src/
  types.h              — Enums, colors, weather/achievement/difficulty data
  constants.h          — All physics/threshold/scoring constants
  reactor_state.h      — Shared ReactorState struct, message queue
  xenon.h/.cpp         — Xenon-135 build/decay system
  turbine.h/.cpp       — Turbine RPM, steam pressure, electricity
  emergency.h/.cpp     — ECCS + diesel generator
  radiation.h/.cpp     — Radiation level + exposure tracking
  containment.h/.cpp   — Containment integrity + breach detection
  weather.h/.cpp       — Dynamic weather transitions
  grid.h/.cpp          — Power grid demand simulation
  scoring.h/.cpp       — Statistics tracking
  achievements.h/.cpp  — 16 achievement checks + unlock
  persistence.h/.cpp   — Save/load/highscore file I/O
  events.h/.cpp        — 11 random event types
  safety.h/.cpp        — SCRAM + meltdown detection
  physics.h/.cpp       — Core physics orchestrator
  renderer.h/.cpp      — All display/UI code
  input.h/.cpp         — Command parsing + dispatch
  reactor.h/.cpp       — Game loop orchestrator
  main.cpp             — Entry point + difficulty selection
Makefile               — Build configuration
```

---

## 📚 Technical Details

- **C++11** compatible
- Multi-file architecture with shared state pattern
- Uses modern `<random>` for RNG (std::mt19937)
- ANSI color codes for terminal output
- Persistent storage for saves, high scores, and achievements
- ~2,500 lines across 36 source files

---

## ✨ Contributions

PRs and suggestions welcome! Open an issue or submit a PR.

---

## ⚠️ Disclaimer

This simulator is for learning and entertainment only.
Do not use as a reference for real nuclear power plant operation.

---

## 👤 Author

**Arjun Myanger**
GitHub: https://github.com/amyanger

---

## 🏷️ License

MIT License. Go wild, but don't blame me if you melt your keyboard.
