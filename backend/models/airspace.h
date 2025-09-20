#ifndef MODELS_AIRSPACE_H
#define MODELS_AIRSPACE_H

#include <string>
#include <vector>
#include <chrono>
#include <optional>

namespace models {
    struct Coordinate {
        double latitude;
        double longitude;
    };

    enum class AirspaceStatus {
        ACTIVE,
        RESTRICTED,
        CLOSED
    };

    class Airspace {
    public:
        std::string id;
        std::string name;
        std::string description;
        double min_altitude;
        double max_altitude;
        std::vector<Coordinate> boundary_coordinates;
        int capacity;
        int current_flights;
        AirspaceStatus status;
        std::string restriction_reason;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point updated_at;

        // Constructors
        Airspace() : min_altitude(0.0), max_altitude(0.0), capacity(100),
                    current_flights(0), status(AirspaceStatus::ACTIVE) {}

        // Helper methods
        std::string get_status_string() const;
        static AirspaceStatus parse_status_string(const std::string& status_str);
        bool is_point_in_airspace(double latitude, double longitude) const;
        std::string boundary_to_json() const;
        void boundary_from_json(const std::string& json_str);
    };
}

#endif // MODELS_AIRSPACE_H