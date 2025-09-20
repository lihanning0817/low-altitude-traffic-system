#ifndef MODELS_EMERGENCY_LANDING_POINT_DAO_H
#define MODELS_EMERGENCY_LANDING_POINT_DAO_H

#include "emergency_landing_point.h"
#include <cppconn/connection.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class EmergencyLandingPointDAO {
    private:
        std::shared_ptr<sql::Connection> conn_;

    public:
        explicit EmergencyLandingPointDAO(std::shared_ptr<sql::Connection> conn) : conn_(conn) {}

        // Create
        std::string create_emergency_landing_point(const EmergencyLandingPoint& landing_point);

        // Read
        std::optional<EmergencyLandingPoint> get_emergency_landing_point_by_id(const std::string& id);
        std::vector<EmergencyLandingPoint> get_active_emergency_landing_points();
        std::vector<EmergencyLandingPoint> get_emergency_landing_points_by_capacity(int min_capacity);
        std::vector<EmergencyLandingPoint> get_nearest_emergency_landing_points(double latitude, double longitude, int limit = 5);

        // Update
        bool update_emergency_landing_point(const EmergencyLandingPoint& landing_point);
        bool activate_emergency_landing_point(const std::string& id);
        bool deactivate_emergency_landing_point(const std::string& id);

        // Delete
        bool delete_emergency_landing_point(const std::string& id);

    private:
        EmergencyLandingPoint result_set_to_emergency_landing_point(sql::ResultSet* rs);
    };
}

#endif // MODELS_EMERGENCY_LANDING_POINT_DAO_H