#include "route_dao.h"
#include <iostream>
#include <json/json.h>

namespace models {
    std::string RouteDAO::create_route(const Route& route) {
        try {
            // 构建SQL查询
            std::string sql = "INSERT INTO routes (id, name, start_latitude, start_longitude, end_latitude, end_longitude, "
                              "total_distance, total_time, waypoints, creator_id, flight_task_id) "
                              "VALUES ('" +
                              mysql_real_escape_string(mysql_, new char[route.id.length() * 2 + 1], route.id.c_str(), route.id.length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[route.name.length() * 2 + 1], route.name.c_str(), route.name.length()) + "', " +
                              std::to_string(route.start_point.latitude) + ", " +
                              std::to_string(route.start_point.longitude) + ", " +
                              std::to_string(route.end_point.latitude) + ", " +
                              std::to_string(route.end_point.longitude) + ", " +
                              std::to_string(route.total_distance) + ", " +
                              std::to_string(route.total_time) + ", '" +
                              mysql_real_escape_string(mysql_, new char[waypoints_to_json(route.waypoints).length() * 2 + 1], waypoints_to_json(route.waypoints).c_str(), waypoints_to_json(route.waypoints).length()) + "', " +
                              std::to_string(route.creator_id) + ", '" +
                              mysql_real_escape_string(mysql_, new char[route.flight_task_id.length() * 2 + 1], route.flight_task_id.c_str(), route.flight_task_id.length()) + "')";

            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "插入航线失败: " << mysql_error(mysql_) << std::endl;
                return "";
            }

            // 检查是否插入成功
            if (mysql_affected_rows(mysql_) > 0) {
                return route.id;
            }
            return "";
        } catch (const std::exception& e) {
            std::cerr << "插入航线失败: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<Route> RouteDAO::get_route_by_id(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM routes WHERE id = '" + 
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据ID查询航线失败: " << mysql_error(mysql_) << std::endl;
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
                std::optional<Route> route = result_set_to_route(row);
                
                // 释放结果集
                mysql_free_result(result);
                return route;
            }
            
            // 释放结果集
            mysql_free_result(result);
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "根据ID查询航线失败: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<Route> RouteDAO::get_routes_by_creator(int creator_id) {
        std::vector<Route> routes;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM routes WHERE creator_id = " + std::to_string(creator_id);
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据创建者查询航线失败: " << mysql_error(mysql_) << std::endl;
                return routes;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return routes;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                routes.push_back(result_set_to_route(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据创建者查询航线失败: " << e.what() << std::endl;
        }
        return routes;
    }

    std::vector<Route> RouteDAO::get_routes_by_flight_task(const std::string& flight_task_id) {
        std::vector<Route> routes;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM routes WHERE flight_task_id = '" + 
                              mysql_real_escape_string(mysql_, new char[flight_task_id.length() * 2 + 1], flight_task_id.c_str(), flight_task_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据飞行任务查询航线失败: " << mysql_error(mysql_) << std::endl;
                return routes;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return routes;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                routes.push_back(result_set_to_route(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据飞行任务查询航线失败: " << e.what() << std::endl;
        }
        return routes;
    }

    std::vector<Route> RouteDAO::get_all_routes() {
        std::vector<Route> routes;
        try {
            // 执行查询
            if (mysql_query(mysql_, "SELECT * FROM routes")) {
                std::cerr << "查询所有航线失败: " << mysql_error(mysql_) << std::endl;
                return routes;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return routes;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                routes.push_back(result_set_to_route(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "查询所有航线失败: " << e.what() << std::endl;
        }
        return routes;
    }

    bool RouteDAO::update_route(const Route& route) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE routes SET name = '" +
                              mysql_real_escape_string(mysql_, new char[route.name.length() * 2 + 1], route.name.c_str(), route.name.length()) + "', " +
                              "start_latitude = " + std::to_string(route.start_point.latitude) + ", " +
                              "start_longitude = " + std::to_string(route.start_point.longitude) + ", " +
                              "end_latitude = " + std::to_string(route.end_point.latitude) + ", " +
                              "end_longitude = " + std::to_string(route.end_point.longitude) + ", " +
                              "total_distance = " + std::to_string(route.total_distance) + ", " +
                              "total_time = " + std::to_string(route.total_time) + ", " +
                              "waypoints = '" +
                              mysql_real_escape_string(mysql_, new char[waypoints_to_json(route.waypoints).length() * 2 + 1], waypoints_to_json(route.waypoints).c_str(), waypoints_to_json(route.waypoints).length()) + "', " +
                              "creator_id = " + std::to_string(route.creator_id) + ", " +
                              "flight_task_id = '" +
                              mysql_real_escape_string(mysql_, new char[route.flight_task_id.length() * 2 + 1], route.flight_task_id.c_str(), route.flight_task_id.length()) + "' " +
                              "WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[route.id.length() * 2 + 1], route.id.c_str(), route.id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新航线失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新航线失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool RouteDAO::delete_route(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "DELETE FROM routes WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "删除航线失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否删除成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "删除航线失败: " << e.what() << std::endl;
            return false;
        }
    }

    Route RouteDAO::result_set_to_route(MYSQL_ROW row) {
        Route route;
        route.id = row[0] ? row[0] : "";
        route.name = row[1] ? row[1] : "";
        route.start_point.latitude = row[2] ? std::stod(row[2]) : 0.0;
        route.start_point.longitude = row[3] ? std::stod(row[3]) : 0.0;
        route.end_point.latitude = row[4] ? std::stod(row[4]) : 0.0;
        route.end_point.longitude = row[5] ? std::stod(row[5]) : 0.0;
        route.total_distance = row[6] ? std::stod(row[6]) : 0.0;
        route.total_time = row[7] ? std::stoi(row[7]) : 0;
        route.waypoints = json_to_waypoints(row[8] ? row[8] : "");
        route.creator_id = row[9] ? std::stoi(row[9]) : 0;
        route.flight_task_id = row[10] ? row[10] : "";

        // Handle timestamps
        if (row[11]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[11], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            route.created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        // Reconstruct segments from waypoints
        for (size_t i = 0; i < route.waypoints.size() - 1; ++i) {
            RouteSegment segment;
            segment.from = route.waypoints[i];
            segment.to = route.waypoints[i + 1];
            // Calculate distance between waypoints (simplified)
            segment.distance = 0.0; // In a real implementation, calculate actual distance
            route.segments.push_back(segment);
        }

        return route;
    }

    std::string RouteDAO::waypoints_to_json(const std::vector<Waypoint>& waypoints) {
        Json::Value root(Json::arrayValue);
        for (const auto& waypoint : waypoints) {
            Json::Value wp;
            wp["latitude"] = waypoint.latitude;
            wp["longitude"] = waypoint.longitude;
            wp["is_main_road"] = waypoint.is_main_road;
            wp["is_start"] = waypoint.is_start;
            wp["is_end"] = waypoint.is_end;
            root.append(wp);
        }

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    }

    std::vector<Waypoint> RouteDAO::json_to_waypoints(const std::string& json_str) {
        std::vector<Waypoint> waypoints;
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        std::string errors;

        if (reader->parse(json_str.c_str(), json_str.c_str() + json_str.length(), &root, &errors)) {
            if (root.isArray()) {
                for (const auto& wp_value : root) {
                    Waypoint wp;
                    wp.latitude = wp_value["latitude"].asDouble();
                    wp.longitude = wp_value["longitude"].asDouble();
                    wp.is_main_road = wp_value["is_main_road"].asBool();
                    wp.is_start = wp_value.get("is_start", false).asBool();
                    wp.is_end = wp_value.get("is_end", false).asBool();
                    waypoints.push_back(wp);
                }
            }
        }

        return waypoints;
    }
}
