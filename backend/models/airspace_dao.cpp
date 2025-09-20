#include "airspace_dao.h"
#include <iostream>

namespace models {
    std::string AirspaceDAO::create_airspace(const Airspace& airspace) {
        try {
            // 构建SQL查询
            std::string sql = "INSERT INTO airspaces (id, name, description, min_altitude, max_altitude, "
                              "boundary_coordinates, capacity, current_flights, status, restriction_reason) "
                              "VALUES ('" +
                              mysql_real_escape_string(mysql_, new char[airspace.id.length() * 2 + 1], airspace.id.c_str(), airspace.id.length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[airspace.name.length() * 2 + 1], airspace.name.c_str(), airspace.name.length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[airspace.description.length() * 2 + 1], airspace.description.c_str(), airspace.description.length()) + "', " +
                              std::to_string(airspace.min_altitude) + ", " +
                              std::to_string(airspace.max_altitude) + ", '" +
                              mysql_real_escape_string(mysql_, new char[airspace.boundary_to_json().length() * 2 + 1], airspace.boundary_to_json().c_str(), airspace.boundary_to_json().length()) + "', " +
                              std::to_string(airspace.capacity) + ", " +
                              std::to_string(airspace.current_flights) + ", '" +
                              mysql_real_escape_string(mysql_, new char[airspace.get_status_string().length() * 2 + 1], airspace.get_status_string().c_str(), airspace.get_status_string().length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[airspace.restriction_reason.length() * 2 + 1], airspace.restriction_reason.c_str(), airspace.restriction_reason.length()) + "')";

            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "插入空域失败: " << mysql_error(mysql_) << std::endl;
                return "";
            }

            // 检查是否插入成功
            if (mysql_affected_rows(mysql_) > 0) {
                return airspace.id;
            }
            return "";
        } catch (const std::exception& e) {
            std::cerr << "插入空域失败: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<Airspace> AirspaceDAO::get_airspace_by_id(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM airspaces WHERE id = '" + 
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据ID查询空域失败: " << mysql_error(mysql_) << std::endl;
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
                std::optional<Airspace> airspace = result_set_to_airspace(row);
                
                // 释放结果集
                mysql_free_result(result);
                return airspace;
            }
            
            // 释放结果集
            mysql_free_result(result);
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "根据ID查询空域失败: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<Airspace> AirspaceDAO::get_airspace_by_status(AirspaceStatus status) {
        std::vector<Airspace> airspaces;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM airspaces WHERE status = '" + 
                              mysql_real_escape_string(mysql_, new char[Airspace::get_status_string().length() * 2 + 1], Airspace::get_status_string().c_str(), Airspace::get_status_string().length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据状态查询空域失败: " << mysql_error(mysql_) << std::endl;
                return airspaces;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return airspaces;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                airspaces.push_back(result_set_to_airspace(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据状态查询空域失败: " << e.what() << std::endl;
        }
        return airspaces;
    }

    std::vector<Airspace> AirspaceDAO::get_all_airspaces() {
        std::vector<Airspace> airspaces;
        try {
            // 执行查询
            if (mysql_query(mysql_, "SELECT * FROM airspaces")) {
                std::cerr << "查询所有空域失败: " << mysql_error(mysql_) << std::endl;
                return airspaces;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return airspaces;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                airspaces.push_back(result_set_to_airspace(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "查询所有空域失败: " << e.what() << std::endl;
        }
        return airspaces;
    }

    bool AirspaceDAO::update_airspace(const Airspace& airspace) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE airspaces SET name = '" +
                              mysql_real_escape_string(mysql_, new char[airspace.name.length() * 2 + 1], airspace.name.c_str(), airspace.name.length()) + "', " +
                              "description = '" +
                              mysql_real_escape_string(mysql_, new char[airspace.description.length() * 2 + 1], airspace.description.c_str(), airspace.description.length()) + "', " +
                              "min_altitude = " + std::to_string(airspace.min_altitude) + ", " +
                              "max_altitude = " + std::to_string(airspace.max_altitude) + ", " +
                              "boundary_coordinates = '" +
                              mysql_real_escape_string(mysql_, new char[airspace.boundary_to_json().length() * 2 + 1], airspace.boundary_to_json().c_str(), airspace.boundary_to_json().length()) + "', " +
                              "capacity = " + std::to_string(airspace.capacity) + ", " +
                              "current_flights = " + std::to_string(airspace.current_flights) + ", " +
                              "status = '" +
                              mysql_real_escape_string(mysql_, new char[airspace.get_status_string().length() * 2 + 1], airspace.get_status_string().c_str(), airspace.get_status_string().length()) + "', " +
                              "restriction_reason = '" +
                              mysql_real_escape_string(mysql_, new char[airspace.restriction_reason.length() * 2 + 1], airspace.restriction_reason.c_str(), airspace.restriction_reason.length()) + "', " +
                              "updated_at = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[airspace.id.length() * 2 + 1], airspace.id.c_str(), airspace.id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新空域失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新空域失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool AirspaceDAO::update_airspace_status(const std::string& airspace_id, AirspaceStatus status,
                                           const std::string& restriction_reason) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE airspaces SET status = '" +
                              mysql_real_escape_string(mysql_, new char[Airspace::get_status_string().length() * 2 + 1], Airspace::get_status_string().c_str(), Airspace::get_status_string().length()) + "', " +
                              "restriction_reason = '" +
                              mysql_real_escape_string(mysql_, new char[restriction_reason.length() * 2 + 1], restriction_reason.c_str(), restriction_reason.length()) + "', " +
                              "updated_at = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[airspace_id.length() * 2 + 1], airspace_id.c_str(), airspace_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新空域状态失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新空域状态失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool AirspaceDAO::update_airspace_flight_count(const std::string& airspace_id, int current_flights) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE airspaces SET current_flights = " + std::to_string(current_flights) +
                              ", updated_at = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[airspace_id.length() * 2 + 1], airspace_id.c_str(), airspace_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新空域飞行数量失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新空域飞行数量失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool AirspaceDAO::delete_airspace(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "DELETE FROM airspaces WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "删除空域失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否删除成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "删除空域失败: " << e.what() << std::endl;
            return false;
        }
    }

    Airspace AirspaceDAO::result_set_to_airspace(MYSQL_ROW row) {
        Airspace airspace;
        airspace.id = row[0] ? row[0] : "";
        airspace.name = row[1] ? row[1] : "";
        airspace.description = row[2] ? row[2] : "";
        airspace.min_altitude = row[3] ? std::stod(row[3]) : 0.0;
        airspace.max_altitude = row[4] ? std::stod(row[4]) : 0.0;
        airspace.boundary_from_json(row[5] ? row[5] : "");
        airspace.capacity = row[6] ? std::stoi(row[6]) : 0;
        airspace.current_flights = row[7] ? std::stoi(row[7]) : 0;
        airspace.status = Airspace::parse_status_string(row[8] ? row[8] : "");
        airspace.restriction_reason = row[9] ? row[9] : "";

        // Handle timestamps
        if (row[10]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[10], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            airspace.created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[11]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[11], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            airspace.updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        return airspace;
    }
}
