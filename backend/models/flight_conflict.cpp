#include "flight_conflict.h"
#include <stdexcept>

namespace models {
    std::string FlightConflict::get_severity_string() const {
        switch (severity) {
            case ConflictSeverity::LOW:
                return "low";
            case ConflictSeverity::MEDIUM:
                return "medium";
            case ConflictSeverity::HIGH:
                return "high";
            case ConflictSeverity::CRITICAL:
                return "critical";
            default:
                return "medium";
        }
    }

    std::string FlightConflict::get_resolution_status_string() const {
        switch (resolution_status) {
            case ResolutionStatus::PENDING:
                return "pending";
            case ResolutionStatus::RESOLVED:
                return "resolved";
            case ResolutionStatus::IGNORED:
                return "ignored";
            default:
                return "pending";
        }
    }

    ConflictSeverity FlightConflict::parse_severity_string(const std::string& severity_str) {
        if (severity_str == "low") {
            return ConflictSeverity::LOW;
        } else if (severity_str == "medium") {
            return ConflictSeverity::MEDIUM;
        } else if (severity_str == "high") {
            return ConflictSeverity::HIGH;
        } else if (severity_str == "critical") {
            return ConflictSeverity::CRITICAL;
        } else {
            throw std::invalid_argument("Invalid severity string: " + severity_str);
        }
    }

    ResolutionStatus FlightConflict::parse_resolution_status_string(const std::string& status_str) {
        if (status_str == "pending") {
            return ResolutionStatus::PENDING;
        } else if (status_str == "resolved") {
            return ResolutionStatus::RESOLVED;
        } else if (status_str == "ignored") {
            return ResolutionStatus::IGNORED;
        } else {
            throw std::invalid_argument("Invalid resolution status string: " + status_str);
        }
    }
}