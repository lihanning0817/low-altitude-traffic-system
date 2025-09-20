#include "device_dao.h"
#include <iostream>

namespace models {
    std::string DeviceDAO::create_device(const Device& device) {
        try {
            // 构建SQL查询
            std::string sql = "INSERT INTO devices (id, name, type, model, manufacturer, serial_number, status, "
                              "signal_strength, battery_level, latitude, longitude, altitude";
            
            // 如果有owner_id，则包含它
            if (device.owner_id.has_value()) {
                sql += ", owner_id";
            }
            
            sql += ") VALUES ('" +
                   mysql_real_escape_string(mysql_, new char[device.id.length() * 2 + 1], device.id.c_str(), device.id.length()) + "', '" +
                   mysql_real_escape_string(mysql_, new char[device.name.length() * 2 + 1], device.name.c_str(), device.name.length()) + "', '" +
                   mysql_real_escape_string(mysql_, new char[device.get_type_string().length() * 2 + 1], device.get_type_string().c_str(), device.get_type_string().length()) + "', '" +
                   mysql_real_escape_string(mysql_, new char[device.model.length() * 2 + 1], device.model.c_str(), device.model.length()) + "', '" +
                   mysql_real_escape_string(mysql_, new char[device.manufacturer.length() * 2 + 1], device.manufacturer.c_str(), device.manufacturer.length()) + "', '" +
                   mysql_real_escape_string(mysql_, new char[device.serial_number.length() * 2 + 1], device.serial_number.c_str(), device.serial_number.length()) + "', '" +
                   mysql_real_escape_string(mysql_, new char[device.get_status_string().length() * 2 + 1], device.get_status_string().c_str(), device.get_status_string().length()) + "', " +
                   std::to_string(device.signal_strength) + ", " +
                   std::to_string(device.battery_level) + ", " +
                   std::to_string(device.latitude) + ", " +
                   std::to_string(device.longitude) + ", " +
                   std::to_string(device.altitude);
            
            // 如果有owner_id，则包含它
            if (device.owner_id.has_value()) {
                sql += ", " + std::to_string(device.owner_id.value());
            }
            
            sql += ")";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "插入设备失败: " << mysql_error(mysql_) << std::endl;
                return "";
            }
            
            // 检查是否插入成功
            if (mysql_affected_rows(mysql_) > 0) {
                return device.id;
            }
            return "";
        } catch (const std::exception& e) {
            std::cerr << "插入设备失败: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<Device> DeviceDAO::get_device_by_id(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM devices WHERE id = '" + 
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据ID查询设备失败: " << mysql_error(mysql_) << std::endl;
                return std::nullopt;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return std::nullopt;
            }
            
            // 检查是否有结果
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                std::optional<Device> device = result_set_to_device(row);
                
                // 释放结果集
                mysql_free_result(result);
                return device;
            }
            
            // 释放结果集
            mysql_free_result(result);
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "根据ID查询设备失败: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<Device> DeviceDAO::get_devices_by_type(DeviceType type) {
        std::vector<Device> devices;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM devices WHERE type = '" + 
                              mysql_real_escape_string(mysql_, new char[Device::get_type_string().length() * 2 + 1], Device::get_type_string().c_str(), Device::get_type_string().length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据类型查询设备失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                devices.push_back(result_set_to_device(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据类型查询设备失败: " << e.what() << std::endl;
        }
        return devices;
    }

    std::vector<Device> DeviceDAO::get_devices_by_status(DeviceStatus status) {
        std::vector<Device> devices;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM devices WHERE status = '" + 
                              mysql_real_escape_string(mysql_, new char[Device::get_status_string().length() * 2 + 1], Device::get_status_string().c_str(), Device::get_status_string().length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据状态查询设备失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                devices.push_back(result_set_to_device(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据状态查询设备失败: " << e.what() << std::endl;
        }
        return devices;
    }

    std::vector<Device> DeviceDAO::get_devices_by_owner(int owner_id) {
        std::vector<Device> devices;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM devices WHERE owner_id = " + std::to_string(owner_id);
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据所有者查询设备失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                devices.push_back(result_set_to_device(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据所有者查询设备失败: " << e.what() << std::endl;
        }
        return devices;
    }

    std::vector<Device> DeviceDAO::get_all_devices() {
        std::vector<Device> devices;
        try {
            // 执行查询
            if (mysql_query(mysql_, "SELECT * FROM devices")) {
                std::cerr << "查询所有设备失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return devices;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                devices.push_back(result_set_to_device(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "查询所有设备失败: " << e.what() << std::endl;
        }
        return devices;
    }

    bool DeviceDAO::update_device(const Device& device) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE devices SET name = '" +
                              mysql_real_escape_string(mysql_, new char[device.name.length() * 2 + 1], device.name.c_str(), device.name.length()) + "', " +
                              "type = '" +
                              mysql_real_escape_string(mysql_, new char[device.get_type_string().length() * 2 + 1], device.get_type_string().c_str(), device.get_type_string().length()) + "', " +
                              "model = '" +
                              mysql_real_escape_string(mysql_, new char[device.model.length() * 2 + 1], device.model.c_str(), device.model.length()) + "', " +
                              "manufacturer = '" +
                              mysql_real_escape_string(mysql_, new char[device.manufacturer.length() * 2 + 1], device.manufacturer.c_str(), device.manufacturer.length()) + "', " +
                              "serial_number = '" +
                              mysql_real_escape_string(mysql_, new char[device.serial_number.length() * 2 + 1], device.serial_number.c_str(), device.serial_number.length()) + "', " +
                              "status = '" +
                              mysql_real_escape_string(mysql_, new char[device.get_status_string().length() * 2 + 1], device.get_status_string().c_str(), device.get_status_string().length()) + "', " +
                              "signal_strength = " + std::to_string(device.signal_strength) + ", " +
                              "battery_level = " + std::to_string(device.battery_level) + ", " +
                              "latitude = " + std::to_string(device.latitude) + ", " +
                              "longitude = " + std::to_string(device.longitude) + ", " +
                              "altitude = " + std::to_string(device.altitude) + ", " +
                              "last_update = NOW()";
            
            // 如果有owner_id，则包含它
            if (device.owner_id.has_value()) {
                sql += ", owner_id = " + std::to_string(device.owner_id.value());
            }
            
            sql += " WHERE id = '" +
                   mysql_real_escape_string(mysql_, new char[device.id.length() * 2 + 1], device.id.c_str(), device.id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新设备失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新设备失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool DeviceDAO::update_device_status(const std::string& device_id, DeviceStatus status) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE devices SET status = '" +
                              mysql_real_escape_string(mysql_, new char[Device::get_status_string().length() * 2 + 1], Device::get_status_string().c_str(), Device::get_status_string().length()) + "', " +
                              "last_update = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[device_id.length() * 2 + 1], device_id.c_str(), device_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新设备状态失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新设备状态失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool DeviceDAO::update_device_location(const std::string& device_id, double latitude, double longitude, double altitude) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE devices SET latitude = " + std::to_string(latitude) +
                              ", longitude = " + std::to_string(longitude) +
                              ", altitude = " + std::to_string(altitude) +
                              ", last_update = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[device_id.length() * 2 + 1], device_id.c_str(), device_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新设备位置失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新设备位置失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool DeviceDAO::update_device_battery(const std::string& device_id, int battery_level) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE devices SET battery_level = " + std::to_string(battery_level) +
                              ", last_update = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[device_id.length() * 2 + 1], device_id.c_str(), device_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新设备电池电量失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新设备电池电量失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool DeviceDAO::update_device_signal(const std::string& device_id, int signal_strength) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE devices SET signal_strength = " + std::to_string(signal_strength) +
                              ", last_update = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[device_id.length() * 2 + 1], device_id.c_str(), device_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新设备信号强度失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新设备信号强度失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool DeviceDAO::delete_device(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "DELETE FROM devices WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "删除设备失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否删除成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "删除设备失败: " << e.what() << std::endl;
            return false;
        }
    }

    Device DeviceDAO::result_set_to_device(MYSQL_ROW row) {
        Device device;
        device.id = row[0] ? row[0] : "";
        device.name = row[1] ? row[1] : "";
        device.type = Device::parse_type_string(row[2] ? row[2] : "");
        device.model = row[3] ? row[3] : "";
        device.manufacturer = row[4] ? row[4] : "";
        device.serial_number = row[5] ? row[5] : "";
        device.status = Device::parse_status_string(row[6] ? row[6] : "");
        device.signal_strength = row[7] ? std::stoi(row[7]) : 0;
        device.battery_level = row[8] ? std::stoi(row[8]) : 0;
        device.latitude = row[9] ? std::stod(row[9]) : 0.0;
        device.longitude = row[10] ? std::stod(row[10]) : 0.0;
        device.altitude = row[11] ? std::stod(row[11]) : 0.0;

        // Handle timestamps
        if (row[12]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[12], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            device.last_update = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[13]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[13], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            device.created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[14]) {
            device.owner_id = std::stoi(row[14]);
        }

        return device;
    }
}
