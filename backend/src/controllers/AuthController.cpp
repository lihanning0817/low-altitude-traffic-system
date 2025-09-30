#include "AuthController.h"
#include "utils/HttpResponse.h"
#include <spdlog/spdlog.h>
#include <regex>
#include <unordered_map>
#include <chrono>

namespace controllers {

// 用于跟踪登录失败次数的静态映射（实际项目中应使用Redis等缓存）
static std::unordered_map<std::string, int> failedLoginCounts;
static std::unordered_map<std::string, std::chrono::system_clock::time_point> lockoutTimes;

AuthController::AuthController(
    std::shared_ptr<repositories::UserRepository> userRepository,
    std::shared_ptr<auth::JwtService> jwtService)
    : userRepository_(userRepository), jwtService_(jwtService) {

    spdlog::info("AuthController已初始化");
}

// ========== 认证相关接口实现 ==========

http::response<http::string_body> AuthController::login(const http::request<http::string_body>& req) {
    try {
        // 解析请求体JSON
        nlohmann::json loginData;
        try {
            loginData = nlohmann::json::parse(req.body());
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::warn("登录请求JSON解析失败: {}", e.what());
            return utils::HttpResponse::createErrorResponse("请求格式错误", 400, "INVALID_JSON");
        }

        // 验证请求参数
        auto [isValid, errorMessage] = validateLoginRequest(loginData);
        if (!isValid) {
            return utils::HttpResponse::createErrorResponse(errorMessage, 400, "VALIDATION_ERROR");
        }

        std::string username = loginData["username"].get<std::string>();
        std::string password = loginData["password"].get<std::string>();
        std::string expectedRole = loginData["role"].get<std::string>();

        // 检查用户是否被锁定
        if (isUserLocked(username)) {
            spdlog::warn("用户 {} 尝试登录但账户被锁定", username);
            return utils::HttpResponse::createErrorResponse("账户已被锁定，请稍后重试", 423, "ACCOUNT_LOCKED");
        }

        // 查找用户
        auto userOpt = userRepository_->getUserByUsername(username);
        if (!userOpt.has_value()) {
            // 尝试用邮箱查找
            userOpt = userRepository_->getUserByEmail(username);
        }

        if (!userOpt.has_value()) {
            logLoginAttempt(username, false, "未知");
            incrementFailedLoginCount(username);
            return utils::HttpResponse::createErrorResponse("用户名或密码错误", 401, "UNAUTHORIZED");
        }

        models::User user = userOpt.value();

        // 验证密码
        if (!models::User::verifyPassword(password, user.getPasswordHash())) {
            logLoginAttempt(username, false, "未知");
            incrementFailedLoginCount(username);
            return utils::HttpResponse::createErrorResponse("用户名或密码错误", 401, "UNAUTHORIZED");
        }

        // 验证用户角色是否匹配
        std::string actualRole = models::User::roleToString(user.getRole());
        if (actualRole != expectedRole) {
            logLoginAttempt(username, false, "未知");
            incrementFailedLoginCount(username);
            spdlog::warn("用户 {} 角色不匹配: 期望 {}, 实际 {}", username, expectedRole, actualRole);
            return utils::HttpResponse::createErrorResponse("身份错误", 401, "ROLE_MISMATCH");
        }

        // 检查用户状态
        if (user.getStatus() != models::UserStatus::ACTIVE) {
            std::string statusStr = models::User::statusToString(user.getStatus());
            spdlog::warn("用户 {} 尝试登录但状态为: {}", username, statusStr);
            return utils::HttpResponse::createErrorResponse("账户状态异常", 403, "ACCOUNT_DISABLED");
        }

        // 生成Token
        std::string accessToken = jwtService_->generateAccessToken(user);
        std::string refreshToken = jwtService_->generateRefreshToken(user);

        if (accessToken.empty() || refreshToken.empty()) {
            spdlog::error("为用户 {} 生成Token失败", username);
            return utils::HttpResponse::createErrorResponse("Token生成失败", 500, "INTERNAL_ERROR");
        }

        // 更新用户最后登录时间
        userRepository_->updateLastLogin(user.getId());

        // 重置失败计数
        resetFailedLoginCount(username);

        // 记录成功登录
        logLoginAttempt(username, true, "未知");

        // 构建响应数据
        nlohmann::json responseData;
        responseData["access_token"] = accessToken;
        responseData["refresh_token"] = refreshToken;
        responseData["user"] = createUserInfoJson(user);

        spdlog::info("用户 {} 登录成功", username);
        return utils::HttpResponse::createSuccessResponse(responseData, "登录成功");

    } catch (const std::exception& e) {
        spdlog::error("登录处理异常: {}", e.what());
        return utils::HttpResponse::createErrorResponse("登录处理失败", 500, "INTERNAL_ERROR");
    }
}

http::response<http::string_body> AuthController::registerUser(const http::request<http::string_body>& req) {
    try {
        // 解析请求体JSON
        nlohmann::json registerData;
        try {
            registerData = nlohmann::json::parse(req.body());
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::warn("注册请求JSON解析失败: {}", e.what());
            return utils::HttpResponse::createErrorResponse("请求格式错误", 400, "INVALID_JSON");
        }

        // 验证请求参数
        auto [isValid, errorMessage] = validateRegisterRequest(registerData);
        if (!isValid) {
            return utils::HttpResponse::createErrorResponse(errorMessage, 400, "VALIDATION_ERROR");
        }

        std::string username = registerData["username"].get<std::string>();
        std::string email = registerData["email"].get<std::string>();
        std::string password = registerData["password"].get<std::string>();
        std::string fullName = registerData.value("full_name", username);

        // 处理角色参数
        std::string roleStr = registerData.value("role", "user"); // 默认为user
        models::UserRole userRole = models::UserRole::USER; // 默认角色

        if (roleStr == "admin") {
            userRole = models::UserRole::ADMIN;
        } else if (roleStr == "operator") {
            userRole = models::UserRole::OPERATOR;
        } else {
            userRole = models::UserRole::USER;
        }

        // 检查用户名是否已存在
        if (userRepository_->getUserByUsername(username).has_value()) {
            return utils::HttpResponse::createErrorResponse("用户名已存在", 409, "USERNAME_EXISTS");
        }

        // 检查邮箱是否已存在
        if (userRepository_->getUserByEmail(email).has_value()) {
            return utils::HttpResponse::createErrorResponse("邮箱已被注册", 409, "EMAIL_EXISTS");
        }

        // 创建新用户
        models::User newUser;
        newUser.setUsername(username);
        newUser.setEmail(email);
        newUser.setFullName(fullName);
        newUser.setRole(userRole); // 使用前端传递的角色
        newUser.setStatus(models::UserStatus::ACTIVE); // 默认激活状态

        // 设置密码哈希
        std::string passwordHash = models::User::hashPassword(password);
        newUser.setPasswordHash(passwordHash);

        // 保存用户到数据库
        int64_t userId = userRepository_->createUser(newUser);
        if (userId <= 0) {
            spdlog::error("创建用户失败: {}", username);
            return utils::HttpResponse::createErrorResponse("用户创建失败", 500, "INTERNAL_ERROR");
        }

        newUser.setId(userId);

        // 构建响应数据
        nlohmann::json responseData;
        responseData["user"] = createUserInfoJson(newUser);

        spdlog::info("用户 {} 注册成功，ID: {}", username, userId);
        return utils::HttpResponse::createSuccessResponse(responseData, "注册成功", 201);

    } catch (const std::exception& e) {
        spdlog::error("注册处理异常: {}", e.what());
        return utils::HttpResponse::createErrorResponse("注册处理失败", 500, "INTERNAL_ERROR");
    }
}

http::response<http::string_body> AuthController::refreshToken(const http::request<http::string_body>& req) {
    try {
        // 解析请求体
        nlohmann::json refreshData;
        try {
            refreshData = nlohmann::json::parse(req.body());
        } catch (const nlohmann::json::parse_error& e) {
            return utils::HttpResponse::createErrorResponse("请求格式错误", 400, "INVALID_JSON");
        }

        if (!refreshData.contains("refresh_token") || !refreshData["refresh_token"].is_string()) {
            return utils::HttpResponse::createErrorResponse("缺少refresh_token参数", 400, "MISSING_REFRESH_TOKEN");
        }

        std::string refreshToken = refreshData["refresh_token"].get<std::string>();

        // 验证refresh token
        auto validationResult = jwtService_->validateToken(refreshToken);
        if (validationResult != auth::JwtValidationResult::VALID) {
            std::string errorMsg = auth::jwtValidationResultToString(validationResult);
            return utils::HttpResponse::createErrorResponse("Refresh token无效: " + errorMsg, 401, "UNAUTHORIZED");
        }

        // 解析token获取用户信息
        auto tokenInfo = jwtService_->parseToken(refreshToken);
        if (!tokenInfo.has_value()) {
            return utils::HttpResponse::createErrorResponse("Token解析失败", 401, "UNAUTHORIZED");
        }

        // 获取用户信息
        auto userOpt = userRepository_->getUserById(tokenInfo->user_id);
        if (!userOpt.has_value()) {
            return utils::HttpResponse::createErrorResponse("用户不存在", 401, "UNAUTHORIZED");
        }

        models::User user = userOpt.value();

        // 检查用户状态
        if (user.getStatus() != models::UserStatus::ACTIVE) {
            return utils::HttpResponse::createErrorResponse("用户账户已禁用", 403, "FORBIDDEN");
        }

        // 生成新的access token
        std::string newAccessToken = jwtService_->generateAccessToken(user);
        if (newAccessToken.empty()) {
            return utils::HttpResponse::createErrorResponse("生成新Token失败", 500, "INTERNAL_ERROR");
        }

        // 构建响应数据
        nlohmann::json responseData;
        responseData["access_token"] = newAccessToken;
        responseData["user"] = createUserInfoJson(user);

        spdlog::info("用户 {} 刷新Token成功", user.getUsername());
        return utils::HttpResponse::createSuccessResponse(responseData, "Token刷新成功");

    } catch (const std::exception& e) {
        spdlog::error("Token刷新异常: {}", e.what());
        return utils::HttpResponse::createErrorResponse("Token刷新失败", 500, "INTERNAL_ERROR");
    }
}

http::response<http::string_body> AuthController::getCurrentUser(const http::request<http::string_body>& req) {
    try {
        // 提取Bearer Token
        std::string token = extractBearerToken(req);
        if (token.empty()) {
            return utils::HttpResponse::createErrorResponse("缺少认证Token", 401, "UNAUTHORIZED");
        }

        // 验证Token
        auto validationResult = jwtService_->validateToken(token);
        if (validationResult != auth::JwtValidationResult::VALID) {
            std::string errorMsg = auth::jwtValidationResultToString(validationResult);
            return utils::HttpResponse::createErrorResponse("Token无效: " + errorMsg, 401, "UNAUTHORIZED");
        }

        // 解析Token获取用户ID
        int64_t userId = jwtService_->extractUserId(token);
        if (userId <= 0) {
            return utils::HttpResponse::createErrorResponse("Token中用户信息无效", 401, "UNAUTHORIZED");
        }

        // 获取用户信息
        auto userOpt = userRepository_->getUserById(userId);
        if (!userOpt.has_value()) {
            return utils::HttpResponse::createErrorResponse("用户不存在", 401, "UNAUTHORIZED");
        }

        models::User user = userOpt.value();

        // 检查用户状态
        if (user.getStatus() != models::UserStatus::ACTIVE) {
            return utils::HttpResponse::createErrorResponse("用户账户已禁用", 403, "FORBIDDEN");
        }

        // 返回用户信息
        nlohmann::json responseData = createUserInfoJson(user);
        return utils::HttpResponse::createSuccessResponse(responseData, "获取用户信息成功");

    } catch (const std::exception& e) {
        spdlog::error("获取当前用户信息异常: {}", e.what());
        return utils::HttpResponse::createErrorResponse("获取用户信息失败", 500, "INTERNAL_ERROR");
    }
}

http::response<http::string_body> AuthController::changePassword(const http::request<http::string_body>& req) {
    try {
        // 提取Bearer Token
        std::string token = extractBearerToken(req);
        if (token.empty()) {
            return utils::HttpResponse::createErrorResponse("缺少认证Token", 401, "UNAUTHORIZED");
        }

        // 验证Token
        auto validationResult = jwtService_->validateToken(token);
        if (validationResult != auth::JwtValidationResult::VALID) {
            return utils::HttpResponse::createErrorResponse("Token无效", 401, "UNAUTHORIZED");
        }

        // 解析请求体
        nlohmann::json passwordData;
        try {
            passwordData = nlohmann::json::parse(req.body());
        } catch (const nlohmann::json::parse_error& e) {
            return utils::HttpResponse::createErrorResponse("请求格式错误", 400, "INVALID_JSON");
        }

        if (!passwordData.contains("old_password") || !passwordData.contains("new_password")) {
            return utils::HttpResponse::createErrorResponse("缺少必要参数", 400, "MISSING_PARAMETERS");
        }

        std::string oldPassword = passwordData["old_password"].get<std::string>();
        std::string newPassword = passwordData["new_password"].get<std::string>();

        // 验证新密码强度
        auto [isValid, errorMessage] = validatePasswordStrength(newPassword);
        if (!isValid) {
            return utils::HttpResponse::createErrorResponse(errorMessage, 400, "WEAK_PASSWORD");
        }

        // 获取当前用户
        int64_t userId = jwtService_->extractUserId(token);
        auto userOpt = userRepository_->getUserById(userId);
        if (!userOpt.has_value()) {
            return utils::HttpResponse::createErrorResponse("用户不存在", 401, "UNAUTHORIZED");
        }

        models::User user = userOpt.value();

        // 验证旧密码
        if (!models::User::verifyPassword(oldPassword, user.getPasswordHash())) {
            return utils::HttpResponse::createErrorResponse("原密码错误", 400, "INVALID_OLD_PASSWORD");
        }

        // 设置新密码哈希
        std::string newPasswordHash = models::User::hashPassword(newPassword);

        // 更新数据库中的密码
        if (!userRepository_->updatePassword(user.getId(), newPassword)) {
            return utils::HttpResponse::createErrorResponse("密码更新失败", 500, "INTERNAL_ERROR");
        }

        spdlog::info("用户 {} 修改密码成功", user.getUsername());
        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "密码修改成功");

    } catch (const std::exception& e) {
        spdlog::error("修改密码异常: {}", e.what());
        return utils::HttpResponse::createErrorResponse("密码修改失败", 500, "INTERNAL_ERROR");
    }
}

http::response<http::string_body> AuthController::logout(const http::request<http::string_body>& req) {
    try {
        // 提取Bearer Token
        std::string token = extractBearerToken(req);
        if (token.empty()) {
            return utils::HttpResponse::createErrorResponse("缺少认证Token", 401, "UNAUTHORIZED");
        }

        // 获取JWT ID（用于黑名单机制，实际项目中需要实现）
        std::string jwtId = jwtService_->extractJwtId(token);

        // TODO: 将JWT ID添加到黑名单缓存中（Redis等）
        // 这里暂时只记录日志
        spdlog::info("用户登出，JWT ID: {}", jwtId);

        return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "登出成功");

    } catch (const std::exception& e) {
        spdlog::error("登出处理异常: {}", e.what());
        return utils::HttpResponse::createErrorResponse("登出处理失败", 500, "INTERNAL_ERROR");
    }
}

// ========== 辅助方法实现 ==========

std::pair<bool, std::string> AuthController::validateLoginRequest(const nlohmann::json& loginData) {
    if (!loginData.contains("username") || !loginData["username"].is_string()) {
        return {false, "缺少用户名参数"};
    }

    if (!loginData.contains("password") || !loginData["password"].is_string()) {
        return {false, "缺少密码参数"};
    }

    if (!loginData.contains("role") || !loginData["role"].is_string()) {
        return {false, "缺少身份参数"};
    }

    std::string username = loginData["username"].get<std::string>();
    std::string password = loginData["password"].get<std::string>();
    std::string role = loginData["role"].get<std::string>();

    if (username.empty()) {
        return {false, "用户名不能为空"};
    }

    if (password.empty()) {
        return {false, "密码不能为空"};
    }

    if (role.empty()) {
        return {false, "身份不能为空"};
    }

    if (role != "admin" && role != "user" && role != "operator") {
        return {false, "身份参数无效"};
    }

    return {true, ""};
}

std::pair<bool, std::string> AuthController::validateRegisterRequest(const nlohmann::json& registerData) {
    // 检查必需字段
    if (!registerData.contains("username") || !registerData["username"].is_string()) {
        return {false, "缺少用户名参数"};
    }

    if (!registerData.contains("email") || !registerData["email"].is_string()) {
        return {false, "缺少邮箱参数"};
    }

    if (!registerData.contains("password") || !registerData["password"].is_string()) {
        return {false, "缺少密码参数"};
    }

    std::string username = registerData["username"].get<std::string>();
    std::string email = registerData["email"].get<std::string>();
    std::string password = registerData["password"].get<std::string>();

    // 验证用户名
    auto [usernameValid, usernameError] = validateUsername(username);
    if (!usernameValid) {
        return {false, usernameError};
    }

    // 验证邮箱
    if (!isValidEmail(email)) {
        return {false, "邮箱格式不正确"};
    }

    // 验证密码强度
    auto [passwordValid, passwordError] = validatePasswordStrength(password);
    if (!passwordValid) {
        return {false, passwordError};
    }

    return {true, ""};
}

std::string AuthController::extractBearerToken(const http::request<http::string_body>& req) {
    auto authHeader = req.find(http::field::authorization);
    if (authHeader == req.end()) {
        return "";
    }

    std::string authValue = authHeader->value();
    const std::string bearerPrefix = "Bearer ";

    if (authValue.length() <= bearerPrefix.length() ||
        authValue.substr(0, bearerPrefix.length()) != bearerPrefix) {
        return "";
    }

    return authValue.substr(bearerPrefix.length());
}

std::pair<bool, std::string> AuthController::validatePasswordStrength(const std::string& password) {
    if (password.length() < 8) {
        return {false, "密码长度不能少于8位"};
    }

    if (password.length() > 64) {
        return {false, "密码长度不能超过64位"};
    }

    // 检查是否包含字母
    bool hasLetter = std::any_of(password.begin(), password.end(), ::isalpha);
    if (!hasLetter) {
        return {false, "密码必须包含至少一个字母"};
    }

    // 检查是否包含数字
    bool hasDigit = std::any_of(password.begin(), password.end(), ::isdigit);
    if (!hasDigit) {
        return {false, "密码必须包含至少一个数字"};
    }

    return {true, ""};
}

bool AuthController::isValidEmail(const std::string& email) {
    const std::regex emailPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, emailPattern);
}

std::pair<bool, std::string> AuthController::validateUsername(const std::string& username) {
    if (username.length() < 3) {
        return {false, "用户名长度不能少于3位"};
    }

    if (username.length() > 20) {
        return {false, "用户名长度不能超过20位"};
    }

    // 检查用户名只包含字母、数字和下划线
    const std::regex usernamePattern(R"([a-zA-Z0-9_]+)");
    if (!std::regex_match(username, usernamePattern)) {
        return {false, "用户名只能包含字母、数字和下划线"};
    }

    return {true, ""};
}

nlohmann::json AuthController::createUserInfoJson(const models::User& user) {
    nlohmann::json userInfo;
    userInfo["id"] = user.getId();
    userInfo["username"] = user.getUsername();
    userInfo["email"] = user.getEmail();
    userInfo["full_name"] = user.getFullName();
    userInfo["role"] = models::User::roleToString(user.getRole());
    userInfo["status"] = models::User::statusToString(user.getStatus());
    // 转换时间戳为Unix时间
    auto created_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        user.getCreatedAt().time_since_epoch()).count();
    auto updated_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        user.getUpdatedAt().time_since_epoch()).count();

    userInfo["created_at"] = created_timestamp;
    userInfo["updated_at"] = updated_timestamp;

    // 不包含密码哈希等敏感信息
    return userInfo;
}

void AuthController::logLoginAttempt(const std::string& username, bool success, const std::string& ip_address) {
    if (success) {
        spdlog::info("登录成功 - 用户: {}, IP: {}", username, ip_address);
    } else {
        spdlog::warn("登录失败 - 用户: {}, IP: {}", username, ip_address);
    }
}

bool AuthController::isUserLocked(const std::string& username) {
    auto lockIter = lockoutTimes.find(username);
    if (lockIter == lockoutTimes.end()) {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto lockoutDuration = std::chrono::minutes(30); // 锁定30分钟

    if (now - lockIter->second > lockoutDuration) {
        // 锁定时间已过，移除锁定记录
        lockoutTimes.erase(lockIter);
        failedLoginCounts.erase(username);
        return false;
    }

    return true;
}

void AuthController::incrementFailedLoginCount(const std::string& username) {
    failedLoginCounts[username]++;

    // 如果失败次数达到5次，锁定账户
    if (failedLoginCounts[username] >= 5) {
        lockoutTimes[username] = std::chrono::system_clock::now();
        spdlog::warn("用户 {} 登录失败次数过多，账户已被锁定", username);
    }
}

void AuthController::resetFailedLoginCount(const std::string& username) {
    failedLoginCounts.erase(username);
    lockoutTimes.erase(username);
}

} // namespace controllers