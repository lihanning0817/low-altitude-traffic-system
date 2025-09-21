#include <iostream>
#include <string>
#include <mysql/mysql.h>

int main() {
    std::cout << "=== 基础MySQL连接测试 ===" << std::endl;

    MYSQL* conn = mysql_init(nullptr);
    if (conn == nullptr) {
        std::cerr << "mysql_init() failed" << std::endl;
        return 1;
    }

    // 连接参数
    const char* host = "localhost";
    const char* user = "root";
    const char* password = "Aabc123456+";
    const char* database = "low_altitude_traffic_system";
    unsigned int port = 3306;

    std::cout << "尝试连接到: " << host << ":" << port << "/" << database << std::endl;

    MYSQL* result = mysql_real_connect(conn, host, user, password, database, port, nullptr, 0);

    if (result == nullptr) {
        std::cerr << "连接失败: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return 1;
    }

    std::cout << "✅ MySQL连接成功!" << std::endl;
    std::cout << "服务器版本: " << mysql_get_server_info(conn) << std::endl;

    // 测试查询
    if (mysql_query(conn, "SELECT 1 as test")) {
        std::cerr << "查询失败: " << mysql_error(conn) << std::endl;
    } else {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                std::cout << "✅ 查询测试成功，结果: " << row[0] << std::endl;
            }
            mysql_free_result(res);
        }
    }

    // 检查X Plugin是否安装
    std::cout << "\n检查X Plugin状态..." << std::endl;
    if (mysql_query(conn, "SHOW PLUGINS LIKE 'mysqlx'")) {
        std::cerr << "无法查询插件: " << mysql_error(conn) << std::endl;
    } else {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            if (mysql_num_rows(res) > 0) {
                MYSQL_ROW row = mysql_fetch_row(res);
                std::cout << "X Plugin状态: " << row[1] << std::endl;
            } else {
                std::cout << "❌ X Plugin未安装" << std::endl;
            }
            mysql_free_result(res);
        }
    }

    mysql_close(conn);
    return 0;
}