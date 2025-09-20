#include "emergency_event.h"
#include <stdexcept>

namespace models {
    std::string EmergencyEvent::get_type_string() const {
        switch (type) {
            case EmergencyType::ACCIDENT:
                return "accident";
            case EmergencyType::MALFUNCTION:
                return "malfunction";
            case EmergencyType::WEATHER:
                return "weather";
            case EmergencyType::OTHER:
                return "other";
            default:
                return "other";
        }
    }

    std::string EmergencyEvent::get_severity_string() const {
        switch (severity) {
            case EmergencySeverity::LOW:
                return "low";
            case EmergencySeverity::MEDIUM:
                return "medium";
            case EmergencySeverity::HIGH:
                return "high";
            case EmergencySeverity::CRITICAL:
                return "critical";
            default:
                return "medium";
        }
    }

    std::string EmergencyEvent::get_status_string() const {
        switch (status) {
            case EmergencyStatus::REPORTED:
                return "reported";
            case EmergencyStatus::RESPONDING:
                return "responding";
            case EmergencyStatus::RESOLVED:
                return "resolved";
            case EmergencyStatus::CLOSED:
                return "closed";
            default:
                return "reported";
        }
    }

    EmergencyType EmergencyEvent::parse_type_string(const std::string& type_str) {
        if (type_str == "accident") {
            return EmergencyType::ACCIDENT;
        } else if (type_str == "malfunction") {
            return EmergencyType::MALFUNCTION;
        } else if (type_str == "weather") {
            return EmergencyType::WEATHER;
        } else if (type_str == "other") {
            return EmergencyType::OTHER;
        } else {
            throw std::invalid_argument("Invalid emergency type string: " + type_str);
        }
    }

    EmergencySeverity EmergencyEvent::parse_severity_string(const std::string& severity_str) {
        if (severity_str == "low") {
            return EmergencySeverity::LOW;
        } else if (severity_str == "medium") {
            return EmergencySeverity::MEDIUM;
        } else if (severity_str == "high") {
            return EmergencySeverity::HIGH;
        } else if (severity_str == "critical") {
            return EmergencySeverity::CRITICAL;
        } else {
            throw std::invalid_argument("Invalid emergency severity string: " + severity_str);
        }
    }

    EmergencyStatus EmergencyEvent::parse_status_string(const std::string& status_str) {
        if (status_str == "reported") {
            return EmergencyStatus::REPORTED;
        } else if (status_str == "responding") {
            return EmergencyStatus::RESPONDING;
        } else if (status_str == "resolved") {
            return EmergencyStatus::RESOLVED;
        } else if (status_str == "closed") {
            return EmergencyStatus::CLOSED;
        } else {
            throw std::invalid_argument("Invalid emergency status string: " + status_str);
        }
    }
}