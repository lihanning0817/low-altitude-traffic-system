#ifndef SERVICES_AIRSPACE_SERVICE_H
#define SERVICES_AIRSPACE_SERVICE_H

#include "../models/airspace.h"
#include "../models/airspace_dao.h"
#include "../models/flight_permit.h"
#include "../models/flight_permit_dao.h"
#include "../models/device.h"
#include "../models/device_dao.h"
#include "../models/flight_task.h"
#include "../models/flight_task_dao.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class AirspaceService {
    private:
        std::shared_ptr<models::AirspaceDAO> airspace_dao_;
        std::shared_ptr<models::FlightPermitDAO> permit_dao_;
        std::shared_ptr<models::DeviceDAO> device_dao_;
        std::shared_ptr<models::FlightTaskDAO> flight_task_dao_;

    public:
        explicit AirspaceService(std::shared_ptr<models::AirspaceDAO> airspace_dao,
                               std::shared_ptr<models::FlightPermitDAO> permit_dao,
                               std::shared_ptr<models::DeviceDAO> device_dao,
                               std::shared_ptr<models::FlightTaskDAO> flight_task_dao)
            : airspace_dao_(airspace_dao), permit_dao_(permit_dao),
              device_dao_(device_dao), flight_task_dao_(flight_task_dao) {}

        // Airspace management
        std::optional<models::Airspace> create_airspace(const std::string& name,
                                                      const std::string& description,
                                                      double min_altitude,
                                                      double max_altitude,
                                                      const std::vector<models::Coordinate>& boundary,
                                                      int capacity = 100);

        std::optional<models::Airspace> get_airspace_by_id(const std::string& id);
        std::vector<models::Airspace> get_airspace_by_status(models::AirspaceStatus status);
        std::vector<models::Airspace> get_all_airspaces();

        bool update_airspace(const models::Airspace& airspace);
        bool update_airspace_status(const std::string& airspace_id,
                                  models::AirspaceStatus status,
                                  const std::string& restriction_reason = "");
        bool restrict_airspace(const std::string& airspace_id, const std::string& reason);
        bool activate_airspace(const std::string& airspace_id);

        bool delete_airspace(const std::string& id);

        // Flight permit management
        std::optional<models::FlightPermit> apply_for_flight_permit(const std::string& flight_task_id,
                                                                  const std::string& airspace_id,
                                                                  int applicant_id,
                                                                  const std::chrono::system_clock::time_point& start_time,
                                                                  const std::chrono::system_clock::time_point& end_time,
                                                                  const std::string& remarks = "");

        std::optional<models::FlightPermit> get_flight_permit_by_id(const std::string& id);
        std::vector<models::FlightPermit> get_flight_permits_by_applicant(int applicant_id);
        std::vector<models::FlightPermit> get_flight_permits_by_airspace(const std::string& airspace_id);
        std::vector<models::FlightPermit> get_pending_flight_permits();
        std::vector<models::FlightPermit> get_all_flight_permits();

        bool approve_flight_permit(const std::string& permit_id, int approver_id, const std::string& remarks = "");
        bool reject_flight_permit(const std::string& permit_id, int approver_id, const std::string& remarks = "");
        bool cancel_flight_permit(const std::string& permit_id);

        // Helper methods
        std::string generate_airspace_id();
        std::string generate_permit_id();
        bool is_airspace_available(const std::string& airspace_id,
                                 const std::chrono::system_clock::time_point& start_time,
                                 const std::chrono::system_clock::time_point& end_time);
        int get_airspace_usage(const std::string& airspace_id,
                              const std::chrono::system_clock::time_point& start_time,
                              const std::chrono::system_clock::time_point& end_time);
    };
}

#endif // SERVICES_AIRSPACE_SERVICE_H