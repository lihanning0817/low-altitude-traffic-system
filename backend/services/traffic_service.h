#ifndef SERVICES_TRAFFIC_SERVICE_H
#define SERVICES_TRAFFIC_SERVICE_H

#include "../models/flight_conflict.h"
#include "../models/flight_conflict_dao.h"
#include "../models/device.h"
#include "../models/device_dao.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class TrafficService {
    private:
        std::shared_ptr<models::FlightConflictDAO> conflict_dao_;
        std::shared_ptr<models::DeviceDAO> device_dao_;
        double safe_distance_meters_; // Safe distance between flights in meters

    public:
        explicit TrafficService(std::shared_ptr<models::FlightConflictDAO> conflict_dao,
                               std::shared_ptr<models::DeviceDAO> device_dao,
                               double safe_distance_meters = 50.0)
            : conflict_dao_(conflict_dao), device_dao_(device_dao),
              safe_distance_meters_(safe_distance_meters) {}

        // Traffic monitoring
        std::vector<models::FlightConflict> detect_flight_conflicts();
        std::vector<models::FlightConflict> detect_flight_conflicts_for_device(const std::string& device_id);
        std::optional<models::FlightConflict> get_flight_conflict_by_id(const std::string& id);
        std::vector<models::FlightConflict> get_unresolved_flight_conflicts();
        std::vector<models::FlightConflict> get_all_flight_conflicts();

        bool resolve_flight_conflict(const std::string& conflict_id,
                                   models::ResolutionStatus status,
                                   const std::optional<models::ResolutionAction>& action = std::nullopt);

        bool delete_flight_conflict(const std::string& id);

        // Helper methods
        std::string generate_conflict_id();
        double calculate_distance(double lat1, double lng1, double lat2, double lng2);
        models::ConflictSeverity calculate_conflict_severity(double distance);
        std::optional<models::ResolutionAction> generate_resolution_action(models::ConflictSeverity severity);
    };
}

#endif // SERVICES_TRAFFIC_SERVICE_H