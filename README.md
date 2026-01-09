☢️ C++ Nuclear Reactor Simulator

A simple, interactive C++ nuclear reactor simulation that lets you try your hand as a reactor operator. Adjust control rods, manage coolant, and try not to melt the core! Great for learning C++ basics, simulation logic, and having a bit of radioactive fun. ⚡️

⸻

🚀 Features
	•	Real-time command-line gameplay: Control rods, coolant, and reactor safety in your hands
	•	Random disaster events: Expect coolant leaks and power surges at the worst moments!
	•	Fuel burnup & auto SCRAM: Realistic depletion of fuel and emergency shutdowns for operator mistakes
	•	Manual and auto reset: Recover from SCRAMs and try again
	•	Super-friendly code comments: Perfect for C++ learners!
	•	ASCII dashboard (optional): See real-time reactor status

⸻

🛠️ Getting Started
	1.	Clone This Repo

git clone https://github.com/yourusername/reactor-sim.git
cd reactor-sim
	2.	Compile the Code (macOS/Linux)

clang++ reactor.cpp -o reactor

(Or use g++ on Linux/Windows)
	3.	Run the Simulator

./reactor

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

Welcome to the C++ Nuclear Reactor Simulator v0.1
Try not to melt the core. Type ‘q’ to quit.

Neutrons: 1000.00 | Control Rods: 50% in | Temp: 300.00C | Coolant: 100.0% | Fuel: 100.0%
Set control rod level (0-100%, current 50%, or ‘r’ to refill coolant): 0

Neutrons: 1050.00 | Control Rods: 0% in | Temp: 301.05C | Coolant: 99.7% | Fuel: 99.9%
…
!!! RANDOM EVENT: Coolant Leak! Lost 10% coolant! !!!
…
*** AUTO SCRAM! Emergency shutdown! ***
Type ‘reset’ to attempt reactor restart, or ‘q’ to quit:

⸻

📚 Learnings & Concepts
	•	C++ basics: variables, loops, conditionals, user input
	•	Random number generation
	•	Exception handling for robust user input
	•	Game/simulation logic
	•	Code commenting and readability

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
GitHub: https://github.com/amyanger

⸻

🏷️ License

MIT License.
Go wild, but don’t blame me if you actually melt your keyboard.
