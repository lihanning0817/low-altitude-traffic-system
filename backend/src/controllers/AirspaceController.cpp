#include "AirspaceController.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

using namespace controllers;
using json = nlohmann::json;

AirspaceController::AirspaceController(
    std::shared_ptr<mysqlx::Session> dbSession,
    std::shared_ptr<auth::JwtService> jwtService)
    : dbSession_(dbSession), jwtService_(jwtService)
{
}

// ========== 空域管理API接口实现 ==========

http::response<http::string_body> AirspaceController::getAirspaces(
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

        // 3. 构建查询SQL - 使用DATE_FORMAT将DATETIME转换为字符串
        std::string sql = "SELECT id, airspace_code, name, type, status, "
                          "min_altitude, max_altitude, geometry, description, "
                          "DATE_FORMAT(effective_time, '%Y-%m-%d %H:%i:%s') as effective_time, "
                          "DATE_FORMAT(expiry_time, '%Y-%m-%d %H:%i:%s') as expiry_time, "
                          "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
                          "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at "
                          "FROM low_altitude_traffic_system.airspaces WHERE 1=1";

        // 添加类型过滤
        if (queryParams.count("type"))
        {
            sql += " AND type = :type";
        }

        // 添加状态过滤
        if (queryParams.count("status"))
        {
            sql += " AND status = :status";
        }

        sql += " ORDER BY created_at DESC";

        // 4. 执行查询
        auto session_sql = dbSession_->sql(sql);

        // 绑定参数
        if (queryParams.count("type"))
        {
            session_sql.bind("type", queryParams["type"]);
        }
        if (queryParams.count("status"))
        {
            session_sql.bind("status", queryParams["status"]);
        }

        auto result = session_sql.execute();

        // 5. 构建响应数据
        json airspaces = json::array();
        for (auto row : result)
        {
            try
            {
                json airspace;
                airspace["id"] = row[0].get<int64_t>();
                airspace["airspace_code"] = row[1].get<std::string>();
                airspace["name"] = row[2].get<std::string>();
                airspace["type"] = row[3].get<std::string>();
                airspace["status"] = row[4].get<std::string>();
                airspace["min_altitude"] = row[5].get<double>();
                airspace["max_altitude"] = row[6].get<double>();

                // geometry字段（JSON格式）
                if (!row[7].isNull())
                {
                    std::string geometry_str = row[7].get<std::string>();
                    try
                    {
                        airspace["geometry"] = json::parse(geometry_str);
                    }
                    catch (...)
                    {
                        airspace["geometry"] = geometry_str;
                    }
                }

                // 可选字段
                if (!row[8].isNull())
                    airspace["description"] = row[8].get<std::string>();
                if (!row[9].isNull())
                    airspace["effective_time"] = row[9].get<std::string>();
                if (!row[10].isNull())
                    airspace["expiry_time"] = row[10].get<std::string>();
                if (!row[11].isNull())
                    airspace["created_at"] = row[11].get<std::string>();
                if (!row[12].isNull())
                    airspace["updated_at"] = row[12].get<std::string>();

                airspaces.push_back(airspace);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error processing airspace record: " << e.what() << std::endl;
                continue;
            }
        }

        json responseData = {{"airspaces", airspaces}};
        return utils::HttpResponse::createSuccessResponse(responseData, "获取空域列表成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取空域列表失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取空域列表失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> AirspaceController::getAirspaceById(
    const http::request<http::string_body> &req,
    const std::string &airspace_id)
{
    try
    {
        // 1. 验证JWT Token
        int64_t user_id = validateTokenAndGetUserId(req);
        if (user_id == -1)
        {
            return utils::HttpResponse::createUnauthorizedResponse("无效的认证Token");
        }

        // 2. 查询空域详情
        std::string sql = "SELECT id, airspace_code, name, type, status, "
                          "min_altitude, max_altitude, geometry, description, "
                          "DATE_FORMAT(effective_time, '%Y-%m-%d %H:%i:%s') as effective_time, "
                          "DATE_FORMAT(expiry_time, '%Y-%m-%d %H:%i:%s') as expiry_time, "
                          "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
                          "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at "
                          "FROM low_altitude_traffic_system.airspaces WHERE id = ?";

        auto result = dbSession_->sql(sql).bind(std::stoll(airspace_id)).execute();

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
            return utils::HttpResponse::createNotFoundResponse("空域不存在");
        }

        // 4. 构建响应数据
        json airspace;
        airspace["id"] = row[0].get<int64_t>();
        airspace["airspace_code"] = row[1].get<std::string>();
        airspace["name"] = row[2].get<std::string>();
        airspace["type"] = row[3].get<std::string>();
        airspace["status"] = row[4].get<std::string>();
        airspace["min_altitude"] = row[5].get<double>();
        airspace["max_altitude"] = row[6].get<double>();

        // geometry字段（JSON格式）
        if (!row[7].isNull())
        {
            std::string geometry_str = row[7].get<std::string>();
            try
            {
                airspace["geometry"] = json::parse(geometry_str);
            }
            catch (...)
            {
                airspace["geometry"] = geometry_str;
            }
        }

        // 可选字段
        if (!row[8].isNull())
            airspace["description"] = row[8].get<std::string>();
        if (!row[9].isNull())
            airspace["effective_time"] = row[9].get<std::string>();
        if (!row[10].isNull())
            airspace["expiry_time"] = row[10].get<std::string>();
        if (!row[11].isNull())
            airspace["created_at"] = row[11].get<std::string>();
        if (!row[12].isNull())
            airspace["updated_at"] = row[12].get<std::string>();

        return utils::HttpResponse::createSuccessResponse(airspace, "获取空域详情成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取空域详情失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取空域详情失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> AirspaceController::createAirspace(
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

        // 2. 验证管理员权限
        if (!isAdmin(user_id))
        {
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有管理员可以创建空域");
        }

        // 3. 解析请求体 (允许宽松的UTF-8解析)
        json requestData = json::parse(req.body(), nullptr, true, true);

        // 4. 验证必填字段
        if (!requestData.contains("name") || !requestData.contains("type") ||
            !requestData.contains("min_altitude") || !requestData.contains("max_altitude") ||
            !requestData.contains("geometry"))
        {
            return utils::HttpResponse::createErrorResponse("缺少必填字段: name, type, min_altitude, max_altitude, geometry");
        }

        // 5. 生成空域编号
        std::string airspace_code = generateAirspaceCode();

        // 6. 准备geometry字段（转换为JSON字符串）
        std::string geometry_str = requestData["geometry"].dump();

        // 7. 插入数据库
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        auto table = schema.getTable("airspaces");

        auto insert_op = table.insert("airspace_code", "name", "type", "status",
                                       "min_altitude", "max_altitude", "geometry");

        insert_op = insert_op.values(airspace_code,
                                      requestData["name"].get<std::string>(),
                                      requestData["type"].get<std::string>(),
                                      requestData.value("status", "active"),
                                      requestData["min_altitude"].get<double>(),
                                      requestData["max_altitude"].get<double>(),
                                      geometry_str);

        auto result = insert_op.execute();
        int64_t airspace_id = result.getAutoIncrementValue();

        // 8. 如果有可选字段，使用UPDATE语句添加
        if (requestData.contains("description") || requestData.contains("effective_time") || requestData.contains("expiry_time"))
        {
            std::vector<std::string> updates;
            std::vector<std::string> binds;

            if (requestData.contains("description"))
            {
                updates.push_back("description = ?");
                binds.push_back(requestData["description"].get<std::string>());
            }
            if (requestData.contains("effective_time"))
            {
                updates.push_back("effective_time = ?");
                binds.push_back(requestData["effective_time"].get<std::string>());
            }
            if (requestData.contains("expiry_time"))
            {
                updates.push_back("expiry_time = ?");
                binds.push_back(requestData["expiry_time"].get<std::string>());
            }

            if (!updates.empty())
            {
                std::string update_sql = "UPDATE airspaces SET ";
                for (size_t i = 0; i < updates.size(); ++i)
                {
                    if (i > 0)
                        update_sql += ", ";
                    update_sql += updates[i];
                }
                update_sql += " WHERE id = ?";

                auto update_stmt = dbSession_->sql(update_sql);
                for (const auto &bind_val : binds)
                {
                    update_stmt.bind(bind_val);
                }
                update_stmt.bind(airspace_id).execute();
            }
        }

        // 9. 返回成功响应
        json responseData = {
            {"airspace_id", airspace_id},
            {"airspace_code", airspace_code}};

        std::cout << "空域创建成功: ID=" << airspace_id << ", CODE=" << airspace_code << std::endl;
        return utils::HttpResponse::createSuccessResponse(responseData, "空域创建成功");
    }
    catch (const json::parse_error &e)
    {
        return utils::HttpResponse::createErrorResponse("JSON解析失败: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "创建空域失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("创建空域失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> AirspaceController::updateAirspace(
    const http::request<http::string_body> &req,
    const std::string &airspace_id)
{
    try
    {
        // 1. 验证JWT Token
        int64_t user_id = validateTokenAndGetUserId(req);
        if (user_id == -1)
        {
            return utils::HttpResponse::createUnauthorizedResponse("无效的认证Token");
        }

        // 2. 验证管理员权限
        if (!isAdmin(user_id))
        {
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有管理员可以更新空域");
        }

        // 3. 解析请求体 (允许宽松的UTF-8解析)
        json requestData = json::parse(req.body(), nullptr, true, true);

        // 4. 构建更新SQL
        std::vector<std::string> updates;
        std::vector<std::string> binds;

        if (requestData.contains("name"))
        {
            updates.push_back("name = ?");
            binds.push_back(requestData["name"].get<std::string>());
        }
        if (requestData.contains("type"))
        {
            updates.push_back("type = ?");
            binds.push_back(requestData["type"].get<std::string>());
        }
        if (requestData.contains("status"))
        {
            updates.push_back("status = ?");
            binds.push_back(requestData["status"].get<std::string>());
        }
        if (requestData.contains("min_altitude"))
        {
            updates.push_back("min_altitude = ?");
            binds.push_back(std::to_string(requestData["min_altitude"].get<double>()));
        }
        if (requestData.contains("max_altitude"))
        {
            updates.push_back("max_altitude = ?");
            binds.push_back(std::to_string(requestData["max_altitude"].get<double>()));
        }
        if (requestData.contains("description"))
        {
            updates.push_back("description = ?");
            binds.push_back(requestData["description"].get<std::string>());
        }

        // 如果没有要更新的字段
        if (updates.empty())
        {
            return utils::HttpResponse::createErrorResponse("没有要更新的字段");
        }

        // 添加updated_at字段
        updates.push_back("updated_at = NOW()");

        // 5. 执行更新
        std::string update_sql = "UPDATE airspaces SET ";
        for (size_t i = 0; i < updates.size(); ++i)
        {
            if (i > 0)
                update_sql += ", ";
            update_sql += updates[i];
        }
        update_sql += " WHERE id = ?";

        auto update_stmt = dbSession_->sql(update_sql);
        for (const auto &bind_val : binds)
        {
            update_stmt.bind(bind_val);
        }
        auto result = update_stmt.bind(std::stoll(airspace_id)).execute();

        if (result.getAffectedItemsCount() == 0)
        {
            return utils::HttpResponse::createNotFoundResponse("空域不存在");
        }

        std::cout << "空域更新成功: ID=" << airspace_id << std::endl;
        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "空域更新成功");
    }
    catch (const json::parse_error &e)
    {
        return utils::HttpResponse::createErrorResponse("JSON解析失败: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "更新空域失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("更新空域失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> AirspaceController::deleteAirspace(
    const http::request<http::string_body> &req,
    const std::string &airspace_id)
{
    try
    {
        // 1. 验证JWT Token
        int64_t user_id = validateTokenAndGetUserId(req);
        if (user_id == -1)
        {
            return utils::HttpResponse::createUnauthorizedResponse("无效的认证Token");
        }

        // 2. 验证管理员权限
        if (!isAdmin(user_id))
        {
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有管理员可以删除空域");
        }

        // 3. 检查空域是否存在
        auto check_result = dbSession_->sql("SELECT id FROM airspaces WHERE id = ?")
                                .bind(std::stoll(airspace_id))
                                .execute();

        bool exists = false;
        for (auto r : check_result)
        {
            exists = true;
            break;
        }

        if (!exists)
        {
            return utils::HttpResponse::createNotFoundResponse("空域不存在");
        }

        // 4. 检查是否有关联的飞行许可
        auto permit_result = dbSession_->sql("SELECT COUNT(*) FROM flight_permits WHERE airspace_id = ?")
                                 .bind(std::stoll(airspace_id))
                                 .execute();

        int64_t permit_count = 0;
        for (auto row : permit_result)
        {
            permit_count = row[0].get<int64_t>();
        }

        if (permit_count > 0)
        {
            return utils::HttpResponse::createErrorResponse("该空域存在关联的飞行许可，无法删除");
        }

        // 5. 删除空域
        auto result = dbSession_->sql("DELETE FROM airspaces WHERE id = ?")
                          .bind(std::stoll(airspace_id))
                          .execute();

        std::cout << "空域删除成功: ID=" << airspace_id << std::endl;
        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "空域删除成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "删除空域失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("删除空域失败: " + std::string(e.what()));
    }
}

// ========== 辅助方法实现 ==========

std::string AirspaceController::extractBearerToken(const http::request<http::string_body> &req)
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

int64_t AirspaceController::validateTokenAndGetUserId(const http::request<http::string_body> &req)
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

bool AirspaceController::isAdmin(int64_t user_id)
{
    try
    {
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        auto result = dbSession_->sql("SELECT role FROM users WHERE id = ?")
                          .bind(user_id)
                          .execute();

        for (auto row : result)
        {
            std::string role = row[0].get<std::string>();
            return role == "admin";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "检查管理员权限失败: " << e.what() << std::endl;
    }

    return false;
}

std::string AirspaceController::generateAirspaceCode()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm timeinfo;
    localtime_s(&timeinfo, &time); // 使用 localtime_s 代替 localtime

    std::stringstream ss;
    ss << "AS" << std::put_time(&timeinfo, "%Y%m%d%H%M%S")
       << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::map<std::string, std::string> AirspaceController::parseQueryParams(
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
