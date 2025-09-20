#ifndef MODELS_DEVICE_DAO_H
#define MODELS_DEVICE_DAO_H

#include "device.h"
#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class DeviceDAO {
    private:
        MYSQL* mysql_;

    public:
        explicit DeviceDAO(MYSQL* mysql) : mysql_(mysql) {}

        // Create
        std::string create_device(const Device& device);

        // Read
        std::optional<Device> get_device_by_id(const std::string& id);
        std::vector<Device> get_devices_by_type(DeviceType type);
        std::vector<Device> get_devices_by_status(DeviceStatus status);
        std::vector<Device> get_devices_by_owner(int owner_id);
        std::vector<Device> get_all_devices();

        // Update
        bool update_device(const Device& device);
        bool update_device_status(const std::string& device_id, DeviceStatus status);
        bool update_device_location(const std::string& device_id, double latitude, double longitude, double altitude);
        bool update_device_battery(const std::string& device_id, int battery_level);
        bool update_device_signal(const std::string& device_id, int signal_strength);

        // Delete
        bool delete_device(const std::string& id);

    private:
        Device result_set_to_device(MYSQL_ROW row);
    };
}

#endif // MODELS_DEVICE_DAO_H
