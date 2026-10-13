# C++ Nuclear Reactor Simulator

A turn-based terminal game where you run a nuclear power plant. You set the control rods, keep the coolant topped up, run the turbine, and try to match grid demand while random events, weather, and xenon poisoning work against you. Push too hard and the core trips or melts down.

## Building

You need a C++17 compiler (clang or gcc) and `make`. It builds on macOS and Linux.

```bash
git clone https://github.com/amyanger/reactor-sim.git
cd reactor-sim
make          # builds ./reactor
./reactor
```

Other targets:

| Target | What it does |
|--------|--------------|
| `make test` | Builds and runs the unit tests |
| `make debug` | Unoptimized build with debug symbols |
| `make asan` | Builds with AddressSanitizer and UBSan, then runs the tests |
| `make clean` | Removes `build/` and the binary |

Objects go in `build/`. Run `make clean` when switching between `debug`, `asan`, and the normal build.

## How to play

Each turn you type a command and press Enter. Pressing Enter on its own, or entering a rod position, advances one turn (6 minutes of plant time).

- **Control rods.** Enter a number from 0 to 100 (a trailing `%` is fine) to set the rod target. 100 is fully inserted. The rods move toward the target at most 5% per turn, so big changes take several turns.
- **Startup.** The plant starts at 50% power with rods at 62%. The core has no xenon yet, so it is more reactive than it will be later. Withdraw rods slowly at first. Pulling them hard on turn 1 will trip the reactor on overpower.
- **Xenon.** Xenon builds up from iodine as you run at power and eats into your reactivity. After a SCRAM it keeps rising for a while before it decays, so restarting soon after a trip needs rods further out than before.
- **Decay heat.** A shut-down core still makes heat. Keep the pumps powered (turbine or diesel) and the coolant up after a trip, or the temperature will climb.
- **Grid demand.** Demand follows the time of day and drifts between about 200 and 1000 MW. You score by delivering electricity up to demand, get a bonus for staying within 95 to 110% of it, and lose points for badly under- or over-supplying.
- **Refills cost you.** Coolant (`r`, +40%) costs 50 points and a turn. Diesel fuel (`df`) costs 25 points and a turn. ECCS costs 100 points and then needs 10 turns to recharge.
- **SCRAM.** The reactor trips automatically on high temperature, high neutron flux (130% power), or prompt criticality. A trip costs 1000 points. Type `reset` once the core has cooled below 90% of the SCRAM temperature, then withdraw rods and bring the turbine back with `t`.
- **Game over.** The game ends if the core passes the meltdown temperature or containment integrity reaches 0%. Neither can be reset.

### Controls

| Command | Action |
|---------|--------|
| `0`-`100` | Set control rod target (%) |
| Enter | Advance one turn |
| `r` | Refill coolant (costs a turn and 50 points) |
| `t` | Toggle turbine online/offline |
| `e` | Activate ECCS emergency cooling |
| `d` | Start or stop the diesel generator |
| `df` | Refill diesel fuel (costs a turn and 25 points) |
| `da` | Toggle diesel auto-start |
| `reset` | Reset after a SCRAM |
| `p` / `pause` | Pause or resume. Plant commands are blocked while paused |
| `s` / `save` | Save the game |
| `l` / `load` | Load the saved game |
| `a` | Show achievements |
| `stats` | Show session statistics |
| `log` | Show the operator log |
| `sound` | Toggle terminal bell alerts |
| `tips` | Toggle operator tips |
| `h` / `help` | Show help |
| `q` / `quit` | Quit |

Commands are case-insensitive. Unknown input shows a message and does not advance the turn.

## Difficulty levels

| Difficulty | Fuel burn (%/turn at full power) | Coolant leak (%/turn) | Event chance | SCRAM temp | Meltdown temp | Score multiplier |
|------------|------|------|---------|--------|--------|----|
| Easy       | 0.05 | 0.15 | 1 in 15 | 1000°C | 1600°C | 1x |
| Normal     | 0.10 | 0.30 | 1 in 10 | 900°C  | 1400°C | 2x |
| Hard       | 0.15 | 0.50 | 1 in 7  | 850°C  | 1250°C | 3x |
| Nightmare  | 0.20 | 0.70 | 1 in 5  | 800°C  | 1100°C | 5x |

Harder levels also have lower turbine efficiency and faster xenon buildup.

## Plant systems

- **Turbine.** Converts the heat removed from the core into electricity. Efficiency peaks at about 550°C. While it spins above 1800 RPM it also powers the main coolant pumps.
- **Diesel generator.** Powers the pumps when the turbine can't. With auto-start on, it starts when the core needs cooling and the turbine is down, and stops when it is no longer needed. It does not feed the grid.
- **ECCS.** Injects coolant and drops the core temperature by up to 100°C.
- **Steam pressure.** Rises with temperature, and faster with the turbine offline. The relief valve opens at 130 bar and vents heat and coolant. Above 160 bar a pipe ruptures.
- **Containment.** Wears down under high temperature and pressure and recovers slowly when the plant is calm. A breach raises radiation and costs points every turn. At 0% the game is over.
- **Radiation.** Tracks dose rate and cumulative exposure, with warnings at 100, 500, and 2000 mSv/h.
- **Weather.** Clear, cloudy, rain, storm, heatwave, and cold snap. Weather changes cooling and how often events happen. Storms can cause lightning strikes.
- **Random events.** Nine kinds: coolant leak, power surge, pump failure, xenon spike, steam leak, turbine trip, efficiency boost, coolant delivery, and maintenance crew.

## Simulation model

The physics is simplified and tuned for play. It borrows the structure of real reactor models but the numbers are chosen to make an interesting game.

- Reactivity is tracked in dollars, so 1$ is prompt critical. It is the sum of excess reactivity, minus rod worth, xenon, fuel burnup, and a negative temperature coefficient.
- Rod worth follows an S-curve, so rods are most effective in the middle of their travel.
- Neutron population uses point kinetics with a prompt-jump approximation, run in 10 substeps per turn so temperature feedback can act during a transient. A small source term lets a shut-down core restart.
- Iodine-135 decays into xenon-135, which decays and is also burned out by neutron flux. This produces an equilibrium level at power and a xenon peak after shutdown.
- Cooling is Newtonian: heat removed is proportional to the gap between core and heat sink temperature, scaled by pump power, coolant level, and weather.
- Decay heat follows a fraction of recent fission power and fades slowly after shutdown.

The tuning constants are in `src/constants.h`.

## Achievements

| Achievement | Condition |
|-------------|-----------|
| First Steps | Complete 10 turns |
| Survivor | Complete 50 turns |
| Veteran Operator | Complete 100 turns |
| Marathon Runner | Complete 500 turns |
| Power Player | Generate 5,000 MWh |
| Energy Baron | Generate 25,000 MWh |
| Cool Under Pressure | Restore 300+ MW output after 3 SCRAMs |
| Perfect Run | 50 turns without a SCRAM |
| Xenon Master | Ride out 5 xenon transients |
| Nightmare Survivor | 25 turns on Nightmare |
| Electrician | 90%+ of max turbine output for 10 turns |
| Grid Hero | Supply within 90 to 110% of demand for 20 turns in a row |
| Weather Warrior | Survive 5 storms |
| Diesel Dependent | 50+ turns on diesel power |
| Pressure Perfect | 50 turns generating without opening the relief valve |
| Radiation Safe | Radiation below 100 mSv/h for 100 turns in a row |

## Save files

Saves, high scores (one per difficulty), and unlocked achievements are stored in `$XDG_DATA_HOME/reactor-sim/`, or `~/.local/share/reactor-sim/` if `XDG_DATA_HOME` is not set. If neither is available they go in `./.reactor-sim/`.

Save files are plain text with a version header. A save that is truncated, out of range, or from an unknown version is rejected and the current game is left as it was.

## Project layout

```
src/
  main.cpp             Entry point and difficulty selection
  reactor.h/.cpp       Game loop
  reactor_state.h      Shared ReactorState struct and message queue
  types.h              Enums, colors, weather, achievement, and difficulty tables
  constants.h          Physics, threshold, and scoring constants
  physics.h/.cpp       Reactivity, kinetics, heat balance, turn orchestration
  xenon.h/.cpp         Iodine and xenon chain
  turbine.h/.cpp       Turbine RPM, steam pressure, electricity
  emergency.h/.cpp     ECCS, diesel generator, refills
  safety.h/.cpp        SCRAM, meltdown, reset
  containment.h/.cpp   Containment integrity and breach
  radiation.h/.cpp     Dose rate and exposure
  weather.h/.cpp       Weather changes and lightning
  grid.h/.cpp          Grid demand and supply scoring
  events.h/.cpp        Random events
  scoring.h/.cpp       Score and session statistics
  achievements.h/.cpp  Achievement checks
  persistence.h/.cpp   Save, load, high scores, data directory
  input.h/.cpp         Command parsing and dispatch
  renderer.h/.cpp      Dashboard and screens
tests/                 doctest unit tests, run with `make test`
.github/workflows/     CI that builds and tests on Ubuntu and macOS
```

## Disclaimer

This is a game. Don't use it as a reference for operating a real reactor.

## License

MIT. See [LICENSE](LICENSE).
