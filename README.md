☢️ C++ Nuclear Reactor Simulator

A simple, interactive C++ nuclear reactor simulation that lets you try your hand as a reactor operator. Adjust control rods, manage coolant, and try not to melt the core! Great for learning C++ basics, simulation logic, and having a bit of radioactive fun. ⚡️

**Version:** 0.1.0

⸻

🚀 Features
	•	Real-time command-line gameplay: Control rods, coolant, and reactor safety in your hands
	•	Random disaster events: Expect coolant leaks and power surges at the worst moments!
	•	Fuel burnup & auto SCRAM: Realistic depletion of fuel and emergency shutdowns for operator mistakes
	•	Manual and auto reset: Recover from SCRAMs and try again
	•	Modular architecture: Clean separation of concerns with physics, UI, and game logic
	•	Configurable parameters: Easy tuning via Constants.h
	•	ASCII dashboard: See real-time reactor status with visual indicators

⸻

🛠️ Getting Started

### 1. Clone This Repo

```bash
git clone https://github.com/yourusername/reactor-sim.git
cd reactor-sim
```

### 2. Compile the Code

#### Option A: Using Make (Quick & Simple)

```bash
make
```

Available make targets:
- `make` - Build the simulator
- `make run` - Build and run
- `make debug` - Build with debug symbols
- `make clean` - Remove build artifacts
- `make help` - Show all targets

#### Option B: Using CMake (Cross-platform)

```bash
mkdir build
cd build
cmake ..
make
```

#### Option C: Manual Compilation

```bash
# Using g++
g++ -std=c++11 -Wall -Wextra -O2 main.cpp Reactor_impl.cpp PhysicsEngine.cpp UI.cpp GameController.cpp RandomEvents.cpp -o reactor

# Using clang++
clang++ -std=c++11 -Wall -Wextra -O2 main.cpp Reactor_impl.cpp PhysicsEngine.cpp UI.cpp GameController.cpp RandomEvents.cpp -o reactor
```

### 3. Run the Simulator

```bash
./reactor
```

⸻

🎮 How to Play
	•	Set control rod level: Enter a number (0–100) each turn.
	•	0% = rods fully out (max reaction, risky!)
	•	100% = rods fully in (safest, but low power)
	•	Refill coolant: Type r at any prompt to refill the coolant.
	•	Quit: Type q to exit.
	•	Reset after SCRAM: Type reset if you trigger an emergency shutdown.

Survive as long as possible without melting the core!
Watch out for random disasters and manage your fuel wisely.

⸻

💡 Example Gameplay

```
Welcome to the C++ Nuclear Reactor Simulator v1.0
Try not to melt the core. Type 'q' to quit.

=== Reactor Dashboard ===
Temp    [████████░░░░░░░░░░░░░░░░░░░░]  300.0°C
Coolant [████████████████████████████]  100.0%
Fuel    [████████████████████████████]  100.0%

Neutrons: 1000.00 | Control Rods: 50% in | Temp: 300.00C | Coolant: 100.0% | Fuel: 100.0%
Set control rod level (0-100%, or 'r' to refill coolant): 0

Neutrons: 1050.00 | Control Rods: 0% in | Temp: 301.05C | Coolant: 99.7% | Fuel: 99.9%
...
!!! RANDOM EVENT: Coolant Leak! Lost 10.0% coolant! !!!
...
*** AUTO SCRAM! Emergency shutdown! ***
Type 'reset' to attempt reactor restart, or 'q' to quit:
```

⸻

📚 Learnings & Concepts
	•	C++ basics: variables, loops, conditionals, user input
	•	Modern C++11: random library, constexpr, auto
	•	Exception handling for robust user input
	•	Modular design: separation of concerns, single responsibility principle
	•	Game/simulation logic
	•	Build systems: Make and CMake

⸻

🏗️ Architecture

The simulator follows a clean modular architecture:

```
┌─────────────────┐
│   main.cpp      │  Entry point
└────────┬────────┘
         │
┌────────▼────────────┐
│ GameController      │  Main game loop & orchestration
└─────────┬───────────┘
          │
    ┌─────┴─────┬─────────┬──────────┐
    │           │         │          │
┌───▼───┐  ┌───▼──┐  ┌──▼─────┐  ┌─▼──────────┐
│Reactor│  │Physics│ │RandomEv│  │    UI      │
│       │  │Engine │ │ents    │  │            │
└───────┘  └───────┘ └────────┘  └────────────┘
    │           │
    └─────┬─────┘
    ┌─────▼─────┐
    │ Constants │  Configuration
    └───────────┘
```

**Modules:**
- **GameController**: Orchestrates the main game loop and coordinates all subsystems
- **Reactor**: Maintains reactor state (temperature, neutrons, fuel, coolant, etc.)
- **PhysicsEngine**: Handles all physics calculations (neutron multiplication, heat generation, cooling)
- **UI**: Manages all user input/output and display formatting
- **RandomEvents**: Generates random disaster events (coolant leaks, power surges)
- **Constants**: Centralized configuration for all simulation parameters

⸻

⚙️ Configuration

All simulation parameters can be tuned in `Constants.h`:

**Reactor Physics:**
- `K_EFF_BASE` - Neutron multiplication factor
- `ROD_ABSORPTION` - Control rod effectiveness
- `FUEL_DEPLETION_RATE` - How fast fuel depletes

**Safety Thresholds:**
- `SCRAM_TEMP` - Auto-shutdown temperature (default: 1000°C)
- `MELTDOWN_TEMP` - Game over temperature (default: 2000°C)
- `CRITICAL_COOLANT` - Low coolant warning level

**Random Events:**
- `EVENT_COOLANT_LEAK_AMOUNT` - Coolant lost in leak event
- `EVENT_POWER_SURGE_TEMP` - Temperature spike from surge

See `Constants.h` for all 25+ configurable parameters with detailed documentation.

⸻

✨ Contributions

PRs and suggestions welcome! Want more features (like leaderboards, sound, or real-world reactor physics)? Open an issue or submit a PR.

⸻

⚠️ Disclaimer

This simulator is for learning and entertainment only.
Do not use as a reference for real nuclear power plant operation (unless you want to be featured on the news).

⸻

👤 Author

Arjun Myanger
GitHub: https://github.com/arjun-myanger

⸻

🏷️ License

MIT License.
Go wild, but don’t blame me if you actually melt your keyboard.
