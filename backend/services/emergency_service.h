#ifndef SERVICES_EMERGENCY_SERVICE_H
#define SERVICES_EMERGENCY_SERVICE_H

#include "../models/emergency_event.h"
#include "../models/emergency_event_dao.h"
#include "../models/emergency_landing_point.h"
#include "../models/emergency_landing_point_dao.h"
#include "../models/device.h"
#include "../models/device_dao.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class EmergencyService {
    private:
        std::shared_ptr<models::EmergencyEventDAO> emergency_dao_;
        std::shared_ptr<models::EmergencyLandingPointDAO> landing_point_dao_;
        std::shared_ptr<models::DeviceDAO> device_dao_;

    public:
        explicit EmergencyService(std::shared_ptr<models::EmergencyEventDAO> emergency_dao,
                                std::shared_ptr<models::EmergencyLandingPointDAO> landing_point_dao,
                                std::shared_ptr<models::DeviceDAO> device_dao)
            : emergency_dao_(emergency_dao), landing_point_dao_(landing_point_dao), device_dao_(device_dao) {}

        // Emergency event management
        std::optional<models::EmergencyEvent> report_emergency(models::EmergencyType type,
                                                             models::EmergencySeverity severity,
                                                             double latitude, double longitude, double altitude,
                                                             const std::string& description,
                                                             int reporter_id = 0);

        std::optional<models::EmergencyEvent> get_emergency_event_by_id(const std::string& id);
        std::vector<models::EmergencyEvent> get_emergency_events_by_status(models::EmergencyStatus status);
        std::vector<models::EmergencyEvent> get_emergency_events_by_severity(models::EmergencySeverity severity);
        std::vector<models::EmergencyEvent> get_active_emergency_events();
        std::vector<models::EmergencyEvent> get_all_emergency_events();

        bool update_emergency_event(const models::EmergencyEvent& emergency);
        bool assign_responder_to_emergency(const std::string& emergency_id, int responder_id);
        bool update_emergency_event_status(const std::string& emergency_id, models::EmergencyStatus status, int updater_id = 0);
        bool resolve_emergency_event(const std::string& emergency_id, int resolver_id = 0);

        bool delete_emergency_event(const std::string& id);

        // Emergency landing point management
        std::optional<models::EmergencyLandingPoint> register_emergency_landing_point(const std::string& name,
                                                                                   double latitude, double longitude, double altitude,
                                                                                   int capacity, const std::string& description);

        std::optional<models::EmergencyLandingPoint> get_emergency_landing_point_by_id(const std::string& id);
        std::vector<models::EmergencyLandingPoint> get_active_emergency_landing_points();
        std::vector<models::EmergencyLandingPoint> get_emergency_landing_points_by_capacity(int min_capacity);
        std::vector<models::EmergencyLandingPoint> get_nearest_emergency_landing_points(double latitude, double longitude, int limit = 5);

        bool update_emergency_landing_point(const models::EmergencyLandingPoint& landing_point);
        bool activate_emergency_landing_point(const std::string& id);
        bool deactivate_emergency_landing_point(const std::string& id);

        bool delete_emergency_landing_point(const std::string& id);

        // Helper methods
        std::string generate_emergency_id();
        std::string generate_landing_point_id();
        std::optional<models::EmergencyLandingPoint> find_nearest_available_landing_point(double latitude, double longitude);
        models::EmergencySeverity assess_emergency_severity(models::EmergencyType type, double altitude);
    };
}

#endif // SERVICES_EMERGENCY_SERVICE_H