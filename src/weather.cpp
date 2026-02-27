#include "weather.h"

#include <sstream>
#include <string>

void WeatherSystem::update(ReactorState& state) {
    state.weatherDuration--;

    if (state.weatherDuration <= 0) {
        // Change weather
        std::uniform_int_distribution<int> weatherDist(0, 5);
        Weather newWeather = static_cast<Weather>(weatherDist(state.rng));

        if (newWeather != state.currentWeather) {
            // Track storm survival
            if (state.currentWeather == Weather::STORM) {
                state.stormsSurvived++;
            }

            WeatherInfo info = getWeatherInfo(newWeather);
            {
                std::ostringstream oss;
                oss << Color::CYAN << "\xf0\x9f\x8c\xa1\xef\xb8\x8f Weather change: " << info.icon << " " << info.name
                    << Color::DIM << " - " << info.description << Color::RESET << "\n";
                state.addMessage(oss.str());
            }
            state.addLogEntry("EVENT", "Weather changed to " + info.name);
            state.currentWeather = newWeather;
        }

        // Random duration between 5-20 turns
        std::uniform_int_distribution<int> durationDist(5, 20);
        state.weatherDuration = durationDist(state.rng);
    }

    // Storm can cause random equipment damage
    if (state.currentWeather == Weather::STORM) {
        std::uniform_int_distribution<int> stormDist(0, 20);
        if (stormDist(state.rng) == 0) {
            {
                std::ostringstream oss;
                oss << Color::YELLOW << Color::BOLD
                    << "\xe2\x9a\xa1 LIGHTNING STRIKE near the facility!"
                    << Color::RESET << "\n";
                state.addMessage(oss.str());
            }
            state.addLogEntry("WARNING", "Lightning strike detected");

            // Random effect
            std::uniform_int_distribution<int> effectDist(0, 2);
            switch (effectDist(state.rng)) {
                case 0: {
                    std::ostringstream oss;
                    oss << Color::YELLOW << "   Turbine RPM fluctuation" << Color::RESET << "\n";
                    state.addMessage(oss.str());
                    if (state.turbineOnline) state.turbineRPM *= 0.9;
                    break;
                }
                case 1: {
                    std::ostringstream oss;
                    oss << Color::YELLOW << "   Minor sensor interference" << Color::RESET << "\n";
                    state.addMessage(oss.str());
                    break;
                }
                case 2: {
                    std::ostringstream oss;
                    oss << Color::RED << "   External power grid disruption!" << Color::RESET << "\n";
                    state.addMessage(oss.str());
                    if (!state.dieselRunning && state.dieselAutoStart && state.dieselFuel > 0) {
                        state.dieselRunning = true;
                        std::ostringstream oss2;
                        oss2 << Color::GREEN << "   Diesel generator auto-started." << Color::RESET << "\n";
                        state.addMessage(oss2.str());
                    }
                    break;
                }
            }
        }
    }
}
