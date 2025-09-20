#include "traffic_service.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace services {
    std::vector<models::FlightConflict> TrafficService::detect_flight_conflicts() {
        std::vector<models::FlightConflict> conflicts;

        // Get all online drones
        auto online_drones = device_dao_->get_devices_by_type(models::DeviceType::DRONE);
        // Filter only online devices
        online_drones.erase(
            std::remove_if(online_drones.begin(), online_drones.end(),
                          [this](const models::Device& device) {
                              return device.status != models::DeviceStatus::ONLINE;
                          }),
            online_drones.end()
        );

        // Compare each pair of drones
        for (size_t i = 0; i < online_drones.size(); ++i) {
            for (size_t j = i + 1; j < online_drones.size(); ++j) {
                const auto& drone1 = online_drones[i];
                const auto& drone2 = online_drones[j];

                // Calculate distance between drones
                double distance = calculate_distance(drone1.latitude, drone1.longitude,
                                                   drone2.latitude, drone2.longitude);

                // Convert distance to meters (assuming calculate_distance returns kilometers)
                double distance_meters = distance * 1000;

                // If distance is less than safe distance, create conflict
                if (distance_meters < safe_distance_meters_) {
                    models::FlightConflict conflict;
                    conflict.id = generate_conflict_id();
                    conflict.flight1_id = drone1.id;
                    conflict.flight2_id = drone2.id;
                    conflict.conflict_time = std::chrono::system_clock::now();
                    conflict.distance = distance_meters;
                    conflict.severity = calculate_conflict_severity(distance_meters);
                    conflict.resolution_status = models::ResolutionStatus::PENDING;
                    conflict.created_at = std::chrono::system_clock::now();

                    // Save to database
                    std::string conflict_id = conflict_dao_->create_flight_conflict(conflict);
                    if (!conflict_id.empty()) {
                        conflict.id = conflict_id;
                        conflicts.push_back(conflict);
                    }
                }
            }
        }

        return conflicts;
    }

    std::vector<models::FlightConflict> TrafficService::detect_flight_conflicts_for_device(const std::string& device_id) {
        std::vector<models::FlightConflict> conflicts;

        // Get the device
        auto device_opt = device_dao_->get_device_by_id(device_id);
        if (!device_opt.has_value() || device_opt.value().status != models::DeviceStatus::ONLINE) {
            return conflicts; // Device not found or not online
        }

        auto device = device_opt.value();

        // Get all online drones
        auto online_drones = device_dao_->get_devices_by_type(models::DeviceType::DRONE);
        // Filter only online devices (excluding the device itself)
        online_drones.erase(
            std::remove_if(online_drones.begin(), online_drones.end(),
                          [this, &device_id](const models::Device& drone) {
                              return drone.status != models::DeviceStatus::ONLINE || drone.id == device_id;
                          }),
            online_drones.end()
        );

        // Compare with each other drone
        for (const auto& other_drone : online_drones) {
            // Calculate distance between drones
            double distance = calculate_distance(device.latitude, device.longitude,
                                               other_drone.latitude, other_drone.longitude);

            // Convert distance to meters (assuming calculate_distance returns kilometers)
            double distance_meters = distance * 1000;

            // If distance is less than safe distance, create conflict
            if (distance_meters < safe_distance_meters_) {
                models::FlightConflict conflict;
                conflict.id = generate_conflict_id();
                conflict.flight1_id = device.id;
                conflict.flight2_id = other_drone.id;
                conflict.conflict_time = std::chrono::system_clock::now();
                conflict.distance = distance_meters;
                conflict.severity = calculate_conflict_severity(distance_meters);
                conflict.resolution_status = models::ResolutionStatus::PENDING;
                conflict.created_at = std::chrono::system_clock::now();

                // Save to database
                std::string conflict_id = conflict_dao_->create_flight_conflict(conflict);
                if (!conflict_id.empty()) {
                    conflict.id = conflict_id;
                    conflicts.push_back(conflict);
                }
            }
        }

        return conflicts;
    }

    std::optional<models::FlightConflict> TrafficService::get_flight_conflict_by_id(const std::string& id) {
        return conflict_dao_->get_flight_conflict_by_id(id);
    }

    std::vector<models::FlightConflict> TrafficService::get_unresolved_flight_conflicts() {
        return conflict_dao_->get_unresolved_flight_conflicts();
    }

    std::vector<models::FlightConflict> TrafficService::get_all_flight_conflicts() {
        return conflict_dao_->get_all_flight_conflicts();
    }

    bool TrafficService::resolve_flight_conflict(const std::string& conflict_id,
                                               models::ResolutionStatus status,
                                               const std::optional<models::ResolutionAction>& action) {
        return conflict_dao_->update_flight_conflict_resolution(conflict_id, status, action);
    }

    bool TrafficService::delete_flight_conflict(const std::string& id) {
        return conflict_dao_->delete_flight_conflict(id);
    }

    std::string TrafficService::generate_conflict_id() {
        // Generate a unique conflict ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "FC" << std::setfill('0') << std::setw(10) << (millis % 10000000000ULL);

        return ss.str();
    }

    double TrafficService::calculate_distance(double lat1, double lng1, double lat2, double lng2) {
        // Haversine formula to calculate distance between two points
        const double R = 6371.0; // Earth radius in kilometers
        const double to_radians = M_PI / 180.0;

        double dlat = (lat2 - lat1) * to_radians;
        double dlng = (lng2 - lng1) * to_radians;

        double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
                   std::cos(lat1 * to_radians) * std::cos(lat2 * to_radians) *
                   std::sin(dlng / 2) * std::sin(dlng / 2);

        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return R * c;
    }

    models::ConflictSeverity TrafficService::calculate_conflict_severity(double distance_meters) {
        if (distance_meters < safe_distance_meters_ / 3) {
            return models::ConflictSeverity::CRITICAL;
        } else if (distance_meters < safe_distance_meters_ * 2 / 3) {
            return models::ConflictSeverity::HIGH;
        } else if (distance_meters < safe_distance_meters_) {
            return models::ConflictSeverity::MEDIUM;
        } else {
            return models::ConflictSeverity::LOW;
        }
    }

    std::optional<models::ResolutionAction> TrafficService::generate_resolution_action(models::ConflictSeverity severity) {
        models::ResolutionAction action;

        switch (severity) {
            case models::ConflictSeverity::CRITICAL:
                action.type = "immediate_separation";
                action.actions = {
                    "hold_position_for_10_seconds",
                    "adjust_altitude_by_10_meters"
                };
                break;
            case models::ConflictSeverity::HIGH:
                action.type = "route_adjustment";
                action.actions = {
                    "reduce_speed_by_20_percent",
                    "change_heading_by_30_degrees"
                };
                break;
            case models::ConflictSeverity::MEDIUM:
                action.type = "monitoring";
                action.actions = {
                    "continue_monitoring",
                    "maintain_current_course"
                };
                break;
            case models::ConflictSeverity::LOW:
                action.type = "informational";
                action.actions = {
                    "notify_pilots",
                    "log_conflict"
                };
                break;
            default:
                return std::nullopt;
        }

        return action;
    }
}