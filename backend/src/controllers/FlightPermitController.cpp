#include "FlightPermitController.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

using namespace controllers;
using json = nlohmann::json;

FlightPermitController::FlightPermitController(
    std::shared_ptr<mysqlx::Session> dbSession,
    std::shared_ptr<auth::JwtService> jwtService)
    : dbSession_(dbSession), jwtService_(jwtService)
{
}

// ========== 飞行许可API接口实现 ==========

http::response<http::string_body> FlightPermitController::applyFlightPermit(
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
        if (!requestData.contains("task_id") || !requestData.contains("airspace_id") ||
            !requestData.contains("start_time") || !requestData.contains("end_time"))
        {
            return utils::HttpResponse::createErrorResponse("缺少必填字段: task_id, airspace_id, start_time, end_time");
        }

        // 4. 生成许可编号
        std::string permit_code = generatePermitCode();

        // 5. 插入数据库
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        auto table = schema.getTable("flight_permits");

        auto result = table.insert("permit_code", "task_id", "airspace_id", "applicant_id",
                                    "start_time", "end_time", "remarks", "status")
                          .values(permit_code,
                                  requestData["task_id"].get<int64_t>(),
                                  requestData["airspace_id"].get<int64_t>(),
                                  user_id,
                                  requestData["start_time"].get<std::string>(),
                                  requestData["end_time"].get<std::string>(),
                                  requestData.value("remarks", ""),
                                  "pending")
                          .execute();

        int64_t permit_id = result.getAutoIncrementValue();

        // 6. 返回成功响应
        json responseData = {
            {"permit_id", permit_id},
            {"permit_code", permit_code},
            {"status", "pending"}};

        return utils::HttpResponse::createSuccessResponse(responseData, "飞行许可申请成功");
    }
    catch (const json::parse_error &e)
    {
        return utils::HttpResponse::createErrorResponse("JSON解析失败: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "申请飞行许可失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("申请飞行许可失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> FlightPermitController::getFlightPermits(
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
        std::string sql = "SELECT id, permit_code, task_id, airspace_id, applicant_id, approver_id, "
                          "status, "
                          "DATE_FORMAT(application_time, '%Y-%m-%d %H:%i:%s') as application_time, "
                          "DATE_FORMAT(approval_time, '%Y-%m-%d %H:%i:%s') as approval_time, "
                          "DATE_FORMAT(start_time, '%Y-%m-%d %H:%i:%s') as start_time, "
                          "DATE_FORMAT(end_time, '%Y-%m-%d %H:%i:%s') as end_time, "
                          "remarks "
                          "FROM low_altitude_traffic_system.flight_permits WHERE 1=1";

        // 添加状态过滤
        if (queryParams.count("status"))
        {
            sql += " AND status = :status";
        }

        // 添加申请人过滤
        if (queryParams.count("applicant_id"))
        {
            sql += " AND applicant_id = :applicant_id";
        }

        sql += " ORDER BY application_time DESC";

        // 4. 执行查询
        auto schema = dbSession_->getSchema("low_altitude_traffic_system");
        auto session_sql = dbSession_->sql(sql);

        // 绑定参数
        if (queryParams.count("status"))
        {
            session_sql.bind("status", queryParams["status"]);
        }
        if (queryParams.count("applicant_id"))
        {
            session_sql.bind("applicant_id", std::stoll(queryParams["applicant_id"]));
        }

        auto result = session_sql.execute();

        // 5. 构建响应数据
        json permits = json::array();
        for (auto row : result)
        {
            try {
                json permit;

                // 逐个字段添加，并记录日志
                std::cerr << "Processing permit record..." << std::endl;

                permit["id"] = row[0].get<int64_t>();
                std::cerr << "  id: " << row[0].get<int64_t>() << std::endl;

                std::string permit_code = row[1].get<std::string>();
                std::cerr << "  permit_code: " << permit_code << std::endl;
                permit["permit_code"] = permit_code;

                permit["task_id"] = row[2].get<int64_t>();
                std::cerr << "  task_id: " << row[2].get<int64_t>() << std::endl;

                permit["airspace_id"] = row[3].get<int64_t>();
                permit["applicant_id"] = row[4].get<int64_t>();

                std::string status = row[6].get<std::string>();
                std::cerr << "  status: " << status << std::endl;
                permit["status"] = status;

                // 可选字段
                if (!row[5].isNull()) {
                    permit["approver_id"] = row[5].get<int64_t>();
                }

                // DATETIME字段已通过DATE_FORMAT转换为字符串
                if (!row[7].isNull()) {
                    permit["application_time"] = row[7].get<std::string>();
                }

                if (!row[8].isNull()) {
                    permit["approval_time"] = row[8].get<std::string>();
                }

                if (!row[9].isNull()) {
                    permit["start_time"] = row[9].get<std::string>();
                }

                if (!row[10].isNull()) {
                    permit["end_time"] = row[10].get<std::string>();
                }

                if (!row[11].isNull()) {
                    std::string remarks = row[11].get<std::string>();
                    std::cerr << "  remarks: " << remarks << std::endl;
                    permit["remarks"] = remarks;
                }

                std::cerr << "  Record processed successfully" << std::endl;
                permits.push_back(permit);
            } catch (const std::exception& e) {
                std::cerr << "Error processing permit record: " << e.what() << std::endl;
                throw;
            }
        }

        json responseData = {{"permits", permits}};
        return utils::HttpResponse::createSuccessResponse(responseData, "获取飞行许可列表成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取飞行许可列表失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取飞行许可列表失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> FlightPermitController::approveFlightPermit(
    const http::request<http::string_body> &req,
    const std::string &permit_id)
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
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有管理员可以审批飞行许可");
        }

        // 3. 更新许可状态 - 使用SQL API以支持NOW()函数
        auto result = dbSession_->sql(
            "UPDATE flight_permits SET status = ?, approver_id = ?, approval_time = NOW(), updated_at = NOW() WHERE id = ?"
        )
        .bind("approved")
        .bind(user_id)
        .bind(std::stoll(permit_id))
        .execute();

        if (result.getAffectedItemsCount() == 0)
        {
            return utils::HttpResponse::createNotFoundResponse("飞行许可不存在");
        }

        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "飞行许可审批通过");
    }
    catch (const std::exception &e)
    {
        std::cerr << "审批飞行许可失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("审批飞行许可失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> FlightPermitController::rejectFlightPermit(
    const http::request<http::string_body> &req,
    const std::string &permit_id)
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
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有管理员可以拒绝飞行许可");
        }

        // 3. 更新许可状态 - 使用SQL API以支持NOW()函数
        auto result = dbSession_->sql(
            "UPDATE flight_permits SET status = ?, approver_id = ?, approval_time = NOW(), updated_at = NOW() WHERE id = ?"
        )
        .bind("rejected")
        .bind(user_id)
        .bind(std::stoll(permit_id))
        .execute();

        if (result.getAffectedItemsCount() == 0)
        {
            return utils::HttpResponse::createNotFoundResponse("飞行许可不存在");
        }

        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "飞行许可已拒绝");
    }
    catch (const std::exception &e)
    {
        std::cerr << "拒绝飞行许可失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("拒绝飞行许可失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> FlightPermitController::getFlightPermitById(
    const http::request<http::string_body> &req,
    const std::string &permit_id)
{
    try
    {
        // 1. 验证JWT Token
        int64_t user_id = validateTokenAndGetUserId(req);
        if (user_id == -1)
        {
            return utils::HttpResponse::createUnauthorizedResponse("无效的认证Token");
        }

        // 2. 查询飞行许可详情
        std::string sql = "SELECT id, permit_code, task_id, airspace_id, applicant_id, approver_id, "
                          "status, "
                          "DATE_FORMAT(application_time, '%Y-%m-%d %H:%i:%s') as application_time, "
                          "DATE_FORMAT(approval_time, '%Y-%m-%d %H:%i:%s') as approval_time, "
                          "DATE_FORMAT(start_time, '%Y-%m-%d %H:%i:%s') as start_time, "
                          "DATE_FORMAT(end_time, '%Y-%m-%d %H:%i:%s') as end_time, "
                          "remarks "
                          "FROM low_altitude_traffic_system.flight_permits WHERE id = ?";

        auto result = dbSession_->sql(sql).bind(std::stoll(permit_id)).execute();

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
            return utils::HttpResponse::createNotFoundResponse("飞行许可不存在");
        }

        // 4. 构建响应数据
        json permit;

        permit["id"] = row[0].get<int64_t>();
        permit["permit_code"] = row[1].get<std::string>();
        permit["task_id"] = row[2].get<int64_t>();
        permit["airspace_id"] = row[3].get<int64_t>();
        permit["applicant_id"] = row[4].get<int64_t>();

        // 可选字段
        if (!row[5].isNull())
        {
            permit["approver_id"] = row[5].get<int64_t>();
        }

        permit["status"] = row[6].get<std::string>();

        // DATETIME字段已通过DATE_FORMAT转换为字���串
        if (!row[7].isNull())
        {
            permit["application_time"] = row[7].get<std::string>();
        }

        if (!row[8].isNull())
        {
            permit["approval_time"] = row[8].get<std::string>();
        }

        if (!row[9].isNull())
        {
            permit["start_time"] = row[9].get<std::string>();
        }

        if (!row[10].isNull())
        {
            permit["end_time"] = row[10].get<std::string>();
        }

        if (!row[11].isNull())
        {
            permit["remarks"] = row[11].get<std::string>();
        }

        return utils::HttpResponse::createSuccessResponse(permit, "获取飞行许可详情成功");
    }
    catch (const std::exception &e)
    {
        std::cerr << "获取飞行许可详情失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取飞行许可详情失败: " + std::string(e.what()));
    }
}

http::response<http::string_body> FlightPermitController::revokeFlightPermit(
    const http::request<http::string_body> &req,
    const std::string &permit_id)
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
        std::string reason = "";
        try
        {
            json requestData = json::parse(req.body(), nullptr, true, true);
            if (requestData.contains("reason"))
            {
                reason = requestData["reason"].get<std::string>();
            }
        }
        catch (...)
        {
            // 请求体为空或格式错误时忽略
        }

        // 3. 检查许可是否存在且属于当前用户或用户是管理员
        auto check_result = dbSession_->sql(
            "SELECT applicant_id, status FROM flight_permits WHERE id = ?"
        )
        .bind(std::stoll(permit_id))
        .execute();

        bool found = false;
        int64_t applicant_id = 0;
        std::string current_status;

        for (auto row : check_result)
        {
            applicant_id = row[0].get<int64_t>();
            current_status = row[1].get<std::string>();
            found = true;
            break;
        }

        if (!found)
        {
            return utils::HttpResponse::createNotFoundResponse("飞行许可不存在");
        }

        // 检查权限：只有申请人本人或管理员可以撤销
        if (applicant_id != user_id && !isAdmin(user_id))
        {
            return utils::HttpResponse::createForbiddenResponse("权限不足，只有申请人或管理员可以撤销飞行许可");
        }

        // 检查状态：已经被拒绝或已撤销的许可不能再撤销
        if (current_status == "rejected" || current_status == "revoked")
        {
            return utils::HttpResponse::createErrorResponse("该飞行许可已经被拒绝或撤销��无法再次撤销");
        }

        // 4. 更新许可状态为revoked
        std::string update_sql = "UPDATE flight_permits SET status = ?, updated_at = NOW()";
        if (!reason.empty())
        {
            update_sql += ", remarks = CONCAT(IFNULL(remarks, ''), '\n撤销原因: ', ?)";
        }
        update_sql += " WHERE id = ?";

        auto update_stmt = dbSession_->sql(update_sql).bind("revoked");
        if (!reason.empty())
        {
            update_stmt.bind(reason);
        }
        auto result = update_stmt.bind(std::stoll(permit_id)).execute();

        std::cout << "飞行许可撤销成功: ID=" << permit_id << ", User=" << user_id << std::endl;
        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "飞行许可已撤销");
    }
    catch (const std::exception &e)
    {
        std::cerr << "撤销飞行许可失败: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("撤销飞行许可失败: " + std::string(e.what()));
    }
}

// ========== 辅助方法实现 ==========

std::string FlightPermitController::extractBearerToken(const http::request<http::string_body> &req)
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

int64_t FlightPermitController::validateTokenAndGetUserId(const http::request<http::string_body> &req)
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

bool FlightPermitController::isAdmin(int64_t user_id)
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

std::string FlightPermitController::generatePermitCode()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm timeinfo;
    localtime_s(&timeinfo, &time);  // 使用 localtime_s 代替 localtime

    std::stringstream ss;
    ss << "FP" << std::put_time(&timeinfo, "%Y%m%d%H%M%S")
       << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::map<std::string, std::string> FlightPermitController::parseQueryParams(
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
