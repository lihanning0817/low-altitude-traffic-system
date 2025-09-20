#include "emergency_landing_point.h"
#include <cmath>

namespace models {
    double EmergencyLandingPoint::distance_to(double lat, double lng) const {
        // Haversine formula to calculate distance between two points
        const double R = 6371.0; // Earth radius in kilometers
        const double to_radians = M_PI / 180.0;

        double dlat = (lat - latitude) * to_radians;
        double dlng = (lng - longitude) * to_radians;

        double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
                   std::cos(latitude * to_radians) * std::cos(lat * to_radians) *
                   std::sin(dlng / 2) * std::sin(dlng / 2);

        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
        return R * c; // Distance in kilometers
    }
}