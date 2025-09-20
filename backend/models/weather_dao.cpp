#include "weather_dao.h"
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <stdexcept>
#include <iostream>

namespace models {
    std::string WeatherDAO::create_weather_data(const WeatherData& weather) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "INSERT INTO weather_data (id, latitude, longitude, temperature, humidity, "
                    "wind_speed, wind_direction, pressure, visibility, weather_condition, "
                    "risk_level) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                )
            );

            pstmt->setString(1, weather.id);
            pstmt->setDouble(2, weather.latitude);
            pstmt->setDouble(3, weather.longitude);
            pstmt->setDouble(4, weather.temperature);
            pstmt->setDouble(5, weather.humidity);
            pstmt->setDouble(6, weather.wind_speed);
            pstmt->setInt(7, weather.wind_direction);
            pstmt->setDouble(8, weather.pressure);
            pstmt->setDouble(9, weather.visibility);
            pstmt->setString(10, weather.weather_condition);
            pstmt->setString(11, weather.get_risk_level_string());

            int rows_affected = pstmt->executeUpdate();
            if (rows_affected > 0) {
                return weather.id;
            }
            return "";
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in create_weather_data: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<WeatherData> WeatherDAO::get_weather_data_by_id(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM weather_data WHERE id = ?")
            );
            pstmt->setString(1, id);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            if (rs->next()) {
                return result_set_to_weather_data(rs.get());
            }
            return std::nullopt;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_weather_data_by_id: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<WeatherData> WeatherDAO::get_latest_weather_data(double latitude, double longitude) {
        try {
            // Find weather data within a small radius (approximately 1km)
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "SELECT * FROM weather_data WHERE "
                    "ABS(latitude - ?) < 0.01 AND ABS(longitude - ?) < 0.01 "
                    "ORDER BY recorded_at DESC LIMIT 1"
                )
            );
            pstmt->setDouble(1, latitude);
            pstmt->setDouble(2, longitude);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            if (rs->next()) {
                return result_set_to_weather_data(rs.get());
            }
            return std::nullopt;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_latest_weather_data: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<WeatherData> WeatherDAO::get_weather_data_by_location(double latitude, double longitude,
                                                                    const std::chrono::system_clock::time_point& start_time,
                                                                    const std::chrono::system_clock::time_point& end_time) {
        std::vector<WeatherData> weather_data;
        try {
            // Find weather data within a small radius (approximately 1km)
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "SELECT * FROM weather_data WHERE "
                    "ABS(latitude - ?) < 0.01 AND ABS(longitude - ?) < 0.01 "
                    "AND recorded_at BETWEEN ? AND ? "
                    "ORDER BY recorded_at DESC"
                )
            );
            pstmt->setDouble(1, latitude);
            pstmt->setDouble(2, longitude);

            auto start_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                start_time.time_since_epoch()).count();
            pstmt->setTimestamp(3, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(start_time_ts)));

            auto end_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time.time_since_epoch()).count();
            pstmt->setTimestamp(4, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(end_time_ts)));

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                weather_data.push_back(result_set_to_weather_data(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_weather_data_by_location: " << e.what() << std::endl;
        }
        return weather_data;
    }

    std::vector<WeatherData> WeatherDAO::get_weather_data_by_risk_level(WeatherRiskLevel risk_level) {
        std::vector<WeatherData> weather_data;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM weather_data WHERE risk_level = ? ORDER BY recorded_at DESC")
            );
            pstmt->setString(1, WeatherData::get_risk_level_string());

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                weather_data.push_back(result_set_to_weather_data(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_weather_data_by_risk_level: " << e.what() << std::endl;
        }
        return weather_data;
    }

    bool WeatherDAO::update_weather_data(const WeatherData& weather) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "UPDATE weather_data SET latitude = ?, longitude = ?, temperature = ?, humidity = ?, "
                    "wind_speed = ?, wind_direction = ?, pressure = ?, visibility = ?, "
                    "weather_condition = ?, risk_level = ?, recorded_at = ? WHERE id = ?"
                )
            );

            pstmt->setDouble(1, weather.latitude);
            pstmt->setDouble(2, weather.longitude);
            pstmt->setDouble(3, weather.temperature);
            pstmt->setDouble(4, weather.humidity);
            pstmt->setDouble(5, weather.wind_speed);
            pstmt->setInt(6, weather.wind_direction);
            pstmt->setDouble(7, weather.pressure);
            pstmt->setDouble(8, weather.visibility);
            pstmt->setString(9, weather.weather_condition);
            pstmt->setString(10, weather.get_risk_level_string());

            auto recorded_at_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                weather.recorded_at.time_since_epoch()).count();
            pstmt->setTimestamp(11, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(recorded_at_ts)));

            pstmt->setString(12, weather.id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in update_weather_data: " << e.what() << std::endl;
            return false;
        }
    }

    bool WeatherDAO::delete_weather_data(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("DELETE FROM weather_data WHERE id = ?")
            );
            pstmt->setString(1, id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in delete_weather_data: " << e.what() << std::endl;
            return false;
        }
    }

    bool WeatherDAO::delete_old_weather_data(int days_old) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "DELETE FROM weather_data WHERE recorded_at < DATE_SUB(NOW(), INTERVAL ? DAY)"
                )
            );
            pstmt->setInt(1, days_old);

            int rows_affected = pstmt->executeUpdate();
            return true;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in delete_old_weather_data: " << e.what() << std::endl;
            return false;
        }
    }

    WeatherData WeatherDAO::result_set_to_weather_data(sql::ResultSet* rs) {
        WeatherData weather;
        weather.id = rs->getString("id");
        weather.latitude = rs->getDouble("latitude");
        weather.longitude = rs->getDouble("longitude");
        weather.temperature = rs->getDouble("temperature");
        weather.humidity = rs->getDouble("humidity");
        weather.wind_speed = rs->getDouble("wind_speed");
        weather.wind_direction = rs->getInt("wind_direction");
        weather.pressure = rs->getDouble("pressure");
        weather.visibility = rs->getDouble("visibility");
        weather.weather_condition = rs->getString("weather_condition");
        weather.risk_level = WeatherData::parse_risk_level_string(rs->getString("risk_level"));

        // Handle timestamps
        auto recorded_at_ts = rs->getTimestamp("recorded_at");
        weather.recorded_at = std::chrono::system_clock::from_time_t(recorded_at_ts->getTime() / 1000);

        auto created_at_ts = rs->getTimestamp("created_at");
        weather.created_at = std::chrono::system_clock::from_time_t(created_at_ts->getTime() / 1000);

        return weather;
    }
}