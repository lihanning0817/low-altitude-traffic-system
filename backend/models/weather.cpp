#include "weather.h"
#include <stdexcept>

namespace models {
    std::string WeatherData::get_risk_level_string() const {
        switch (risk_level) {
            case WeatherRiskLevel::LOW:
                return "low";
            case WeatherRiskLevel::MEDIUM:
                return "medium";
            case WeatherRiskLevel::HIGH:
                return "high";
            case WeatherRiskLevel::CRITICAL:
                return "critical";
            default:
                return "low";
        }
    }

    WeatherRiskLevel WeatherData::parse_risk_level_string(const std::string& risk_level_str) {
        if (risk_level_str == "low") {
            return WeatherRiskLevel::LOW;
        } else if (risk_level_str == "medium") {
            return WeatherRiskLevel::MEDIUM;
        } else if (risk_level_str == "high") {
            return WeatherRiskLevel::HIGH;
        } else if (risk_level_str == "critical") {
            return WeatherRiskLevel::CRITICAL;
        } else {
            throw std::invalid_argument("Invalid weather risk level string: " + risk_level_str);
        }
    }
}