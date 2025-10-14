#include "EmergencyLandingController.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace controllers;
using json = nlohmann::json;

EmergencyLandingController::EmergencyLandingController(
    std::shared_ptr<mysqlx::Session> dbSession,
    std::shared_ptr<auth::JwtService> jwtService)
    : dbSession_(dbSession), jwtService_(jwtService)
{
}

// ========== 紧急降落点API接口实现 ==========

http::response<http::string_body> EmergencyLandingController::getEmergencyLandingPoints(
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

        // 3. 构建查询SQL - 使用DATE_FORMAT将TIMESTAMP转换为字符串
        std::string sql = "SELECT id, point_code, name, type, lat, lng, altitude, "
                          "capacity, status, safety_rating, weather_protected, accessible_24h, "
                          "contact_name, contact_phone, address, description, "
                          "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
                          "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at, "
                          "facilities "
                          "FROM low_altitude_traffic_system.emergency_landing_points WHERE 1=1";

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

        // 添加安全等级过滤
        if (queryParams.count("safety_rating"))
        {
            sql += " AND safety_rating = :safety_rating";
        }

        sql += " ORDER BY safety_rating ASC, name ASC";

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
        if (queryParams.count("safety_rating"))
        {
            session_sql.bind("safety_rating", queryParams["safety_rating"]);
        }

        auto result = session_sql.execute();

        // 5. 构建响应数据
        json landing_points = json::array();
        for (auto row : result)
        {
            json point = {
                {"id", row[0].get<int64_t>()},
                {"point_code", row[1].get<std::string>()},
                {"name", row[2].get<std::string>()},
                {"type", row[3].get<std::string>()},
                {"lat", row[4].get<double>()},
                {"lng", row[5].get<double>()},
                {"capacity", row[7].get<int>()},
                {"status", row[8].get<std::string>()}};

            // 可选字段
            if (!row[6].isNull())
                point["altitude"] = row[6].get<double>();
            if (!row[9].isNull())
                point["safety_rating"] = row[9].get<std::string>();
            if (!row[10].isNull())
                point["weather_protected"] = row[10].get<bool>();
            if (!row[11].isNull())
                point["accessible_24h"] = row[11].get<bool>();
            if (!row[12].isNull())
                point["contact_name"] = row[12].get<std::string>();
            if (!row[13].isNull())
                point["contact_phone"] = row[13].get<std::string>();
            if (!row[14].isNull())
                point["address"] = row[14].get<std::string>();
            if (!row[15].isNull())
                point["description"] = row[15].get<std::string>();

            // TIMESTAMP字段已通过DATE_FORMAT转换为字符串
            if (!row[16].isNull())
                point["created_at"] = row[16].get<std::string>();
            if (!row[17].isNull())
                point["updated_at"] = row[17].get<std::string>();
            if (!row[18].isNull())
                point["facilities"] = row[18].get<std::string>();

            landing_points.push_back(point);
        }

        json responseData = {{"landing_points", landing_points}};
        return utils::HttpResponse::createSuccessResponse(responseData, "获取紧急降落点列表成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取紧急降落点列表失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取紧急降落点列表失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> EmergencyLandingController::addEmergencyLandingPoint(
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
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有管理员可以添加紧急降落点");
        }

        // 3. 解析请求体 (允许宽松的UTF-8解析)
        json requestData = json::parse(req.body(), nullptr, true, true);

        // 4. 验证必填字段
        if (!requestData.contains("name") || !requestData.contains("type") ||
            !requestData.contains("lat") || !requestData.contains("lng"))
        {
            return utils::HttpResponse::createErrorResponse("缺少必填字段: name, type, lat, lng");
        }

        // 5. 生成降落点编号
        std::string point_code = generatePointCode();

        // 6. 插入数据库
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        auto table = schema.getTable("emergency_landing_points");

        auto result = table.insert("point_code", "name", "type", "lat", "lng",
                                    "altitude", "capacity", "status", "safety_rating",
                                    "weather_protected", "accessible_24h")
                          .values(point_code,
                                  requestData["name"].get<std::string>(),
                                  requestData["type"].get<std::string>(),
                                  requestData["lat"].get<double>(),
                                  requestData["lng"].get<double>(),
                                  requestData.value("altitude", 0.0),
                                  requestData.value("capacity", 1),
                                  "active",
                                  requestData.value("safety_rating", "C"),
                                  requestData.value("weather_protected", false),
                                  requestData.value("accessible_24h", true))
                          .execute();

        int64_t point_id = result.getAutoIncrementValue();

        // 7. 返回成功响应
        json responseData = {
            {"point_id", point_id},
            {"point_code", point_code}};

        return utils::HttpResponse::createSuccessResponse(responseData, "紧急降落点添加成功");
    }
    catch (const json::parse_error &e)
    {
        return utils::HttpResponse::createErrorResponse("JSON解析失败: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "添加紧急降落点失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("添加紧急降落点失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> EmergencyLandingController::findNearestLandingPoints(
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

        // 3. 验证必填参数
        if (!queryParams.count("lat") || !queryParams.count("lng"))
        {
            return utils::HttpResponse::createErrorResponse("缺少必填参数: lat, lng");
        }

        double current_lat = std::stod(queryParams["lat"]);
        double current_lng = std::stod(queryParams["lng"]);
        int limit = queryParams.count("limit") ? std::stoi(queryParams["limit"]) : 5;
        double radius = queryParams.count("radius") ? std::stod(queryParams["radius"]) : 10000.0;

        // 4. 查询所有活跃的降落点 - 使用DATE_FORMAT将TIMESTAMP转换为字符串
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        auto result = dbSession_->sql(
            "SELECT id, point_code, name, type, lat, lng, altitude, "
            "capacity, status, safety_rating, weather_protected, accessible_24h, "
            "contact_name, contact_phone, address, description, "
            "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
            "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at, "
            "facilities "
            "FROM low_altitude_traffic_system.emergency_landing_points WHERE status = 'active'"
        ).execute();

        // 5. 计算距离并排序
        struct LandingPointWithDistance
        {
            json point;
            double distance;
        };

        std::vector<LandingPointWithDistance> points_with_distance;

        for (auto row : result)
        {
            double point_lat = row[4].get<double>();
            double point_lng = row[5].get<double>();
            double distance = calculateDistance(current_lat, current_lng, point_lat, point_lng);

            // 只保留在搜索半径内的降落点
            if (distance <= radius)
            {
                json point = {
                    {"id", row[0].get<int64_t>()},
                    {"point_code", row[1].get<std::string>()},
                    {"name", row[2].get<std::string>()},
                    {"type", row[3].get<std::string>()},
                    {"lat", point_lat},
                    {"lng", point_lng},
                    {"capacity", row[7].get<int>()},
                    {"distance", distance}};

                // 可选字段
                if (!row[6].isNull())
                    point["altitude"] = row[6].get<double>();
                if (!row[9].isNull())
                    point["safety_rating"] = row[9].get<std::string>();
                if (!row[10].isNull())
                    point["weather_protected"] = row[10].get<bool>();
                if (!row[11].isNull())
                    point["accessible_24h"] = row[11].get<bool>();

                points_with_distance.push_back({point, distance});
            }
        }

        // 按距离排序
        std::sort(points_with_distance.begin(), points_with_distance.end(),
                  [](const LandingPointWithDistance &a, const LandingPointWithDistance &b)
                  {
                      return a.distance < b.distance;
                  });

        // 限制返回数量
        json landing_points = json::array();
        int count = 0;
        for (const auto &item : points_with_distance)
        {
            if (count >= limit)
                break;
            landing_points.push_back(item.point);
            count++;
        }

        json responseData = {{"landing_points", landing_points}};
        return utils::HttpResponse::createSuccessResponse(responseData, "查找最近的紧急降落点成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "查找最近的紧急降落点失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("查找最近的紧急降落点失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> EmergencyLandingController::updateEmergencyLandingPoint(
    const http::request<http::string_body> &req,
    const std::string &point_id)
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
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有管理员可以更新紧急降落点");
        }

        // 3. 解析请求体 (允许宽松的UTF-8解析)
        json requestData = json::parse(req.body(), nullptr, true, true);

        // 4. 更新数据库
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        auto table = schema.getTable("emergency_landing_points");

        auto update_op = table.update();

        // Add all .set() calls first, before .where().bind()
        if (requestData.contains("name"))
        {
            update_op = update_op.set("name", requestData["name"].get<std::string>());
        }
        if (requestData.contains("status"))
        {
            update_op = update_op.set("status", requestData["status"].get<std::string>());
        }
        if (requestData.contains("capacity"))
        {
            update_op = update_op.set("capacity", requestData["capacity"].get<int>());
        }
        if (requestData.contains("safety_rating"))
        {
            update_op = update_op.set("safety_rating", requestData["safety_rating"].get<std::string>());
        }

        // Then call .where().bind() and execute
        auto result = update_op.where("id = :id").bind("id", std::stoll(point_id)).execute();

        if (result.getAffectedItemsCount() == 0)
        {
            return utils::HttpResponse::createNotFoundResponse("紧急降落点不存在");
        }

        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "紧急降落点更新成功");
    }
    catch (const json::parse_error &e)
    {
        return utils::HttpResponse::createErrorResponse("JSON解析失败: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "更新紧急降落点失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("更新紧急降落点失败: " + std::string(e.what()));
    }
}

// ========== 辅助方法实现 ==========

std::string EmergencyLandingController::extractBearerToken(const http::request<http::string_body> &req)
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

int64_t EmergencyLandingController::validateTokenAndGetUserId(const http::request<http::string_body> &req)
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

bool EmergencyLandingController::isAdmin(int64_t user_id)
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

std::string EmergencyLandingController::generatePointCode()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm timeinfo;
    localtime_s(&timeinfo, &time);  // 使用 localtime_s 代替 localtime

    std::stringstream ss;
    ss << "ELP" << std::put_time(&timeinfo, "%Y%m%d%H%M%S")
       << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::map<std::string, std::string> EmergencyLandingController::parseQueryParams(
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

double EmergencyLandingController::calculateDistance(double lat1, double lng1, double lat2, double lng2)
{
    // Haversine公式计算两点间距离
    const double R = 6371000.0; // 地球半径（米）
    const double PI = 3.14159265358979323846;

    double lat1_rad = lat1 * PI / 180.0;
    double lat2_rad = lat2 * PI / 180.0;
    double delta_lat = (lat2 - lat1) * PI / 180.0;
    double delta_lng = (lng2 - lng1) * PI / 180.0;

    double a = std::sin(delta_lat / 2) * std::sin(delta_lat / 2) +
               std::cos(lat1_rad) * std::cos(lat2_rad) *
                   std::sin(delta_lng / 2) * std::sin(delta_lng / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return R * c;
}
