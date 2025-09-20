#include "database.h"
#include <cstdlib>
#include <string>

namespace config {
    AppConfig get_app_config() {
        AppConfig config;

        // 数据库配置
        const char* db_host = std::getenv("DB_HOST");
        if (db_host) config.database.host = std::string(db_host);

        const char* db_port = std::getenv("DB_PORT");
        if (db_port) config.database.port = std::stoi(db_port);

        const char* db_username = std::getenv("DB_USERNAME");
        if (db_username) config.database.username = std::string(db_username);

        const char* db_password = std::getenv("DB_PASSWORD");
        if (db_password) config.database.password = std::string(db_password);

        const char* db_name = std::getenv("DB_NAME");
        if (db_name) config.database.database = std::string(db_name);

        const char* db_pool_size = std::getenv("DB_POOL_SIZE");
        if (db_pool_size) config.database.connection_pool_size = std::stoi(db_pool_size);

        // 服务器配置
        const char* server_host = std::getenv("SERVER_HOST");
        if (server_host) config.server.host = std::string(server_host);

        const char* server_port = std::getenv("SERVER_PORT");
        if (server_port) config.server.port = std::stoi(server_port);

        const char* api_prefix = std::getenv("API_PREFIX");
        if (api_prefix) config.server.api_prefix = std::string(api_prefix);

        const char* jwt_secret = std::getenv("JWT_SECRET");
        if (jwt_secret) config.server.jwt_secret = std::string(jwt_secret);

        const char* jwt_expire = std::getenv("JWT_EXPIRE_HOURS");
        if (jwt_expire) config.server.jwt_expire_hours = std::stoi(jwt_expire);

        return config;
    }
}