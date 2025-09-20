#include "device.h"
#include <stdexcept>

namespace models {
    std::string Device::get_type_string() const {
        switch (type) {
            case DeviceType::DRONE:
                return "drone";
            case DeviceType::CAMERA:
                return "camera";
            case DeviceType::RADAR:
                return "radar";
            case DeviceType::SENSOR:
                return "sensor";
            default:
                return "drone";
        }
    }

    std::string Device::get_status_string() const {
        switch (status) {
            case DeviceStatus::ONLINE:
                return "online";
            case DeviceStatus::OFFLINE:
                return "offline";
            case DeviceStatus::MAINTENANCE:
                return "maintenance";
            case DeviceStatus::RETIRED:
                return "retired";
            default:
                return "offline";
        }
    }

    DeviceType Device::parse_type_string(const std::string& type_str) {
        if (type_str == "drone") {
            return DeviceType::DRONE;
        } else if (type_str == "camera") {
            return DeviceType::CAMERA;
        } else if (type_str == "radar") {
            return DeviceType::RADAR;
        } else if (type_str == "sensor") {
            return DeviceType::SENSOR;
        } else {
            throw std::invalid_argument("Invalid device type string: " + type_str);
        }
    }

    DeviceStatus Device::parse_status_string(const std::string& status_str) {
        if (status_str == "online") {
            return DeviceStatus::ONLINE;
        } else if (status_str == "offline") {
            return DeviceStatus::OFFLINE;
        } else if (status_str == "maintenance") {
            return DeviceStatus::MAINTENANCE;
        } else if (status_str == "retired") {
            return DeviceStatus::RETIRED;
        } else {
            throw std::invalid_argument("Invalid device status string: " + status_str);
        }
    }
}