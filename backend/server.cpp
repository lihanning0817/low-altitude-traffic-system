#include "server.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>
// #include <mysql/psi/mysql_thread.h>  // 暂时注释掉不需要的头文件

Server::Server() {
    // 加载数据库配置
    loadDatabaseConfig();

    // 初始化高德路线规划服务
    route_service_ = std::make_unique<services::AmapRouteService>("1872806f332dab32a1a3dc895b0ad542");

    // 设置CORS头，允许跨域请求
    this->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });

    // 处理预检请求
    this->Options(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status = 200;
    });

    // 添加基础API端点
    setupBasicRoutes();
}

Server::~Server() {
    closeDatabase();
}

bool Server::initializeDatabase() {
    try {
        // 初始化MySQL连接
        mysql = mysql_init(nullptr);
        if (!mysql) {
            std::cerr << "MySQL初始化失败" << std::endl;
            return false;
        }
        
        // 设置连接选项
        // 注意：MYSQL_OPT_RECONNECT 已弃用，现代应用应手动处理重连
        
        // 建立连接
        if (!mysql_real_connect(mysql, dbHost.c_str(), dbUser.c_str(), 
                               dbPassword.c_str(), dbName.c_str(), 
                               dbPort, nullptr, 0)) {
            std::cerr << "数据库连接失败: " << mysql_error(mysql) << std::endl;
            mysql_close(mysql);
            mysql = nullptr;
            return false;
        }
        
        // 设置字符集
        if (mysql_set_character_set(mysql, "utf8mb4")) {
            std::cerr << "设置字符集失败: " << mysql_error(mysql) << std::endl;
            closeDatabase();
            return false;
        }
        
        std::cout << "数据库连接成功" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "初始化数据库连接时发生异常: " << e.what() << std::endl;
        return false;
    }
}

MYSQL_RES* Server::executeQuery(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);

    // 检查连接状态，如果断开则尝试重连
    if (!isDatabaseConnected()) {
        std::cout << "数据库连接断开，尝试重连..." << std::endl;
        if (!reconnectDatabase()) {
            std::cerr << "数据库重连失败" << std::endl;
            return nullptr;
        }
    }

    // 执行查询
    if (mysql_query(mysql, sql.c_str())) {
        unsigned int error_code = mysql_errno(mysql);

        // 检查是否是连接相关错误
        if (error_code == CR_SERVER_GONE_ERROR || error_code == CR_SERVER_LOST) {
            std::cout << "检测到连接错误，尝试重连后重试..." << std::endl;
            if (reconnectDatabase() && mysql_query(mysql, sql.c_str()) == 0) {
                // 重连成功，继续执行
            } else {
                std::cerr << "查询执行失败: " << mysql_error(mysql) << std::endl;
                std::cerr << "SQL: " << sql << std::endl;
                return nullptr;
            }
        } else {
            std::cerr << "查询执行失败: " << mysql_error(mysql) << std::endl;
            std::cerr << "SQL: " << sql << std::endl;
            return nullptr;
        }
    }

    // 获取结果集
    MYSQL_RES* result = mysql_store_result(mysql);
    if (!result) {
        std::cerr << "获取结果集失败: " << mysql_error(mysql) << std::endl;
        return nullptr;
    }

    return result;
}

int Server::executeUpdate(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);

    // 检查连接状态，如果断开则尝试重连
    if (!isDatabaseConnected()) {
        std::cout << "数据库连接断开，尝试重连..." << std::endl;
        if (!reconnectDatabase()) {
            std::cerr << "数据库重连失败" << std::endl;
            return -1;
        }
    }

    // 执行更新
    if (mysql_query(mysql, sql.c_str())) {
        unsigned int error_code = mysql_errno(mysql);

        // 检查是否是连接相关错误
        if (error_code == CR_SERVER_GONE_ERROR || error_code == CR_SERVER_LOST) {
            std::cout << "检测到连接错误，尝试重连后重试..." << std::endl;
            if (reconnectDatabase() && mysql_query(mysql, sql.c_str()) == 0) {
                // 重连成功，继续执行
            } else {
                std::cerr << "更新执行失败: " << mysql_error(mysql) << std::endl;
                std::cerr << "SQL: " << sql << std::endl;
                return -1;
            }
        } else {
            std::cerr << "更新执行失败: " << mysql_error(mysql) << std::endl;
            std::cerr << "SQL: " << sql << std::endl;
            return -1;
        }
    }

    // 返回影响的行数
    return static_cast<int>(mysql_affected_rows(mysql));
}

void Server::closeDatabase() {
    if (mysql) {
        mysql_close(mysql);
        mysql = nullptr;
        std::cout << "数据库连接已关闭" << std::endl;
    }
}

bool Server::isDatabaseConnected() {
    if (!mysql) {
        return false;
    }

    // 使用 mysql_ping 检查连接是否有效
    if (mysql_ping(mysql) != 0) {
        std::cerr << "数据库连接已断开: " << mysql_error(mysql) << std::endl;
        return false;
    }

    return true;
}

bool Server::reconnectDatabase() {
    std::cout << "尝试重新连接数据库..." << std::endl;

    // 关闭现有连接
    closeDatabase();

    // 重新初始化连接
    return initializeDatabase();
}

void Server::setupBasicRoutes() {
    // API状态检查端点
    this->Get("/api/v1/health", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\",\"message\":\"Low Altitude Traffic System API is running\"}", "application/json");
    });

    // 获取系统信息
    this->Get("/api/v1/info", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("{\"name\":\"Low Altitude Traffic System\",\"version\":\"1.0.0\",\"description\":\"低空交通管理系统后端API\"}", "application/json");
    });

    // 路线规划API
    this->Get("/api/route", [this](const httplib::Request& req, httplib::Response& res) {
        handleRouteRequest(req, res);
    });

    // 基础用户API（模拟数据）
    this->Get("/api/v1/users", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("{\"users\":[{\"id\":1,\"username\":\"admin\",\"role\":\"admin\"},{\"id\":2,\"username\":\"user1\",\"role\":\"user\"}]}", "application/json");
    });

    // 无人机状态API（模拟数据）
    this->Get("/api/v1/drones", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("{\"drones\":[{\"id\":\"UAV001\",\"status\":\"available\",\"battery\":85,\"location\":{\"lat\":39.9042,\"lng\":116.4074}},{\"id\":\"UAV002\",\"status\":\"busy\",\"battery\":72,\"location\":{\"lat\":39.9142,\"lng\":116.4174}}]}", "application/json");
    });

    // 任务API（模拟数据）
    this->Get("/api/v1/tasks", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("{\"tasks\":[{\"id\":1,\"type\":\"logistics\",\"status\":\"pending\",\"start\":{\"lat\":39.9042,\"lng\":116.4074},\"end\":{\"lat\":39.9142,\"lng\":116.4174}},{\"id\":2,\"type\":\"inspection\",\"status\":\"in_progress\",\"start\":{\"lat\":39.8942,\"lng\":116.3974},\"end\":{\"lat\":39.9242,\"lng\":116.4274}}]}", "application/json");
    });

    // 用户登录API（模拟数据）
    this->Post("/api/v1/users/login", [](const httplib::Request& req, httplib::Response& res) {
        // 简单的模拟登录验证
        std::string username, password;

        // 解析JSON请求体（简单解析）
        std::string body = req.body;
        size_t usernamePos = body.find("\"username\":");
        size_t passwordPos = body.find("\"password\":");

        if (usernamePos != std::string::npos && passwordPos != std::string::npos) {
            // 提取用户名
            size_t usernameStart = body.find("\"", usernamePos + 11) + 1;
            size_t usernameEnd = body.find("\"", usernameStart);
            username = body.substr(usernameStart, usernameEnd - usernameStart);

            // 提取密码
            size_t passwordStart = body.find("\"", passwordPos + 11) + 1;
            size_t passwordEnd = body.find("\"", passwordStart);
            password = body.substr(passwordStart, passwordEnd - passwordStart);
        }

        // 验证凭据（简单的硬编码验证）
        if ((username == "admin" && password == "admin123") ||
            (username == "user1" && password == "user123")) {

            std::string role = (username == "admin") ? "admin" : "user";
            std::string responseBody = "{\"success\":true,\"data\":{\"user\":{\"id\":"
                + std::to_string((username == "admin") ? 1 : 2) +
                ",\"username\":\"" + username + "\",\"role\":\"" + role + "\"},"
                "\"token\":\"mock_jwt_token_" + username + "\"}}";

            res.set_content(responseBody, "application/json");
        } else {
            res.status = 401;
            res.set_content("{\"success\":false,\"error\":\"用户名或密码错误\"}", "application/json");
        }
    });
}

void Server::loadDatabaseConfig() {
    // 尝试从.env文件加载配置
    std::ifstream envFile(".env");
    if (envFile.is_open()) {
        std::string line;
        while (std::getline(envFile, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "DB_HOST") {
                    dbHost = value;
                } else if (key == "DB_PORT") {
                    dbPort = std::stoi(value);
                } else if (key == "DB_USERNAME") {
                    dbUser = value;
                } else if (key == "DB_PASSWORD") {
                    dbPassword = value;
                } else if (key == "DB_NAME") {
                    dbName = value;
                }
            }
        }
        envFile.close();
        std::cout << "已从.env文件加载数据库配置" << std::endl;
    } else {
        // 尝试从环境变量加载
        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");
        const char* user = std::getenv("DB_USERNAME");
        const char* password = std::getenv("DB_PASSWORD");
        const char* name = std::getenv("DB_NAME");
        
        if (host) dbHost = host;
        if (port) dbPort = std::stoi(port);
        if (user) dbUser = user;
        if (password) dbPassword = password;
        if (name) dbName = name;
        
        std::cout << "已从环境变量加载数据库配置" << std::endl;
    }
    
    std::cout << "数据库配置: " << dbHost << ":" << dbPort << "/" << dbName << " (用户: " << dbUser << ")" << std::endl;
}

void Server::handleRouteRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取请求参数
        std::string origin = req.get_param_value("origin");
        std::string destination = req.get_param_value("destination");
        std::string strategy_str = req.get_param_value("strategy");

        if (origin.empty() || destination.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"缺少origin或destination参数\"}", "application/json");
            return;
        }

        // 解析起点坐标
        size_t comma_pos = origin.find(',');
        if (comma_pos == std::string::npos) {
            res.status = 400;
            res.set_content("{\"error\":\"origin格式错误，应为 lng,lat\"}", "application/json");
            return;
        }
        double origin_lng = std::stod(origin.substr(0, comma_pos));
        double origin_lat = std::stod(origin.substr(comma_pos + 1));

        // 解析终点坐标
        comma_pos = destination.find(',');
        if (comma_pos == std::string::npos) {
            res.status = 400;
            res.set_content("{\"error\":\"destination格式错误，应为 lng,lat\"}", "application/json");
            return;
        }
        double dest_lng = std::stod(destination.substr(0, comma_pos));
        double dest_lat = std::stod(destination.substr(comma_pos + 1));

        // 解析策略参数
        int strategy = 0;  // 默认最快路线
        if (!strategy_str.empty()) {
            strategy = std::stoi(strategy_str);
        }

        std::cout << "路线规划请求: " << origin << " -> " << destination << ", 策略: " << strategy << std::endl;

        // 调用高德路线规划服务
        auto route_result = route_service_->planRoute(origin_lng, origin_lat, dest_lng, dest_lat, strategy);

        if (route_result.has_value()) {
            // 构建成功响应
            std::ostringstream json_response;
            json_response << "{"
                         << "\"success\":true,"
                         << "\"data\":{"
                         << "\"distance\":\"" << route_result->formatted_distance << "\","
                         << "\"duration\":\"" << route_result->formatted_duration << "\","
                         << "\"total_distance\":" << route_result->total_distance << ","
                         << "\"total_duration\":" << route_result->total_duration << ","
                         << "\"coordinates\":[";

            // 添加坐标点数组
            for (size_t i = 0; i < route_result->overview_polyline.size(); ++i) {
                if (i > 0) json_response << ",";
                json_response << "[" << std::fixed << std::setprecision(6)
                             << route_result->overview_polyline[i].lng << ","
                             << route_result->overview_polyline[i].lat << "]";
            }

            json_response << "],"
                         << "\"steps\":[";

            // 添加路线步骤
            for (size_t i = 0; i < route_result->steps.size(); ++i) {
                if (i > 0) json_response << ",";
                const auto& step = route_result->steps[i];
                json_response << "{"
                             << "\"instruction\":\"" << step.instruction << "\","
                             << "\"road_name\":\"" << step.road_name << "\","
                             << "\"distance\":" << step.distance << ","
                             << "\"duration\":" << step.duration
                             << "}";
            }

            json_response << "]"
                         << "}"
                         << "}";

            res.set_content(json_response.str(), "application/json");
            std::cout << "路线规划成功返回" << std::endl;
        } else {
            res.status = 500;
            res.set_content("{\"error\":\"路线规划失败，请检查KEY或参数\"}", "application/json");
            std::cout << "路线规划失败" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "处理路线规划请求异常: " << e.what() << std::endl;
        res.status = 500;
        res.set_content("{\"error\":\"服务器内部错误\"}", "application/json");
    }
}
