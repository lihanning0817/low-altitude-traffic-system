#include "db_pool.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <iostream>
#include <sstream>

namespace utils {
    DatabaseConnectionPool::DatabaseConnectionPool(const std::string& host, int port, const std::string& username,
                                                 const std::string& password, const std::string& database,
                                                 size_t max_pool_size)
        : max_pool_size_(max_pool_size), shutdown_(false), host_(host), port_(port),
          username_(username), password_(password), database_(database) {

        // Initialize the pool with connections
        sql::Driver* driver = sql::mysql::get_mysql_driver_instance();

        for (size_t i = 0; i < max_pool_size_; ++i) {
            try {
                std::shared_ptr<sql::Connection> conn(create_connection());
                if (conn) {
                    pool_.push(conn);
                }
            } catch (const sql::SQLException& e) {
                std::cerr << "Failed to create initial connection: " << e.what() << std::endl;
            }
        }
    }

    DatabaseConnectionPool::~DatabaseConnectionPool() {
        shutdown();
    }

    std::shared_ptr<sql::Connection> DatabaseConnectionPool::acquire_connection() {
        std::unique_lock<std::mutex> lock(mutex_);

        while (pool_.empty() && !shutdown_) {
            condition_.wait(lock);
        }

        if (shutdown_) {
            return nullptr;
        }

        std::shared_ptr<sql::Connection> conn = pool_.front();
        pool_.pop();

        // Check if connection is still valid
        if (!is_connection_valid(conn)) {
            // Create a new connection if the current one is invalid
            try {
                conn = create_connection();
            } catch (const sql::SQLException& e) {
                std::cerr << "Failed to create new connection: " << e.what() << std::endl;
                return nullptr;
            }
        }

        return conn;
    }

    void DatabaseConnectionPool::release_connection(std::shared_ptr<sql::Connection> conn) {
        if (!conn) return;

        std::lock_guard<std::mutex> lock(mutex_);

        if (pool_.size() < max_pool_size_ && is_connection_valid(conn)) {
            pool_.push(conn);
            condition_.notify_one();
        }
        // If pool is full or connection is invalid, it will be destroyed automatically
    }

    void DatabaseConnectionPool::shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        // Clear the pool
        while (!pool_.empty()) {
            pool_.pop();
        }
        condition_.notify_all();
    }

    std::shared_ptr<sql::Connection> DatabaseConnectionPool::create_connection() {
        sql::Driver* driver = sql::mysql::get_mysql_driver_instance();

        std::ostringstream oss;
        oss << "tcp://" << host_ << ":" << port_;

        std::shared_ptr<sql::Connection> conn(driver->connect(oss.str(), username_, password_));
        conn->setSchema(database_);

        return conn;
    }

    bool DatabaseConnectionPool::is_connection_valid(std::shared_ptr<sql::Connection> conn) {
        if (!conn) return false;

        try {
            return !conn->isClosed();
        } catch (const sql::SQLException& e) {
            std::cerr << "Connection validation failed: " << e.what() << std::endl;
            return false;
        }
    }
}