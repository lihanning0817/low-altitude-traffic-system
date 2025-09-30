#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <regex>

#include "auth/JwtService.h"
#include "repositories/UserRepository.h"
#include "models/User.h"
#include "utils/HttpResponse.h"

namespace beast = boost::beast;
namespace http = beast::http;

namespace controllers {

/**
 * @brief 用户认证控制器
 *
 * 处理用户认证相关的HTTP请求，包括：
 * - 用户登录（生成JWT Token）
 * - 用户注册（创建新用户账户）
 * - Token刷新（延长会话时间）
 * - 用户信息获取（通过Token）
 * - 密码修改
 * - 登出（Token失效处理）
 */
class AuthController {
private:
    std::shared_ptr<repositories::UserRepository> userRepository_;
    std::shared_ptr<auth::JwtService> jwtService_;

    // 登录失败计数器（用户名 -> {失败次数, 锁定时间}）
    static std::unordered_map<std::string, std::pair<int, std::chrono::steady_clock::time_point>> failedLoginAttempts_;

public:
    /**
     * @brief 构造函数
     * @param userRepository 用户仓储对象
     * @param jwtService JWT服务对象
     */
    AuthController(
        std::shared_ptr<repositories::UserRepository> userRepository,
        std::shared_ptr<auth::JwtService> jwtService
    );

    // ========== 认证相关接口 ==========

    /**
     * @brief 用户登录接口
     * POST /api/v1/auth/login
     *
     * 请求体格式：
     * {
     *   "username": "string",    // 用户名或邮箱
     *   "password": "string"     // 密码
     * }
     *
     * 响应格式：
     * {
     *   "success": true,
     *   "message": "登录成功",
     *   "data": {
     *     "access_token": "string",
     *     "refresh_token": "string",
     *     "user": {
     *       "id": 123,
     *       "username": "string",
     *       "email": "string",
     *       "role": "string",
     *       "status": "string"
     *     }
     *   }
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> login(const http::request<http::string_body>& req);

    /**
     * @brief 用户注册接口
     * POST /api/v1/auth/register
     *
     * 请求体格式：
     * {
     *   "username": "string",      // 用户名（3-20字符）
     *   "email": "string",         // 邮箱地址
     *   "password": "string",      // 密码（8-64字符）
     *   "full_name": "string",     // 全名（可选）
     *   "role": "string"           // 角色（可选：admin/operator/user，默认为user）
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> registerUser(const http::request<http::string_body>& req);

    /**
     * @brief Token刷新接口
     * POST /api/v1/auth/refresh
     *
     * 请求体格式：
     * {
     *   "refresh_token": "string"
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> refreshToken(const http::request<http::string_body>& req);

    /**
     * @brief 获取当前用户信息接口
     * GET /api/v1/auth/me
     *
     * 需要在Header中提供Authorization: Bearer <access_token>
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getCurrentUser(const http::request<http::string_body>& req);

    /**
     * @brief 修改密码接口
     * PUT /api/v1/auth/password
     *
     * 请求体格式：
     * {
     *   "old_password": "string",
     *   "new_password": "string"
     * }
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> changePassword(const http::request<http::string_body>& req);

    /**
     * @brief 用户登出接口
     * POST /api/v1/auth/logout
     *
     * 需要在Header中提供Authorization: Bearer <access_token>
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> logout(const http::request<http::string_body>& req);

    // ========== 辅助方法 ==========

    /**
     * @brief 验证登录请求参数
     * @param loginData 解析后的JSON数据
     * @return 验证结果和错误信息
     */
    std::pair<bool, std::string> validateLoginRequest(const nlohmann::json& loginData);

    /**
     * @brief 验证注册请求参数
     * @param registerData 解析后的JSON数据
     * @return 验证结果和错误信息
     */
    std::pair<bool, std::string> validateRegisterRequest(const nlohmann::json& registerData);

    /**
     * @brief 从请求头中提取Bearer Token
     * @param req HTTP请求对象
     * @return Token字符串，如果不存在返回空字符串
     */
    std::string extractBearerToken(const http::request<http::string_body>& req);

    /**
     * @brief 验证密码强度
     * @param password 密码字符串
     * @return 验证结果和错误信息
     */
    std::pair<bool, std::string> validatePasswordStrength(const std::string& password);

    /**
     * @brief 验证邮箱格式
     * @param email 邮箱字符串
     * @return 是否为有效邮箱格式
     */
    bool isValidEmail(const std::string& email);

    /**
     * @brief 验证用户名格式
     * @param username 用户名字符串
     * @return 验证结果和错误信息
     */
    std::pair<bool, std::string> validateUsername(const std::string& username);

private:
    /**
     * @brief 创建用户信息JSON对象（不包含敏感信息）
     * @param user 用户对象
     * @return 用户信息JSON
     */
    nlohmann::json createUserInfoJson(const models::User& user);

    /**
     * @brief 记录登录尝试（用于安全审计）
     * @param username 用户名
     * @param success 是否成功
     * @param ip_address 客户端IP地址
     */
    void logLoginAttempt(const std::string& username, bool success, const std::string& ip_address);

    /**
     * @brief 检查用户是否被锁定（防暴力破解）
     * @param username 用户名
     * @return 是否被锁定
     */
    bool isUserLocked(const std::string& username);

    /**
     * @brief 增加失败登录计数
     * @param username 用户名
     */
    void incrementFailedLoginCount(const std::string& username);

    /**
     * @brief 重置失败登录计数
     * @param username 用户名
     */
    void resetFailedLoginCount(const std::string& username);
};

} // namespace controllers