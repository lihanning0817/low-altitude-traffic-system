#ifndef SERVICES_ROUTE_SERVICE_H
#define SERVICES_ROUTE_SERVICE_H

#include "../models/route.h"
#include "../models/route_dao.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class RouteService {
    private:
        std::shared_ptr<models::RouteDAO> route_dao_;

    public:
        explicit RouteService(std::shared_ptr<models::RouteDAO> route_dao) : route_dao_(route_dao) {}

        // Route planning
        std::optional<models::Route> plan_route(double start_lat, double start_lng,
                                              double end_lat, double end_lng,
                                              int creator_id,
                                              const std::string& flight_task_id = "");

        std::optional<models::Route> get_route_by_id(const std::string& id);
        std::vector<models::Route> get_routes_by_creator(int creator_id);
        std::vector<models::Route> get_routes_by_flight_task(const std::string& flight_task_id);
        std::vector<models::Route> get_all_routes();

        bool update_route(const models::Route& route);
        bool delete_route(const std::string& id);

        // Helper methods
        std::string generate_route_id();
        double calculate_distance(double lat1, double lng1, double lat2, double lng2);
        std::vector<models::RouteSegment> create_segments(const std::vector<models::Waypoint>& waypoints);
    };
}

#endif // SERVICES_ROUTE_SERVICE_H