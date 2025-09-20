#include "device_service.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

namespace services {
    std::optional<models::Device> DeviceService::register_device(const std::string& name,
                                                              models::DeviceType type,
                                                              const std::string& model,
                                                              const std::string& manufacturer,
                                                              const std::string& serial_number) {
        // Create device object
        models::Device device;
        device.id = generate_device_id(type);
        device.name = name;
        device.type = type;
        device.model = model;
        device.manufacturer = manufacturer;
        device.serial_number = serial_number;
        device.status = models::DeviceStatus::OFFLINE;
        device.signal_strength = 0;
        device.battery_level = 100;
        device.latitude = 0.0;
        device.longitude = 0.0;
        device.altitude = 0.0;
        device.last_update = std::chrono::system_clock::now();
        device.created_at = std::chrono::system_clock::now();

        // Save to database
        std::string device_id = device_dao_->create_device(device);
        if (!device_id.empty()) {
            device.id = device_id;
            return device;
        }

        return std::nullopt;
    }

    std::optional<models::Device> DeviceService::get_device_by_id(const std::string& id) {
        return device_dao_->get_device_by_id(id);
    }

    std::vector<models::Device> DeviceService::get_devices_by_type(models::DeviceType type) {
        return device_dao_->get_devices_by_type(type);
    }

    std::vector<models::Device> DeviceService::get_devices_by_status(models::DeviceStatus status) {
        return device_dao_->get_devices_by_status(status);
    }

    std::vector<models::Device> DeviceService::get_devices_by_owner(int owner_id) {
        return device_dao_->get_devices_by_owner(owner_id);
    }

    std::vector<models::Device> DeviceService::get_all_devices() {
        return device_dao_->get_all_devices();
    }

    bool DeviceService::update_device(const models::Device& device) {
        return device_dao_->update_device(device);
    }

    bool DeviceService::update_device_status(const std::string& device_id, models::DeviceStatus status) {
        return device_dao_->update_device_status(device_id, status);
    }

    bool DeviceService::update_device_location(const std::string& device_id, double latitude, double longitude, double altitude) {
        return device_dao_->update_device_location(device_id, latitude, longitude, altitude);
    }

    bool DeviceService::update_device_battery(const std::string& device_id, int battery_level) {
        return device_dao_->update_device_battery(device_id, battery_level);
    }

    bool DeviceService::update_device_signal(const std::string& device_id, int signal_strength) {
        return device_dao_->update_device_signal(device_id, signal_strength);
    }

    bool DeviceService::assign_device_to_user(const std::string& device_id, int user_id) {
        auto device = device_dao_->get_device_by_id(device_id);
        if (device.has_value()) {
            device.value().owner_id = user_id;
            device.value().last_update = std::chrono::system_clock::now();
            return device_dao_->update_device(device.value());
        }
        return false;
    }

    bool DeviceService::delete_device(const std::string& id) {
        return device_dao_->delete_device(id);
    }

    std::string DeviceService::generate_device_id(models::DeviceType type) {
        // Generate a unique device ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::string prefix;
        switch (type) {
            case models::DeviceType::DRONE:
                prefix = "UAV";
                break;
            case models::DeviceType::CAMERA:
                prefix = "CAM";
                break;
            case models::DeviceType::RADAR:
                prefix = "RAD";
                break;
            case models::DeviceType::SENSOR:
                prefix = "SEN";
                break;
            default:
                prefix = "DEV";
                break;
        }

        std::stringstream ss;
        ss << prefix << std::setfill('0') << std::setw(6) << (millis % 1000000ULL);

        return ss.str();
    }

    bool DeviceService::is_device_online(const std::string& device_id) {
        auto device = device_dao_->get_device_by_id(device_id);
        if (device.has_value()) {
            // Consider device online if it was updated in the last 5 minutes
            auto now = std::chrono::system_clock::now();
            auto last_update = device.value().last_update;
            auto diff = std::chrono::duration_cast<std::chrono::minutes>(now - last_update);
            return diff.count() <= 5 && device.value().status == models::DeviceStatus::ONLINE;
        }
        return false;
    }
}