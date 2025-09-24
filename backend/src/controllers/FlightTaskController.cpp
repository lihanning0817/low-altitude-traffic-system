#include "FlightTaskController.h"
#include "../utils/JsonUtils.h"
#include <iostream>
#include <sstream>
#include <regex>

namespace controllers
{

    FlightTaskController::FlightTaskController(
        std::shared_ptr<repositories::FlightTaskRepository> taskRepository,
        std::shared_ptr<auth::JwtService> jwtService) : taskRepository_(taskRepository), jwtService_(jwtService)
    {
        std::cout << "[FlightTaskController] Initializing FlightTask Controller" << std::endl;
    }

    http::response<http::string_body> FlightTaskController::getFlightTasks(
        const http::request<http::string_body> &req)
    {
        std::cout << "[FlightTaskController] GET /api/v1/tasks - Getting flight tasks list" << std::endl;

        try
        {
            // 验证JWT Token并获取用户ID
            auto user_id_opt = validateTokenAndGetUserId(req);
            if (!user_id_opt.has_value())
            {
                logApiCall(0, "GET", "/api/v1/tasks", false, "Invalid JWT token");
                return utils::HttpResponse::createUnauthorizedResponse("Token无效或已过期");
            }

            int64_t user_id = user_id_opt.value();
            std::cout << "[FlightTaskController] Authenticated user ID: " << user_id << std::endl;

            // 解析查询参数
            auto query_params = parseQueryParams(req);

            // 获取查询参数
            std::string status_filter = "";
            int limit = 100; // 默认限制
            int offset = 0;  // 默认偏移

            if (query_params.find("status") != query_params.end())
            {
                status_filter = query_params["status"];
            }

            if (query_params.find("limit") != query_params.end())
            {
                try
                {
                    limit = std::stoi(query_params["limit"]);
                    limit = std::min(std::max(limit, 1), 1000); // 限制在1-1000之间
                }
                catch (const std::exception &)
                {
                    limit = 100;
                }
            }

            if (query_params.find("offset") != query_params.end())
            {
                try
                {
                    offset = std::stoi(query_params["offset"]);
                    offset = std::max(offset, 0); // 最小为0
                }
                catch (const std::exception &)
                {
                    offset = 0;
                }
            }

            std::cout << "[FlightTaskController] Query params - status: '" << status_filter
                      << "', limit: " << limit << ", offset: " << offset << std::endl;

            // 查询任务列表
            std::vector<models::FlightTask> tasks;

            if (!status_filter.empty() && isValidTaskStatus(status_filter))
            {
                // 按状态过滤
                auto status = models::FlightTask::stringToStatus(status_filter);
                tasks = taskRepository_->findByStatus(status, user_id);
            }
            else
            {
                // 获取所有任务（分页）
                tasks = taskRepository_->findAll(offset, limit, user_id);
            }

            // 构建响应数据
            nlohmann::json tasks_array = nlohmann::json::array();
            for (const auto &task : tasks)
            {
                tasks_array.push_back(taskToApiJson(task));
            }

            nlohmann::json response_data;
            response_data["tasks"] = tasks_array;

            logApiCall(user_id, "GET", "/api/v1/tasks", true,
                       "Successfully retrieved " + std::to_string(tasks.size()) + " tasks");

            std::cout << "[FlightTaskController] Successfully returned " << tasks.size() << " tasks" << std::endl;

            return utils::HttpResponse::createSuccessResponse(response_data, "获取任务列表成功");
        }
        catch (const std::exception &e)
        {
            std::cerr << "[FlightTaskController] Error in getFlightTasks: " << e.what() << std::endl;
            logApiCall(0, "GET", "/api/v1/tasks", false, "Exception: " + std::string(e.what()));
            return utils::HttpResponse::createInternalErrorResponse("获取任务列表失败");
        }
    }

    http::response<http::string_body> FlightTaskController::createFlightTask(
        const http::request<http::string_body> &req)
    {
        std::cout << "[FlightTaskController] POST /api/v1/tasks - Creating new flight task" << std::endl;

        try
        {
            // 验证JWT Token并获取用户ID
            auto user_id_opt = validateTokenAndGetUserId(req);
            if (!user_id_opt.has_value())
            {
                logApiCall(0, "POST", "/api/v1/tasks", false, "Invalid JWT token");
                return utils::HttpResponse::createUnauthorizedResponse("Token无效或已过期");
            }

            int64_t user_id = user_id_opt.value();
            std::cout << "[FlightTaskController] Authenticated user ID: " << user_id << std::endl;

            // 解析请求体JSON（使用UTF-8安全解析）
            nlohmann::json request_data;
            try
            {
                std::cout << "[FlightTaskController] Raw request body: " << req.body() << std::endl;
                request_data = utils::JsonUtils::parseJsonSafe(req.body());
            }
            catch (const std::exception &e)
            {
                std::cerr << "[FlightTaskController] JSON parse error: " << e.what() << std::endl;
                logApiCall(user_id, "POST", "/api/v1/tasks", false, "Invalid JSON format");
                return utils::HttpResponse::createErrorResponse("请求体JSON格式错误");
            }

            std::cout << "[FlightTaskController] Request data: " << request_data.dump(2) << std::endl;

            // 验证请求参数
            auto [valid, error_message] = validateCreateTaskRequest(request_data);
            if (!valid)
            {
                std::cout << "[FlightTaskController] Validation failed: " << error_message << std::endl;
                logApiCall(user_id, "POST", "/api/v1/tasks", false, "Validation failed: " + error_message);
                return utils::HttpResponse::createErrorResponse(error_message);
            }

            // 创建FlightTask对象
            auto task = jsonToTask(request_data, user_id);

            // 保存到数据库
            int64_t task_id = taskRepository_->create(task);
            if (task_id <= 0)
            {
                std::cerr << "[FlightTaskController] Failed to create task in database" << std::endl;
                logApiCall(user_id, "POST", "/api/v1/tasks", false, "Database creation failed");
                return utils::HttpResponse::createInternalErrorResponse("创建任务失败");
            }

            // 获取完整的任务信息
            auto created_task_opt = taskRepository_->findById(task_id);
            if (!created_task_opt.has_value())
            {
                std::cerr << "[FlightTaskController] Failed to retrieve created task" << std::endl;
                logApiCall(user_id, "POST", "/api/v1/tasks", false, "Failed to retrieve created task");
                return utils::HttpResponse::createInternalErrorResponse("创建任务后无法获取任务信息");
            }

            // 构建响应数据
            nlohmann::json response_data;
            response_data["task"] = taskToApiJson(created_task_opt.value());

            logApiCall(user_id, "POST", "/api/v1/tasks", true,
                       "Successfully created task with ID: " + std::to_string(task_id));

            std::cout << "[FlightTaskController] Successfully created task with ID: " << task_id << std::endl;

            return utils::HttpResponse::createSuccessResponse(response_data, "任务创建成功", 201);
        }
        catch (const std::exception &e)
        {
            std::cerr << "[FlightTaskController] Error in createFlightTask: " << e.what() << std::endl;
            logApiCall(0, "POST", "/api/v1/tasks", false, "Exception: " + std::string(e.what()));
            return utils::HttpResponse::createInternalErrorResponse("创建任务失败");
        }
    }

    http::response<http::string_body> FlightTaskController::updateFlightTask(
        const http::request<http::string_body> &req,
        const std::string &task_id)
    {
        std::cout << "[FlightTaskController] PUT /api/v1/tasks/" << task_id
                  << " - Updating flight task" << std::endl;

        try
        {
            // 验证JWT Token并获取用户ID
            auto user_id_opt = validateTokenAndGetUserId(req);
            if (!user_id_opt.has_value())
            {
                logApiCall(0, "PUT", "/api/v1/tasks/" + task_id, false, "Invalid JWT token");
                return utils::HttpResponse::createUnauthorizedResponse("Token无效或已过期");
            }

            int64_t user_id = user_id_opt.value();
            std::cout << "[FlightTaskController] Authenticated user ID: " << user_id << std::endl;

            // 验证任务ID格式
            int64_t task_id_num;
            try
            {
                task_id_num = std::stoll(task_id);
            }
            catch (const std::exception &)
            {
                logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, false, "Invalid task ID format");
                return utils::HttpResponse::createErrorResponse("任务ID格式错误");
            }

            // 检查任务是否存在以及用户权限
            if (!checkTaskPermission(task_id_num, user_id))
            {
                logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, false, "Task not found or no permission");
                return utils::HttpResponse::createNotFoundResponse("任务不存在或无权限");
            }

            // 解析请求体JSON（使用UTF-8安全解析）
            nlohmann::json request_data;
            try
            {
                std::cout << "[FlightTaskController] Raw request body (PUT): " << req.body() << std::endl;
                request_data = utils::JsonUtils::parseJsonSafe(req.body());
            }
            catch (const std::exception &e)
            {
                std::cerr << "[FlightTaskController] JSON parse error: " << e.what() << std::endl;
                logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, false, "Invalid JSON format");
                return utils::HttpResponse::createErrorResponse("请求体JSON格式错误");
            }

            std::cout << "[FlightTaskController] Update data: " << request_data.dump(2) << std::endl;

            // 验证更新请求参数
            auto [valid, error_message] = validateUpdateTaskRequest(request_data);
            if (!valid)
            {
                std::cout << "[FlightTaskController] Validation failed: " << error_message << std::endl;
                logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, false, "Validation failed: " + error_message);
                return utils::HttpResponse::createErrorResponse(error_message);
            }

            // 获取现有任务
            auto existing_task_opt = taskRepository_->findById(task_id_num);
            if (!existing_task_opt.has_value())
            {
                logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, false, "Task not found");
                return utils::HttpResponse::createNotFoundResponse("任务不存在");
            }

            auto task = existing_task_opt.value();

            // 更新任务字段
            if (request_data.contains("name") && !request_data["name"].is_null())
            {
                task.setName(request_data["name"].get<std::string>());
            }

            if (request_data.contains("description"))
            {
                if (request_data["description"].is_null())
                {
                    task.setDescription("");
                }
                else
                {
                    task.setDescription(request_data["description"].get<std::string>());
                }
            }

            if (request_data.contains("route"))
            {
                if (request_data["route"].is_null())
                {
                    task.setRoute(nlohmann::json::object());
                }
                else
                {
                    task.setRoute(request_data["route"]);
                }
            }

            if (request_data.contains("status") && !request_data["status"].is_null())
            {
                std::string status_str = request_data["status"].get<std::string>();
                task.setStatus(models::FlightTask::stringToStatus(status_str));
            }

            // 保存更新
            bool success = taskRepository_->update(task_id_num, task);
            if (!success)
            {
                std::cerr << "[FlightTaskController] Failed to update task in database" << std::endl;
                logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, false, "Database update failed");
                return utils::HttpResponse::createInternalErrorResponse("更新任务失败");
            }

            // 获取更新后的任务信息
            auto updated_task_opt = taskRepository_->findById(task_id_num);
            if (!updated_task_opt.has_value())
            {
                std::cerr << "[FlightTaskController] Failed to retrieve updated task" << std::endl;
                logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, false, "Failed to retrieve updated task");
                return utils::HttpResponse::createInternalErrorResponse("更新任务后无法获取任务信息");
            }

            // 构建响应数据
            nlohmann::json response_data;
            response_data["task"] = taskToApiJson(updated_task_opt.value());

            logApiCall(user_id, "PUT", "/api/v1/tasks/" + task_id, true,
                       "Successfully updated task");

            std::cout << "[FlightTaskController] Successfully updated task ID: " << task_id << std::endl;

            return utils::HttpResponse::createSuccessResponse(response_data, "任务更新成功");
        }
        catch (const std::exception &e)
        {
            std::cerr << "[FlightTaskController] Error in updateFlightTask: " << e.what() << std::endl;
            logApiCall(0, "PUT", "/api/v1/tasks/" + task_id, false, "Exception: " + std::string(e.what()));
            return utils::HttpResponse::createInternalErrorResponse("更新任务失败");
        }
    }

    http::response<http::string_body> FlightTaskController::deleteFlightTask(
        const http::request<http::string_body> &req,
        const std::string &task_id)
    {
        std::cout << "[FlightTaskController] DELETE /api/v1/tasks/" << task_id
                  << " - Deleting flight task" << std::endl;

        try
        {
            // 验证JWT Token并获取用户ID
            auto user_id_opt = validateTokenAndGetUserId(req);
            if (!user_id_opt.has_value())
            {
                logApiCall(0, "DELETE", "/api/v1/tasks/" + task_id, false, "Invalid JWT token");
                return utils::HttpResponse::createUnauthorizedResponse("Token无效或已过期");
            }

            int64_t user_id = user_id_opt.value();
            std::cout << "[FlightTaskController] Authenticated user ID: " << user_id << std::endl;

            // 验证任务ID格式
            int64_t task_id_num;
            try
            {
                task_id_num = std::stoll(task_id);
            }
            catch (const std::exception &)
            {
                logApiCall(user_id, "DELETE", "/api/v1/tasks/" + task_id, false, "Invalid task ID format");
                return utils::HttpResponse::createErrorResponse("任务ID格式错误");
            }

            // 检查任务是否存在以及用户权限
            if (!checkTaskPermission(task_id_num, user_id))
            {
                logApiCall(user_id, "DELETE", "/api/v1/tasks/" + task_id, false, "Task not found or no permission");
                return utils::HttpResponse::createNotFoundResponse("任务不存在或无权限");
            }

            // 删除任务
            bool success = taskRepository_->deleteById(task_id_num);
            if (!success)
            {
                std::cerr << "[FlightTaskController] Failed to delete task from database" << std::endl;
                logApiCall(user_id, "DELETE", "/api/v1/tasks/" + task_id, false, "Database deletion failed");
                return utils::HttpResponse::createInternalErrorResponse("删除任务失败");
            }

            logApiCall(user_id, "DELETE", "/api/v1/tasks/" + task_id, true,
                       "Successfully deleted task");

            std::cout << "[FlightTaskController] Successfully deleted task ID: " << task_id << std::endl;

            return utils::HttpResponse::createSuccessResponse(nlohmann::json::object(), "任务删除成功");
        }
        catch (const std::exception &e)
        {
            std::cerr << "[FlightTaskController] Error in deleteFlightTask: " << e.what() << std::endl;
            logApiCall(0, "DELETE", "/api/v1/tasks/" + task_id, false, "Exception: " + std::string(e.what()));
            return utils::HttpResponse::createInternalErrorResponse("删除任务失败");
        }
    }

    http::response<http::string_body> FlightTaskController::getFlightTaskById(
        const http::request<http::string_body> &req,
        const std::string &task_id)
    {
        std::cout << "[FlightTaskController] GET /api/v1/tasks/" << task_id
                  << " - Getting flight task details" << std::endl;

        try
        {
            // 验证JWT Token并获取用户ID
            auto user_id_opt = validateTokenAndGetUserId(req);
            if (!user_id_opt.has_value())
            {
                logApiCall(0, "GET", "/api/v1/tasks/" + task_id, false, "Invalid JWT token");
                return utils::HttpResponse::createUnauthorizedResponse("Token无效或已过期");
            }

            int64_t user_id = user_id_opt.value();
            std::cout << "[FlightTaskController] Authenticated user ID: " << user_id << std::endl;

            // 验证任务ID格式
            int64_t task_id_num;
            try
            {
                task_id_num = std::stoll(task_id);
            }
            catch (const std::exception &)
            {
                logApiCall(user_id, "GET", "/api/v1/tasks/" + task_id, false, "Invalid task ID format");
                return utils::HttpResponse::createErrorResponse("任务ID格式错误");
            }

            // 检查任务是否存在以及用户权限
            if (!checkTaskPermission(task_id_num, user_id))
            {
                logApiCall(user_id, "GET", "/api/v1/tasks/" + task_id, false, "Task not found or no permission");
                return utils::HttpResponse::createNotFoundResponse("任务不存在或无权限");
            }

            // 获取任务详情
            auto task_opt = taskRepository_->findById(task_id_num);
            if (!task_opt.has_value())
            {
                logApiCall(user_id, "GET", "/api/v1/tasks/" + task_id, false, "Task not found");
                return utils::HttpResponse::createNotFoundResponse("任务不存在");
            }

            // 构建响应数据
            nlohmann::json response_data;
            response_data["task"] = taskToApiJson(task_opt.value());

            logApiCall(user_id, "GET", "/api/v1/tasks/" + task_id, true,
                       "Successfully retrieved task details");

            std::cout << "[FlightTaskController] Successfully returned task details for ID: " << task_id << std::endl;

            return utils::HttpResponse::createSuccessResponse(response_data, "获取任务详情成功");
        }
        catch (const std::exception &e)
        {
            std::cerr << "[FlightTaskController] Error in getFlightTaskById: " << e.what() << std::endl;
            logApiCall(0, "GET", "/api/v1/tasks/" + task_id, false, "Exception: " + std::string(e.what()));
            return utils::HttpResponse::createInternalErrorResponse("获取任务详情失败");
        }
    }

    // ========== 辅助方法实现 ==========

    std::string FlightTaskController::extractBearerToken(const http::request<http::string_body> &req)
    {
        auto it = req.find(http::field::authorization);
        if (it != req.end())
        {
            std::string auth_header = it->value();
            const std::string bearer_prefix = "Bearer ";
            if (auth_header.substr(0, bearer_prefix.length()) == bearer_prefix)
            {
                return auth_header.substr(bearer_prefix.length());
            }
        }
        return "";
    }

    std::optional<int64_t> FlightTaskController::validateTokenAndGetUserId(
        const http::request<http::string_body> &req)
    {
        std::string token = extractBearerToken(req);
        if (token.empty())
        {
            std::cout << "[FlightTaskController] No Bearer token found in request" << std::endl;
            return std::nullopt;
        }

        try
        {
            auto validationResult = jwtService_->validateToken(token);
            if (validationResult == auth::JwtValidationResult::VALID)
            {
                auto tokenInfo = jwtService_->parseToken(token);
                if (tokenInfo.has_value())
                {
                    int64_t user_id = tokenInfo->user_id;
                    std::cout << "[FlightTaskController] JWT validation successful for user ID: " << user_id << std::endl;
                    return user_id;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[FlightTaskController] JWT validation error: " << e.what() << std::endl;
        }

        std::cout << "[FlightTaskController] JWT validation failed" << std::endl;
        return std::nullopt;
    }

    std::pair<bool, std::string> FlightTaskController::validateCreateTaskRequest(const nlohmann::json &taskData)
    {
        // 检查必填字段
        if (!taskData.contains("name") || taskData["name"].is_null() || taskData["name"].get<std::string>().empty())
        {
            return {false, "任务名称不能为空"};
        }

        std::string name = taskData["name"].get<std::string>();
        if (name.length() > 255)
        {
            return {false, "任务名称长度不能超过255个字符"};
        }

        // 检查描述长度
        if (taskData.contains("description") && !taskData["description"].is_null())
        {
            std::string description = taskData["description"].get<std::string>();
            if (description.length() > 65535)
            {
                return {false, "任务描述长度不能超过65535个字符"};
            }
        }

        // 检查状态
        if (taskData.contains("status") && !taskData["status"].is_null())
        {
            std::string status = taskData["status"].get<std::string>();
            if (!isValidTaskStatus(status))
            {
                return {false, "任务状态必须是 pending、ongoing 或 completed"};
            }
        }

        // 检查路径格式（简单验证）
        if (taskData.contains("route") && !taskData["route"].is_null())
        {
            if (!taskData["route"].is_object())
            {
                return {false, "路径信息必须是JSON对象"};
            }
        }

        return {true, ""};
    }

    std::pair<bool, std::string> FlightTaskController::validateUpdateTaskRequest(const nlohmann::json &taskData)
    {
        // 更新请求的字段都是可选的，但如果提供了就需要验证

        if (taskData.contains("name") && !taskData["name"].is_null())
        {
            std::string name = taskData["name"].get<std::string>();
            if (name.empty() || name.length() > 255)
            {
                return {false, "任务名称不能为空且长度不能超过255个字符"};
            }
        }

        if (taskData.contains("description") && !taskData["description"].is_null())
        {
            std::string description = taskData["description"].get<std::string>();
            if (description.length() > 65535)
            {
                return {false, "任务描述长度不能超过65535个字符"};
            }
        }

        if (taskData.contains("status") && !taskData["status"].is_null())
        {
            std::string status = taskData["status"].get<std::string>();
            if (!isValidTaskStatus(status))
            {
                return {false, "任务状态必须是 pending、ongoing 或 completed"};
            }
        }

        if (taskData.contains("route") && !taskData["route"].is_null())
        {
            if (!taskData["route"].is_object())
            {
                return {false, "路径信息必须是JSON对象"};
            }
        }

        return {true, ""};
    }

    bool FlightTaskController::isValidTaskStatus(const std::string &status)
    {
        return status == "pending" || status == "ongoing" || status == "completed";
    }

    std::map<std::string, std::string> FlightTaskController::parseQueryParams(
        const http::request<http::string_body> &req)
    {
        std::map<std::string, std::string> params;

        std::string target = req.target();
        size_t query_pos = target.find('?');

        if (query_pos != std::string::npos)
        {
            std::string query = target.substr(query_pos + 1);

            // 简单的查询参数解析
            std::stringstream ss(query);
            std::string param;

            while (std::getline(ss, param, '&'))
            {
                size_t eq_pos = param.find('=');
                if (eq_pos != std::string::npos)
                {
                    std::string key = param.substr(0, eq_pos);
                    std::string value = param.substr(eq_pos + 1);
                    params[key] = value;
                }
            }
        }

        return params;
    }

    void FlightTaskController::logApiCall(
        int64_t user_id,
        const std::string &method,
        const std::string &path,
        bool success,
        const std::string &message)
    {
        std::cout << "[FlightTaskController] API Call Log - "
                  << "User: " << user_id << ", "
                  << "Method: " << method << ", "
                  << "Path: " << path << ", "
                  << "Success: " << (success ? "true" : "false") << ", "
                  << "Message: " << message << std::endl;
    }

    bool FlightTaskController::checkTaskPermission(int64_t task_id, int64_t user_id)
    {
        try
        {
            return taskRepository_->isTaskOwnedByUser(task_id, user_id);
        }
        catch (const std::exception &e)
        {
            std::cerr << "[FlightTaskController] Error checking task permission: " << e.what() << std::endl;
            return false;
        }
    }

    nlohmann::json FlightTaskController::taskToApiJson(const models::FlightTask &task)
    {
        return task.toJson();
    }

    models::FlightTask FlightTaskController::jsonToTask(const nlohmann::json &json, int64_t user_id)
    {
        models::FlightTask task;

        if (json.contains("name"))
        {
            task.setName(json["name"].get<std::string>());
        }

        if (json.contains("description"))
        {
            if (json["description"].is_null())
            {
                task.setDescription("");
            }
            else
            {
                task.setDescription(json["description"].get<std::string>());
            }
        }

        if (json.contains("route"))
        {
            if (json["route"].is_null())
            {
                task.setRoute(nlohmann::json::object());
            }
            else
            {
                task.setRoute(json["route"]);
            }
        }

        if (json.contains("status"))
        {
            std::string status_str = json["status"].get<std::string>();
            task.setStatus(models::FlightTask::stringToStatus(status_str));
        }
        else
        {
            task.setStatus(models::FlightTaskStatus::PENDING);
        }

        task.setUserId(user_id);

        // scheduled_time 处理（简化）
        if (json.contains("scheduled_time") && !json["scheduled_time"].is_null())
        {
            // 这里应该实现完整的时间解析，目前简化处理
            task.setScheduledTime(std::chrono::system_clock::now());
        }

        return task;
    }

} // namespace controllers