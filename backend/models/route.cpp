#include "route.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace models {
    void Route::calculate_total_distance() {
        total_distance = 0.0;
        for (const auto& segment : segments) {
            total_distance += segment.distance;
        }
    }

    void Route::calculate_total_time(double average_speed_kmh) {
        if (average_speed_kmh > 0) {
            // Time in minutes = (Distance in km / Speed in km/h) * 60
            total_time = static_cast<int>((total_distance / average_speed_kmh) * 60);
        } else {
            total_time = 0;
        }
    }

    std::string Route::to_json() const {
        std::stringstream ss;
        ss << "{";
        ss << "\"id\":\"" << id << "\",";
        ss << "\"name\":\"" << name << "\",";
        ss << "\"total_distance\":" << std::fixed << std::setprecision(3) << total_distance << ",";
        ss << "\"total_time\":" << total_time << ",";
        ss << "\"start_point\":{";
        ss << "\"latitude\":" << std::fixed << std::setprecision(8) << start_point.latitude << ",";
        ss << "\"longitude\":" << std::fixed << std::setprecision(8) << start_point.longitude;
        ss << "},";
        ss << "\"end_point\":{";
        ss << "\"latitude\":" << std::fixed << std::setprecision(8) << end_point.latitude << ",";
        ss << "\"longitude\":" << std::fixed << std::setprecision(8) << end_point.longitude;
        ss << "},";

        // Add waypoints
        ss << "\"waypoints\":[";
        for (size_t i = 0; i < waypoints.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "{";
            ss << "\"latitude\":" << std::fixed << std::setprecision(8) << waypoints[i].latitude << ",";
            ss << "\"longitude\":" << std::fixed << std::setprecision(8) << waypoints[i].longitude << ",";
            ss << "\"is_main_road\":" << (waypoints[i].is_main_road ? "true" : "false");
            ss << "}";
        }
        ss << "],";

        // Add segments
        ss << "\"segments\":[";
        for (size_t i = 0; i < segments.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "{";
            ss << "\"from\":{";
            ss << "\"latitude\":" << std::fixed << std::setprecision(8) << segments[i].from.latitude << ",";
            ss << "\"longitude\":" << std::fixed << std::setprecision(8) << segments[i].from.longitude;
            ss << "},";
            ss << "\"to\":{";
            ss << "\"latitude\":" << std::fixed << std::setprecision(8) << segments[i].to.latitude << ",";
            ss << "\"longitude\":" << std::fixed << std::setprecision(8) << segments[i].to.longitude;
            ss << "},";
            ss << "\"distance\":" << std::fixed << std::setprecision(3) << segments[i].distance;
            ss << "}";
        }
        ss << "]";

        ss << "}";
        return ss.str();
    }
}