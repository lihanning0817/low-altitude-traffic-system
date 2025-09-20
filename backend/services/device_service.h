#ifndef SERVICES_DEVICE_SERVICE_H
#define SERVICES_DEVICE_SERVICE_H

#include "../models/device.h"
#include "../models/device_dao.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class DeviceService {
    private:
        std::shared_ptr<models::DeviceDAO> device_dao_;

    public:
        explicit DeviceService(std::shared_ptr<models::DeviceDAO> device_dao) : device_dao_(device_dao) {}

        // Device management
        std::optional<models::Device> register_device(const std::string& name,
                                                     models::DeviceType type,
                                                     const std::string& model,
                                                     const std::string& manufacturer,
                                                     const std::string& serial_number);

        std::optional<models::Device> get_device_by_id(const std::string& id);
        std::vector<models::Device> get_devices_by_type(models::DeviceType type);
        std::vector<models::Device> get_devices_by_status(models::DeviceStatus status);
        std::vector<models::Device> get_devices_by_owner(int owner_id);
        std::vector<models::Device> get_all_devices();

        bool update_device(const models::Device& device);
        bool update_device_status(const std::string& device_id, models::DeviceStatus status);
        bool update_device_location(const std::string& device_id, double latitude, double longitude, double altitude);
        bool update_device_battery(const std::string& device_id, int battery_level);
        bool update_device_signal(const std::string& device_id, int signal_strength);
        bool assign_device_to_user(const std::string& device_id, int user_id);

        bool delete_device(const std::string& id);

        // Helper methods
        std::string generate_device_id(models::DeviceType type);
        bool is_device_online(const std::string& device_id);
    };
}

#endif // SERVICES_DEVICE_SERVICE_H