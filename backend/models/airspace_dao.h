#ifndef MODELS_AIRSPACE_DAO_H
#define MODELS_AIRSPACE_DAO_H

#include "airspace.h"
#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class AirspaceDAO {
    private:
        MYSQL* mysql_;

    public:
        explicit AirspaceDAO(MYSQL* mysql) : mysql_(mysql) {}

        // Create
        std::string create_airspace(const Airspace& airspace);

        // Read
        std::optional<Airspace> get_airspace_by_id(const std::string& id);
        std::vector<Airspace> get_airspace_by_status(AirspaceStatus status);
        std::vector<Airspace> get_all_airspaces();

        // Update
        bool update_airspace(const Airspace& airspace);
        bool update_airspace_status(const std::string& airspace_id, AirspaceStatus status,
                                  const std::string& restriction_reason = "");
        bool update_airspace_flight_count(const std::string& airspace_id, int current_flights);

        // Delete
        bool delete_airspace(const std::string& id);

    private:
        Airspace result_set_to_airspace(MYSQL_ROW row);
    };
}

#endif // MODELS_AIRSPACE_DAO_H
