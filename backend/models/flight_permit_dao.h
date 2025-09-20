#ifndef MODELS_FLIGHT_PERMIT_DAO_H
#define MODELS_FLIGHT_PERMIT_DAO_H

#include "flight_permit.h"
#include <cppconn/connection.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class FlightPermitDAO {
    private:
        std::shared_ptr<sql::Connection> conn_;

    public:
        explicit FlightPermitDAO(std::shared_ptr<sql::Connection> conn) : conn_(conn) {}

        // Create
        std::string create_flight_permit(const FlightPermit& permit);

        // Read
        std::optional<FlightPermit> get_flight_permit_by_id(const std::string& id);
        std::vector<FlightPermit> get_flight_permits_by_applicant(int applicant_id);
        std::vector<FlightPermit> get_flight_permits_by_airspace(const std::string& airspace_id);
        std::vector<FlightPermit> get_flight_permits_by_status(PermitStatus status);
        std::vector<FlightPermit> get_pending_flight_permits();
        std::vector<FlightPermit> get_all_flight_permits();

        // Update
        bool update_flight_permit(const FlightPermit& permit);
        bool update_flight_permit_status(const std::string& permit_id, PermitStatus status,
                                       int approver_id = 0, const std::string& remarks = "");

        // Delete
        bool delete_flight_permit(const std::string& id);

    private:
        FlightPermit result_set_to_flight_permit(sql::ResultSet* rs);
    };
}

#endif // MODELS_FLIGHT_PERMIT_DAO_H