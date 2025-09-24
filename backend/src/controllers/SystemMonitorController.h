#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <chrono>

#include "auth/JwtService.h"
#include "repositories/UserRepository.h"
#include "database/DatabaseManager.h"
#include "utils/HttpResponse.h"

namespace beast = boost::beast;
namespace http = beast::http;

namespace controllers {

/**
 * @brief 系统监控控制器
 *
 * 处理系统监控相关的HTTP请求，包括：
 * - 系统健康检查（无需认证）
 * - 系统信息获取（无需认证）
 * - 无人机列表查询（需要认证）
 * - 用户列表查询（需要管理员权限）
 */
class SystemMonitorController {
private:
    std::shared_ptr<auth::JwtService> jwtService_;
    std::shared_ptr<repositories::UserRepository> userRepository_;

    // 系统启动时间
    static std::chrono::steady_clock::time_point serverStartTime_;

public:
    /**
     * @brief 构造函数
     * @param jwtService JWT服务对象
     * @param userRepository 用户仓储对象
     */
    SystemMonitorController(
        std::shared_ptr<auth::JwtService> jwtService,
        std::shared_ptr<repositories::UserRepository> userRepository
    );

    /**
     * @brief 初始化服务器启动时间
     */
    static void initializeStartTime();

    // ========== 监控相关接口 ==========

    /**
     * @brief 系统健康检查接口
     * GET /api/v1/health
     *
     * 无需认证，返回系统基本状态
     *
     * 响应格式：
     * {
     *   "status": "ok",
     *   "timestamp": "datetime",
     *   "services": {
     *     "database": "ok|error",
     *     "external_apis": "ok|error"
     *   }
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> healthCheck(const http::request<http::string_body>& req);

    /**
     * @brief 获取系统信息接口
     * GET /api/v1/info
     *
     * 无需认证，返回系统基本信息
     *
     * 响应格式：
     * {
     *   "success": true,
     *   "data": {
     *     "name": "系统名称",
     *     "version": "版本号",
     *     "start_time": "启动时间",
     *     "uptime_seconds": 运行时间（秒）,
     *     "environment": "环境标识"
     *   }
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getSystemInfo(const http::request<http::string_body>& req);

    /**
     * @brief 获取无人机列表接口
     * GET /api/v1/drones
     *
     * 需要用户认证，返回无人机状态列表
     *
     * 响应格式：
     * {
     *   "success": true,
     *   "data": {
     *     "drones": [
     *       {
     *         "id": "number",
     *         "drone_id": "string",
     *         "name": "string",
     *         "model": "string",
     *         "status": "active|inactive|maintenance",
     *         "battery": "number",
     *         "location": {"lat": number, "lng": number}
     *       }
     *     ],
     *     "total": "number",
     *     "active_count": "number"
     *   }
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getDronesList(const http::request<http::string_body>& req);

    /**
     * @brief 获取用户列表接口（管理员权限）
     * GET /api/v1/users
     *
     * 需要管理员认证，返回系统用户列表
     * 查询参数：
     * - offset: 偏移量（默认0）
     * - limit: 限制数量（默认50，最大100）
     * - role: 角色筛选（可选）
     *
     * 响应格式：
     * {
     *   "success": true,
     *   "data": {
     *     "users": [
     *       {
     *         "id": "number",
     *         "username": "string",
     *         "email": "string",
     *         "full_name": "string",
     *         "role": "string",
     *         "status": "string",
     *         "created_at": "datetime",
     *         "last_login": "datetime"
     *       }
     *     ],
     *     "pagination": {
     *       "total": "number",
     *       "offset": "number",
     *       "limit": "number",
     *       "count": "number"
     *     }
     *   }
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getUsersList(const http::request<http::string_body>& req);

    // ========== 辅助方法 ==========

    /**
     * @brief 从请求头中提取Bearer Token
     * @param req HTTP请求对象
     * @return Token字符串，如果不存在返回空字符串
     */
    std::string extractBearerToken(const http::request<http::string_body>& req);

    /**
     * @brief 验证用户权限
     * @param req HTTP请求对象
     * @param required_role 所需角色（可选，默认为任何认证用户）
     * @return {是否有权限, 用户信息JSON}
     */
    std::pair<bool, nlohmann::json> validateUserPermission(
        const http::request<http::string_body>& req,
        const std::string& required_role = ""
    );

    /**
     * @brief 检查数据库连接状态
     * @return 数据库状态字符串
     */
    std::string checkDatabaseStatus();

    /**
     * @brief 检查外部API状态
     * @return 外部API状态字符串
     */
    std::string checkExternalApiStatus();

    /**
     * @brief 解析查询参数
     * @param target 请求目标路径
     * @return 参数映射
     */
    std::unordered_map<std::string, std::string> parseQueryParams(const std::string& target);

private:
    /**
     * @brief 获取Mock无人机数据
     * @return 无人机数据JSON数组
     */
    nlohmann::json getMockDroneData();

    /**
     * @brief 计算系统运行时间
     * @return 运行时间（秒）
     */
    std::chrono::seconds getSystemUptime();
};

} // namespace controllers