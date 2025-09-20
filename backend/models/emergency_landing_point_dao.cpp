#include "emergency_landing_point_dao.h"
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <stdexcept>
#include <iostream>

namespace models {
    std::string EmergencyLandingPointDAO::create_emergency_landing_point(const EmergencyLandingPoint& landing_point) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "INSERT INTO emergency_landing_points (id, name, latitude, longitude, altitude, "
                    "capacity, is_active, description) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
                )
            );

            pstmt->setString(1, landing_point.id);
            pstmt->setString(2, landing_point.name);
            pstmt->setDouble(3, landing_point.latitude);
            pstmt->setDouble(4, landing_point.longitude);
            pstmt->setDouble(5, landing_point.altitude);
            pstmt->setInt(6, landing_point.capacity);
            pstmt->setBoolean(7, landing_point.is_active);
            pstmt->setString(8, landing_point.description);

            int rows_affected = pstmt->executeUpdate();
            if (rows_affected > 0) {
                return landing_point.id;
            }
            return "";
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in create_emergency_landing_point: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<EmergencyLandingPoint> EmergencyLandingPointDAO::get_emergency_landing_point_by_id(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM emergency_landing_points WHERE id = ?")
            );
            pstmt->setString(1, id);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            if (rs->next()) {
                return result_set_to_emergency_landing_point(rs.get());
            }
            return std::nullopt;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_emergency_landing_point_by_id: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<EmergencyLandingPoint> EmergencyLandingPointDAO::get_active_emergency_landing_points() {
        std::vector<EmergencyLandingPoint> landing_points;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM emergency_landing_points WHERE is_active = TRUE ORDER BY name")
            );

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                landing_points.push_back(result_set_to_emergency_landing_point(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_active_emergency_landing_points: " << e.what() << std::endl;
        }
        return landing_points;
    }

    std::vector<EmergencyLandingPoint> EmergencyLandingPointDAO::get_emergency_landing_points_by_capacity(int min_capacity) {
        std::vector<EmergencyLandingPoint> landing_points;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM emergency_landing_points WHERE capacity >= ? AND is_active = TRUE ORDER BY capacity DESC")
            );
            pstmt->setInt(1, min_capacity);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                landing_points.push_back(result_set_to_emergency_landing_point(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_emergency_landing_points_by_capacity: " << e.what() << std::endl;
        }
        return landing_points;
    }

    std::vector<EmergencyLandingPoint> EmergencyLandingPointDAO::get_nearest_emergency_landing_points(double latitude, double longitude, int limit) {
        std::vector<EmergencyLandingPoint> landing_points;
        try {
            // This is a simplified approach - in a real implementation, you'd use a proper geospatial query
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "SELECT *, "
                    "(6371 * acos(cos(radians(?)) * cos(radians(latitude)) * cos(radians(longitude) - radians(?)) + "
                    "sin(radians(?)) * sin(radians(latitude)))) AS distance "
                    "FROM emergency_landing_points WHERE is_active = TRUE "
                    "ORDER BY distance ASC LIMIT ?"
                )
            );
            pstmt->setDouble(1, latitude);
            pstmt->setDouble(2, longitude);
            pstmt->setDouble(3, latitude);
            pstmt->setInt(4, limit);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                landing_points.push_back(result_set_to_emergency_landing_point(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_nearest_emergency_landing_points: " << e.what() << std::endl;
        }
        return landing_points;
    }

    bool EmergencyLandingPointDAO::update_emergency_landing_point(const EmergencyLandingPoint& landing_point) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "UPDATE emergency_landing_points SET name = ?, latitude = ?, longitude = ?, altitude = ?, "
                    "capacity = ?, is_active = ?, description = ?, updated_at = NOW() WHERE id = ?"
                )
            );

            pstmt->setString(1, landing_point.name);
            pstmt->setDouble(2, landing_point.latitude);
            pstmt->setDouble(3, landing_point.longitude);
            pstmt->setDouble(4, landing_point.altitude);
            pstmt->setInt(5, landing_point.capacity);
            pstmt->setBoolean(6, landing_point.is_active);
            pstmt->setString(7, landing_point.description);
            pstmt->setString(8, landing_point.id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in update_emergency_landing_point: " << e.what() << std::endl;
            return false;
        }
    }

    bool EmergencyLandingPointDAO::activate_emergency_landing_point(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("UPDATE emergency_landing_points SET is_active = TRUE, updated_at = NOW() WHERE id = ?")
            );
            pstmt->setString(1, id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in activate_emergency_landing_point: " << e.what() << std::endl;
            return false;
        }
    }

    bool EmergencyLandingPointDAO::deactivate_emergency_landing_point(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("UPDATE emergency_landing_points SET is_active = FALSE, updated_at = NOW() WHERE id = ?")
            );
            pstmt->setString(1, id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in deactivate_emergency_landing_point: " << e.what() << std::endl;
            return false;
        }
    }

    bool EmergencyLandingPointDAO::delete_emergency_landing_point(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("DELETE FROM emergency_landing_points WHERE id = ?")
            );
            pstmt->setString(1, id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in delete_emergency_landing_point: " << e.what() << std::endl;
            return false;
        }
    }

    EmergencyLandingPoint EmergencyLandingPointDAO::result_set_to_emergency_landing_point(sql::ResultSet* rs) {
        EmergencyLandingPoint landing_point;
        landing_point.id = rs->getString("id");
        landing_point.name = rs->getString("name");
        landing_point.latitude = rs->getDouble("latitude");
        landing_point.longitude = rs->getDouble("longitude");
        landing_point.altitude = rs->getDouble("altitude");
        landing_point.capacity = rs->getInt("capacity");
        landing_point.is_active = rs->getBoolean("is_active");
        landing_point.description = rs->getString("description");

        // Handle timestamps
        auto created_at_ts = rs->getTimestamp("created_at");
        landing_point.created_at = std::chrono::system_clock::from_time_t(created_at_ts->getTime() / 1000);

        auto updated_at_ts = rs->getTimestamp("updated_at");
        landing_point.updated_at = std::chrono::system_clock::from_time_t(updated_at_ts->getTime() / 1000);

        return landing_point;
    }
}