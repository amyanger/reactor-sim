# Reactor Simulator

C++11 nuclear reactor simulator — multi-file educational CLI game.

## Commands

```bash
# Build (requires MSYS2 MinGW-w64 in PATH)
PATH="/c/msys64/mingw64/bin:$PATH" g++ -std=c++11 -Wall -Wextra -Isrc src/*.cpp -o reactor.exe

# Or using Makefile (after setting PATH)
PATH="/c/msys64/mingw64/bin:$PATH" make

# Run
./reactor.exe

# Clean
make clean
```

No automated test suite. Testing is manual through gameplay.

## Architecture

Multi-file architecture in `src/` with shared `ReactorState` struct and message queue pattern:

### Headers (contracts)
- `types.h` — Enums, Color (`constexpr const char*`), Sound, WeatherInfo, AchievementInfo, DifficultySettings
- `constants.h` — All constexpr physics/threshold/scoring constants in `RC` namespace
- `reactor_state.h` — `ReactorState` struct (all mutable state), `GameMessage`, `LogEntry`

### Physics Subsystems (each operates on `ReactorState&`)
- `xenon.h/.cpp` — XenonSystem: xenon build/decay
- `turbine.h/.cpp` — TurbineSystem: RPM, steam pressure, electricity
- `emergency.h/.cpp` — EmergencySystem: ECCS + diesel generator
- `radiation.h/.cpp` — RadiationSystem: radiation + exposure tracking
- `containment.h/.cpp` — ContainmentSystem: integrity + breach
- `weather.h/.cpp` — WeatherSystem: weather transitions + effects
- `grid.h/.cpp` — GridSystem: demand simulation + satisfaction
- `scoring.h/.cpp` — ScoringSystem: statistics tracking

### Supporting Systems
- `achievements.h/.cpp` — AchievementSystem: 16 checks + unlock
- `persistence.h/.cpp` — Save/load/highscore/achievement I/O
- `events.h/.cpp` — RandomEventSystem: 11 event types
- `safety.h/.cpp` — SafetySystem: SCRAM + meltdown + reset

### Orchestration & UI
- `physics.h/.cpp` — CorePhysics: orchestrates subsystems per turn
- `renderer.h/.cpp` — All display/UI code + message draining
- `input.h/.cpp` — Command parsing + dispatch
- `reactor.h/.cpp` — ReactorSimulator: thin orchestrator (game loop)
- `main.cpp` — selectDifficulty() + main()

### Key Patterns
- **Message queue**: Subsystems call `state.addMessage()` instead of `cout`. `Renderer::drainMessages()` prints them after physics completes.
- **Shared state**: All mutable game state lives in `ReactorState`. Subsystems operate on `ReactorState&`.
- **No circular deps**: All subsystems depend only on `reactor_state.h` + `constants.h`.

## Style

- Constants MUST use `static constexpr` with SCREAMING_SNAKE_CASE in `RC` namespace
- Enum values MUST use SCREAMING_SNAKE_CASE
- Color constants MUST be `constexpr const char*` (not `std::string`) to avoid ODR violations
- New subsystems follow the existing pattern: class with static methods taking `ReactorState&`

## Workflow

- MUST use subagents (Task tool) for multi-step development — use `Explore` for codebase research, `developer` for implementation, `reviewer` for code review, `debugger` for investigating issues
- Run parallel agents when tasks are independent
- After changes, always verify the build compiles: `PATH="/c/msys64/mingw64/bin:$PATH" g++ -std=c++11 -Wall -Wextra -Isrc src/*.cpp -o reactor.exe`

## Git Rules

- MUST NOT add Claude or any AI assistant as a GitHub contributor
- MUST NOT include `Co-Authored-By` lines for Claude/AI in commit messages
- All commits MUST be attributed solely to the human author
