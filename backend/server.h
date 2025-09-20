#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <queue>

#include "httplib.h"
#include <mysql/mysql.h>
#include "services/amap_route_service.h"

/**
 * @brief HTTP服务器包装类
 * 
 * 封装 httplib::Server，提供数据库连接管理和基本服务功能
 */
class Server : public httplib::Server {
private:
    MYSQL* mysql;
    std::mutex dbMutex;
    std::unique_ptr<services::AmapRouteService> route_service_;

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
    int dbPort = 3306;
    
    /**
     * @brief 从环境变量加载数据库配置
     */
    void loadDatabaseConfig();

    /**
     * @brief 设置基础API路由
     */
    void setupBasicRoutes();

    /**
     * @brief 处理路线规划请求
     */
    void handleRouteRequest(const httplib::Request& req, httplib::Response& res);
};
