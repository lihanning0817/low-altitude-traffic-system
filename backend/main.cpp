#include <iostream>
#include <memory>
#include "server.h"

int main() {
    try {
        // 创建服务器实例
        auto server = std::make_shared<Server>();

        // 初始化数据库连接
        if (!server->initializeDatabase()) {
            std::cerr << "数据库初始化失败" << std::endl;
            return -1;
        }

        // 启动HTTP服务器
        int port = 8081;
        std::cout << "正在启动低空交通系统后端服务..." << std::endl;
        std::cout << "HTTP服务将在 http://localhost:" << port << " 上运行" << std::endl;
        std::cout << "按 Ctrl+C 停止服务" << std::endl;

        // 启动服务器
        server->listen("localhost", port);

    } catch (const std::exception& e) {
        std::cerr << "服务启动失败: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}