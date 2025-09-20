#ifndef MODELS_EMERGENCY_LANDING_POINT_H
#define MODELS_EMERGENCY_LANDING_POINT_H

#include <string>
#include <chrono>

namespace models {
    class EmergencyLandingPoint {
    public:
        std::string id;
        std::string name;
        double latitude;
        double longitude;
        double altitude;
        int capacity;
        bool is_active;
        std::string description;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point updated_at;

        // Constructors
        EmergencyLandingPoint() : latitude(0.0), longitude(0.0), altitude(0.0),
                                capacity(1), is_active(true) {}

        // Helper methods
        double distance_to(double lat, double lng) const;
    };
}

#endif // MODELS_EMERGENCY_LANDING_POINT_H