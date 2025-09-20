#ifndef MODELS_EMERGENCY_EVENT_H
#define MODELS_EMERGENCY_EVENT_H

#include <string>
#include <chrono>
#include <optional>

namespace models {
    enum class EmergencyType {
        ACCIDENT,
        MALFUNCTION,
        WEATHER,
        OTHER
    };

    enum class EmergencySeverity {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };

    enum class EmergencyStatus {
        REPORTED,
        RESPONDING,
        RESOLVED,
        CLOSED
    };

    class EmergencyEvent {
    public:
        std::string id;
        EmergencyType type;
        EmergencySeverity severity;
        EmergencyStatus status;
        double latitude;
        double longitude;
        double altitude;
        std::string description;
        std::optional<int> reporter_id;
        std::optional<int> responder_id;
        std::chrono::system_clock::time_point reported_at;
        std::optional<std::chrono::system_clock::time_point> resolved_at;
        std::chrono::system_clock::time_point created_at;

        // Constructors
        EmergencyEvent() : type(EmergencyType::OTHER), severity(EmergencySeverity::MEDIUM),
                          status(EmergencyStatus::REPORTED), latitude(0.0), longitude(0.0), altitude(0.0) {}

        // Helper methods
        std::string get_type_string() const;
        std::string get_severity_string() const;
        std::string get_status_string() const;
        static EmergencyType parse_type_string(const std::string& type_str);
        static EmergencySeverity parse_severity_string(const std::string& severity_str);
        static EmergencyStatus parse_status_string(const std::string& status_str);
    };
}

#endif // MODELS_EMERGENCY_EVENT_H