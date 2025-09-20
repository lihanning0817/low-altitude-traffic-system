#ifndef SERVICES_WEATHER_SERVICE_H
#define SERVICES_WEATHER_SERVICE_H

#include "../models/weather.h"
#include "../models/weather_dao.h"
#include "../models/route.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class WeatherService {
    private:
        std::shared_ptr<models::WeatherDAO> weather_dao_;

    public:
        explicit WeatherService(std::shared_ptr<models::WeatherDAO> weather_dao) : weather_dao_(weather_dao) {}

        // Weather data management
        std::optional<models::WeatherData> record_weather_data(double latitude, double longitude,
                                                             double temperature, double humidity,
                                                             double wind_speed, int wind_direction,
                                                             double pressure, double visibility,
                                                             const std::string& weather_condition);

        std::optional<models::WeatherData> get_current_weather(double latitude, double longitude);
        std::vector<models::WeatherData> get_weather_forecast(double latitude, double longitude,
                                                            int hours_ahead = 24);
        std::vector<models::WeatherData> get_weather_data_by_risk_level(models::WeatherRiskLevel risk_level);

        bool update_weather_data(const models::WeatherData& weather);
        bool delete_weather_data(const std::string& id);

        // Route weather assessment
        std::vector<models::WeatherData> get_route_weather(const models::Route& route);
        models::WeatherRiskLevel assess_route_weather_risk(const models::Route& route);

        // Helper methods
        std::string generate_weather_id();
        models::WeatherRiskLevel calculate_weather_risk(double wind_speed, double visibility,
                                                      const std::string& weather_condition);
        bool is_weather_safe_for_flight(double wind_speed, double visibility,
                                      const std::string& weather_condition);
    };
}

#endif // SERVICES_WEATHER_SERVICE_H