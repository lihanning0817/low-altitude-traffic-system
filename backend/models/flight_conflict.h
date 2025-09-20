#ifndef MODELS_FLIGHT_CONFLICT_H
#define MODELS_FLIGHT_CONFLICT_H

#include <string>
#include <chrono>
#include <optional>

namespace models {
    enum class ConflictSeverity {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };

    enum class ResolutionStatus {
        PENDING,
        RESOLVED,
        IGNORED
    };

    struct ResolutionAction {
        std::string type;
        std::vector<std::string> actions;
    };

    class FlightConflict {
    public:
        std::string id;
        std::string flight1_id;
        std::string flight2_id;
        std::chrono::system_clock::time_point conflict_time;
        ConflictSeverity severity;
        double distance; // meters
        ResolutionStatus resolution_status;
        std::optional<ResolutionAction> resolution_action;
        std::optional<std::chrono::system_clock::time_point> resolved_at;
        std::chrono::system_clock::time_point created_at;

        // Constructors
        FlightConflict() : severity(ConflictSeverity::MEDIUM), distance(0.0),
                          resolution_status(ResolutionStatus::PENDING) {}

        // Helper methods
        std::string get_severity_string() const;
        std::string get_resolution_status_string() const;
        static ConflictSeverity parse_severity_string(const std::string& severity_str);
        static ResolutionStatus parse_resolution_status_string(const std::string& status_str);
    };
}

#endif // MODELS_FLIGHT_CONFLICT_H