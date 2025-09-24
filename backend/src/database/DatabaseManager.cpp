#include "DatabaseManager.h"
#include <sstream>
#include <chrono>

namespace database {

// 静态成员定义
std::unique_ptr<DatabaseManager> DatabaseManager::instance_ = nullptr;
std::mutex DatabaseManager::instance_mutex_;

// ==================== QueryResult 实现 ====================

QueryResult::QueryResult(mysqlx::RowResult result)
    : row_result_(std::move(result)), is_row_result_(true) {}

QueryResult::QueryResult(mysqlx::SqlResult result)
    : sql_result_(std::move(result)), is_row_result_(false) {}

mysqlx::Row QueryResult::fetchRow() {
    if (is_row_result_) {
        if (row_result_.count() > 0) {
            return row_result_.fetchOne();
        }
    } else {
        if (sql_result_.count() > 0) {
            return sql_result_.fetchOne();
        }
    }
    return mysqlx::Row();
}

bool QueryResult::hasData() const {
    if (is_row_result_) {
        return const_cast<mysqlx::RowResult&>(row_result_).count() > 0;
    } else {
        return const_cast<mysqlx::SqlResult&>(sql_result_).count() > 0;
    }
}

uint64_t QueryResult::getAffectedRows() const {
    if (is_row_result_) {
        return 0;  // RowResult不支持受影响行数
    } else {
        return const_cast<mysqlx::SqlResult&>(sql_result_).getAffectedItemsCount();
    }
}

uint64_t QueryResult::getAutoIncrementValue() const {
    if (is_row_result_) {
        return 0;  // RowResult不支持自增值
    } else {
        return const_cast<mysqlx::SqlResult&>(sql_result_).getAutoIncrementValue();
    }
}

nlohmann::json QueryResult::toJson() {
    nlohmann::json result = nlohmann::json::array();

    try {
        // 简化实现，避免复杂的类型转换问题
        // 这个方法主要用于调试，不是核心功能
        result.push_back(nlohmann::json::object({{"status", "data_available"}}));
    } catch (const std::exception& e) {
        spdlog::error("Error converting result to JSON: {}", e.what());
    }

    return result;
}

std::vector<std::string> QueryResult::getColumnNames() const {
    std::vector<std::string> column_names;

    try {
        if (is_row_result_) {
            auto& columns = const_cast<mysqlx::RowResult&>(row_result_).getColumns();
            for (const auto& col : columns) {
                column_names.push_back(col.getColumnName());
            }
        } else {
            auto& columns = const_cast<mysqlx::SqlResult&>(sql_result_).getColumns();
            for (const auto& col : columns) {
                column_names.push_back(col.getColumnName());
            }
        }
    } catch (const std::exception& e) {
        spdlog::error("Error getting column names: {}", e.what());
    }

    return column_names;
}

// ==================== DatabaseManager 实现 ====================

DatabaseManager& DatabaseManager::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<DatabaseManager>(new DatabaseManager());
    }
    return *instance_;
}

DatabaseManager::~DatabaseManager() {
    shutdown();
}

bool DatabaseManager::initialize(const DatabaseConfig& config) {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (initialized_) {
        spdlog::warn("DatabaseManager already initialized");
        return true;
    }

    config_ = config;

    try {
        // 建立连接
        if (!connect()) {
            spdlog::error("Failed to connect to database");
            return false;
        }

        // 初始化数据库和表
        if (!initializeDatabase()) {
            spdlog::error("Failed to initialize database");
            return false;
        }

        initialized_ = true;
        spdlog::info("DatabaseManager initialized successfully");
        return true;

    } catch (const std::exception& e) {
        spdlog::error("DatabaseManager initialization failed: {}", e.what());
        initialized_ = false;
        return false;
    }
}

bool DatabaseManager::isInitialized() const {
    return initialized_;
}

std::unique_ptr<QueryResult> DatabaseManager::executeQuery(const std::string& sql) {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized or connection invalid");
        return nullptr;
    }

    try {
        spdlog::debug("Executing query: {}", sql);
        auto result = session_->sql(sql).execute();
        return std::make_unique<QueryResult>(std::move(result));
    } catch (const std::exception& e) {
        spdlog::error("Query execution failed: {} - SQL: {}", e.what(), sql);
        return nullptr;
    }
}

uint64_t DatabaseManager::executeUpdate(const std::string& sql) {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized or connection invalid");
        return 0;
    }

    try {
        spdlog::debug("Executing update: {}", sql);
        auto result = session_->sql(sql).execute();
        return result.getAffectedItemsCount();
    } catch (const std::exception& e) {
        spdlog::error("Update execution failed: {} - SQL: {}", e.what(), sql);
        return 0;
    }
}

std::unique_ptr<QueryResult> DatabaseManager::executePreparedQuery(
    const std::string& sql,
    const std::vector<mysqlx::Value>& params) {

    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized or connection invalid");
        return nullptr;
    }

    try {
        spdlog::debug("Executing prepared query: {}", sql);
        auto stmt = session_->sql(sql);
        for (const auto& param : params) {
            stmt.bind(param);
        }
        auto result = stmt.execute();
        return std::make_unique<QueryResult>(std::move(result));
    } catch (const std::exception& e) {
        spdlog::error("Prepared query execution failed: {} - SQL: {}", e.what(), sql);
        return nullptr;
    }
}

uint64_t DatabaseManager::executePreparedUpdate(
    const std::string& sql,
    const std::vector<mysqlx::Value>& params) {

    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized or connection invalid");
        return 0;
    }

    try {
        spdlog::debug("Executing prepared update: {}", sql);
        auto stmt = session_->sql(sql);
        for (const auto& param : params) {
            stmt.bind(param);
        }
        auto result = stmt.execute();
        return result.getAffectedItemsCount();
    } catch (const std::exception& e) {
        spdlog::error("Prepared update execution failed: {} - SQL: {}", e.what(), sql);
        return 0;
    }
}

mysqlx::Table DatabaseManager::getTable(const std::string& table_name) {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !schema_) {
        throw std::runtime_error("Database not initialized");
    }

    return schema_->getTable(table_name);
}

mysqlx::Collection DatabaseManager::getCollection(const std::string& collection_name) {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !schema_) {
        throw std::runtime_error("Database not initialized");
    }

    return schema_->getCollection(collection_name);
}

uint64_t DatabaseManager::getLastInsertId() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized");
        return 0;
    }

    try {
        auto result = session_->sql("SELECT LAST_INSERT_ID()").execute();
        if (result.count() > 0) {
            auto row = result.fetchOne();
            return static_cast<uint64_t>(row[0]);
        }
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("Failed to get last insert ID: {}", e.what());
        return 0;
    }
}

bool DatabaseManager::startTransaction() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized or connection invalid");
        return false;
    }

    try {
        session_->startTransaction();
        spdlog::debug("Transaction started");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to start transaction: {}", e.what());
        return false;
    }
}

bool DatabaseManager::commitTransaction() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized or connection invalid");
        return false;
    }

    try {
        session_->commit();
        spdlog::debug("Transaction committed");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to commit transaction: {}", e.what());
        return false;
    }
}

bool DatabaseManager::rollbackTransaction() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    if (!initialized_ || !session_) {
        spdlog::error("Database not initialized or connection invalid");
        return false;
    }

    try {
        session_->rollback();
        spdlog::debug("Transaction rolled back");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to rollback transaction: {}", e.what());
        return false;
    }
}

bool DatabaseManager::testConnection() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    try {
        if (!session_) {
            return false;
        }

        // 执行简单查询测试连接
        auto result = session_->sql("SELECT 1 as test").execute();
        auto row = result.fetchOne();
        return row && static_cast<int>(row[0]) == 1;
    } catch (const std::exception& e) {
        spdlog::error("Connection test failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::reconnect() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    try {
        // 关闭现有连接
        if (session_) {
            session_->close();
        }

        // 重新连接
        if (connect()) {
            spdlog::info("Database reconnected successfully");
            return true;
        } else {
            spdlog::error("Failed to reconnect to database");
            return false;
        }
    } catch (const std::exception& e) {
        spdlog::error("Reconnection failed: {}", e.what());
        return false;
    }
}

nlohmann::json DatabaseManager::getStatus() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    nlohmann::json status;
    status["initialized"] = initialized_;

    // 检查连接有效性（不使用isConnectionValid以避免死锁）
    bool connection_valid = false;
    if (session_) {
        try {
            auto result = session_->sql("SELECT 1").execute();
            connection_valid = result.count() > 0;
        } catch (...) {
            connection_valid = false;
        }
    }
    status["connection_valid"] = connection_valid;

    status["host"] = config_.host;
    status["port"] = config_.port;
    status["database"] = config_.database;
    status["username"] = config_.username;

    if (session_) {
        try {
            auto result = session_->sql("SELECT CONNECTION_ID() as id").execute();
            auto row = result.fetchOne();
            if (row) {
                status["connection_id"] = static_cast<uint64_t>(row[0]);
            }
        } catch (...) {
            status["connection_id"] = nullptr;
        }
    }

    return status;
}

void DatabaseManager::shutdown() {
    std::lock_guard<std::mutex> lock(session_mutex_);

    try {
        if (session_) {
            session_->close();
            session_.reset();
        }

        schema_.reset();
        initialized_ = false;
        spdlog::info("DatabaseManager shutdown completed");
    } catch (const std::exception& e) {
        spdlog::error("Error during shutdown: {}", e.what());
    }
}

bool DatabaseManager::initializeDatabase() {
    try {
        // 创建数据库（如果不存在）
        std::string create_db_sql = "CREATE DATABASE IF NOT EXISTS " + config_.database +
                                   " CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
        session_->sql(create_db_sql).execute();
        spdlog::info("Database '{}' ensured to exist", config_.database);

        // 选择数据库
        schema_ = std::make_unique<mysqlx::Schema>(session_->getSchema(config_.database));

        // 创建用户表
        if (!createUserTable()) {
            return false;
        }

        // 创建飞行任务表
        if (!createFlightTaskTable()) {
            return false;
        }

        spdlog::info("Database initialization completed");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Database initialization failed: {}", e.what());
        return false;
    }
}

bool DatabaseManager::connect() {
    try {
        // 构建连接字符串
        std::stringstream conn_str;
        conn_str << "mysqlx://" << config_.username;
        if (!config_.password.empty()) {
            conn_str << ":" << config_.password;
        }
        conn_str << "@" << config_.host << ":" << config_.port;

        // 添加连接参数
        conn_str << "?connect-timeout=" << config_.connection_timeout * 1000;  // 毫秒
        if (!config_.use_ssl) {
            conn_str << "&ssl-mode=DISABLED";
        }

        spdlog::info("Connecting to database: {}://{}@{}:{}",
                    "mysqlx", config_.username, config_.host, config_.port);

        // 创建会话
        session_ = std::make_unique<mysqlx::Session>(conn_str.str());

        spdlog::info("Database connection established successfully");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Database connection failed: {}", e.what());
        session_.reset();
        return false;
    }
}

bool DatabaseManager::isConnectionValid() {
    std::lock_guard<std::mutex> lock(session_mutex_);
    try {
        if (!session_) {
            return false;
        }

        // 尝试执行简单查询
        auto result = session_->sql("SELECT 1").execute();
        return result.count() > 0;
    } catch (...) {
        return false;
    }
}

bool DatabaseManager::createUserTable() {
    try {
        std::string create_table_sql = R"(
            CREATE TABLE IF NOT EXISTS )" + config_.database + R"(.users (
                id BIGINT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) NOT NULL UNIQUE,
                email VARCHAR(100) NOT NULL UNIQUE,
                password_hash VARCHAR(255) NOT NULL,
                full_name VARCHAR(100),
                phone VARCHAR(20),
                avatar_url VARCHAR(255),
                role ENUM('admin', 'operator', 'user') DEFAULT 'user',
                status ENUM('active', 'inactive', 'banned') DEFAULT 'active',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                last_login_at TIMESTAMP NULL,
                login_attempts INT DEFAULT 0,
                locked_until TIMESTAMP NULL,

                INDEX idx_username (username),
                INDEX idx_email (email),
                INDEX idx_status (status),
                INDEX idx_created_at (created_at)
            ) ENGINE=InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
        )";

        session_->sql(create_table_sql).execute();
        spdlog::info("Users table ensured to exist");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create users table: {}", e.what());
        return false;
    }
}

bool DatabaseManager::createFlightTaskTable() {
    try {
        std::string create_table_sql = R"(
            CREATE TABLE IF NOT EXISTS )" + config_.database + R"(.api_flight_tasks (
                id INT AUTO_INCREMENT PRIMARY KEY,
                name VARCHAR(255) NOT NULL,
                description TEXT,
                route JSON,
                status ENUM('pending', 'ongoing', 'completed') DEFAULT 'pending',
                scheduled_time DATETIME,
                user_id BIGINT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
                INDEX idx_user_id (user_id),
                INDEX idx_status (status),
                INDEX idx_scheduled_time (scheduled_time),
                INDEX idx_created_at (created_at)
            ) ENGINE=InnoDB CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
        )";

        session_->sql(create_table_sql).execute();
        spdlog::info("FlightTask table ensured to exist");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create flight task table: {}", e.what());
        return false;
    }
}

// ==================== TransactionGuard 实现 ====================

TransactionGuard::TransactionGuard(DatabaseManager& db_manager)
    : db_manager_(db_manager), committed_(false), started_(false) {
    started_ = db_manager_.startTransaction();
    if (!started_) {
        spdlog::warn("Failed to start transaction in TransactionGuard");
    }
}

TransactionGuard::~TransactionGuard() {
    if (started_ && !committed_) {
        rollback();
    }
}

bool TransactionGuard::commit() {
    if (!started_) {
        spdlog::warn("Cannot commit - transaction was not started");
        return false;
    }

    if (committed_) {
        spdlog::warn("Transaction already committed");
        return true;
    }

    committed_ = db_manager_.commitTransaction();
    return committed_;
}

bool TransactionGuard::rollback() {
    if (!started_) {
        spdlog::warn("Cannot rollback - transaction was not started");
        return false;
    }

    if (committed_) {
        spdlog::warn("Cannot rollback - transaction already committed");
        return false;
    }

    return db_manager_.rollbackTransaction();
}

} // namespace database