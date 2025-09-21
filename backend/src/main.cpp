#include <iostream>
#include <csignal>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "config/Config.h"
#include "server/HttpServer.h"
#include "database/DatabaseManager.h"
#include "repositories/UserRepository.h"
#include "controllers/AuthController.h"
#include "auth/JwtService.h"

// 全局服务器实例，用于信号处理
std::unique_ptr<server::HttpServer> g_server = nullptr;

/**
 * @brief 信号处理函数，优雅关闭服务器
 */
void signalHandler(int signal) {
    spdlog::info("Received signal {}, shutting down gracefully...", signal);
    if (g_server) {
        g_server->stop();
    }
    std::exit(0);
}

/**
 * @brief 初始化日志系统
 */
void initLogging() {
    auto& config = config::Config::getInstance();

    // 获取日志配置
    std::string log_level = config.getString("logging.level", "info");
    std::string log_file = config.getString("logging.file", "logs/server.log");

    // 设置日志级别
    if (log_level == "debug") {
        spdlog::set_level(spdlog::level::debug);
    } else if (log_level == "info") {
        spdlog::set_level(spdlog::level::info);
    } else if (log_level == "warn") {
        spdlog::set_level(spdlog::level::warn);
    } else if (log_level == "error") {
        spdlog::set_level(spdlog::level::err);
    }

    try {
        // 创建多个sink：控制台和文件
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file, 1024 * 1024 * 10, 10);  // 10MB, 10个文件
        file_sink->set_level(spdlog::level::debug);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");

        // 创建logger
        auto logger = std::make_shared<spdlog::logger>("multi_sink",
            spdlog::sinks_init_list{console_sink, file_sink});
        logger->set_level(spdlog::level::debug);

        // 设置为默认logger
        spdlog::set_default_logger(logger);

        spdlog::info("Logging system initialized");
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

/**
 * @brief 设置API路由
 */
void setupRoutes(server::HttpServer& server) {
    // 初始化认证控制器
    auto userRepo = std::make_shared<repositories::UserRepository>();
    auto jwtService = std::make_shared<auth::JwtService>(auth::JwtService::fromConfig());
    auto authController = std::make_shared<controllers::AuthController>(userRepo, jwtService);

    // ========== 认证相关API ==========

    // 用户注册
    server.post("/api/v1/auth/register", [authController](const auto& req, auto& res) {
        auto response = authController->registerUser(req);
        res = std::move(response);
    });

    // 用户登录
    server.post("/api/v1/auth/login", [authController](const auto& req, auto& res) {
        auto response = authController->login(req);
        res = std::move(response);
    });

    // Token刷新
    server.post("/api/v1/auth/refresh", [authController](const auto& req, auto& res) {
        auto response = authController->refreshToken(req);
        res = std::move(response);
    });

    // 获取当前用户信息
    server.get("/api/v1/auth/me", [authController](const auto& req, auto& res) {
        auto response = authController->getCurrentUser(req);
        res = std::move(response);
    });

    // 修改密码
    server.put("/api/v1/auth/password", [authController](const auto& req, auto& res) {
        auto response = authController->changePassword(req);
        res = std::move(response);
    });

    // 用户登出
    server.post("/api/v1/auth/logout", [authController](const auto& req, auto& res) {
        auto response = authController->logout(req);
        res = std::move(response);
    });

    // ========== 系统API ==========

    // 健康检查端点
    server.get("/api/v1/health", [](const auto& req, auto& res) {
        nlohmann::json response = {
            {"success", true},
            {"message", "Service is healthy"},
            {"timestamp", std::time(nullptr)},
            {"data", {
                {"status", "ok"},
                {"service", "low-altitude-traffic-system"},
                {"version", "1.0.0"}
            }}
        };

        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
        res.body() = response.dump();
        res.prepare_payload();
    });

    // 系统信息端点
    server.get("/api/v1/info", [](const auto& req, auto& res) {
        auto& config = config::Config::getInstance();

        nlohmann::json response = {
            {"success", true},
            {"message", "System information retrieved"},
            {"timestamp", std::time(nullptr)},
            {"data", {
                {"name", config.getString("system.name", "城市智能低空交通系统")},
                {"version", config.getString("system.version", "1.0.0")},
                {"server_time", std::time(nullptr)},
                {"environment", "development"}
            }}
        };

        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
        res.body() = response.dump();
        res.prepare_payload();
    });

    // 用户相关API（使用真实的UserRepository）
    server.get("/api/v1/users", [](const auto& req, auto& res) {
        try {
            repositories::UserRepository userRepo;

            // 解析查询参数
            // 这里简化处理，实际应该从请求URL中解析参数
            int offset = 0;
            int limit = 50;

            auto users = userRepo.getUsers(offset, limit);
            int total = userRepo.getUserCount();

            // 将用户对象转换为JSON（不包含敏感信息）
            nlohmann::json usersJson = nlohmann::json::array();
            for (const auto& user : users) {
                usersJson.push_back(user.toJson(false)); // false表示不包含密码哈希
            }

            nlohmann::json response = {
                {"success", true},
                {"message", "Users retrieved successfully"},
                {"timestamp", std::time(nullptr)},
                {"data", {
                    {"users", usersJson},
                    {"pagination", {
                        {"total", total},
                        {"offset", offset},
                        {"limit", limit},
                        {"count", users.size()}
                    }}
                }}
            };

            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = response.dump();
            res.prepare_payload();

        } catch (const std::exception& e) {
            spdlog::error("Error retrieving users: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error", "Failed to retrieve users"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 无人机列表API
    server.get("/api/v1/drones", [](const auto& req, auto& res) {
        nlohmann::json response = {
            {"success", true},
            {"message", "Drones retrieved successfully"},
            {"timestamp", std::time(nullptr)},
            {"data", nlohmann::json::array({
                {
                    {"id", 1},
                    {"drone_id", "DJI-001"},
                    {"name", "Phantom 4 Pro"},
                    {"model", "Phantom 4 Pro"},
                    {"status", "active"},
                    {"battery", 85.5},
                    {"location", {
                        {"lat", 39.904200},
                        {"lng", 116.407396}
                    }}
                },
                {
                    {"id", 2},
                    {"drone_id", "DJI-002"},
                    {"name", "Mavic 3"},
                    {"model", "Mavic 3"},
                    {"status", "active"},
                    {"battery", 72.3},
                    {"location", {
                        {"lat", 39.914200},
                        {"lng", 116.417396}
                    }}
                }
            })}
        };

        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
        res.body() = response.dump();
        res.prepare_payload();
    });

    // 任务列表API
    server.get("/api/v1/tasks", [](const auto& req, auto& res) {
        nlohmann::json response = {
            {"success", true},
            {"message", "Tasks retrieved successfully"},
            {"timestamp", std::time(nullptr)},
            {"data", nlohmann::json::array({
                {
                    {"id", 1},
                    {"task_id", "TASK-001"},
                    {"task_name", "市中心巡检任务"},
                    {"task_type", "inspection"},
                    {"status", "pending"},
                    {"priority", "medium"},
                    {"start_location", {
                        {"lat", 39.904200},
                        {"lng", 116.407396},
                        {"address", "市中心广场"}
                    }},
                    {"end_location", {
                        {"lat", 39.924200},
                        {"lng", 116.427396},
                        {"address", "商业区"}
                    }},
                    {"created_at", "2024-01-15T14:30:00Z"}
                },
                {
                    {"id", 2},
                    {"task_id", "TASK-002"},
                    {"task_name", "物流配送任务"},
                    {"task_type", "delivery"},
                    {"status", "executing"},
                    {"priority", "high"},
                    {"start_location", {
                        {"lat", 39.914200},
                        {"lng", 116.417396},
                        {"address", "火车站"}
                    }},
                    {"end_location", {
                        {"lat", 39.944200},
                        {"lng", 116.447396},
                        {"address", "大学城"}
                    }},
                    {"created_at", "2024-01-15T13:15:00Z"}
                }
            })}
        };

        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
        res.body() = response.dump();
        res.prepare_payload();
    });

    spdlog::info("API routes configured");
}

int main(int argc, char* argv[]) {
    try {
        // 加载配置
        auto& config = config::Config::getInstance();

        // 尝试从配置文件加载，如果失败则使用默认配置
        std::string config_file = "config/server.json";
        if (argc > 1) {
            config_file = argv[1];
        }

        if (!config.loadFromFile(config_file)) {
            spdlog::warn("Could not load config file {}, using defaults", config_file);
        }

        // 初始化日志系统
        initLogging();

        spdlog::info("=== 城市智能低空交通系统后端服务启动 ===");
        spdlog::info("Service: {}", config.getString("system.name", "城市智能低空交通系统"));
        spdlog::info("Version: {}", config.getString("system.version", "1.0.0"));

        // 初始化数据库
        spdlog::info("Initializing database connection...");
        auto& db_manager = database::DatabaseManager::getInstance();
        database::DatabaseConfig db_config{
            config.getString("database.host", "localhost"),
            config.getInt("database.port", 33060),  // X Protocol端口
            config.getString("database.database", "low_altitude_traffic_system"),
            config.getString("database.username", "root"),
            config.getString("database.password", ""),
            config.getInt("database.connection_timeout", 10),
            config.getBool("database.use_ssl", false)
        };

        if (!db_manager.initialize(db_config)) {
            spdlog::error("Failed to initialize database connection");
            return 1;
        }
        spdlog::info("Database connection initialized successfully");

        // 设置信号处理
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // 创建HTTP服务器
        std::string host = config.getString("server.host", "0.0.0.0");
        int port = config.getInt("server.port", 8081);
        int threads = config.getInt("server.threads", 4);

        g_server = std::make_unique<server::HttpServer>(host, port);

        // 设置路由
        setupRoutes(*g_server);

        spdlog::info("Starting server on {}:{} with {} threads", host, port, threads);

        // 运行服务器
        g_server->run(threads);

    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }

    return 0;
}