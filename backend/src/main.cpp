#include <iostream>
#include <csignal>
#include <sstream>
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
#include "controllers/WeatherController.h"
#include "controllers/EmergencyController.h"
#include "controllers/FlightPermitController.h"
#include "controllers/ConflictDetectionController.h"
#include "controllers/EmergencyLandingController.h"
#include "controllers/DroneController.h"
#include "auth/JwtService.h"
#include "services/RouteService.h"
#include "services/WeatherService.h"
#include "repositories/EmergencyEventRepository.h"

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

    // 初始化天气服务和控制器
    auto& config = config::Config::getInstance();
    std::string weatherApiKey = config.getString("weather.api_key", "");
    auto weatherService = std::make_shared<services::WeatherService>(weatherApiKey);
    auto weatherController = std::make_shared<controllers::WeatherController>(weatherService, jwtService);

    // 初始化应急响应控制器
    auto emergencyRepo = std::make_shared<repositories::EmergencyEventRepository>();
    auto emergencyController = std::make_shared<controllers::EmergencyController>(emergencyRepo, jwtService);

    // 初始化新的控制器 - 使用数据库会话
    auto dbSession = std::make_shared<mysqlx::Session>(
        config.getString("database.host", "localhost"),
        config.getInt("database.port", 33060),
        config.getString("database.username", "root"),
        config.getString("database.password", ""));

    auto permitController = std::make_shared<controllers::FlightPermitController>(dbSession, jwtService);
    auto conflictController = std::make_shared<controllers::ConflictDetectionController>(dbSession, jwtService);
    auto landingController = std::make_shared<controllers::EmergencyLandingController>(dbSession, jwtService);
    auto droneController = std::make_shared<controllers::DroneController>(dbSession, jwtService);

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

    // 设备管理API别名 - 重定向到drones API (设备管理功能已合并到无人机管理)
    server.get("/api/v1/devices", [systemMonitorController](const auto& req, auto& res) {
        spdlog::info("Handling GET /api/v1/devices (redirecting to drones API)");
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
        (void)req;  // 未使用，但保留参数以匹配接口
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
        (void)req;  // 未使用，但保留参数以匹配接口
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

    // URL解码辅助函数
    auto urlDecode = [](const std::string& str) -> std::string {
        std::string result;
        result.reserve(str.length());

        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                // 解析十六进制编码
                int hex_value = 0;
                std::istringstream iss(str.substr(i + 1, 2));
                if (iss >> std::hex >> hex_value) {
                    result += static_cast<char>(hex_value);
                    i += 2;
                } else {
                    result += str[i];
                }
            } else if (str[i] == '+') {
                // '+' 表示空格
                result += ' ';
            } else {
                result += str[i];
            }
        }

        return result;
    };

    // 地理编码API
    server.get("/api/v1/map/geocode", [urlDecode](const auto& req, auto& res) {
        try {
            std::string query_string = std::string(req.target());
            std::string address;

            // 解析地址参数
            size_t addr_pos = query_string.find("address=");
            if (addr_pos != std::string::npos) {
                size_t start = addr_pos + 8;  // "address="的长度
                size_t end = query_string.find("&", start);
                if (end == std::string::npos) end = query_string.length();
                std::string encoded_address = query_string.substr(start, end - start);
                // URL解码地址参数
                address = urlDecode(encoded_address);
                spdlog::debug("Geocoding request - Encoded: {}, Decoded: {}", encoded_address, address);
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
            std::string amap_key = config.getString("external_apis.amap.key", "1872806f332dab32a1a3dc895b0ad542");
            spdlog::debug("Using Amap API key for geocoding: {}", amap_key.substr(0, 8) + "...");
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

    // ========== 天气API ==========

    // 根据城市获取当前天气
    server.get("/api/v1/weather/current", [weatherController](const auto& req, auto& res) {
        auto response = weatherController->getCurrentWeather(req);
        res = std::move(response);
    });

    // 根据坐标获取当前天气
    server.get("/api/v1/weather/current/coords", [weatherController](const auto& req, auto& res) {
        auto response = weatherController->getCurrentWeatherByCoords(req);
        res = std::move(response);
    });

    // 根据坐标获取天气预报
    server.get("/api/v1/weather/forecast", [weatherController](const auto& req, auto& res) {
        auto response = weatherController->getForecast(req);
        res = std::move(response);
    });

    // 检查飞行安全性
    server.get("/api/v1/weather/flight-safety", [weatherController](const auto& req, auto& res) {
        auto response = weatherController->checkFlightSafety(req);
        res = std::move(response);
    });

    // ========== 应急响应API ==========

    // 创建紧急事件
    server.post("/api/v1/emergency/events", [emergencyController](const auto& req, auto& res) {
        auto response = emergencyController->createEvent(req);
        res = std::move(response);
    });

    // 获取所有紧急事件
    server.get("/api/v1/emergency/events", [emergencyController](const auto& req, auto& res) {
        auto response = emergencyController->getAllEvents(req);
        res = std::move(response);
    });

    // 获取紧急事件统计
    server.get("/api/v1/emergency/statistics", [emergencyController](const auto& req, auto& res) {
        auto response = emergencyController->getStatistics(req);
        res = std::move(response);
    });

    // 根据ID获取紧急事件
    server.get("/api/v1/emergency/events/", [emergencyController](const auto& req, auto& res) {
        std::string target = req.target();

        // 匹配 /api/v1/emergency/events/{id} 格式
        std::regex pattern("/api/v1/emergency/events/(\\d+)$");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                int64_t event_id = std::stoll(matches[1].str());
                auto response = emergencyController->getEventById(req, event_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in GET /api/v1/emergency/events/{}: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            spdlog::warn("Invalid event ID format in path: {}", target);
            res = utils::HttpResponse::createErrorResponse("无效的事件ID格式");
        }
    });

    // 响应紧急事件 - respond
    server.post("/api/v1/emergency/events/{id}/respond", [emergencyController](const auto& req, auto& res) {
        std::string target = req.target();

        std::regex pattern("/api/v1/emergency/events/(\\d+)/respond");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                int64_t event_id = std::stoll(matches[1].str());
                auto response = emergencyController->respondToEvent(req, event_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in POST /api/v1/emergency/events/{}/respond: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 解决紧急事件 - resolve
    server.post("/api/v1/emergency/events/{id}/resolve", [emergencyController](const auto& req, auto& res) {
        std::string target = req.target();

        std::regex pattern("/api/v1/emergency/events/(\\d+)/resolve");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                int64_t event_id = std::stoll(matches[1].str());
                auto response = emergencyController->resolveEvent(req, event_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in POST /api/v1/emergency/events/{}/resolve: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 取消紧急事件 - cancel
    server.post("/api/v1/emergency/events/{id}/cancel", [emergencyController](const auto& req, auto& res) {
        std::string target = req.target();

        std::regex pattern("/api/v1/emergency/events/(\\d+)/cancel");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                int64_t event_id = std::stoll(matches[1].str());
                auto response = emergencyController->cancelEvent(req, event_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in POST /api/v1/emergency/events/{}/cancel: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // ========== 飞行许可API ==========

    // 申请飞行许可
    server.post("/api/v1/flight-permits", [permitController](const auto& req, auto& res) {
        auto response = permitController->applyFlightPermit(req);
        res = std::move(response);
    });

    // 获取飞行许可列表
    server.get("/api/v1/flight-permits", [permitController](const auto& req, auto& res) {
        auto response = permitController->getFlightPermits(req);
        res = std::move(response);
    });

    // 审批飞行许可
    server.post("/api/v1/flight-permits/{id}/approve", [permitController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/flight-permits/(\\d+)/approve");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string permit_id = matches[1].str();
                auto response = permitController->approveFlightPermit(req, permit_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in POST /api/v1/flight-permits/{}/approve: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 拒绝飞行许可
    server.post("/api/v1/flight-permits/{id}/reject", [permitController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/flight-permits/(\\d+)/reject");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string permit_id = matches[1].str();
                auto response = permitController->rejectFlightPermit(req, permit_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in POST /api/v1/flight-permits/{}/reject: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 获取指定飞行许可详情
    server.get("/api/v1/flight-permits/{id}", [permitController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/flight-permits/(\\d+)");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string permit_id = matches[1].str();
                auto response = permitController->getFlightPermitById(req, permit_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in GET /api/v1/flight-permits/{}: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 撤销飞行许可
    server.post("/api/v1/flight-permits/{id}/revoke", [permitController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/flight-permits/(\\d+)/revoke");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string permit_id = matches[1].str();
                auto response = permitController->revokeFlightPermit(req, permit_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in POST /api/v1/flight-permits/{}/revoke: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // ========== 飞行冲突检测API ==========

    // 飞行注册（自动冲突检测）
    server.post("/api/v1/flights", [conflictController](const auto& req, auto& res) {
        auto response = conflictController->registerFlight(req);
        res = std::move(response);
    });

    // 获取飞行冲突列表
    server.get("/api/v1/flight-conflicts", [conflictController](const auto& req, auto& res) {
        auto response = conflictController->getFlightConflicts(req);
        res = std::move(response);
    });

    // 解决飞行冲突
    server.post("/api/v1/flight-conflicts/{id}/resolve", [conflictController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/flight-conflicts/(\\d+)/resolve");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string conflict_id = matches[1].str();
                auto response = conflictController->resolveConflict(req, conflict_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in POST /api/v1/flight-conflicts/{}/resolve: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 获取指定冲突详情
    server.get("/api/v1/flight-conflicts/{id}", [conflictController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/flight-conflicts/(\\d+)");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string conflict_id = matches[1].str();
                auto response = conflictController->getConflictById(req, conflict_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in GET /api/v1/flight-conflicts/{}: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 获取冲突统计信息
    server.get("/api/v1/conflict-detection/conflicts/statistics", [conflictController](const auto& req, auto& res) {
        try {
            auto response = conflictController->getConflictStatistics(req);
            res = std::move(response);
        } catch (const std::exception& e) {
            spdlog::error("Error in GET /api/v1/conflict-detection/conflicts/statistics: {}", e.what());
            res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
        }
    });

    // ========== 紧急降落点API ==========

    // 获取紧急降落点列表
    server.get("/api/v1/emergency-landing-points", [landingController](const auto& req, auto& res) {
        auto response = landingController->getEmergencyLandingPoints(req);
        res = std::move(response);
    });

    // 添加紧急降落点（管理员）
    server.post("/api/v1/emergency-landing-points", [landingController](const auto& req, auto& res) {
        auto response = landingController->addEmergencyLandingPoint(req);
        res = std::move(response);
    });

    // 查找最近的紧急降落点
    server.get("/api/v1/emergency-landing-points/nearest", [landingController](const auto& req, auto& res) {
        auto response = landingController->findNearestLandingPoints(req);
        res = std::move(response);
    });

    // 更新紧急降落点（管理员）
    server.put("/api/v1/emergency-landing-points/{id}", [landingController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/emergency-landing-points/(\\d+)");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string point_id = matches[1].str();
                auto response = landingController->updateEmergencyLandingPoint(req, point_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in PUT /api/v1/emergency-landing-points/{}: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 获取指定紧急降落点详情
    server.get("/api/v1/emergency-landing-points/{id}", [landingController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/emergency-landing-points/(\\d+)");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string point_id = matches[1].str();
                auto response = landingController->getEmergencyLandingPointById(req, point_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in GET /api/v1/emergency-landing-points/{}: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // 删除紧急降落点（管理员）
    server.del("/api/v1/emergency-landing-points/{id}", [landingController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/emergency-landing-points/(\\d+)");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string point_id = matches[1].str();
                auto response = landingController->deleteEmergencyLandingPoint(req, point_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in DELETE /api/v1/emergency-landing-points/{}: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // ========== 无人机管理API ==========

    // 获取指定无人机详情
    server.get("/api/v1/drones/{id}", [droneController](const auto& req, auto& res) {
        std::string target = req.target();
        std::regex pattern("/api/v1/drones/(\\d+)");
        std::smatch matches;

        if (std::regex_match(target, matches, pattern)) {
            try {
                std::string drone_id = matches[1].str();
                auto response = droneController->getDroneById(req, drone_id);
                res = std::move(response);
            } catch (const std::exception& e) {
                spdlog::error("Error in GET /api/v1/drones/{}: {}", matches[1].str(), e.what());
                res = utils::HttpResponse::createInternalErrorResponse("服务器内部错误");
            }
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的请求路径");
        }
    });

    // ========== 空域管理API ==========

    // 获取空域列表
    server.get("/api/v1/airspaces", [jwtService](const auto& req, auto& res) {
        spdlog::info("Handling GET /api/v1/airspaces");
        try {
            auto& db_manager = database::DatabaseManager::getInstance();

            // 查询所有空域
            auto result = db_manager.executeQuery(
                "SELECT id, airspace_id, name, type, description, north_lat, south_lat, east_lng, west_lng, "
                "min_altitude, max_altitude, status, authority, contact_info, max_concurrent_flights, "
                "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
                "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at "
                "FROM low_altitude_traffic_system.airspaces ORDER BY created_at DESC"
            );

            nlohmann::json airspaces = nlohmann::json::array();

            if (result) {
                while (auto row = result->fetchRow()) {
                    airspaces.push_back({
                        {"id", static_cast<int>(row[0])},
                        {"airspace_id", static_cast<std::string>(row[1])},
                        {"name", static_cast<std::string>(row[2])},
                        {"type", static_cast<std::string>(row[3])},
                        {"description", row[4].isNull() ? "" : static_cast<std::string>(row[4])},
                        {"north_lat", static_cast<double>(row[5])},
                        {"south_lat", static_cast<double>(row[6])},
                        {"east_lng", static_cast<double>(row[7])},
                        {"west_lng", static_cast<double>(row[8])},
                        {"min_altitude", row[9].isNull() ? 0.0 : static_cast<double>(row[9])},
                        {"max_altitude", row[10].isNull() ? 300.0 : static_cast<double>(row[10])},
                        {"status", static_cast<std::string>(row[11])},
                        {"authority", row[12].isNull() ? "" : static_cast<std::string>(row[12])},
                        {"contact_info", row[13].isNull() ? "" : static_cast<std::string>(row[13])},
                        {"max_concurrent_flights", row[14].isNull() ? 10 : static_cast<int>(row[14])},
                        {"created_at", static_cast<std::string>(row[15])},
                        {"updated_at", static_cast<std::string>(row[16])}
                    });
                }
            }

            nlohmann::json response = {
                {"success", true},
                {"message", "获取空域列表成功"},
                {"timestamp", std::time(nullptr)},
                {"data", {
                    {"airspaces", airspaces},
                    {"total", airspaces.size()}
                }}
            };

            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = response.dump();
            res.prepare_payload();

            spdlog::info("Successfully returned {} airspaces", airspaces.size());

        } catch (const std::exception& e) {
            spdlog::error("Error retrieving airspaces: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error_code", "INTERNAL_ERROR"},
                {"message", "获取空域列表失败"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 获取指定空域详情
    server.get("/api/v1/airspaces/{id}", [jwtService](const auto& req, auto& res) {
        spdlog::info("Handling GET /api/v1/airspaces/{{id}}");
        std::string target = std::string(req.target());

        // 提取空域ID
        std::regex id_regex(R"(/api/v1/airspaces/(\d+)(?:\?.*)?$)");
        std::smatch matches;
        if (std::regex_match(target, matches, id_regex) && matches.size() > 1) {
            std::string airspace_id = matches[1].str();
            spdlog::info("Extracted airspace ID: {}", airspace_id);

            try {
                auto& db_manager = database::DatabaseManager::getInstance();

                // 查询指定空域
                auto result = db_manager.executeQuery(
                    "SELECT id, airspace_id, name, type, description, north_lat, south_lat, east_lng, west_lng, "
                    "min_altitude, max_altitude, status, authority, contact_info, max_concurrent_flights, "
                    "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
                    "DATE_FORMAT(updated_at, '%Y-%m-%d %H:%i:%s') as updated_at "
                    "FROM low_altitude_traffic_system.airspaces WHERE id = " + airspace_id
                );

                if (result && result->hasData()) {
                    if (auto row = result->fetchRow()) {
                        nlohmann::json airspace = {
                        {"id", static_cast<int>(row[0])},
                        {"airspace_id", static_cast<std::string>(row[1])},
                        {"name", static_cast<std::string>(row[2])},
                        {"type", static_cast<std::string>(row[3])},
                        {"description", row[4].isNull() ? "" : static_cast<std::string>(row[4])},
                        {"north_lat", static_cast<double>(row[5])},
                        {"south_lat", static_cast<double>(row[6])},
                        {"east_lng", static_cast<double>(row[7])},
                        {"west_lng", static_cast<double>(row[8])},
                        {"min_altitude", row[9].isNull() ? 0.0 : static_cast<double>(row[9])},
                        {"max_altitude", row[10].isNull() ? 300.0 : static_cast<double>(row[10])},
                        {"status", static_cast<std::string>(row[11])},
                        {"authority", row[12].isNull() ? "" : static_cast<std::string>(row[12])},
                        {"contact_info", row[13].isNull() ? "" : static_cast<std::string>(row[13])},
                        {"max_concurrent_flights", row[14].isNull() ? 10 : static_cast<int>(row[14])},
                        {"created_at", static_cast<std::string>(row[15])},
                        {"updated_at", static_cast<std::string>(row[16])}
                    };

                    nlohmann::json response = {
                        {"success", true},
                        {"message", "获取空域详情成功"},
                        {"timestamp", std::time(nullptr)},
                        {"data", {
                            {"airspace", airspace}
                        }}
                    };

                    res.result(boost::beast::http::status::ok);
                    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                    res.body() = response.dump();
                    res.prepare_payload();

                    spdlog::info("Successfully returned airspace details for ID: {}", airspace_id);
                    }
                } else {
                    nlohmann::json error_response = {
                        {"success", false},
                        {"error_code", "NOT_FOUND"},
                        {"message", "空域不存在"},
                        {"timestamp", std::time(nullptr)}
                    };

                    res.result(boost::beast::http::status::not_found);
                    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                    res.body() = error_response.dump();
                    res.prepare_payload();
                }

            } catch (const std::exception& e) {
                spdlog::error("Error retrieving airspace {}: {}", airspace_id, e.what());

                nlohmann::json error_response = {
                    {"success", false},
                    {"error_code", "INTERNAL_ERROR"},
                    {"message", "获取空域详情失败"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::internal_server_error);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
            }
        } else {
            spdlog::warn("Invalid airspace ID format in path: {}", target);

            nlohmann::json error_response = {
                {"success", false},
                {"error_code", "INVALID_PARAMETER"},
                {"message", "无效的空域ID格式"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::bad_request);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 创建新空域
    server.post("/api/v1/airspaces", [jwtService](const auto& req, auto& res) {
        spdlog::info("Handling POST /api/v1/airspaces");
        try {
            // 解析请求体JSON
            auto json_data = nlohmann::json::parse(req.body());

            // 验证必需字段
            if (!json_data.contains("airspace_id") || !json_data.contains("name") ||
                !json_data.contains("type") || !json_data.contains("north_lat") ||
                !json_data.contains("south_lat") || !json_data.contains("east_lng") ||
                !json_data.contains("west_lng")) {
                nlohmann::json error_response = {
                    {"success", false},
                    {"error_code", "MISSING_REQUIRED_FIELDS"},
                    {"message", "缺少必需字段: airspace_id, name, type, north_lat, south_lat, east_lng, west_lng"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::bad_request);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
                return;
            }

            std::string airspace_id = json_data["airspace_id"].get<std::string>();
            std::string name = json_data["name"].get<std::string>();
            std::string type = json_data["type"].get<std::string>();
            std::string description = json_data.value("description", "");
            double north_lat = json_data["north_lat"].get<double>();
            double south_lat = json_data["south_lat"].get<double>();
            double east_lng = json_data["east_lng"].get<double>();
            double west_lng = json_data["west_lng"].get<double>();
            double min_altitude = json_data.value("min_altitude", 0.0);
            double max_altitude = json_data.value("max_altitude", 300.0);
            std::string status = json_data.value("status", "active");
            std::string authority = json_data.value("authority", "");
            std::string contact_info = json_data.value("contact_info", "");
            int max_concurrent_flights = json_data.value("max_concurrent_flights", 10);

            auto& db_manager = database::DatabaseManager::getInstance();

            // 使用预编译语句插入空域
            std::vector<mysqlx::Value> params = {
                airspace_id, name, type, description,
                north_lat, south_lat, east_lng, west_lng,
                min_altitude, max_altitude, status,
                authority, contact_info, max_concurrent_flights
            };

            uint64_t affected_rows = db_manager.executePreparedUpdate(
                "INSERT INTO low_altitude_traffic_system.airspaces "
                "(airspace_id, name, type, description, north_lat, south_lat, east_lng, west_lng, "
                "min_altitude, max_altitude, status, authority, contact_info, max_concurrent_flights, "
                "created_at, updated_at) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())",
                params
            );

            if (affected_rows > 0) {
                uint64_t new_id = db_manager.getLastInsertId();

                nlohmann::json response = {
                    {"success", true},
                    {"message", "创建空域成功"},
                    {"timestamp", std::time(nullptr)},
                    {"data", {
                        {"id", static_cast<int>(new_id)},
                        {"airspace_id", airspace_id},
                        {"name", name},
                        {"type", type},
                        {"description", description},
                        {"north_lat", north_lat},
                        {"south_lat", south_lat},
                        {"east_lng", east_lng},
                        {"west_lng", west_lng},
                        {"min_altitude", min_altitude},
                        {"max_altitude", max_altitude},
                        {"status", status},
                        {"authority", authority},
                        {"contact_info", contact_info},
                        {"max_concurrent_flights", max_concurrent_flights}
                    }}
                };

                res.result(boost::beast::http::status::created);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = response.dump();
                res.prepare_payload();

                spdlog::info("Successfully created airspace with ID: {}", new_id);
            } else {
                nlohmann::json error_response = {
                    {"success", false},
                    {"error_code", "DATABASE_ERROR"},
                    {"message", "创建空域失败"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::internal_server_error);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
            }

        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("JSON parse error in airspace creation: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error_code", "INVALID_JSON"},
                {"message", std::string("JSON格式错误: ") + e.what()},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::bad_request);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();

        } catch (const std::exception& e) {
            spdlog::error("Error creating airspace: {}", e.what());

            nlohmann::json error_response = {
                {"success", false},
                {"error_code", "INTERNAL_ERROR"},
                {"message", "创建空域失败"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::internal_server_error);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 更新空域
    server.put("/api/v1/airspaces/{id}", [jwtService](const auto& req, auto& res) {
        spdlog::info("Handling PUT /api/v1/airspaces/{{id}}");
        std::string target = std::string(req.target());

        // 提取空域ID
        std::regex id_regex(R"(/api/v1/airspaces/(\d+)(?:\?.*)?$)");
        std::smatch matches;
        if (std::regex_match(target, matches, id_regex) && matches.size() > 1) {
            std::string airspace_id = matches[1].str();
            spdlog::info("Extracted airspace ID: {}", airspace_id);

            try {
                // 解析请求体JSON
                auto json_data = nlohmann::json::parse(req.body());

                // 构建UPDATE语句
                std::string update_query = "UPDATE low_altitude_traffic_system.airspaces SET ";
                std::vector<std::string> updates;
                std::vector<mysqlx::Value> params;

                if (json_data.contains("name")) {
                    updates.push_back("name = ?");
                    params.push_back(json_data["name"].get<std::string>());
                }
                if (json_data.contains("type")) {
                    updates.push_back("type = ?");
                    params.push_back(json_data["type"].get<std::string>());
                }
                if (json_data.contains("description")) {
                    updates.push_back("description = ?");
                    params.push_back(json_data["description"].get<std::string>());
                }
                if (json_data.contains("north_lat")) {
                    updates.push_back("north_lat = ?");
                    params.push_back(json_data["north_lat"].get<double>());
                }
                if (json_data.contains("south_lat")) {
                    updates.push_back("south_lat = ?");
                    params.push_back(json_data["south_lat"].get<double>());
                }
                if (json_data.contains("east_lng")) {
                    updates.push_back("east_lng = ?");
                    params.push_back(json_data["east_lng"].get<double>());
                }
                if (json_data.contains("west_lng")) {
                    updates.push_back("west_lng = ?");
                    params.push_back(json_data["west_lng"].get<double>());
                }
                if (json_data.contains("min_altitude")) {
                    updates.push_back("min_altitude = ?");
                    params.push_back(json_data["min_altitude"].get<double>());
                }
                if (json_data.contains("max_altitude")) {
                    updates.push_back("max_altitude = ?");
                    params.push_back(json_data["max_altitude"].get<double>());
                }
                if (json_data.contains("status")) {
                    updates.push_back("status = ?");
                    params.push_back(json_data["status"].get<std::string>());
                }
                if (json_data.contains("authority")) {
                    updates.push_back("authority = ?");
                    params.push_back(json_data["authority"].get<std::string>());
                }
                if (json_data.contains("contact_info")) {
                    updates.push_back("contact_info = ?");
                    params.push_back(json_data["contact_info"].get<std::string>());
                }
                if (json_data.contains("max_concurrent_flights")) {
                    updates.push_back("max_concurrent_flights = ?");
                    params.push_back(json_data["max_concurrent_flights"].get<int>());
                }

                updates.push_back("updated_at = NOW()");

                if (updates.empty() || updates.size() == 1) {
                    nlohmann::json error_response = {
                        {"success", false},
                        {"error_code", "NO_FIELDS_TO_UPDATE"},
                        {"message", "没有可更新的字段"},
                        {"timestamp", std::time(nullptr)}
                    };

                    res.result(boost::beast::http::status::bad_request);
                    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                    res.body() = error_response.dump();
                    res.prepare_payload();
                    return;
                }

                // 拼接UPDATE语句
                for (size_t i = 0; i < updates.size(); ++i) {
                    update_query += updates[i];
                    if (i < updates.size() - 1) {
                        update_query += ", ";
                    }
                }
                update_query += " WHERE id = ?";
                params.push_back(std::stoi(airspace_id));

                auto& db_manager = database::DatabaseManager::getInstance();
                uint64_t affected_rows = db_manager.executePreparedUpdate(update_query, params);

                if (affected_rows > 0) {
                    nlohmann::json response = {
                        {"success", true},
                        {"message", "更新空域成功"},
                        {"timestamp", std::time(nullptr)},
                        {"data", {
                            {"id", std::stoi(airspace_id)},
                            {"updated_fields", json_data}
                        }}
                    };

                    res.result(boost::beast::http::status::ok);
                    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                    res.body() = response.dump();
                    res.prepare_payload();

                    spdlog::info("Successfully updated airspace ID: {}", airspace_id);
                } else {
                    nlohmann::json error_response = {
                        {"success", false},
                        {"error_code", "NOT_FOUND"},
                        {"message", "空域不存在或无更新"},
                        {"timestamp", std::time(nullptr)}
                    };

                    res.result(boost::beast::http::status::not_found);
                    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                    res.body() = error_response.dump();
                    res.prepare_payload();
                }

            } catch (const nlohmann::json::parse_error& e) {
                spdlog::error("JSON parse error in airspace update: {}", e.what());

                nlohmann::json error_response = {
                    {"success", false},
                    {"error_code", "INVALID_JSON"},
                    {"message", std::string("JSON格式错误: ") + e.what()},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::bad_request);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();

            } catch (const std::exception& e) {
                spdlog::error("Error updating airspace {}: {}", airspace_id, e.what());

                nlohmann::json error_response = {
                    {"success", false},
                    {"error_code", "INTERNAL_ERROR"},
                    {"message", "更新空域失败"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::internal_server_error);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
            }
        } else {
            spdlog::warn("Invalid airspace ID format in path: {}", target);

            nlohmann::json error_response = {
                {"success", false},
                {"error_code", "INVALID_PARAMETER"},
                {"message", "无效的空域ID格式"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::bad_request);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    // 删除空域
    server.del("/api/v1/airspaces/{id}", [jwtService](const auto& req, auto& res) {
        spdlog::info("Handling DELETE /api/v1/airspaces/{{id}}");
        std::string target = std::string(req.target());

        // 提取空域ID
        std::regex id_regex(R"(/api/v1/airspaces/(\d+)(?:\?.*)?$)");
        std::smatch matches;
        if (std::regex_match(target, matches, id_regex) && matches.size() > 1) {
            std::string airspace_id = matches[1].str();
            spdlog::info("Extracted airspace ID: {}", airspace_id);

            try {
                auto& db_manager = database::DatabaseManager::getInstance();

                // 删除空域
                std::vector<mysqlx::Value> params = {std::stoi(airspace_id)};
                uint64_t affected_rows = db_manager.executePreparedUpdate(
                    "DELETE FROM low_altitude_traffic_system.airspaces WHERE id = ?",
                    params
                );

                if (affected_rows > 0) {
                    nlohmann::json response = {
                        {"success", true},
                        {"message", "删除空域成功"},
                        {"timestamp", std::time(nullptr)},
                        {"data", {
                            {"id", std::stoi(airspace_id)},
                            {"deleted", true}
                        }}
                    };

                    res.result(boost::beast::http::status::ok);
                    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                    res.body() = response.dump();
                    res.prepare_payload();

                    spdlog::info("Successfully deleted airspace ID: {}", airspace_id);
                } else {
                    nlohmann::json error_response = {
                        {"success", false},
                        {"error_code", "NOT_FOUND"},
                        {"message", "空域不存在"},
                        {"timestamp", std::time(nullptr)}
                    };

                    res.result(boost::beast::http::status::not_found);
                    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                    res.body() = error_response.dump();
                    res.prepare_payload();
                }

            } catch (const std::exception& e) {
                spdlog::error("Error deleting airspace {}: {}", airspace_id, e.what());

                nlohmann::json error_response = {
                    {"success", false},
                    {"error_code", "INTERNAL_ERROR"},
                    {"message", "删除空域失败"},
                    {"timestamp", std::time(nullptr)}
                };

                res.result(boost::beast::http::status::internal_server_error);
                res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
                res.body() = error_response.dump();
                res.prepare_payload();
            }
        } else {
            spdlog::warn("Invalid airspace ID format in path: {}", target);

            nlohmann::json error_response = {
                {"success", false},
                {"error_code", "INVALID_PARAMETER"},
                {"message", "无效的空域ID格式"},
                {"timestamp", std::time(nullptr)}
            };

            res.result(boost::beast::http::status::bad_request);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = error_response.dump();
            res.prepare_payload();
        }
    });

    spdlog::info("API routes configured (including FlightTask, Weather, Emergency, FlightPermit, Conflict, EmergencyLanding and Airspace APIs)");
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