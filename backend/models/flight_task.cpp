#include "flight_task.h"
#include <stdexcept>

namespace models {
    std::string FlightTask::get_status_string() const {
        switch (status) {
            case FlightTaskStatus::PENDING:
                return "pending";
            case FlightTaskStatus::APPROVED:
                return "approved";
            case FlightTaskStatus::IN_PROGRESS:
                return "in_progress";
            case FlightTaskStatus::COMPLETED:
                return "completed";
            case FlightTaskStatus::CANCELLED:
                return "cancelled";
            case FlightTaskStatus::FAILED:
                return "failed";
            default:
                return "pending";
        }
    }

    std::string FlightTask::get_priority_string() const {
        switch (priority) {
            case FlightTaskPriority::LOW:
                return "low";
            case FlightTaskPriority::MEDIUM:
                return "medium";
            case FlightTaskPriority::HIGH:
                return "high";
            case FlightTaskPriority::CRITICAL:
                return "critical";
            default:
                return "medium";
        }
    }

    FlightTaskStatus FlightTask::parse_status_string(const std::string& status_str) {
        if (status_str == "pending") {
            return FlightTaskStatus::PENDING;
        } else if (status_str == "approved") {
            return FlightTaskStatus::APPROVED;
        } else if (status_str == "in_progress") {
            return FlightTaskStatus::IN_PROGRESS;
        } else if (status_str == "completed") {
            return FlightTaskStatus::COMPLETED;
        } else if (status_str == "cancelled") {
            return FlightTaskStatus::CANCELLED;
        } else if (status_str == "failed") {
            return FlightTaskStatus::FAILED;
        } else {
            throw std::invalid_argument("Invalid status string: " + status_str);
        }
    }

    FlightTaskPriority FlightTask::parse_priority_string(const std::string& priority_str) {
        if (priority_str == "low") {
            return FlightTaskPriority::LOW;
        } else if (priority_str == "medium") {
            return FlightTaskPriority::MEDIUM;
        } else if (priority_str == "high") {
            return FlightTaskPriority::HIGH;
        } else if (priority_str == "critical") {
            return FlightTaskPriority::CRITICAL;
        } else {
            throw std::invalid_argument("Invalid priority string: " + priority_str);
        }
    }
}