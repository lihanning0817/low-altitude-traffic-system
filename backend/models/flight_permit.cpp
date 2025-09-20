#include "flight_permit.h"
#include <stdexcept>

namespace models {
    std::string FlightPermit::get_status_string() const {
        switch (status) {
            case PermitStatus::PENDING:
                return "pending";
            case PermitStatus::APPROVED:
                return "approved";
            case PermitStatus::REJECTED:
                return "rejected";
            case PermitStatus::EXPIRED:
                return "expired";
            default:
                return "pending";
        }
    }

    PermitStatus FlightPermit::parse_status_string(const std::string& status_str) {
        if (status_str == "pending") {
            return PermitStatus::PENDING;
        } else if (status_str == "approved") {
            return PermitStatus::APPROVED;
        } else if (status_str == "rejected") {
            return PermitStatus::REJECTED;
        } else if (status_str == "expired") {
            return PermitStatus::EXPIRED;
        } else {
            throw std::invalid_argument("Invalid permit status string: " + status_str);
        }
    }
}