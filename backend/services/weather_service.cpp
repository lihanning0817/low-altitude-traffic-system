#include "weather_service.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace services {
    std::optional<models::WeatherData> WeatherService::record_weather_data(double latitude, double longitude,
                                                                        double temperature, double humidity,
                                                                        double wind_speed, int wind_direction,
                                                                        double pressure, double visibility,
                                                                        const std::string& weather_condition) {
        // Create weather data object
        models::WeatherData weather;
        weather.id = generate_weather_id();
        weather.latitude = latitude;
        weather.longitude = longitude;
        weather.temperature = temperature;
        weather.humidity = humidity;
        weather.wind_speed = wind_speed;
        weather.wind_direction = wind_direction;
        weather.pressure = pressure;
        weather.visibility = visibility;
        weather.weather_condition = weather_condition;
        weather.risk_level = calculate_weather_risk(wind_speed, visibility, weather_condition);
        weather.recorded_at = std::chrono::system_clock::now();
        weather.created_at = std::chrono::system_clock::now();

        // Save to database
        std::string weather_id = weather_dao_->create_weather_data(weather);
        if (!weather_id.empty()) {
            weather.id = weather_id;
            return weather;
        }

        return std::nullopt;
    }

    std::optional<models::WeatherData> WeatherService::get_current_weather(double latitude, double longitude) {
        return weather_dao_->get_latest_weather_data(latitude, longitude);
    }

    std::vector<models::WeatherData> WeatherService::get_weather_forecast(double latitude, double longitude,
                                                                        int hours_ahead) {
        auto now = std::chrono::system_clock::now();
        auto end_time = now + std::chrono::hours(hours_ahead);
        return weather_dao_->get_weather_data_by_location(latitude, longitude, now, end_time);
    }

    std::vector<models::WeatherData> WeatherService::get_weather_data_by_risk_level(models::WeatherRiskLevel risk_level) {
        return weather_dao_->get_weather_data_by_risk_level(risk_level);
    }

    bool WeatherService::update_weather_data(const models::WeatherData& weather) {
        return weather_dao_->update_weather_data(weather);
    }

    bool WeatherService::delete_weather_data(const std::string& id) {
        return weather_dao_->delete_weather_data(id);
    }

    std::vector<models::WeatherData> WeatherService::get_route_weather(const models::Route& route) {
        std::vector<models::WeatherData> route_weather;

        // Get weather data for each waypoint in the route
        for (const auto& waypoint : route.waypoints) {
            auto weather = get_current_weather(waypoint.latitude, waypoint.longitude);
            if (weather.has_value()) {
                route_weather.push_back(weather.value());
            }
        }

        return route_weather;
    }

    models::WeatherRiskLevel WeatherService::assess_route_weather_risk(const models::Route& route) {
        auto route_weather = get_route_weather(route);

        // If no weather data, assume low risk
        if (route_weather.empty()) {
            return models::WeatherRiskLevel::LOW;
        }

        // Find the highest risk level along the route
        models::WeatherRiskLevel max_risk = models::WeatherRiskLevel::LOW;
        for (const auto& weather : route_weather) {
            if (weather.risk_level > max_risk) {
                max_risk = weather.risk_level;
            }
        }

        return max_risk;
    }

    std::string WeatherService::generate_weather_id() {
        // Generate a unique weather ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "WD" << std::setfill('0') << std::setw(10) << (millis % 10000000000ULL);

        return ss.str();
    }

    models::WeatherRiskLevel WeatherService::calculate_weather_risk(double wind_speed, double visibility,
                                                                 const std::string& weather_condition) {
        // Calculate risk based on wind speed, visibility, and weather condition
        if (wind_speed > 15.0 || visibility < 1.0) { // High wind or very low visibility
            return models::WeatherRiskLevel::CRITICAL;
        } else if (wind_speed > 10.0 || visibility < 2.0) { // Moderate wind or low visibility
            return models::WeatherRiskLevel::HIGH;
        } else if (wind_speed > 5.0 || visibility < 5.0) { // Light wind or moderate visibility
            return models::WeatherRiskLevel::MEDIUM;
        } else {
            // Check weather condition
            std::string condition = weather_condition;
            std::transform(condition.begin(), condition.end(), condition.begin(), ::tolower);

            if (condition.find("rain") != std::string::npos ||
                condition.find("storm") != std::string::npos ||
                condition.find("snow") != std::string::npos) {
                return models::WeatherRiskLevel::MEDIUM;
            }

            return models::WeatherRiskLevel::LOW;
        }
    }

    bool WeatherService::is_weather_safe_for_flight(double wind_speed, double visibility,
                                                  const std::string& weather_condition) {
        auto risk_level = calculate_weather_risk(wind_speed, visibility, weather_condition);
        return risk_level != models::WeatherRiskLevel::CRITICAL;
    }
}