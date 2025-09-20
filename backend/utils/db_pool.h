#ifndef UTILS_DB_POOL_H
#define UTILS_DB_POOL_H

#include <cppconn/connection.h>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace utils {
    class DatabaseConnectionPool {
    private:
        std::queue<std::shared_ptr<sql::Connection>> pool_;
        std::mutex mutex_;
        std::condition_variable condition_;
        size_t max_pool_size_;
        bool shutdown_;

        std::string host_;
        int port_;
        std::string username_;
        std::string password_;
        std::string database_;

    public:
        DatabaseConnectionPool(const std::string& host, int port, const std::string& username,
                              const std::string& password, const std::string& database,
                              size_t max_pool_size = 10);

        ~DatabaseConnectionPool();

        std::shared_ptr<sql::Connection> acquire_connection();
        void release_connection(std::shared_ptr<sql::Connection> conn);

        void shutdown();

    private:
        std::shared_ptr<sql::Connection> create_connection();
        bool is_connection_valid(std::shared_ptr<sql::Connection> conn);
    };
}

#endif // UTILS_DB_POOL_H