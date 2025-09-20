#include "route_service.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace services {
    std::optional<models::Route> RouteService::plan_route(double start_lat, double start_lng,
                                                        double end_lat, double end_lng,
                                                        int creator_id,
                                                        const std::string& flight_task_id) {
        // Create route object
        models::Route route;
        route.id = generate_route_id();
        route.name = "Route from (" + std::to_string(start_lat) + "," + std::to_string(start_lng) +
                    ") to (" + std::to_string(end_lat) + "," + std::to_string(end_lng) + ")";
        route.start_point.latitude = start_lat;
        route.start_point.longitude = start_lng;
        route.start_point.is_start = true;
        route.end_point.latitude = end_lat;
        route.end_point.longitude = end_lng;
        route.end_point.is_end = true;
        route.creator_id = creator_id;
        route.flight_task_id = flight_task_id;
        route.created_at = std::chrono::system_clock::now();

        // In a real implementation, this would use an actual routing algorithm
        // For now, we'll create a simple straight-line route with intermediate points
        route.waypoints.push_back(route.start_point);

        // Add intermediate waypoints (simplified)
        int num_waypoints = 5;
        double lat_step = (end_lat - start_lat) / (num_waypoints + 1);
        double lng_step = (end_lng - start_lng) / (num_waypoints + 1);

        for (int i = 1; i <= num_waypoints; ++i) {
            models::Waypoint wp;
            wp.latitude = start_lat + i * lat_step;
            wp.longitude = start_lng + i * lng_step;
            wp.is_main_road = true;
            route.waypoints.push_back(wp);
        }

        route.waypoints.push_back(route.end_point);

        // Create segments
        route.segments = create_segments(route.waypoints);

        // Calculate distance and time
        route.calculate_total_distance();
        route.calculate_total_time();

        // Save to database
        std::string route_id = route_dao_->create_route(route);
        if (!route_id.empty()) {
            route.id = route_id;
            return route;
        }

        return std::nullopt;
    }

    std::optional<models::Route> RouteService::get_route_by_id(const std::string& id) {
        return route_dao_->get_route_by_id(id);
    }

    std::vector<models::Route> RouteService::get_routes_by_creator(int creator_id) {
        return route_dao_->get_routes_by_creator(creator_id);
    }

    std::vector<models::Route> RouteService::get_routes_by_flight_task(const std::string& flight_task_id) {
        return route_dao_->get_routes_by_flight_task(flight_task_id);
    }

    std::vector<models::Route> RouteService::get_all_routes() {
        return route_dao_->get_all_routes();
    }

    bool RouteService::update_route(const models::Route& route) {
        return route_dao_->update_route(route);
    }

    bool RouteService::delete_route(const std::string& id) {
        return route_dao_->delete_route(id);
    }

    std::string RouteService::generate_route_id() {
        // Generate a unique route ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "RT" << std::setfill('0') << std::setw(10) << (millis % 10000000000ULL);

        return ss.str();
    }

    double RouteService::calculate_distance(double lat1, double lng1, double lat2, double lng2) {
        // Haversine formula to calculate distance between two points
        const double R = 6371.0; // Earth radius in kilometers
        const double to_radians = M_PI / 180.0;

        double dlat = (lat2 - lat1) * to_radians;
        double dlng = (lng2 - lng1) * to_radians;

        double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
                   std::cos(lat1 * to_radians) * std::cos(lat2 * to_radians) *
                   std::sin(dlng / 2) * std::sin(dlng / 2);

        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return R * c;
    }

    std::vector<models::RouteSegment> RouteService::create_segments(const std::vector<models::Waypoint>& waypoints) {
        std::vector<models::RouteSegment> segments;
        for (size_t i = 0; i < waypoints.size() - 1; ++i) {
            models::RouteSegment segment;
            segment.from = waypoints[i];
            segment.to = waypoints[i + 1];
            segment.distance = calculate_distance(segment.from.latitude, segment.from.longitude,
                                                segment.to.latitude, segment.to.longitude);
            segments.push_back(segment);
        }
        return segments;
    }
}