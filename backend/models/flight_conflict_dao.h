#ifndef MODELS_FLIGHT_CONFLICT_DAO_H
#define MODELS_FLIGHT_CONFLICT_DAO_H

#include "flight_conflict.h"
#include <cppconn/connection.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class FlightConflictDAO {
    private:
        std::shared_ptr<sql::Connection> conn_;

    public:
        explicit FlightConflictDAO(std::shared_ptr<sql::Connection> conn) : conn_(conn) {}

        // Create
        std::string create_flight_conflict(const FlightConflict& conflict);

        // Read
        std::optional<FlightConflict> get_flight_conflict_by_id(const std::string& id);
        std::vector<FlightConflict> get_flight_conflicts_by_flight(const std::string& flight_id);
        std::vector<FlightConflict> get_flight_conflicts_by_severity(ConflictSeverity severity);
        std::vector<FlightConflict> get_flight_conflicts_by_resolution_status(ResolutionStatus status);
        std::vector<FlightConflict> get_unresolved_flight_conflicts();
        std::vector<FlightConflict> get_all_flight_conflicts();

        // Update
        bool update_flight_conflict(const FlightConflict& conflict);
        bool update_flight_conflict_resolution(const std::string& conflict_id,
                                             ResolutionStatus status,
                                             const std::optional<ResolutionAction>& action = std::nullopt);

        // Delete
        bool delete_flight_conflict(const std::string& id);

    private:
        FlightConflict result_set_to_flight_conflict(sql::ResultSet* rs);
        std::string resolution_action_to_json(const std::optional<ResolutionAction>& action);
        std::optional<ResolutionAction> json_to_resolution_action(const std::string& json_str);
    };
}

#endif // MODELS_FLIGHT_CONFLICT_DAO_H