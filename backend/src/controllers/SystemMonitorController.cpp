#include "SystemMonitorController.h"
#include "config/Config.h"
#include "config/MockData.h"
#include "utils/ParamParser.h"
#include <spdlog/spdlog.h>
#include <regex>
#include <iomanip>
#include <sstream>

namespace controllers {

// 静态成员初始化
std::chrono::steady_clock::time_point SystemMonitorController::serverStartTime_;

SystemMonitorController::SystemMonitorController(
    std::shared_ptr<auth::JwtService> jwtService,
    std::shared_ptr<repositories::UserRepository> userRepository)
    : jwtService_(jwtService), userRepository_(userRepository) {
    spdlog::info("SystemMonitorController initialized");
}

void SystemMonitorController::initializeStartTime() {
    serverStartTime_ = std::chrono::steady_clock::now();
}

// ========== 监控相关接口实现 ==========

http::response<http::string_body> SystemMonitorController::healthCheck(const http::request<http::string_body>& req) {
    try {
        std::string db_status = checkDatabaseStatus();
        std::string api_status = checkExternalApiStatus();

        bool overall_healthy = (db_status == "ok" && api_status == "ok");

        nlohmann::json health_data = {
            {"status", overall_healthy ? "ok" : "degraded"},
            {"timestamp", std::time(nullptr)},
            {"services", {
                {"database", db_status},
                {"external_apis", api_status}
            }},
            {"uptime_seconds", getSystemUptime().count()}
        };

        return utils::HttpResponse::createSuccessResponse(health_data, "Health check completed");

    } catch (const std::exception& e) {
        spdlog::error("Error in health check: {}", e.what());

        nlohmann::json error_data = {
            {"status", "error"},
            {"timestamp", std::time(nullptr)},
            {"error", e.what()}
        };

        return utils::HttpResponse::createInternalErrorResponse("Health check failed");
    }
}

http::response<http::string_body> SystemMonitorController::getSystemInfo(const http::request<http::string_body>& req) {
    try {
        auto& config = config::Config::getInstance();
        auto uptime = getSystemUptime();

        // 格式化启动时间
        auto start_time_t = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() - uptime
        );
        std::stringstream ss;

        // 使用线程安全的 gmtime_s (Windows) / gmtime_r (POSIX)
        std::tm gmt_tm = {};
#ifdef _WIN32
        gmtime_s(&gmt_tm, &start_time_t);  // Windows 线程安全版本
#else
        gmtime_r(&start_time_t, &gmt_tm);  // POSIX 线程安全版本
#endif
        ss << std::put_time(&gmt_tm, "%Y-%m-%d %H:%M:%S UTC");

        nlohmann::json system_info = {
            {"name", config.getString("system.name", "低空交通系统")},
            {"version", config.getString("system.version", "1.0.0")},
            {"start_time", ss.str()},
            {"uptime_seconds", uptime.count()},
            {"environment", config.getString("system.environment", "development")},
            {"server_time", std::time(nullptr)}
        };

        return utils::HttpResponse::createSuccessResponse(system_info, "System information retrieved");

    } catch (const std::exception& e) {
        spdlog::error("Error getting system info: {}", e.what());
        return utils::HttpResponse::createInternalErrorResponse("Failed to retrieve system information");
    }
}

http::response<http::string_body> SystemMonitorController::getDronesList(const http::request<http::string_body>& req) {
    try {
        // 验证用户权限
        auto [hasPermission, userInfo] = validateUserPermission(req);
        if (!hasPermission) {
            return utils::HttpResponse::createUnauthorizedResponse("需要用户认证");
        }

        // 获取Mock无人机数据（实际项目中应该从数据库查询）
        auto drones = getMockDroneData();

        int total = static_cast<int>(drones.size());
        int active_count = 0;

        // 统计活跃无人机数量
        for (const auto& drone : drones) {
            if (drone["status"] == "active") {
                active_count++;
            }
        }

        nlohmann::json response_data = {
            {"drones", drones},
            {"total", total},
            {"active_count", active_count}
        };

        return utils::HttpResponse::createSuccessResponse(response_data, "Drones retrieved successfully");

    } catch (const std::exception& e) {
        spdlog::error("Error getting drones list: {}", e.what());
        return utils::HttpResponse::createInternalErrorResponse("Failed to retrieve drones list");
    }
}

http::response<http::string_body> SystemMonitorController::getUsersList(const http::request<http::string_body>& req) {
    try {
        spdlog::info("SystemMonitorController::getUsersList called");

        // 验证管理员权限
        auto [hasPermission, userInfo] = validateUserPermission(req, "admin");
        if (!hasPermission) {
            return utils::HttpResponse::createForbiddenResponse("需要管理员权限");
        }

        // 解析查询参数
        auto params = parseQueryParams(std::string(req.target()));

        int offset = 0;
        int limit = 50;
        std::optional<models::UserRole> role_filter = std::nullopt;

        if (params.find("offset") != params.end()) {
            // 使用ParamParser安全解析，防止异常和无效值
            offset = utils::ParamParser::parseInt(params["offset"], 0, 0, std::nullopt);
        }
        if (params.find("limit") != params.end()) {
            // 使用ParamParser安全解析，限制范围在1-100之间
            int parsed_limit = utils::ParamParser::parseInt(params["limit"], 50, 1, 100);
            limit = parsed_limit;
        }
        if (params.find("role") != params.end()) {
            std::string role_str = params["role"];
            if (role_str == "admin") {
                role_filter = models::UserRole::ADMIN;
            } else if (role_str == "operator") {
                role_filter = models::UserRole::OPERATOR;
            } else if (role_str == "user") {
                role_filter = models::UserRole::USER;
            }
        }

        // 从数据库获取用户列表
        auto users = userRepository_->getUsers(offset, limit, std::nullopt, role_filter);
        int total = userRepository_->getUserCount(std::nullopt, role_filter);

        // 转换为JSON格式（不包含密码等敏感信息）
        nlohmann::json users_json = nlohmann::json::array();
        for (const auto& user : users) {
            auto user_json = user.toJson(false); // false表示不包含密码哈希

            // 确保last_login字段存在（如果User::toJson没有设置，则设为null）
            if (!user_json.contains("last_login")) {
                user_json["last_login"] = nlohmann::json(); // 设置为null
            }

            users_json.push_back(user_json);
        }

        nlohmann::json response_data = {
            {"users", users_json},
            {"pagination", {
                {"total", total},
                {"offset", offset},
                {"limit", limit},
                {"count", users.size()}
            }}
        };

        return utils::HttpResponse::createSuccessResponse(response_data, "Users retrieved successfully");

    } catch (const std::exception& e) {
        spdlog::error("Error getting users list: {}", e.what());
        return utils::HttpResponse::createInternalErrorResponse("Failed to retrieve users list");
    }
}

// ========== 辅助方法实现 ==========

std::string SystemMonitorController::extractBearerToken(const http::request<http::string_body>& req) {
    auto auth_header = req.find(http::field::authorization);
    if (auth_header == req.end()) {
        return "";
    }

    std::string auth_value = auth_header->value();
    const std::string bearer_prefix = "Bearer ";

    if (auth_value.substr(0, bearer_prefix.length()) == bearer_prefix) {
        return auth_value.substr(bearer_prefix.length());
    }

    return "";
}

std::pair<bool, nlohmann::json> SystemMonitorController::validateUserPermission(
    const http::request<http::string_body>& req,
    const std::string& required_role) {

    std::string token = extractBearerToken(req);
    if (token.empty()) {
        return {false, nlohmann::json::object()};
    }

    try {
        // 验证JWT Token
        auto validation_result = jwtService_->validateToken(token);
        if (validation_result != auth::JwtValidationResult::VALID) {
            return {false, nlohmann::json::object()};
        }

        // 解析Token获取用户信息
        auto token_info = jwtService_->parseToken(token);
        if (!token_info.has_value()) {
            return {false, nlohmann::json::object()};
        }

        // 转换为JSON格式
        nlohmann::json user_claims = {
            {"user_id", token_info->user_id},
            {"username", token_info->username},
            {"email", token_info->email},
            {"role", token_info->role == models::UserRole::ADMIN ? "admin" :
                     token_info->role == models::UserRole::OPERATOR ? "operator" : "user"}
        };

        // 检查是否需要特定角色
        if (!required_role.empty()) {
            std::string user_role = user_claims.value("role", "");
            if (user_role != required_role) {
                return {false, user_claims};
            }
        }

        return {true, user_claims};

    } catch (const std::exception& e) {
        spdlog::warn("Token validation failed: {}", e.what());
        return {false, nlohmann::json::object()};
    }
}

std::string SystemMonitorController::checkDatabaseStatus() {
    try {
        auto& db_manager = database::DatabaseManager::getInstance();

        // 执行简单的测试查询
        auto result = db_manager.executeQuery("SELECT 1 as test");
        if (result && result->hasData()) {
            return "ok";
        } else {
            return "error";
        }

    } catch (const std::exception& e) {
        spdlog::error("Database health check failed: {}", e.what());
        return "error";
    }
}

std::string SystemMonitorController::checkExternalApiStatus() {
    // 这里可以检查外部API的状态，比如高德地图API等
    // 简化实现，直接返回ok
    // 实际项目中应该发送测试请求到外部API
    try {
        auto& config = config::Config::getInstance();
        std::string amap_key = config.getString("external_apis.amap.key", "");

        if (amap_key.empty()) {
            return "warning"; // 配置缺失但不影响核心功能
        }

        // 这里可以添加实际的API健康检查
        return "ok";

    } catch (const std::exception& e) {
        spdlog::error("External API health check failed: {}", e.what());
        return "error";
    }
}

std::unordered_map<std::string, std::string> SystemMonitorController::parseQueryParams(const std::string& target) {
    std::unordered_map<std::string, std::string> params;

    size_t query_pos = target.find('?');
    if (query_pos == std::string::npos) {
        return params;
    }

    std::string query_string = target.substr(query_pos + 1);
    std::regex param_regex(R"(([^&=]+)=([^&]*))");
    std::sregex_iterator iter(query_string.begin(), query_string.end(), param_regex);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        if (match.size() == 3) {
            params[match[1].str()] = match[2].str();
        }
    }

    return params;
}

nlohmann::json SystemMonitorController::getMockDroneData() {
    // 从配置文件中获取Mock数据
    return config::getMockDroneData();
}

std::chrono::seconds SystemMonitorController::getSystemUptime() {
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - serverStartTime_);
    return uptime;
}

} // namespace controllers