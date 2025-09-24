#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <queue>

#include "crow.h"
#include <mysql/mysql.h>
#include "services/amap_route_service.h"

/**
 * @brief Crow HTTP服务器包装类
 *
 * 封装 Crow 应用，提供数据库连接管理和基本服务功能
 */
class Server {
private:
    MYSQL* mysql;
    std::mutex dbMutex;
    std::unique_ptr<services::AmapRouteService> route_service_;
    crow::SimpleApp app;

public:
    /**
     * @brief 构造函数
     */
    Server();

    /**
     * @brief 析构函数
     */
    ~Server();

    /**
     * @brief 初始化数据库连接
     * @return 成功返回true，失败返回false
     */
    bool initializeDatabase();

    /**
     * @brief 执行SQL查询
     * @param sql SQL语句
     * @return 结果集指针
     */
    MYSQL_RES* executeQuery(const std::string& sql);

    /**
     * @brief 执行SQL更新
     * @param sql SQL语句
     * @return 影响的行数
     */
    int executeUpdate(const std::string& sql);

    /**
     * @brief 关闭数据库连接池
     */
    void closeDatabase();

    /**
     * @brief 检查数据库连接是否有效
     * @return 连接有效返回true
     */
    bool isDatabaseConnected();

    /**
     * @brief 重新连接数据库
     * @return 重连成功返回true
     */
    bool reconnectDatabase();

    // 数据库配置
    std::string dbHost = "localhost";
    std::string dbUser = "lat_user";
    std::string dbPassword = "lat_password";
    std::string dbName = "low_altitude_traffic";
    int dbPort = 33060;

    /**
     * @brief 从环境变量加载数据库配置
     */
    void loadDatabaseConfig();

    /**
     * @brief 设置API路由
     */
    void setupRoutes();

    /**
     * @brief 启动服务器
     * @param host 主机地址
     * @param port 端口号
     */
    void listen(const std::string& host, int port);

    /**
     * @brief 处理路线规划请求
     */
    crow::response handleRouteRequest(const crow::request& req);

    /**
     * @brief 处理标记点管理请求
     */
    crow::response handleMarkersRequest(const crow::request& req);

    /**
     * @brief 处理获取标记点请求
     */
    crow::response handleGetMarkersRequest();

    /**
     * @brief 处理删除所有标记点请求
     */
    crow::response handleDeleteMarkersRequest();
};
