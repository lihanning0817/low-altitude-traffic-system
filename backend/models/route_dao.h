#ifndef MODELS_ROUTE_DAO_H
#define MODELS_ROUTE_DAO_H

#include "route.h"
#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class RouteDAO {
    private:
        MYSQL* mysql_;

    public:
        explicit RouteDAO(MYSQL* mysql) : mysql_(mysql) {}

        // Create
        std::string create_route(const Route& route);

        // Read
        std::optional<Route> get_route_by_id(const std::string& id);
        std::vector<Route> get_routes_by_creator(int creator_id);
        std::vector<Route> get_routes_by_flight_task(const std::string& flight_task_id);
        std::vector<Route> get_all_routes();

        // Update
        bool update_route(const Route& route);

        // Delete
        bool delete_route(const std::string& id);

    private:
        Route result_set_to_route(MYSQL_ROW row);
        std::string waypoints_to_json(const std::vector<Waypoint>& waypoints);
        std::vector<Waypoint> json_to_waypoints(const std::string& json_str);
    };
}

#endif // MODELS_ROUTE_DAO_H
