#include "DroneController.h"
#include <iostream>
#include <sstream>

using namespace controllers;
using json = nlohmann::json;

DroneController::DroneController(
    std::shared_ptr<mysqlx::Session> dbSession,
    std::shared_ptr<auth::JwtService> jwtService)
    : dbSession_(dbSession), jwtService_(jwtService)
{
}

// ========== 无人机管理API接口实现 ==========

http::response<http::string_body> DroneController::getDrones(
    const http::request<http::string_body> &req)
{
    try
    {
        // 1. 验证JWT Token
        int64_t user_id = validateTokenAndGetUserId(req);
        if (user_id == -1)
        {
            return utils::HttpResponse::createUnauthorizedResponse("无效的认证Token");
        }

        // 2. 解析查询参数
        auto queryParams = parseQueryParams(req);

        // 3. 构建查询SQL - 使用DATE_FORMAT将DATE/DATETIME转换为字符串
        std::string sql = "SELECT id, drone_code, name, model, manufacturer, type, status, "
                          "owner_id, registration_number, serial_number, "
                          "max_altitude, max_speed, max_flight_time, battery_capacity, weight, "
                          "DATE_FORMAT(purchase_date, '%Y-%m-%d') as purchase_date, "
                          "DATE_FORMAT(last_maintenance_date, '%Y-%m-%d') as last_maintenance_date, "
                          "DATE_FORMAT(next_maintenance_date, '%Y-%m-%d') as next_maintenance_date, "
                          "certifications, description, "
                          "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
                          "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at "
                          "FROM low_altitude_traffic_system.drones WHERE 1=1";

        // 添加状态过滤
        if (queryParams.count("status"))
        {
            sql += " AND status = :status";
        }

        // 添加所有者过滤
        if (queryParams.count("owner_id"))
        {
            sql += " AND owner_id = :owner_id";
        }

        // 添加类型过滤
        if (queryParams.count("type"))
        {
            sql += " AND type = :type";
        }

        sql += " ORDER BY created_at DESC";

        // 4. 执行查询
        auto session_sql = dbSession_->sql(sql);

        // 绑定参数
        if (queryParams.count("status"))
        {
            session_sql.bind("status", queryParams["status"]);
        }
        if (queryParams.count("owner_id"))
        {
            session_sql.bind("owner_id", std::stoll(queryParams["owner_id"]));
        }
        if (queryParams.count("type"))
        {
            session_sql.bind("type", queryParams["type"]);
        }

        auto result = session_sql.execute();

        // 5. 构建响应数据
        json drones = json::array();
        for (auto row : result)
        {
            try
            {
                json drone;
                drone["id"] = row[0].get<int64_t>();
                drone["drone_code"] = row[1].get<std::string>();
                drone["name"] = row[2].get<std::string>();
                drone["model"] = row[3].get<std::string>();

                // 可选字段
                if (!row[4].isNull())
                    drone["manufacturer"] = row[4].get<std::string>();

                drone["type"] = row[5].get<std::string>();
                drone["status"] = row[6].get<std::string>();
                drone["owner_id"] = row[7].get<int64_t>();

                if (!row[8].isNull())
                    drone["registration_number"] = row[8].get<std::string>();
                if (!row[9].isNull())
                    drone["serial_number"] = row[9].get<std::string>();
                if (!row[10].isNull())
                    drone["max_altitude"] = row[10].get<double>();
                if (!row[11].isNull())
                    drone["max_speed"] = row[11].get<double>();
                if (!row[12].isNull())
                    drone["max_flight_time"] = row[12].get<int>();
                if (!row[13].isNull())
                    drone["battery_capacity"] = row[13].get<int>();
                if (!row[14].isNull())
                    drone["weight"] = row[14].get<double>();

                // DATE字段已通过DATE_FORMAT转换为字符串
                if (!row[15].isNull())
                    drone["purchase_date"] = row[15].get<std::string>();
                if (!row[16].isNull())
                    drone["last_maintenance_date"] = row[16].get<std::string>();
                if (!row[17].isNull())
                    drone["next_maintenance_date"] = row[17].get<std::string>();

                if (!row[18].isNull())
                    drone["certifications"] = row[18].get<std::string>();
                if (!row[19].isNull())
                    drone["description"] = row[19].get<std::string>();

                // DATETIME字段
                if (!row[20].isNull())
                    drone["created_at"] = row[20].get<std::string>();
                if (!row[21].isNull())
                    drone["updated_at"] = row[21].get<std::string>();

                drones.push_back(drone);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error processing drone record: " << e.what() << std::endl;
                continue;
            }
        }

        json responseData = {{"drones", drones}};
        return utils::HttpResponse::createSuccessResponse(responseData, "获取无人机列表成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取无人机列表失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取无人机列表失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> DroneController::getDroneById(
    const http::request<http::string_body> &req,
    const std::string &drone_id)
{
    try
    {
        // 1. 验证JWT Token
        int64_t user_id = validateTokenAndGetUserId(req);
        if (user_id == -1)
        {
            return utils::HttpResponse::createUnauthorizedResponse("无效的认证Token");
        }

        // 2. 查询无人机详情
        std::string sql = "SELECT id, drone_code, name, model, manufacturer, type, status, "
                          "owner_id, registration_number, serial_number, "
                          "max_altitude, max_speed, max_flight_time, battery_capacity, weight, "
                          "DATE_FORMAT(purchase_date, '%Y-%m-%d') as purchase_date, "
                          "DATE_FORMAT(last_maintenance_date, '%Y-%m-%d') as last_maintenance_date, "
                          "DATE_FORMAT(next_maintenance_date, '%Y-%m-%d') as next_maintenance_date, "
                          "certifications, description, "
                          "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
                          "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at "
                          "FROM low_altitude_traffic_system.drones WHERE id = ?";

        auto result = dbSession_->sql(sql).bind(std::stoll(drone_id)).execute();

        // 3. 检查是否存在
        bool found = false;
        mysqlx::Row row;
        for (auto r : result)
        {
            row = r;
            found = true;
            break;
        }

        if (!found)
        {
            return utils::HttpResponse::createNotFoundResponse("无人机不存在");
        }

        // 4. 构建响应数据
        json drone;

        drone["id"] = row[0].get<int64_t>();
        drone["drone_code"] = row[1].get<std::string>();
        drone["name"] = row[2].get<std::string>();
        drone["model"] = row[3].get<std::string>();

        // 可选字段
        if (!row[4].isNull())
            drone["manufacturer"] = row[4].get<std::string>();

        drone["type"] = row[5].get<std::string>();
        drone["status"] = row[6].get<std::string>();
        drone["owner_id"] = row[7].get<int64_t>();

        if (!row[8].isNull())
            drone["registration_number"] = row[8].get<std::string>();
        if (!row[9].isNull())
            drone["serial_number"] = row[9].get<std::string>();
        if (!row[10].isNull())
            drone["max_altitude"] = row[10].get<double>();
        if (!row[11].isNull())
            drone["max_speed"] = row[11].get<double>();
        if (!row[12].isNull())
            drone["max_flight_time"] = row[12].get<int>();
        if (!row[13].isNull())
            drone["battery_capacity"] = row[13].get<int>();
        if (!row[14].isNull())
            drone["weight"] = row[14].get<double>();

        // DATE字段已通过DATE_FORMAT转换为字符串
        if (!row[15].isNull())
            drone["purchase_date"] = row[15].get<std::string>();
        if (!row[16].isNull())
            drone["last_maintenance_date"] = row[16].get<std::string>();
        if (!row[17].isNull())
            drone["next_maintenance_date"] = row[17].get<std::string>();

        if (!row[18].isNull())
            drone["certifications"] = row[18].get<std::string>();
        if (!row[19].isNull())
            drone["description"] = row[19].get<std::string>();

        // DATETIME字段
        if (!row[20].isNull())
            drone["created_at"] = row[20].get<std::string>();
        if (!row[21].isNull())
            drone["updated_at"] = row[21].get<std::string>();

        return utils::HttpResponse::createSuccessResponse(drone, "获取无人机详情成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取无人机详情失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取无人机详情失败: " + std::string(e.what()));
    }
}

// ========== 辅助方法实现 ==========

std::string DroneController::extractBearerToken(const http::request<http::string_body> &req)
{
    auto auth_header = req.find(http::field::authorization);
    if (auth_header == req.end())
    {
        return "";
    }

    std::string auth_value = std::string(auth_header->value());
    const std::string bearer_prefix = "Bearer ";

    if (auth_value.substr(0, bearer_prefix.length()) != bearer_prefix)
    {
        return "";
    }

    return auth_value.substr(bearer_prefix.length());
}

int64_t DroneController::validateTokenAndGetUserId(const http::request<http::string_body> &req)
{
    std::string token = extractBearerToken(req);
    if (token.empty())
    {
        return -1;
    }

    try
    {
        auto tokenInfo = jwtService_->parseToken(token);
        if (tokenInfo.has_value())
        {
            return tokenInfo->user_id;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Token验证失败: " << e.what() << std::endl;
    }

    return -1;
}

std::map<std::string, std::string> DroneController::parseQueryParams(
    const http::request<http::string_body> &req)
{
    std::map<std::string, std::string> params;
    std::string target = std::string(req.target());

    size_t query_pos = target.find('?');
    if (query_pos == std::string::npos)
    {
        return params;
    }

    std::string query = target.substr(query_pos + 1);
    std::istringstream query_stream(query);
    std::string param;

    while (std::getline(query_stream, param, '&'))
    {
        size_t eq_pos = param.find('=');
        if (eq_pos != std::string::npos)
        {
            std::string key = param.substr(0, eq_pos);
            std::string value = param.substr(eq_pos + 1);
            params[key] = value;
        }
    }

    return params;
}
