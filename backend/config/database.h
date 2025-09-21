#ifndef CONFIG_DATABASE_H
#define CONFIG_DATABASE_H

#include <string>

namespace config {
    struct DatabaseConfig {
        std::string host = "localhost";
        int port = 33060;
        std::string username = "root";
        std::string password = "password";
        std::string database = "low_altitude_traffic_system";
        int connection_pool_size = 10;
    };

    struct ServerConfig {
        std::string host = "0.0.0.0";
        int port = 8080;
        std::string api_prefix = "/api/v1";
        std::string jwt_secret = "low_altitude_traffic_secret_key";
        int jwt_expire_hours = 24;
    };

    struct AppConfig {
        DatabaseConfig database;
        ServerConfig server;
    };

    // 获取应用配置
    AppConfig get_app_config();
}

#endif // CONFIG_DATABASE_H