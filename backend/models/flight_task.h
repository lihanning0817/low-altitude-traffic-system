#ifndef MODELS_FLIGHT_TASK_H
#define MODELS_FLIGHT_TASK_H

#include <string>
#include <chrono>
#include <optional>

namespace models {
    enum class FlightTaskStatus {
        PENDING,
        APPROVED,
        IN_PROGRESS,
        COMPLETED,
        CANCELLED,
        FAILED
    };

    enum class FlightTaskPriority {
        LOW,
        MEDIUM,
        HIGH,
        CRITICAL
    };

    class FlightTask {
    public:
        std::string id;
        std::string name;
        std::string description;
        FlightTaskStatus status;
        FlightTaskPriority priority;
        int progress; // 0-100
        std::optional<std::chrono::system_clock::time_point> start_time;
        std::optional<std::chrono::system_clock::time_point> end_time;
        std::optional<int> estimated_duration; // minutes
        int creator_id;
        std::optional<std::string> assignee_id; // device ID
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point updated_at;

        // Constructors
        FlightTask() : status(FlightTaskStatus::PENDING), priority(FlightTaskPriority::MEDIUM),
                      progress(0), creator_id(0) {}

        // Helper methods
        std::string get_status_string() const;
        std::string get_priority_string() const;
        static FlightTaskStatus parse_status_string(const std::string& status_str);
        static FlightTaskPriority parse_priority_string(const std::string& priority_str);
    };
}

#endif // MODELS_FLIGHT_TASK_H