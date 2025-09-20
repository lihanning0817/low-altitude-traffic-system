#include "emergency_event_dao.h"
#include <iostream>

namespace models {
    std::string EmergencyEventDAO::create_emergency_event(const EmergencyEvent& emergency) {
        try {
            // 构建SQL查询
            std::string sql = "INSERT INTO emergency_events (id, type, severity, status, latitude, longitude, altitude, "
                              "description, reporter_id, reported_at) VALUES ('" +
                              mysql_real_escape_string(mysql_, new char[emergency.id.length() * 2 + 1], emergency.id.c_str(), emergency.id.length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[emergency.get_type_string().length() * 2 + 1], emergency.get_type_string().c_str(), emergency.get_type_string().length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[emergency.get_severity_string().length() * 2 + 1], emergency.get_severity_string().c_str(), emergency.get_severity_string().length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[emergency.get_status_string().length() * 2 + 1], emergency.get_status_string().c_str(), emergency.get_status_string().length()) + "', " +
                              std::to_string(emergency.latitude) + ", " +
                              std::to_string(emergency.longitude) + ", " +
                              std::to_string(emergency.altitude) + ", '" +
                              mysql_real_escape_string(mysql_, new char[emergency.description.length() * 2 + 1], emergency.description.c_str(), emergency.description.length()) + "', ";
            
            // 如果有reporter_id，则包含它
            if (emergency.reporter_id.has_value()) {
                sql += std::to_string(emergency.reporter_id.value()) + ", ";
            } else {
                sql += "NULL, ";
            }
            
            // 处理reported_at时间戳
            auto reported_at_time_t = std::chrono::system_clock::to_time_t(emergency.reported_at);
            struct tm* tm_info = localtime(&reported_at_time_t);
            char buffer[20];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
            sql += "'" + std::string(buffer) + "')";

            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "插入紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return "";
            }

            // 检查是否插入成功
            if (mysql_affected_rows(mysql_) > 0) {
                return emergency.id;
            }
            return "";
        } catch (const std::exception& e) {
            std::cerr << "插入紧急事件失败: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<EmergencyEvent> EmergencyEventDAO::get_emergency_event_by_id(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM emergency_events WHERE id = '" + 
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据ID查询紧急事件失败: " << mysql_error(mysql_) << std::endl;
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
                std::optional<EmergencyEvent> emergency = result_set_to_emergency_event(row);
                
                // 释放结果集
                mysql_free_result(result);
                return emergency;
            }
            
            // 释放结果集
            mysql_free_result(result);
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "根据ID查询紧急事件失败: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<EmergencyEvent> EmergencyEventDAO::get_emergency_events_by_status(EmergencyStatus status) {
        std::vector<EmergencyEvent> emergencies;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM emergency_events WHERE status = '" +
                              mysql_real_escape_string(mysql_, new char[EmergencyEvent::get_status_string().length() * 2 + 1], EmergencyEvent::get_status_string().c_str(), EmergencyEvent::get_status_string().length()) + "' ORDER BY reported_at DESC";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据状态查询紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                emergencies.push_back(result_set_to_emergency_event(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据状态查询紧急事件失败: " << e.what() << std::endl;
        }
        return emergencies;
    }

    std::vector<EmergencyEvent> EmergencyEventDAO::get_emergency_events_by_severity(EmergencySeverity severity) {
        std::vector<EmergencyEvent> emergencies;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM emergency_events WHERE severity = '" +
                              mysql_real_escape_string(mysql_, new char[EmergencyEvent::get_severity_string().length() * 2 + 1], EmergencyEvent::get_severity_string().c_str(), EmergencyEvent::get_severity_string().length()) + "' ORDER BY reported_at DESC";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据严重程度查询紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                emergencies.push_back(result_set_to_emergency_event(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据严重程度查询紧急事件失败: " << e.what() << std::endl;
        }
        return emergencies;
    }

    std::vector<EmergencyEvent> EmergencyEventDAO::get_emergency_events_by_type(EmergencyType type) {
        std::vector<EmergencyEvent> emergencies;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM emergency_events WHERE type = '" +
                              mysql_real_escape_string(mysql_, new char[EmergencyEvent::get_type_string().length() * 2 + 1], EmergencyEvent::get_type_string().c_str(), EmergencyEvent::get_type_string().length()) + "' ORDER BY reported_at DESC";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据类型查询紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                emergencies.push_back(result_set_to_emergency_event(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据类型查询紧急事件失败: " << e.what() << std::endl;
        }
        return emergencies;
    }

    std::vector<EmergencyEvent> EmergencyEventDAO::get_active_emergency_events() {
        std::vector<EmergencyEvent> emergencies;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM emergency_events WHERE status IN ('reported', 'responding') ORDER BY reported_at DESC";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "查询活动紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                emergencies.push_back(result_set_to_emergency_event(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "查询活动紧急事件失败: " << e.what() << std::endl;
        }
        return emergencies;
    }

    std::vector<EmergencyEvent> EmergencyEventDAO::get_all_emergency_events() {
        std::vector<EmergencyEvent> emergencies;
        try {
            // 执行查询
            if (mysql_query(mysql_, "SELECT * FROM emergency_events ORDER BY reported_at DESC")) {
                std::cerr << "查询所有紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return emergencies;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                emergencies.push_back(result_set_to_emergency_event(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "查询所有紧急事件失败: " << e.what() << std::endl;
        }
        return emergencies;
    }

    bool EmergencyEventDAO::update_emergency_event(const EmergencyEvent& emergency) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE emergency_events SET type = '" +
                              mysql_real_escape_string(mysql_, new char[emergency.get_type_string().length() * 2 + 1], emergency.get_type_string().c_str(), emergency.get_type_string().length()) + "', " +
                              "severity = '" +
                              mysql_real_escape_string(mysql_, new char[emergency.get_severity_string().length() * 2 + 1], emergency.get_severity_string().c_str(), emergency.get_severity_string().length()) + "', " +
                              "status = '" +
                              mysql_real_escape_string(mysql_, new char[emergency.get_status_string().length() * 2 + 1], emergency.get_status_string().c_str(), emergency.get_status_string().length()) + "', " +
                              "latitude = " + std::to_string(emergency.latitude) + ", " +
                              "longitude = " + std::to_string(emergency.longitude) + ", " +
                              "altitude = " + std::to_string(emergency.altitude) + ", " +
                              "description = '" +
                              mysql_real_escape_string(mysql_, new char[emergency.description.length() * 2 + 1], emergency.description.c_str(), emergency.description.length()) + "', ";
            
            // 处理 reporter_id
            if (emergency.reporter_id.has_value()) {
                sql += "reporter_id = " + std::to_string(emergency.reporter_id.value()) + ", ";
            } else {
                sql += "reporter_id = NULL, ";
            }
            
            // 处理 responder_id
            if (emergency.responder_id.has_value()) {
                sql += "responder_id = " + std::to_string(emergency.responder_id.value()) + ", ";
            } else {
                sql += "responder_id = NULL, ";
            }
            
            // 处理 reported_at 时间戳
            auto reported_at_time_t = std::chrono::system_clock::to_time_t(emergency.reported_at);
            struct tm* tm_info = localtime(&reported_at_time_t);
            char buffer[20];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
            sql += "reported_at = '" + std::string(buffer) + "', ";
            
            // 处理 resolved_at 时间戳
            if (emergency.resolved_at.has_value()) {
                auto resolved_at_time_t = std::chrono::system_clock::to_time_t(emergency.resolved_at.value());
                struct tm* tm_info_resolved = localtime(&resolved_at_time_t);
                char buffer_resolved[20];
                strftime(buffer_resolved, sizeof(buffer_resolved), "%Y-%m-%d %H:%M:%S", tm_info_resolved);
                sql += "resolved_at = '" + std::string(buffer_resolved) + "', ";
            } else {
                sql += "resolved_at = NULL, ";
            }
            
            // 更新时间
            sql += "updated_at = NOW() WHERE id = '" +
                   mysql_real_escape_string(mysql_, new char[emergency.id.length() * 2 + 1], emergency.id.c_str(), emergency.id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新紧急事件失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool EmergencyEventDAO::update_emergency_event_status(const std::string& emergency_id,
                                                       EmergencyStatus status,
                                                       int responder_id) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE emergency_events SET status = '" +
                              mysql_real_escape_string(mysql_, new char[EmergencyEvent::get_status_string().length() * 2 + 1], EmergencyEvent::get_status_string().c_str(), EmergencyEvent::get_status_string().length()) + "', ";
            
            // 处理 responder_id
            if (responder_id > 0) {
                sql += "responder_id = " + std::to_string(responder_id) + ", ";
            } else {
                sql += "responder_id = NULL, ";
            }
            
            // 更新时间
            sql += "updated_at = NOW() WHERE id = '" +
                   mysql_real_escape_string(mysql_, new char[emergency_id.length() * 2 + 1], emergency_id.c_str(), emergency_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新紧急事件状态失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新紧急事件状态失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool EmergencyEventDAO::resolve_emergency_event(const std::string& emergency_id, int resolver_id) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE emergency_events SET status = 'resolved', ";
            
            // 处理 responder_id
            if (resolver_id > 0) {
                sql += "responder_id = " + std::to_string(resolver_id) + ", ";
            } else {
                sql += "responder_id = NULL, ";
            }
            
            // 更新时间
            sql += "resolved_at = NOW(), updated_at = NOW() WHERE id = '" +
                   mysql_real_escape_string(mysql_, new char[emergency_id.length() * 2 + 1], emergency_id.c_str(), emergency_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "解决紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "解决紧急事件失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool EmergencyEventDAO::delete_emergency_event(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "DELETE FROM emergency_events WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "删除紧急事件失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否删除成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "删除紧急事件失败: " << e.what() << std::endl;
            return false;
        }
    }

    EmergencyEvent EmergencyEventDAO::result_set_to_emergency_event(MYSQL_ROW row) {
        EmergencyEvent emergency;
        emergency.id = row[0] ? row[0] : "";
        emergency.type = EmergencyEvent::parse_type_string(row[1] ? row[1] : "");
        emergency.severity = EmergencyEvent::parse_severity_string(row[2] ? row[2] : "");
        emergency.status = EmergencyEvent::parse_status_string(row[3] ? row[3] : "");
        emergency.latitude = row[4] ? std::stod(row[4]) : 0.0;
        emergency.longitude = row[5] ? std::stod(row[5]) : 0.0;
        emergency.altitude = row[6] ? std::stod(row[6]) : 0.0;
        emergency.description = row[7] ? row[7] : "";

        if (row[8]) {
            emergency.reporter_id = std::stoi(row[8]);
        }

        if (row[9]) {
            emergency.responder_id = std::stoi(row[9]);
        }

        // Handle timestamps
        if (row[10]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[10], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            emergency.reported_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[11]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[11], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            emergency.resolved_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[12]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[12], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            emergency.created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        return emergency;
    }
}
