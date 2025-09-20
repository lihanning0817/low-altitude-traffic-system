#ifndef MODELS_FLIGHT_PERMIT_H
#define MODELS_FLIGHT_PERMIT_H

#include <string>
#include <chrono>
#include <optional>

namespace models {
    enum class PermitStatus {
        PENDING,
        APPROVED,
        REJECTED,
        EXPIRED
    };

    class FlightPermit {
    public:
        std::string id;
        std::string flight_task_id;
        std::string airspace_id;
        PermitStatus status;
        int applicant_id;
        std::optional<int> approver_id;
        std::chrono::system_clock::time_point application_time;
        std::optional<std::chrono::system_clock::time_point> approval_time;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point end_time;
        std::string remarks;

        // Constructors
        FlightPermit() : status(PermitStatus::PENDING), applicant_id(0) {}

        // Helper methods
        std::string get_status_string() const;
        static PermitStatus parse_status_string(const std::string& status_str);
    };
}

#endif // MODELS_FLIGHT_PERMIT_H