☢️ C++ Nuclear Reactor Simulator

A feature-rich, interactive C++ nuclear reactor simulation that lets you try your hand as a reactor operator. Manage control rods, coolant, turbines, and emergency systems while dealing with random events and xenon poisoning. Try not to melt the core!

---

## 🚀 Features

### Core Gameplay
- **Real-time command-line gameplay**: Control rods, coolant, turbines, and reactor safety
- **4 Difficulty levels**: Easy, Normal, Hard, and Nightmare with unique parameters
- **Scoring system**: Points for survival and power generation with difficulty multipliers
- **High score tracking**: Persistent high scores per difficulty level

### Reactor Systems
- **Turbine Hall**: Generate electricity from steam for bonus points
- **Emergency Core Cooling System (ECCS)**: Emergency coolant injection with cooldown
- **Xenon-135 Poisoning**: Realistic neutron absorption mechanics
- **Fuel burnup & auto SCRAM**: Realistic fuel depletion and emergency shutdowns

### Random Events (12 types)
- Coolant leaks, power surges, pump failures
- Sensor malfunctions, steam leaks, turbine trips
- Control rod stuck, xenon spikes
- Bonus events: efficiency boost, coolant delivery, maintenance crew

### Meta Features
- **10 Achievements**: Unlock achievements for various accomplishments
- **Save/Load**: Continue your game later
- **Sound effects**: Terminal beep alerts for warnings and emergencies
- **Colorful ASCII dashboard**: See real-time reactor and turbine status

---

## 🛠️ Getting Started

### 1. Clone This Repo
```bash
git clone https://github.com/arjun-myanger/reactor-sim.git
cd reactor-sim
```

### 2. Compile the Code
```bash
# macOS
clang++ reactor.cpp -o reactor

# Linux
g++ reactor.cpp -o reactor

# Windows (MinGW)
g++ reactor.cpp -o reactor.exe
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
| `s` / `save` | Save game |
| `l` / `load` | Load saved game |
| `a` | View achievements |
| `sound` | Toggle sound effects |
| `h` / `help` | Display help screen |
| `q` | Quit simulation |
| `reset` | Restart after SCRAM |

### Tips
- **Control Rods**: 0% = max power (risky!), 100% = safest (low power)
- **Turbine**: Keep temperature around 500°C for optimal electricity generation
- **ECCS**: Save it for emergencies - 10 turn cooldown after use
- **Xenon**: Watch for xenon buildup at high power levels

---

## 🏆 Achievements

| Achievement | Description |
|-------------|-------------|
| First Steps | Complete 10 turns |
| Survivor | Complete 50 turns |
| Veteran Operator | Complete 100 turns |
| Power Player | Generate 100 MW·h |
| Energy Baron | Generate 500 MW·h |
| Cool Under Pressure | Recover from 3 SCRAMs |
| Perfect Run | 50 turns without SCRAM |
| Xenon Master | Handle xenon poisoning 5 times |
| Nightmare Survivor | 25 turns on Nightmare |
| Electrician | Max turbine output for 10 turns |

---

## 📊 Difficulty Levels

| Difficulty | Fuel Loss | Coolant Loss | Event Chance | SCRAM Temp | Score Multiplier |
|------------|-----------|--------------|--------------|------------|------------------|
| Easy | 0.05/turn | 0.15/turn | 1 in 15 | 1200°C | 1x |
| Normal | 0.10/turn | 0.30/turn | 1 in 10 | 1000°C | 2x |
| Hard | 0.15/turn | 0.50/turn | 1 in 7 | 800°C | 3x |
| Nightmare | 0.20/turn | 0.70/turn | 1 in 5 | 600°C | 5x |

---

## 📚 Technical Details

- **C++11** compatible
- Uses modern `<random>` for RNG (std::mt19937)
- ANSI color codes for terminal output
- Persistent storage for high scores and achievements
- Approximately 950 lines of well-commented code

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
GitHub: https://github.com/arjun-myanger

---

## 🏷️ License

MIT License. Go wild, but don't blame me if you melt your keyboard.
