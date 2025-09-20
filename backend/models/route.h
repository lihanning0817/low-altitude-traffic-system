#ifndef MODELS_ROUTE_H
#define MODELS_ROUTE_H

#include <string>
#include <vector>
#include <chrono>

namespace models {
    struct Waypoint {
        double latitude;
        double longitude;
        bool is_main_road;
        bool is_start = false;
        bool is_end = false;
    };

    struct RouteSegment {
        Waypoint from;
        Waypoint to;
        double distance; // kilometers
    };

    class Route {
    public:
        std::string id;
        std::string name;
        Waypoint start_point;
        Waypoint end_point;
        double total_distance; // kilometers
        int total_time; // minutes
        std::vector<Waypoint> waypoints;
        std::vector<RouteSegment> segments;
        int creator_id;
        std::string flight_task_id;
        std::chrono::system_clock::time_point created_at;

        // Constructors
        Route() : total_distance(0.0), total_time(0), creator_id(0) {}

        // Helper methods
        void calculate_total_distance();
        void calculate_total_time(double average_speed_kmh = 50.0); // Default 50 km/h
        std::string to_json() const;
    };
}

#endif // MODELS_ROUTE_H