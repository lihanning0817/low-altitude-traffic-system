#include "emergency_service.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace services {
    std::optional<models::EmergencyEvent> EmergencyService::report_emergency(models::EmergencyType type,
                                                                           models::EmergencySeverity severity,
                                                                           double latitude, double longitude, double altitude,
                                                                           const std::string& description,
                                                                           int reporter_id) {
        // Create emergency event object
        models::EmergencyEvent emergency;
        emergency.id = generate_emergency_id();
        emergency.type = type;
        emergency.severity = severity;
        emergency.status = models::EmergencyStatus::REPORTED;
        emergency.latitude = latitude;
        emergency.longitude = longitude;
        emergency.altitude = altitude;
        emergency.description = description;
        if (reporter_id > 0) {
            emergency.reporter_id = reporter_id;
        }
        emergency.reported_at = std::chrono::system_clock::now();
        emergency.created_at = std::chrono::system_clock::now();

        // Save to database
        std::string emergency_id = emergency_dao_->create_emergency_event(emergency);
        if (!emergency_id.empty()) {
            emergency.id = emergency_id;
            return emergency;
        }

        return std::nullopt;
    }

    std::optional<models::EmergencyEvent> EmergencyService::get_emergency_event_by_id(const std::string& id) {
        return emergency_dao_->get_emergency_event_by_id(id);
    }

    std::vector<models::EmergencyEvent> EmergencyService::get_emergency_events_by_status(models::EmergencyStatus status) {
        return emergency_dao_->get_emergency_events_by_status(status);
    }

    std::vector<models::EmergencyEvent> EmergencyService::get_emergency_events_by_severity(models::EmergencySeverity severity) {
        return emergency_dao_->get_emergency_events_by_severity(severity);
    }

    std::vector<models::EmergencyEvent> EmergencyService::get_active_emergency_events() {
        return emergency_dao_->get_active_emergency_events();
    }

    std::vector<models::EmergencyEvent> EmergencyService::get_all_emergency_events() {
        return emergency_dao_->get_all_emergency_events();
    }

    bool EmergencyService::update_emergency_event(const models::EmergencyEvent& emergency) {
        return emergency_dao_->update_emergency_event(emergency);
    }

    bool EmergencyService::assign_responder_to_emergency(const std::string& emergency_id, int responder_id) {
        return emergency_dao_->update_emergency_event_status(emergency_id, models::EmergencyStatus::RESPONDING, responder_id);
    }

    bool EmergencyService::update_emergency_event_status(const std::string& emergency_id, models::EmergencyStatus status, int updater_id) {
        return emergency_dao_->update_emergency_event_status(emergency_id, status, updater_id);
    }

    bool EmergencyService::resolve_emergency_event(const std::string& emergency_id, int resolver_id) {
        return emergency_dao_->resolve_emergency_event(emergency_id, resolver_id);
    }

    bool EmergencyService::delete_emergency_event(const std::string& id) {
        return emergency_dao_->delete_emergency_event(id);
    }

    std::optional<models::EmergencyLandingPoint> EmergencyService::register_emergency_landing_point(const std::string& name,
                                                                                               double latitude, double longitude, double altitude,
                                                                                               int capacity, const std::string& description) {
        // Create emergency landing point object
        models::EmergencyLandingPoint landing_point;
        landing_point.id = generate_landing_point_id();
        landing_point.name = name;
        landing_point.latitude = latitude;
        landing_point.longitude = longitude;
        landing_point.altitude = altitude;
        landing_point.capacity = capacity;
        landing_point.is_active = true;
        landing_point.description = description;
        landing_point.created_at = std::chrono::system_clock::now();
        landing_point.updated_at = std::chrono::system_clock::now();

        // Save to database
        std::string landing_point_id = landing_point_dao_->create_emergency_landing_point(landing_point);
        if (!landing_point_id.empty()) {
            landing_point.id = landing_point_id;
            return landing_point;
        }

        return std::nullopt;
    }

    std::optional<models::EmergencyLandingPoint> EmergencyService::get_emergency_landing_point_by_id(const std::string& id) {
        return landing_point_dao_->get_emergency_landing_point_by_id(id);
    }

    std::vector<models::EmergencyLandingPoint> EmergencyService::get_active_emergency_landing_points() {
        return landing_point_dao_->get_active_emergency_landing_points();
    }

    std::vector<models::EmergencyLandingPoint> EmergencyService::get_emergency_landing_points_by_capacity(int min_capacity) {
        return landing_point_dao_->get_emergency_landing_points_by_capacity(min_capacity);
    }

    std::vector<models::EmergencyLandingPoint> EmergencyService::get_nearest_emergency_landing_points(double latitude, double longitude, int limit) {
        return landing_point_dao_->get_nearest_emergency_landing_points(latitude, longitude, limit);
    }

    bool EmergencyService::update_emergency_landing_point(const models::EmergencyLandingPoint& landing_point) {
        return landing_point_dao_->update_emergency_landing_point(landing_point);
    }

    bool EmergencyService::activate_emergency_landing_point(const std::string& id) {
        return landing_point_dao_->activate_emergency_landing_point(id);
    }

    bool EmergencyService::deactivate_emergency_landing_point(const std::string& id) {
        return landing_point_dao_->deactivate_emergency_landing_point(id);
    }

    bool EmergencyService::delete_emergency_landing_point(const std::string& id) {
        return landing_point_dao_->delete_emergency_landing_point(id);
    }

    std::string EmergencyService::generate_emergency_id() {
        // Generate a unique emergency ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "EE" << std::setfill('0') << std::setw(10) << (millis % 10000000000ULL);

        return ss.str();
    }

    std::string EmergencyService::generate_landing_point_id() {
        // Generate a unique landing point ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "ELP" << std::setfill('0') << std::setw(7) << (millis % 10000000ULL);

        return ss.str();
    }

    std::optional<models::EmergencyLandingPoint> EmergencyService::find_nearest_available_landing_point(double latitude, double longitude) {
        auto nearest_points = get_nearest_emergency_landing_points(latitude, longitude, 1);
        if (!nearest_points.empty()) {
            return nearest_points[0];
        }
        return std::nullopt;
    }

    models::EmergencySeverity EmergencyService::assess_emergency_severity(models::EmergencyType type, double altitude) {
        // Assess severity based on emergency type and altitude
        switch (type) {
            case models::EmergencyType::ACCIDENT:
                return models::EmergencySeverity::CRITICAL;
            case models::EmergencyType::MALFUNCTION:
                if (altitude > 100.0) {
                    return models::EmergencySeverity::HIGH;
                } else {
                    return models::EmergencySeverity::MEDIUM;
                }
            case models::EmergencyType::WEATHER:
                return models::EmergencySeverity::MEDIUM;
            case models::EmergencyType::OTHER:
            default:
                if (altitude > 50.0) {
                    return models::EmergencySeverity::MEDIUM;
                } else {
                    return models::EmergencySeverity::LOW;
                }
        }
    }
}