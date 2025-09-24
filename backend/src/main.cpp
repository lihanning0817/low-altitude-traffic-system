#include <iostream>
#include <csignal>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "config/Config.h"
#include "server/HttpServer.h"
#include "database/DatabaseManager.h"
#include "repositories/UserRepository.h"
#include "repositories/FlightTaskRepository.h"
#include "controllers/AuthController.h"
#include "controllers/FlightTaskController.h"
#include "controllers/SystemMonitorController.h"
#include "auth/JwtService.h"
#include "services/RouteService.h"

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
    // 初始化服务器启动时间（用于系统监控）
    controllers::SystemMonitorController::initializeStartTime();

    // 初始化认证控制器
    auto userRepo = std::make_shared<repositories::UserRepository>();
    auto jwtService = std::make_shared<auth::JwtService>(auth::JwtService::fromConfig());
    auto authController = std::make_shared<controllers::AuthController>(userRepo, jwtService);

    // 初始化飞行任务控制器
    auto taskRepo = std::make_shared<repositories::FlightTaskRepository>();
    auto taskController = std::make_shared<controllers::FlightTaskController>(taskRepo, jwtService);

    // 初始化系统监控控制器
    auto systemMonitorController = std::make_shared<controllers::SystemMonitorController>(jwtService, userRepo);

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

    // ========== 系统监控API ==========

    // 健康检查端点（无需认证）
    server.get("/api/v1/health", [systemMonitorController](const auto& req, auto& res) {
        auto response = systemMonitorController->healthCheck(req);
        res = std::move(response);
    });

    // 系统信息端点（无需认证）
    server.get("/api/v1/info", [systemMonitorController](const auto& req, auto& res) {
        auto response = systemMonitorController->getSystemInfo(req);
        res = std::move(response);
    });

    // 获取无人机列表（需要认证）
    server.get("/api/v1/drones", [systemMonitorController](const auto& req, auto& res) {
        auto response = systemMonitorController->getDronesList(req);
        res = std::move(response);
    });

    // 获取用户列表（需要管理员权限）
    server.get("/api/v1/users", [systemMonitorController](const auto& req, auto& res) {
        auto response = systemMonitorController->getUsersList(req);
        res = std::move(response);
    });

    // 任务列表API (旧的简单版本 - 已被FlightTask替代)
    /*server.get("/api/v1/tasks", [](const auto& req, auto& res) {
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
    });*/

    // ========== 地图相关API ==========

    // 获取地图标记点
    server.get("/api/v1/map/markers", [](const auto& req, auto& res) {
        try {
            auto& db_manager = database::DatabaseManager::getInstance();

            // 查询所有标记点
            auto result = db_manager.executeQuery(
                "SELECT id, lng, lat, title, type, description, created_at FROM low_altitude_traffic_system.map_markers WHERE is_active = 1 ORDER BY created_at DESC"
            );

            nlohmann::json markers = nlohmann::json::array();

            if (result) {
                while (auto row = result->fetchRow()) {
                    markers.push_back({
                        {"id", static_cast<int>(row[0])},
                        {"lng", static_cast<double>(row[1])},
                        {"lat", static_cast<double>(row[2])},
                        {"title", static_cast<std::string>(row[3])},
                        {"type", static_cast<std::string>(row[4])},
                        {"description", row[5].isNull() ? "" : static_cast<std::string>(row[5])},
                        {"created_at", static_cast<std::string>(row[6])}
                    });
                }
            }

            nlohmann::json response = {
                {"success", true},
                {"message", "Markers retrieved successfully"},
                {"timestamp", std::time(nullptr)},
                {"data", markers}
            };

            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = response.dump();
            res.prepare_payload();

        } catch (const std::exception& e) {
            spdlog::error("Error retrieving markers: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error", "Failed to retrieve markers"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 添加地图标记点
    server.post("/api/v1/map/markers", [](const auto& req, auto& res) {
        try {
            // 解析请求体JSON
            auto json_data = nlohmann::json::parse(req.body());

            // 验证必需字段
            if (!json_data.contains("lng") || !json_data.contains("lat") || !json_data.contains("title")) {
                nlohmann::json error_response = {
                    {"success", false},
                    {"error", "Missing required fields: lng, lat, title"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::bad_request);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
                return;
            }

            double lng = json_data["lng"].get<double>();
            double lat = json_data["lat"].get<double>();
            std::string title = json_data["title"].get<std::string>();
            std::string type = json_data.value("type", "marker");
            std::string description = json_data.value("description", "");

            auto& db_manager = database::DatabaseManager::getInstance();

            // 使用预编译语句插入标记点
            std::vector<mysqlx::Value> params = {lng, lat, title, type, description};

            spdlog::debug("Inserting marker - lng: {}, lat: {}, title: {}, type: {}, description: {}", lng, lat, title, type, description);

            uint64_t affected_rows = db_manager.executePreparedUpdate(
                "INSERT INTO low_altitude_traffic_system.map_markers (lng, lat, title, type, description, created_at) VALUES (?, ?, ?, ?, ?, NOW())",
                params
            );

            if (affected_rows > 0) {
                nlohmann::json response = {
                    {"success", true},
                    {"message", "Marker added successfully"},
                    {"timestamp", std::time(nullptr)},
                    {"data", {
                        {"id", static_cast<int>(db_manager.getLastInsertId())},
                        {"lng", lng},
                        {"lat", lat},
                        {"title", title},
                        {"type", type},
                        {"description", description}
                    }}
                };

                res.result(boost::beast::http::status::created);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = response.dump();
                res.prepare_payload();
            } else {
                nlohmann::json error_response = {
                    {"success", false},
                    {"error", "Failed to add marker"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::internal_server_error);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
            }

        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("JSON parse error in marker creation: {}", e.what());
            spdlog::error("Request body: {}", req.body());

            nlohmann::json error_response = {
                {"success", false},
                {"error", std::string("Invalid JSON format: ") + e.what()},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::bad_request);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();

        } catch (const std::exception& e) {
            spdlog::error("Error adding marker: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error", "Internal server error"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 清除所有标记点
    server.del("/api/v1/map/markers", [](const auto& req, auto& res) {
        try {
            auto& db_manager = database::DatabaseManager::getInstance();

            uint64_t affected_rows = db_manager.executeUpdate("DELETE FROM map_markers");

            nlohmann::json response = {
                {"success", true},
                {"message", "All markers cleared successfully"},
                {"timestamp", std::time(nullptr)},
                {"data", {
                    {"cleared_count", static_cast<int>(affected_rows)}
                }}
            };

            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = response.dump();
            res.prepare_payload();

        } catch (const std::exception& e) {
            spdlog::error("Error clearing markers: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error", "Failed to clear markers"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 路线规划API
    server.get("/api/v1/map/route", [](const auto& req, auto& res) {
        try {
            // 从查询参数获取起点、终点和策略
            std::string query_string = std::string(req.target());
            std::string origin, destination, strategy = "0";

            // 简单的查询参数解析
            size_t origin_pos = query_string.find("origin=");
            size_t dest_pos = query_string.find("destination=");
            size_t strategy_pos = query_string.find("strategy=");

            if (origin_pos != std::string::npos) {
                size_t start = origin_pos + 7;  // "origin="的长度
                size_t end = query_string.find("&", start);
                if (end == std::string::npos) end = query_string.length();
                origin = query_string.substr(start, end - start);
            }

            if (dest_pos != std::string::npos) {
                size_t start = dest_pos + 12;  // "destination="的长度
                size_t end = query_string.find("&", start);
                if (end == std::string::npos) end = query_string.length();
                destination = query_string.substr(start, end - start);
            }

            if (strategy_pos != std::string::npos) {
                size_t start = strategy_pos + 9;  // "strategy="的长度
                size_t end = query_string.find("&", start);
                if (end == std::string::npos) end = query_string.length();
                strategy = query_string.substr(start, end - start);
            }

            spdlog::debug("Route planning request - Origin: {}, Destination: {}, Strategy: {}", origin, destination, strategy);

            if (origin.empty() || destination.empty()) {
                nlohmann::json error_response = {
                    {"success", false},
                    {"error", "Missing required parameters: origin and destination"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::bad_request);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
                return;
            }

            // 创建路线规划服务
            auto& config = config::Config::getInstance();
            std::string amap_key = config.getString("external_apis.amap.key", "1872806f332dab32a1a3dc895b0ad542");
            spdlog::debug("Using Amap API key: {}", amap_key.substr(0, 8) + "...");
            services::RouteService route_service(amap_key);

            // 调用路线规划
            auto route_result = route_service.planRoute(origin, destination, strategy);

            if (route_result["success"].get<bool>()) {
                nlohmann::json response = {
                    {"success", true},
                    {"message", "Route planned successfully"},
                    {"timestamp", std::time(nullptr)},
                    {"data", route_result["data"]}
                };

                res.result(boost::beast::http::status::ok);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = response.dump();
                res.prepare_payload();
            } else {
                nlohmann::json error_response = {
                    {"success", false},
                    {"error", route_result.value("error", "Route planning failed")},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::internal_server_error);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
            }

        } catch (const std::exception& e) {
            spdlog::error("Error in route planning: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error", std::string("Route planning error: ") + e.what()},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 地理编码API
    server.get("/api/v1/map/geocode", [](const auto& req, auto& res) {
        try {
            std::string query_string = std::string(req.target());
            std::string address;

            // 解析地址参数
            size_t addr_pos = query_string.find("address=");
            if (addr_pos != std::string::npos) {
                size_t start = addr_pos + 8;  // "address="的长度
                size_t end = query_string.find("&", start);
                if (end == std::string::npos) end = query_string.length();
                address = query_string.substr(start, end - start);
            }

            if (address.empty()) {
                nlohmann::json error_response = {
                    {"success", false},
                    {"error", "Missing required parameter: address"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::bad_request);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
                return;
            }

            // 创建路线规划服务
            auto& config = config::Config::getInstance();
            std::string amap_key = config.getString("amap.api_key", "1872806f332dab32a1a3dc895b0ad542");
            services::RouteService route_service(amap_key);

            // 调用地理编码
            auto geocode_result = route_service.geocode(address);

            nlohmann::json response = {
                {"success", geocode_result["success"].get<bool>()},
                {"message", geocode_result["success"].get<bool>() ? "Geocoding successful" : "Geocoding failed"},
                {"timestamp", std::time(nullptr)},
                {"data", geocode_result.value("data", nlohmann::json::object())}
            };

            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = response.dump();
            res.prepare_payload();

        } catch (const std::exception& e) {
            spdlog::error("Error in geocoding: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error", "Internal server error"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // ========== 飞行任务管理API ==========

    // 获取飞行任务列表
    server.get("/api/v1/tasks", [taskController](const auto& req, auto& res) {
        spdlog::info("Handling GET /api/v1/tasks");
        try {
            auto response = taskController->getFlightTasks(req);
            res = std::move(response);
        } catch (const std::exception& e) {
            spdlog::error("Error in GET /api/v1/tasks: {}", e.what());
            res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
        }
    });

    // 创建飞行任务
    server.post("/api/v1/tasks", [taskController](const auto& req, auto& res) {
        spdlog::info("Handling POST /api/v1/tasks");
        try {
            auto response = taskController->createFlightTask(req);
            res = std::move(response);
        } catch (const std::exception& e) {
            spdlog::error("Error in POST /api/v1/tasks: {}", e.what());
            res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
        }
    });

    // 获取指定飞行任务
    server.get("/api/v1/tasks/{id}", [taskController](const auto& req, auto& res) {
        spdlog::info("Handling GET /api/v1/tasks/{{id}}");
        std::string target = std::string(req.target());

        // 提取任务ID
        std::regex id_regex(R"(/api/v1/tasks/(\d+)(?:\?.*)?$)");
        std::smatch matches;
        if (std::regex_match(target, matches, id_regex) && matches.size() > 1) {
            std::string task_id = matches[1].str();
            spdlog::info("Extracted task ID: {}", task_id);
            try {
                auto response = taskController->getFlightTaskById(req, task_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in GET /api/v1/tasks/{}: {}", task_id, e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            spdlog::warn("Invalid task ID format in path: {}", target);
            res = utils::HttpResponse::createErrorResponse("无效的任务ID格式");
        }
    });

    // 更新飞行任务
    server.put("/api/v1/tasks/{id}", [taskController](const auto& req, auto& res) {
        spdlog::info("Handling PUT /api/v1/tasks/{{id}}");
        std::string target = std::string(req.target());

        // 提取任务ID
        std::regex id_regex(R"(/api/v1/tasks/(\d+)(?:\?.*)?$)");
        std::smatch matches;
        if (std::regex_match(target, matches, id_regex) && matches.size() > 1) {
            std::string task_id = matches[1].str();
            spdlog::info("Extracted task ID: {}", task_id);
            try {
                auto response = taskController->updateFlightTask(req, task_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in PUT /api/v1/tasks/{}: {}", task_id, e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            spdlog::warn("Invalid task ID format in path: {}", target);
            res = utils::HttpResponse::createErrorResponse("无效的任务ID格式");
        }
    });

    // 删除飞行任务
    server.del("/api/v1/tasks/{id}", [taskController](const auto& req, auto& res) {
        spdlog::info("Handling DELETE /api/v1/tasks/{{id}}");
        std::string target = std::string(req.target());

        // 提取任务ID
        std::regex id_regex(R"(/api/v1/tasks/(\d+)(?:\?.*)?$)");
        std::smatch matches;
        if (std::regex_match(target, matches, id_regex) && matches.size() > 1) {
            std::string task_id = matches[1].str();
            spdlog::info("Extracted task ID: {}", task_id);
            try {
                auto response = taskController->deleteFlightTask(req, task_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in DELETE /api/v1/tasks/{}: {}", task_id, e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            spdlog::warn("Invalid task ID format in path: {}", target);
            res = utils::HttpResponse::createErrorResponse("无效的任务ID格式");
        }
    });

    spdlog::info("API routes configured (including FlightTask APIs)");
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