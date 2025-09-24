// FlightTask路由配置代码片段
// 需要在main.cpp的setupRoutes函数中添加的代码

// 在setupRoutes函数的开头，初始化FlightTask相关的服务
void setupRoutes(server::HttpServer& server) {
    // 现有的认证控制器初始化
    auto userRepo = std::make_shared<repositories::UserRepository>();
    auto jwtService = std::make_shared<auth::JwtService>(auth::JwtService::fromConfig());
    auto authController = std::make_shared<controllers::AuthController>(userRepo, jwtService);

    // ========== 新增：FlightTask控制器初始化 ==========
    auto taskRepo = std::make_shared<repositories::FlightTaskRepository>();
    auto taskController = std::make_shared<controllers::FlightTaskController>(taskRepo, jwtService);

    // ========== 现有的认证相关API保持不变 ==========
    // ... (现有的认证路由代码保持不变)

    // ========== 新增：飞行任务管理API ==========

    // 获取飞行任务列表
    server.get("/api/v1/flight-tasks", [taskController](const auto& req, auto& res) {
        auto response = taskController->getFlightTasks(req);
        res = std::move(response);
    });

    // 创建飞行任务
    server.post("/api/v1/flight-tasks", [taskController](const auto& req, auto& res) {
        auto response = taskController->createFlightTask(req);
        res = std::move(response);
    });

    // 获取单个飞行任务详情
    server.get("/api/v1/flight-tasks/{id}", [taskController](const auto& req, auto& res) {
        // 从URL路径中提取任务ID
        std::string target = std::string(req.target());
        std::regex path_regex(R"(/api/v1/flight-tasks/(\d+))");
        std::smatch matches;

        if (std::regex_match(target, matches, path_regex) && matches.size() > 1) {
            std::string task_id = matches[1].str();
            auto response = taskController->getFlightTaskById(req, task_id);
            res = std::move(response);
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的任务ID");
        }
    });

    // 更新飞行任务
    server.put("/api/v1/flight-tasks/{id}", [taskController](const auto& req, auto& res) {
        // 从URL路径中提取任务ID
        std::string target = std::string(req.target());
        std::regex path_regex(R"(/api/v1/flight-tasks/(\d+))");
        std::smatch matches;

        if (std::regex_match(target, matches, path_regex) && matches.size() > 1) {
            std::string task_id = matches[1].str();
            auto response = taskController->updateFlightTask(req, task_id);
            res = std::move(response);
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的任务ID");
        }
    });

    // 删除飞行任务
    server.del("/api/v1/flight-tasks/{id}", [taskController](const auto& req, auto& res) {
        // 从URL路径中提取任务ID
        std::string target = std::string(req.target());
        std::regex path_regex(R"(/api/v1/flight-tasks/(\d+))");
        std::smatch matches;

        if (std::regex_match(target, matches, path_regex) && matches.size() > 1) {
            std::string task_id = matches[1].str();
            auto response = taskController->deleteFlightTask(req, task_id);
            res = std::move(response);
        } else {
            res = utils::HttpResponse::createErrorResponse("无效的任务ID");
        }
    });

    // ========== 现有的其他API保持不变 ==========
    // ... (现有的健康检查、系统信息等API代码保持不变)

    spdlog::info("API routes configured (including FlightTask APIs)");
}