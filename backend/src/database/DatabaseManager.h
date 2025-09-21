#pragma once

#include <mysqlx/xdevapi.h>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace database {

/**
 * @brief 数据库连接配置
 */
struct DatabaseConfig {
    std::string host = "localhost";
    int port = 33060;  // X Protocol默认端口
    std::string database = "low_altitude_traffic_system";
    std::string username = "root";
    std::string password = "";
    int connection_timeout = 10;
    bool use_ssl = false;
};

/**
 * @brief X DevAPI查询结果包装器
 * 提供统一的结果访问接口
 */
class QueryResult {
private:
    mysqlx::RowResult row_result_;
    mysqlx::SqlResult sql_result_;
    bool is_row_result_;

public:
    /**
     * @brief 构造函数 - RowResult
     */
    explicit QueryResult(mysqlx::RowResult result);

    /**
     * @brief 构造函数 - SqlResult
     */
    explicit QueryResult(mysqlx::SqlResult result);

    /**
     * @brief 获取下一行数据
     * @return 如果有数据返回Row，否则返回空
     */
    mysqlx::Row fetchRow();

    /**
     * @brief 检查是否还有更多行
     */
    bool hasData() const;

    /**
     * @brief 获取受影响的行数（仅对UPDATE/INSERT/DELETE有效）
     */
    uint64_t getAffectedRows() const;

    /**
     * @brief 获取插入的ID（仅对INSERT有效）
     */
    uint64_t getAutoIncrementValue() const;

    /**
     * @brief 转换为JSON数组
     */
    nlohmann::json toJson();

    /**
     * @brief 获取列名列表
     * @return 列名向量
     */
    std::vector<std::string> getColumnNames() const;
};

/**
 * @brief 数据库管理器 - 单例模式
 * 基于MySQL X DevAPI实现的现代数据库接口
 */
class DatabaseManager {
private:
    static std::unique_ptr<DatabaseManager> instance_;
    static std::mutex instance_mutex_;

    std::unique_ptr<mysqlx::Session> session_;
    std::unique_ptr<mysqlx::Schema> schema_;
    DatabaseConfig config_;
    std::mutex session_mutex_;
    bool initialized_;

    DatabaseManager() : initialized_(false) {}

public:
    // 禁用拷贝构造和赋值
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /**
     * @brief 获取数据库管理器单例实例
     */
    static DatabaseManager& getInstance();

    /**
     * @brief 析构函数
     */
    ~DatabaseManager();

    /**
     * @brief 初始化数据库连接
     * @param config 数据库配置
     * @return 是否初始化成功
     */
    bool initialize(const DatabaseConfig& config);

    /**
     * @brief 检查是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 执行SQL查询
     * @param sql SQL语句
     * @return 查询结果
     */
    std::unique_ptr<QueryResult> executeQuery(const std::string& sql);

    /**
     * @brief 执行SQL更新语句（INSERT, UPDATE, DELETE）
     * @param sql SQL语句
     * @return 受影响的行数
     */
    uint64_t executeUpdate(const std::string& sql);

    /**
     * @brief 执行预编译SQL查询
     * @param sql SQL语句（带?占位符）
     * @param params 参数列表
     * @return 查询结果
     */
    std::unique_ptr<QueryResult> executePreparedQuery(
        const std::string& sql,
        const std::vector<mysqlx::Value>& params
    );

    /**
     * @brief 执行预编译SQL更新
     * @param sql SQL语句（带?占位符）
     * @param params 参数列表
     * @return 受影响的行数
     */
    uint64_t executePreparedUpdate(
        const std::string& sql,
        const std::vector<mysqlx::Value>& params
    );

    /**
     * @brief 获取表对象
     * @param table_name 表名
     * @return 表对象
     */
    mysqlx::Table getTable(const std::string& table_name);

    /**
     * @brief 获取集合对象（文档存储）
     * @param collection_name 集合名
     * @return 集合对象
     */
    mysqlx::Collection getCollection(const std::string& collection_name);

    /**
     * @brief 获取最后插入的自增ID
     * @return 最后插入的ID，如果没有插入操作返回0
     */
    uint64_t getLastInsertId();

    /**
     * @brief 开始事务
     * @return 是否成功
     */
    bool startTransaction();

    /**
     * @brief 提交事务
     * @return 是否成功
     */
    bool commitTransaction();

    /**
     * @brief 回滚事务
     * @return 是否成功
     */
    bool rollbackTransaction();

    /**
     * @brief 测试数据库连接
     * @return 是否连接成功
     */
    bool testConnection();

    /**
     * @brief 重新连接数据库
     * @return 是否重连成功
     */
    bool reconnect();

    /**
     * @brief 获取连接状态
     * @return 状态JSON
     */
    nlohmann::json getStatus();

    /**
     * @brief 关闭数据库连接
     */
    void shutdown();

    /**
     * @brief 创建数据库和表（如果不存在）
     * @return 是否成功
     */
    bool initializeDatabase();

private:
    /**
     * @brief 建立数据库连接
     * @return 是否成功
     */
    bool connect();

    /**
     * @brief 检查连接是否有效
     * @return 是否有效
     */
    bool isConnectionValid();

    /**
     * @brief 创建用户表
     * @return 是否成功
     */
    bool createUserTable();
};

/**
 * @brief 数据库事务RAII包装器
 * 自动管理事务的开始、提交和回滚
 */
class TransactionGuard {
private:
    DatabaseManager& db_manager_;
    bool committed_;
    bool started_;

public:
    /**
     * @brief 构造函数，开始事务
     * @param db_manager 数据库管理器
     */
    explicit TransactionGuard(DatabaseManager& db_manager);

    /**
     * @brief 析构函数，如果未提交则回滚
     */
    ~TransactionGuard();

    /**
     * @brief 提交事务
     * @return 是否成功
     */
    bool commit();

    /**
     * @brief 回滚事务
     * @return 是否成功
     */
    bool rollback();

    /**
     * @brief 检查事务是否已开始
     */
    bool isStarted() const { return started_; }

    // 禁用拷贝构造和赋值
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
};

} // namespace database