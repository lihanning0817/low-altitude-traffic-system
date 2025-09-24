#include "server.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>

Server::Server() {
    // 加载数据库配置
    loadDatabaseConfig();

    // 初始化高德路线规划服务
    route_service_ = std::make_unique<services::AmapRouteService>("1872806f332dab32a1a3dc895b0ad542");

    // 设置API路由
    setupRoutes();
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

    return mysql_store_result(mysql);
}

int Server::executeUpdate(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);

    // 检查连接状态
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

    return mysql_affected_rows(mysql);
}

void Server::closeDatabase() {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (mysql) {
        mysql_close(mysql);
        mysql = nullptr;
    }
}

bool Server::isDatabaseConnected() {
    if (!mysql) {
        return false;
    }

    // 使用 mysql_ping 检查连接状态
    return mysql_ping(mysql) == 0;
}

bool Server::reconnectDatabase() {
    closeDatabase();
    return initializeDatabase();
}

void Server::loadDatabaseConfig() {
    // 从环境变量读取数据库配置
    if (const char* host = std::getenv("DB_HOST")) {
        dbHost = host;
    }
    if (const char* user = std::getenv("DB_USER")) {
        dbUser = user;
    }
    if (const char* password = std::getenv("DB_PASSWORD")) {
        dbPassword = password;
    }
    if (const char* name = std::getenv("DB_NAME")) {
        dbName = name;
    }
    if (const char* port = std::getenv("DB_PORT")) {
        dbPort = std::stoi(port);
    }

    std::cout << "数据库配置:" << std::endl;
    std::cout << "  主机: " << dbHost << std::endl;
    std::cout << "  端口: " << dbPort << std::endl;
    std::cout << "  数据库: " << dbName << std::endl;
    std::cout << "  用户: " << dbUser << std::endl;
}

void Server::setupRoutes() {
    // 设置 CORS 中间件
    CROW_ROUTE(app, "/").methods("OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response res(200);
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return res;
    });

    // 健康检查端点
    CROW_ROUTE(app, "/health")
    ([this](const crow::request& req) {
        crow::response res(200);
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");
        res.write("{\"status\":\"ok\",\"timestamp\":\"" + std::to_string(time(nullptr)) + "\"}");
        return res;
    });

    // 路线规划端点
    CROW_ROUTE(app, "/api/route").methods("GET"_method)
    ([this](const crow::request& req) {
        return handleRouteRequest(req);
    });

    // 标记点管理端点
    CROW_ROUTE(app, "/api/markers").methods("POST"_method, "GET"_method, "DELETE"_method)
    ([this](const crow::request& req) {
        if (req.method == "GET"_method) {
            return handleGetMarkersRequest();
        } else if (req.method == "DELETE"_method) {
            return handleDeleteMarkersRequest();
        } else {
            return handleMarkersRequest(req);
        }
    });

    // 注意：Crow 的 CORS 中间件可能需要特殊配置
    // 如果编译时出现问题，可以暂时注释这部分，使用手动设置 CORS 头
}

void Server::listen(const std::string& host, int port) {
    app.port(port).multithreaded().run();
}

crow::response Server::handleRouteRequest(const crow::request& req) {
    crow::response res;
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Content-Type", "application/json");

    try {
        // 解析查询参数
        auto params = req.url_params;
        std::string origin = params.get("origin") ? params.get("origin") : "";
        std::string destination = params.get("destination") ? params.get("destination") : "";
        std::string strategy = params.get("strategy") ? params.get("strategy") : "0";

        if (origin.empty() || destination.empty()) {
            res.code = 400;
            res.write("{\"success\":false,\"error\":\"缺少必需参数 origin 或 destination\"}");
            return res;
        }

        // 调用高德地图路线规划服务
        if (route_service_) {
            std::string route_result = route_service_->planRoute(origin, destination, strategy);
            res.write(route_result);
        } else {
            res.code = 500;
            res.write("{\"success\":false,\"error\":\"路线规划服务未初始化\"}");
        }

    } catch (const std::exception& e) {
        res.code = 500;
        res.write("{\"success\":false,\"error\":\"服务器内部错误: " + std::string(e.what()) + "\"}");
    }

    return res;
}

crow::response Server::handleMarkersRequest(const crow::request& req) {
    crow::response res;
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Content-Type", "application/json");

    try {
        // 解析JSON请求体
        auto json_data = crow::json::load(req.body);
        if (!json_data) {
            res.code = 400;
            res.write("{\"success\":false,\"error\":\"无效的JSON格式\"}");
            return res;
        }

        double lng = json_data["lng"].d();
        double lat = json_data["lat"].d();
        std::string title = json_data["title"].s();
        std::string type = json_data.has("type") ? json_data["type"].s() : "marker";

        // 插入标记点到数据库
        std::stringstream sql;
        sql << "INSERT INTO map_markers (lng, lat, title, type, created_at) VALUES ("
            << std::fixed << std::setprecision(6) << lng << ", "
            << std::fixed << std::setprecision(6) << lat << ", '"
            << title << "', '" << type << "', NOW())";

        int affected_rows = executeUpdate(sql.str());
        if (affected_rows > 0) {
            res.write("{\"success\":true,\"message\":\"标记点添加成功\"}");
        } else {
            res.code = 500;
            res.write("{\"success\":false,\"error\":\"标记点添加失败\"}");
        }

    } catch (const std::exception& e) {
        res.code = 500;
        res.write("{\"success\":false,\"error\":\"服务器内部错误: " + std::string(e.what()) + "\"}");
    }

    return res;
}

crow::response Server::handleGetMarkersRequest() {
    crow::response res;
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Content-Type", "application/json");

    try {
        MYSQL_RES* result = executeQuery("SELECT id, lng, lat, title, type, created_at FROM map_markers ORDER BY created_at DESC");

        if (!result) {
            res.code = 500;
            res.write("{\"success\":false,\"error\":\"查询失败\"}");
            return res;
        }

        std::stringstream json;
        json << "{\"success\":true,\"data\":[";

        MYSQL_ROW row;
        bool first = true;
        while ((row = mysql_fetch_row(result))) {
            if (!first) json << ",";
            json << "{"
                 << "\"id\":" << row[0] << ","
                 << "\"lng\":" << row[1] << ","
                 << "\"lat\":" << row[2] << ","
                 << "\"title\":\"" << (row[3] ? row[3] : "") << "\","
                 << "\"type\":\"" << (row[4] ? row[4] : "marker") << "\","
                 << "\"created_at\":\"" << (row[5] ? row[5] : "") << "\""
                 << "}";
            first = false;
        }

        json << "]}";
        mysql_free_result(result);

        res.write(json.str());

    } catch (const std::exception& e) {
        res.code = 500;
        res.write("{\"success\":false,\"error\":\"服务器内部错误: " + std::string(e.what()) + "\"}");
    }

    return res;
}

crow::response Server::handleDeleteMarkersRequest() {
    crow::response res;
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Content-Type", "application/json");

    try {
        int affected_rows = executeUpdate("DELETE FROM map_markers");
        res.write("{\"success\":true,\"message\":\"已清除 " + std::to_string(affected_rows) + " 个标记点\"}");

    } catch (const std::exception& e) {
        res.code = 500;
        res.write("{\"success\":false,\"error\":\"服务器内部错误: " + std::string(e.what()) + "\"}");
    }

    return res;
}