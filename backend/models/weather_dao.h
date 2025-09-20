#ifndef MODELS_WEATHER_DAO_H
#define MODELS_WEATHER_DAO_H

#include "weather.h"
#include <cppconn/connection.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class WeatherDAO {
    private:
        std::shared_ptr<sql::Connection> conn_;

    public:
        explicit WeatherDAO(std::shared_ptr<sql::Connection> conn) : conn_(conn) {}

        // Create
        std::string create_weather_data(const WeatherData& weather);

        // Read
        std::optional<WeatherData> get_weather_data_by_id(const std::string& id);
        std::optional<WeatherData> get_latest_weather_data(double latitude, double longitude);
        std::vector<WeatherData> get_weather_data_by_location(double latitude, double longitude,
                                                            const std::chrono::system_clock::time_point& start_time,
                                                            const std::chrono::system_clock::time_point& end_time);
        std::vector<WeatherData> get_weather_data_by_risk_level(WeatherRiskLevel risk_level);

        // Update
        bool update_weather_data(const WeatherData& weather);

        // Delete
        bool delete_weather_data(const std::string& id);
        bool delete_old_weather_data(int days_old = 7);

    private:
        WeatherData result_set_to_weather_data(sql::ResultSet* rs);
    };
}

#endif // MODELS_WEATHER_DAO_H