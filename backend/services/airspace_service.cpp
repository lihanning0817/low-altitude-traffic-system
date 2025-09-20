#include "airspace_service.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace services {
    std::optional<models::Airspace> AirspaceService::create_airspace(const std::string& name,
                                                                   const std::string& description,
                                                                   double min_altitude,
                                                                   double max_altitude,
                                                                   const std::vector<models::Coordinate>& boundary,
                                                                   int capacity) {
        // Create airspace object
        models::Airspace airspace;
        airspace.id = generate_airspace_id();
        airspace.name = name;
        airspace.description = description;
        airspace.min_altitude = min_altitude;
        airspace.max_altitude = max_altitude;
        airspace.boundary_coordinates = boundary;
        airspace.capacity = capacity;
        airspace.current_flights = 0;
        airspace.status = models::AirspaceStatus::ACTIVE;
        airspace.created_at = std::chrono::system_clock::now();
        airspace.updated_at = std::chrono::system_clock::now();

        // Save to database
        std::string airspace_id = airspace_dao_->create_airspace(airspace);
        if (!airspace_id.empty()) {
            airspace.id = airspace_id;
            return airspace;
        }

        return std::nullopt;
    }

    std::optional<models::Airspace> AirspaceService::get_airspace_by_id(const std::string& id) {
        return airspace_dao_->get_airspace_by_id(id);
    }

    std::vector<models::Airspace> AirspaceService::get_airspace_by_status(models::AirspaceStatus status) {
        return airspace_dao_->get_airspace_by_status(status);
    }

    std::vector<models::Airspace> AirspaceService::get_all_airspaces() {
        return airspace_dao_->get_all_airspaces();
    }

    bool AirspaceService::update_airspace(const models::Airspace& airspace) {
        return airspace_dao_->update_airspace(airspace);
    }

    bool AirspaceService::update_airspace_status(const std::string& airspace_id,
                                               models::AirspaceStatus status,
                                               const std::string& restriction_reason) {
        return airspace_dao_->update_airspace_status(airspace_id, status, restriction_reason);
    }

    bool AirspaceService::restrict_airspace(const std::string& airspace_id, const std::string& reason) {
        return airspace_dao_->update_airspace_status(airspace_id, models::AirspaceStatus::RESTRICTED, reason);
    }

    bool AirspaceService::activate_airspace(const std::string& airspace_id) {
        return airspace_dao_->update_airspace_status(airspace_id, models::AirspaceStatus::ACTIVE, "");
    }

    bool AirspaceService::delete_airspace(const std::string& id) {
        return airspace_dao_->delete_airspace(id);
    }

    std::optional<models::FlightPermit> AirspaceService::apply_for_flight_permit(const std::string& flight_task_id,
                                                                               const std::string& airspace_id,
                                                                               int applicant_id,
                                                                               const std::chrono::system_clock::time_point& start_time,
                                                                               const std::chrono::system_clock::time_point& end_time,
                                                                               const std::string& remarks) {
        // Check if airspace is available
        if (!is_airspace_available(airspace_id, start_time, end_time)) {
            return std::nullopt;
        }

        // Check if flight task exists
        auto flight_task = flight_task_dao_->get_flight_task_by_id(flight_task_id);
        if (!flight_task.has_value()) {
            return std::nullopt;
        }

        // Create flight permit object
        models::FlightPermit permit;
        permit.id = generate_permit_id();
        permit.flight_task_id = flight_task_id;
        permit.airspace_id = airspace_id;
        permit.status = models::PermitStatus::PENDING;
        permit.applicant_id = applicant_id;
        permit.application_time = std::chrono::system_clock::now();
        permit.start_time = start_time;
        permit.end_time = end_time;
        permit.remarks = remarks;

        // Save to database
        std::string permit_id = permit_dao_->create_flight_permit(permit);
        if (!permit_id.empty()) {
            permit.id = permit_id;
            return permit;
        }

        return std::nullopt;
    }

    std::optional<models::FlightPermit> AirspaceService::get_flight_permit_by_id(const std::string& id) {
        return permit_dao_->get_flight_permit_by_id(id);
    }

    std::vector<models::FlightPermit> AirspaceService::get_flight_permits_by_applicant(int applicant_id) {
        return permit_dao_->get_flight_permits_by_applicant(applicant_id);
    }

    std::vector<models::FlightPermit> AirspaceService::get_flight_permits_by_airspace(const std::string& airspace_id) {
        return permit_dao_->get_flight_permits_by_airspace(airspace_id);
    }

    std::vector<models::FlightPermit> AirspaceService::get_pending_flight_permits() {
        return permit_dao_->get_pending_flight_permits();
    }

    std::vector<models::FlightPermit> AirspaceService::get_all_flight_permits() {
        return permit_dao_->get_all_flight_permits();
    }

    bool AirspaceService::approve_flight_permit(const std::string& permit_id, int approver_id, const std::string& remarks) {
        auto permit = permit_dao_->get_flight_permit_by_id(permit_id);
        if (!permit.has_value()) {
            return false;
        }

        // Update airspace flight count
        int current_usage = get_airspace_usage(permit.value().airspace_id,
                                             permit.value().start_time,
                                             permit.value().end_time);
        auto airspace = airspace_dao_->get_airspace_by_id(permit.value().airspace_id);
        if (airspace.has_value()) {
            airspace_dao_->update_airspace_flight_count(permit.value().airspace_id,
                                                     airspace.value().current_flights + 1);
        }

        return permit_dao_->update_flight_permit_status(permit_id, models::PermitStatus::APPROVED, approver_id, remarks);
    }

    bool AirspaceService::reject_flight_permit(const std::string& permit_id, int approver_id, const std::string& remarks) {
        return permit_dao_->update_flight_permit_status(permit_id, models::PermitStatus::REJECTED, approver_id, remarks);
    }

    bool AirspaceService::cancel_flight_permit(const std::string& permit_id) {
        auto permit = permit_dao_->get_flight_permit_by_id(permit_id);
        if (!permit.has_value()) {
            return false;
        }

        // If permit was approved, decrease airspace flight count
        if (permit.value().status == models::PermitStatus::APPROVED) {
            auto airspace = airspace_dao_->get_airspace_by_id(permit.value().airspace_id);
            if (airspace.has_value()) {
                airspace_dao_->update_airspace_flight_count(permit.value().airspace_id,
                                                         std::max(0, airspace.value().current_flights - 1));
            }
        }

        return permit_dao_->update_flight_permit_status(permit_id, models::PermitStatus::REJECTED, 0, "Cancelled");
    }

    std::string AirspaceService::generate_airspace_id() {
        // Generate a unique airspace ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "AS" << std::setfill('0') << std::setw(6) << (millis % 1000000ULL);

        return ss.str();
    }

    std::string AirspaceService::generate_permit_id() {
        // Generate a unique permit ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "FP" << std::setfill('0') << std::setw(10) << (millis % 10000000000ULL);

        return ss.str();
    }

    bool AirspaceService::is_airspace_available(const std::string& airspace_id,
                                              const std::chrono::system_clock::time_point& start_time,
                                              const std::chrono::system_clock::time_point& end_time) {
        // Check if airspace exists and is active
        auto airspace = airspace_dao_->get_airspace_by_id(airspace_id);
        if (!airspace.has_value() || airspace.value().status != models::AirspaceStatus::ACTIVE) {
            return false;
        }

        // Check if airspace has capacity
        int current_usage = get_airspace_usage(airspace_id, start_time, end_time);
        return current_usage < airspace.value().capacity;
    }

    int AirspaceService::get_airspace_usage(const std::string& airspace_id,
                                          const std::chrono::system_clock::time_point& start_time,
                                          const std::chrono::system_clock::time_point& end_time) {
        // Get all approved permits for this airspace
        auto permits = permit_dao_->get_flight_permits_by_airspace(airspace_id);

        // Filter only approved permits that overlap with the requested time
        int usage = 0;
        for (const auto& permit : permits) {
            if (permit.status == models::PermitStatus::APPROVED) {
                // Check if time periods overlap
                if (!(permit.end_time < start_time || end_time < permit.start_time)) {
                    usage++;
                }
            }
        }

        return usage;
    }
}