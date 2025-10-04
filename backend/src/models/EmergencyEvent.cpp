#include "EmergencyEvent.h"
#include <sstream>
#include <iomanip>

namespace models {

nlohmann::json EmergencyEvent::toJson() const {
    nlohmann::json json;

    json["id"] = id;
    json["event_code"] = event_code;
    json["task_id"] = task_id;

    if (drone_id.has_value()) {
        json["drone_id"] = drone_id.value();
    }

    json["type"] = typeToString(type);
    json["severity"] = severityToString(severity);
    json["status"] = statusToString(status);
    json["title"] = title;

    if (description.has_value()) {
        json["description"] = description.value();
    }

    json["location"] = location;

    if (response_action.has_value()) {
        json["response_action"] = response_action.value();
    }

    if (response_notes.has_value()) {
        json["response_notes"] = response_notes.value();
    }

    if (responded_by.has_value()) {
        json["responded_by"] = responded_by.value();
    }

    if (responded_at.has_value()) {
        auto time = std::chrono::system_clock::to_time_t(responded_at.value());
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S");
        json["responded_at"] = ss.str();
    }

    if (resolved_at.has_value()) {
        auto time = std::chrono::system_clock::to_time_t(resolved_at.value());
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S");
        json["resolved_at"] = ss.str();
    }

    auto created_time = std::chrono::system_clock::to_time_t(created_at);
    std::stringstream created_ss;
    created_ss << std::put_time(std::gmtime(&created_time), "%Y-%m-%d %H:%M:%S");
    json["created_at"] = created_ss.str();

    auto updated_time = std::chrono::system_clock::to_time_t(updated_at);
    std::stringstream updated_ss;
    updated_ss << std::put_time(std::gmtime(&updated_time), "%Y-%m-%d %H:%M:%S");
    json["updated_at"] = updated_ss.str();

    return json;
}

EmergencyEvent EmergencyEvent::fromDatabaseJson(const nlohmann::json& json) {
    EmergencyEvent event;

    event.id = json.value("id", 0);
    event.event_code = json.value("event_code", "");
    event.task_id = json.value("task_id", 0);

    if (json.contains("drone_id") && !json["drone_id"].is_null()) {
        event.drone_id = json["drone_id"].get<int64_t>();
    }

    event.type = stringToType(json.value("type", "other"));
    event.severity = stringToSeverity(json.value("severity", "medium"));
    event.status = stringToStatus(json.value("status", "active"));
    event.title = json.value("title", "");

    if (json.contains("description") && !json["description"].is_null()) {
        event.description = json["description"].get<std::string>();
    }

    event.location = json.value("location", nlohmann::json::object());

    if (json.contains("response_action") && !json["response_action"].is_null()) {
        event.response_action = json["response_action"].get<std::string>();
    }

    if (json.contains("response_notes") && !json["response_notes"].is_null()) {
        event.response_notes = json["response_notes"].get<std::string>();
    }

    if (json.contains("responded_by") && !json["responded_by"].is_null()) {
        event.responded_by = json["responded_by"].get<int64_t>();
    }

    // Parse timestamps (simplified - assumes ISO format)
    if (json.contains("responded_at") && !json["responded_at"].is_null()) {
        // Simplified timestamp parsing
        event.responded_at = std::chrono::system_clock::now();
    }

    if (json.contains("resolved_at") && !json["resolved_at"].is_null()) {
        event.resolved_at = std::chrono::system_clock::now();
    }

    event.created_at = std::chrono::system_clock::now();
    event.updated_at = std::chrono::system_clock::now();

    return event;
}

std::string EmergencyEvent::typeToString(EmergencyType type) {
    switch (type) {
        case EmergencyType::EQUIPMENT_FAILURE: return "equipment_failure";
        case EmergencyType::WEATHER_EMERGENCY: return "weather_emergency";
        case EmergencyType::COLLISION_RISK: return "collision_risk";
        case EmergencyType::SIGNAL_LOSS: return "signal_loss";
        case EmergencyType::BATTERY_LOW: return "battery_low";
        case EmergencyType::GEOFENCE_VIOLATION: return "geofence_violation";
        case EmergencyType::MANUAL_EMERGENCY: return "manual_emergency";
        case EmergencyType::OTHER: return "other";
        default: return "other";
    }
}

EmergencyType EmergencyEvent::stringToType(const std::string& str) {
    if (str == "equipment_failure") return EmergencyType::EQUIPMENT_FAILURE;
    if (str == "weather_emergency") return EmergencyType::WEATHER_EMERGENCY;
    if (str == "collision_risk") return EmergencyType::COLLISION_RISK;
    if (str == "signal_loss") return EmergencyType::SIGNAL_LOSS;
    if (str == "battery_low") return EmergencyType::BATTERY_LOW;
    if (str == "geofence_violation") return EmergencyType::GEOFENCE_VIOLATION;
    if (str == "manual_emergency") return EmergencyType::MANUAL_EMERGENCY;
    return EmergencyType::OTHER;
}

std::string EmergencyEvent::severityToString(EmergencySeverity severity) {
    switch (severity) {
        case EmergencySeverity::LOW: return "low";
        case EmergencySeverity::MEDIUM: return "medium";
        case EmergencySeverity::HIGH: return "high";
        case EmergencySeverity::CRITICAL: return "critical";
        default: return "medium";
    }
}

EmergencySeverity EmergencyEvent::stringToSeverity(const std::string& str) {
    if (str == "low") return EmergencySeverity::LOW;
    if (str == "medium") return EmergencySeverity::MEDIUM;
    if (str == "high") return EmergencySeverity::HIGH;
    if (str == "critical") return EmergencySeverity::CRITICAL;
    return EmergencySeverity::MEDIUM;
}

std::string EmergencyEvent::statusToString(EmergencyStatus status) {
    switch (status) {
        case EmergencyStatus::ACTIVE: return "active";
        case EmergencyStatus::RESPONDING: return "responding";
        case EmergencyStatus::RESOLVED: return "resolved";
        case EmergencyStatus::CANCELLED: return "cancelled";
        default: return "active";
    }
}

EmergencyStatus EmergencyEvent::stringToStatus(const std::string& str) {
    if (str == "active") return EmergencyStatus::ACTIVE;
    if (str == "responding") return EmergencyStatus::RESPONDING;
    if (str == "resolved") return EmergencyStatus::RESOLVED;
    if (str == "cancelled") return EmergencyStatus::CANCELLED;
    return EmergencyStatus::ACTIVE;
}

bool EmergencyEvent::validate() const {
    if (event_code.empty()) return false;
    if (task_id <= 0) return false;
    if (title.empty()) return false;
    if (!location.contains("lat") || !location.contains("lon")) return false;
    return true;
}

} // namespace models
