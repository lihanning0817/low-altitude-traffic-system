#include "airspace.h"
#include <stdexcept>
#include <json/json.h>

namespace models {
    std::string Airspace::get_status_string() const {
        switch (status) {
            case AirspaceStatus::ACTIVE:
                return "active";
            case AirspaceStatus::RESTRICTED:
                return "restricted";
            case AirspaceStatus::CLOSED:
                return "closed";
            default:
                return "active";
        }
    }

    AirspaceStatus Airspace::parse_status_string(const std::string& status_str) {
        if (status_str == "active") {
            return AirspaceStatus::ACTIVE;
        } else if (status_str == "restricted") {
            return AirspaceStatus::RESTRICTED;
        } else if (status_str == "closed") {
            return AirspaceStatus::CLOSED;
        } else {
            throw std::invalid_argument("Invalid airspace status string: " + status_str);
        }
    }

    bool Airspace::is_point_in_airspace(double latitude, double longitude) const {
        // Simple bounding box check for now
        // In a real implementation, this would use a more complex polygon containment algorithm
        if (boundary_coordinates.empty()) {
            return false;
        }

        double min_lat = boundary_coordinates[0].latitude;
        double max_lat = boundary_coordinates[0].latitude;
        double min_lng = boundary_coordinates[0].longitude;
        double max_lng = boundary_coordinates[0].longitude;

        for (const auto& coord : boundary_coordinates) {
            if (coord.latitude < min_lat) min_lat = coord.latitude;
            if (coord.latitude > max_lat) max_lat = coord.latitude;
            if (coord.longitude < min_lng) min_lng = coord.longitude;
            if (coord.longitude > max_lng) max_lng = coord.longitude;
        }

        return (latitude >= min_lat && latitude <= max_lat &&
                longitude >= min_lng && longitude <= max_lng);
    }

    std::string Airspace::boundary_to_json() const {
        Json::Value root(Json::arrayValue);
        for (const auto& coord : boundary_coordinates) {
            Json::Value c;
            c["latitude"] = coord.latitude;
            c["longitude"] = coord.longitude;
            root.append(c);
        }

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    }

    void Airspace::boundary_from_json(const std::string& json_str) {
        boundary_coordinates.clear();

        Json::Value root;
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        std::string errors;

        if (reader->parse(json_str.c_str(), json_str.c_str() + json_str.length(), &root, &errors)) {
            if (root.isArray()) {
                for (const auto& coord_value : root) {
                    Coordinate coord;
                    coord.latitude = coord_value["latitude"].asDouble();
                    coord.longitude = coord_value["longitude"].asDouble();
                    boundary_coordinates.push_back(coord);
                }
            }
        }
    }
}