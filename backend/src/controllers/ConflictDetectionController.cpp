#include "ConflictDetectionController.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace controllers;
using json = nlohmann::json;

ConflictDetectionController::ConflictDetectionController(
    std::shared_ptr<mysqlx::Session> dbSession,
    std::shared_ptr<auth::JwtService> jwtService)
    : dbSession_(dbSession), jwtService_(jwtService)
{
}

// ========== 飞行注册与冲突检测API接口实现 ==========

http::response<http::string_body> ConflictDetectionController::registerFlight(
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

        // 2. 解析请求体 (允许宽松的UTF-8解析)
        json requestData = json::parse(req.body(), nullptr, true, true);

        // 3. 验证必填字段
        if (!requestData.contains("task_id") || !requestData.contains("start_time") ||
            !requestData.contains("end_time") || !requestData.contains("route") ||
            !requestData.contains("altitude"))
        {
            return utils::HttpResponse::createErrorResponse("缺少必填字段: task_id, start_time, end_time, route, altitude");
        }

        int64_t task_id = requestData["task_id"].get<int64_t>();
        std::string start_time = requestData["start_time"].get<std::string>();
        std::string end_time = requestData["end_time"].get<std::string>();
        json route = requestData["route"];
        double altitude = requestData["altitude"].get<double>();

        // 4. 执行冲突检测
        auto conflict_ids = detectConflicts(task_id, start_time, end_time, route, altitude);

        // 5. 构建响应数据
        json responseData = {
            {"flight_id", task_id},
            {"conflicts", json::array()}};

        if (!conflict_ids.empty())
        {
            // 查询冲突详情
            auto schema = dbSession_->getSchema("low_altitude_traffic_system");
            for (int64_t conflict_id : conflict_ids)
            {
                auto result = dbSession_->sql("SELECT * FROM low_altitude_traffic_system.flight_conflicts WHERE id = ?")
                                  .bind(conflict_id)
                                  .execute();

                for (auto row : result)
                {
                    json conflict = {
                        {"conflict_id", row[0].get<int64_t>()},
                        {"conflict_type", row[4].get<std::string>()},
                        {"severity", row[5].get<std::string>()},
                        {"conflicting_task_id", row[2].get<int64_t>() == task_id ? row[3].get<int64_t>() : row[2].get<int64_t>()}};

                    responseData["conflicts"].push_back(conflict);
                }
            }
        }

        std::string message = conflict_ids.empty() ? "飞行注册成功，无冲突" : "飞行注册成功，检测到 " + std::to_string(conflict_ids.size()) + " 个冲突";
        return utils::HttpResponse::createSuccessResponse(responseData, message);
    }
    catch (const json::parse_error &e)
    {
        return utils::HttpResponse::createErrorResponse("JSON解析失败: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "飞行注册失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("飞行注册失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> ConflictDetectionController::getFlightConflicts(
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

        // 3. 构建查询SQL
        std::string sql = "SELECT * FROM low_altitude_traffic_system.flight_conflicts WHERE 1=1";

        // 添加状态过滤
        if (queryParams.count("status"))
        {
            sql += " AND status = :status";
        }

        // 添加严重程度过滤
        if (queryParams.count("severity"))
        {
            sql += " AND severity = :severity";
        }

        // 添加任务ID过滤
        if (queryParams.count("task_id"))
        {
            sql += " AND (task_id_1 = :task_id OR task_id_2 = :task_id)";
        }

        sql += " ORDER BY conflict_time DESC";

        // 4. 执行查询
        auto session_sql = dbSession_->sql(sql);

        // 绑定参数
        if (queryParams.count("status"))
        {
            session_sql.bind("status", queryParams["status"]);
        }
        if (queryParams.count("severity"))
        {
            session_sql.bind("severity", queryParams["severity"]);
        }
        if (queryParams.count("task_id"))
        {
            session_sql.bind("task_id", std::stoll(queryParams["task_id"]));
        }

        auto result = session_sql.execute();

        // 5. 构建响应数据
        json conflicts = json::array();
        for (auto row : result)
        {
            json conflict = {
                {"id", row[0].get<int64_t>()},
                {"conflict_code", row[1].get<std::string>()},
                {"task_id_1", row[2].get<int64_t>()},
                {"task_id_2", row[3].get<int64_t>()},
                {"conflict_type", row[4].get<std::string>()},
                {"severity", row[5].get<std::string>()},
                {"status", row[6].get<std::string>()}};

            // 可选字段
            if (!row[7].isNull())
                conflict["conflict_time"] = row[7].get<std::string>();
            if (!row[9].isNull())
                conflict["distance"] = row[9].get<double>();
            if (!row[10].isNull())
                conflict["time_diff"] = row[10].get<int>();

            conflicts.push_back(conflict);
        }

        json responseData = {{"conflicts", conflicts}};
        return utils::HttpResponse::createSuccessResponse(responseData, "获取飞行冲突列表成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取飞行冲突列表失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取飞行冲突列表失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> ConflictDetectionController::resolveConflict(
    const http::request<http::string_body> &req,
    const std::string &conflict_id)
{
    try
    {
        // 1. 验证JWT Token
        int64_t user_id = validateTokenAndGetUserId(req);
        if (user_id == -1)
        {
            return utils::HttpResponse::createUnauthorizedResponse("无效的认证Token");
        }

        // 2. 解析请求体（可选）
        std::string resolution_notes = "";
        try
        {
            json requestData = json::parse(req.body(), nullptr, true, true);
            if (requestData.contains("resolution_notes"))
            {
                resolution_notes = requestData["resolution_notes"].get<std::string>();
            }
        }
        catch (...)
        {
            // 请求体为空或格式错误时忽略
        }

        // 3. 更新冲突状态 - 使用SQL API以支持NOW()函数
        auto result = dbSession_->sql(
            "UPDATE flight_conflicts SET status = ?, resolved_by = ?, resolved_at = NOW(), resolution_notes = ?, updated_at = NOW() WHERE id = ?"
        )
        .bind("resolved")
        .bind(user_id)
        .bind(resolution_notes)
        .bind(std::stoll(conflict_id))
        .execute();

        if (result.getAffectedItemsCount() == 0)
        {
            return utils::HttpResponse::createNotFoundResponse("飞行冲突不存在");
        }

        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "飞行冲突已解决");
    }
    catch (const std::exception &e)
    {
        std::cerr << "解决飞行冲突失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("解决飞行冲突失败: " + std::string(e.what()));
    }
}

// ========== 辅助方法实现 ==========

std::string ConflictDetectionController::extractBearerToken(const http::request<http::string_body> &req)
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

int64_t ConflictDetectionController::validateTokenAndGetUserId(const http::request<http::string_body> &req)
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

std::string ConflictDetectionController::generateConflictCode()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm timeinfo;
    localtime_s(&timeinfo, &time);  // 使用 localtime_s 代替 localtime

    std::stringstream ss;
    ss << "FC" << std::put_time(&timeinfo, "%Y%m%d%H%M%S")
       << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::map<std::string, std::string> ConflictDetectionController::parseQueryParams(
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

std::vector<int64_t> ConflictDetectionController::detectConflicts(
    int64_t task_id,
    const std::string &start_time,
    const std::string &end_time,
    const nlohmann::json &route,
    double altitude [[maybe_unused]])  // 标记为未使用，但保留以备将来扩展
{
    std::vector<int64_t> conflict_ids;

    try
    {
        // 查询同一时间段内的其他飞行任务
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        std::string sql = R"(
            SELECT id, route_data, altitude
            FROM flight_tasks
            WHERE id != ?
            AND status = 'ongoing'
            AND (
                (scheduled_time BETWEEN ? AND ?) OR
                (actual_start_time BETWEEN ? AND ?) OR
                (? BETWEEN scheduled_time AND actual_end_time)
            )
        )";

        auto result = dbSession_->sql(sql)
                          .bind(task_id)
                          .bind(start_time)
                          .bind(end_time)
                          .bind(start_time)
                          .bind(end_time)
                          .bind(start_time)
                          .execute();

        // 提取当前任务的起点坐标
        double lat1 = route["start"]["lat"].get<double>();
        double lng1 = route["start"]["lng"].get<double>();
        // 注意：当前简化版只比较起点距离，未来可扩展使用终点坐标和高度进行更精确的冲突检测

        // 检查与其他任务的冲突
        for (auto row : result)
        {
            int64_t other_task_id = row[0].get<int64_t>();

            // 空间冲突检测（简化版：比较起点距离）
            try
            {
                std::string route_str = row[1].get<std::string>();
                json other_route = json::parse(route_str);

                double other_lat1 = other_route["start"]["lat"].get<double>();
                double other_lng1 = other_route["start"]["lng"].get<double>();
                // 注意：当前简化版只比较起点距离

                double distance = calculateDistance(lat1, lng1, other_lat1, other_lng1);

                // 如果距离小于500米，视为空间冲突
                if (distance < 500.0)
                {
                    // 记录冲突 - 使用SQL方式插入以支持NOW()函数
                    std::string conflict_code = generateConflictCode();

                    auto insert_result = dbSession_->sql(
                        "INSERT INTO flight_conflicts (conflict_code, task_id_1, task_id_2, "
                        "conflict_type, severity, status, conflict_time, distance, created_at, updated_at) "
                        "VALUES (?, ?, ?, ?, ?, ?, NOW(), ?, NOW(), NOW())"
                    )
                    .bind(conflict_code)
                    .bind(task_id)
                    .bind(other_task_id)
                    .bind("space")
                    .bind("high")
                    .bind("active")
                    .bind(distance)
                    .execute();

                    conflict_ids.push_back(insert_result.getAutoIncrementValue());
                }
            }
            catch (...)
            {
                // 忽略解析错误，继续下一个任务
                continue;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "冲突检测失败: " << e.what() << std::endl;
    }

    return conflict_ids;
}

double ConflictDetectionController::calculateDistance(double lat1, double lng1, double lat2, double lng2)
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
