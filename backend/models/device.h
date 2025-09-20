#ifndef MODELS_DEVICE_H
#define MODELS_DEVICE_H

#include <string>
#include <chrono>
#include <optional>

namespace models {
    enum class DeviceType {
        DRONE,
        CAMERA,
        RADAR,
        SENSOR
    };

    enum class DeviceStatus {
        ONLINE,
        OFFLINE,
        MAINTENANCE,
        RETIRED
    };

    class Device {
    public:
        std::string id;
        std::string name;
        DeviceType type;
        std::string model;
        std::string manufacturer;
        std::string serial_number;
        DeviceStatus status;
        int signal_strength;
        int battery_level;
        double latitude;
        double longitude;
        double altitude;
        std::chrono::system_clock::time_point last_update;
        std::optional<int> owner_id;
        std::chrono::system_clock::time_point created_at;

        // Constructors
        Device() : type(DeviceType::DRONE), status(DeviceStatus::OFFLINE),
                  signal_strength(0), battery_level(100), latitude(0.0), longitude(0.0), altitude(0.0) {}

        // Helper methods
        std::string get_type_string() const;
        std::string get_status_string() const;
        static DeviceType parse_type_string(const std::string& type_str);
        static DeviceStatus parse_status_string(const std::string& status_str);
    };
}

#endif // MODELS_DEVICE_H