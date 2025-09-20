#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "server.h"
#include "controllers/base_controller.h"
#include "controllers/user_controller.h"
#include "controllers/route_controller.h"
#include "controllers/flight_task_controller.h"
#include "controllers/WebSocketController.h"
#include "controllers/DroneController.h"
#include "controllers/AuthController.h"
#include "controllers/PaymentController.h"
#include "controllers/TaskController.h"
#include "services/user_service.h"
#include "services/route_service.h"
#include "services/flight_task_service.h"
#include "services/device_service.h"

/**
 * @brief 主函数 - 低空交通系统后端服务入口
 * 
 * 初始化服务器，注册路由，启动HTTP和WebSocket服务
 * 服务监听在8081端口，提供RESTful API和实时通信功能
 */
int main() {
    try {
        // 创建服务器实例
        auto server = std::make_shared<Server>();
        
        // 设置控制器的服务器实例
        controllers::BaseController::setServerInstance(server);
        
        // 初始化数据库连接
        if (!server->initializeDatabase()) {
            std::cerr << "数据库初始化失败" << std::endl;
            return -1;
        }
        
        // 创建服务实例
        auto user_service = std::make_shared<services::UserService>();
        auto route_service = std::make_shared<services::RouteService>();
        auto flight_task_service = std::make_shared<services::FlightTaskService>();
        auto device_service = std::make_shared<services::DeviceService>();
        
        // 创建控制器实例
        controllers::UserController user_controller(user_service);
        controllers::RouteController route_controller(route_service, flight_task_service);
        controllers::FlightTaskController flight_task_controller(flight_task_service, device_service);
        controllers::DroneController drone_controller(device_service);
        controllers::AuthController auth_controller;
        controllers::PaymentController payment_controller;
        controllers::TaskController task_controller;
        
        // 注册路由
        user_controller.setup_routes(*server);
        route_controller.setup_routes(*server);
        flight_task_controller.setup_routes(*server);
        drone_controller.setup_routes(*server);
        auth_controller.setup_routes(*server);
        payment_controller.setup_routes(*server);
        task_controller.setup_routes(*server);
        
        // 启动HTTP服务器
        int port = 8081;
        std::cout << "正在启动低空交通系统后端服务..." << std::endl;
        std::cout << "HTTP服务将在 http://localhost:" << port << " 上运行" << std::endl;
        std::cout << "按 Ctrl+C 停止服务" << std::endl;
        
        // 启动服务器
        server->listen(port);
        
    } catch (const std::exception& e) {
        std::cerr << "服务启动失败: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
