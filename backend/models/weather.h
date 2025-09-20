#ifndef MODELS_WEATHER_H
#define MODELS_WEATHER_H

#include <string>
#include <chrono>

namespace models {
    enum class WeatherRiskLevel {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };

    class WeatherData {
    public:
        std::string id;
        double latitude;
        double longitude;
        double temperature; // Celsius
        double humidity; // Percentage
        double wind_speed; // m/s
        int wind_direction; // degrees
        double pressure; // hPa
        double visibility; // km
        std::string weather_condition;
        WeatherRiskLevel risk_level;
        std::chrono::system_clock::time_point recorded_at;
        std::chrono::system_clock::time_point created_at;

        // Constructors
        WeatherData() : latitude(0.0), longitude(0.0), temperature(0.0), humidity(0.0),
                       wind_speed(0.0), wind_direction(0), pressure(0.0), visibility(0.0),
                       risk_level(WeatherRiskLevel::LOW) {}

        // Helper methods
        std::string get_risk_level_string() const;
        static WeatherRiskLevel parse_risk_level_string(const std::string& risk_level_str);
    };
}

#endif // MODELS_WEATHER_H