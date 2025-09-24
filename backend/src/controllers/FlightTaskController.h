#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <optional>
#include <vector>

#include "auth/JwtService.h"
#include "repositories/FlightTaskRepository.h"
#include "models/FlightTask.h"
#include "utils/HttpResponse.h"

namespace beast = boost::beast;
namespace http = beast::http;

namespace controllers
{

    /**
     * @brief 飞行任务控制器
     *
     * 处理飞行任务相关的HTTP请求，包括：
     * - 获取飞行任务列表（GET /api/v1/tasks）
     * - 创建飞行任务（POST /api/v1/tasks）
     * - 更新飞行任务（PUT /api/v1/tasks/{id}）
     * - 删除飞行任务（DELETE /api/v1/tasks/{id}）
     *
     * 所有接口都需要JWT认证，并基于用户ID进行权限控制
     */
    class FlightTaskController
    {
    private:
        std::shared_ptr<repositories::FlightTaskRepository> taskRepository_;
        std::shared_ptr<auth::JwtService> jwtService_;

    public:
        /**
         * @brief 构造函数
         * @param taskRepository 飞行任务仓储对象
         * @param jwtService JWT服务对象
         */
        FlightTaskController(
            std::shared_ptr<repositories::FlightTaskRepository> taskRepository,
            std::shared_ptr<auth::JwtService> jwtService);

        // ========== 飞行任务API接口 ==========

        /**
         * @brief 获取飞行任务列表接口
         * GET /api/v1/tasks
         *
         * 查询参数（可选）：
         * - status: 任务状态过滤（pending/ongoing/completed）
         * - limit: 返回数量限制（默认100）
         * - offset: 偏移量（默认0）
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取任务列表成功",
         *   "data": {
         *     "tasks": [
         *       {
         *         "id": "1",
         *         "name": "任务名称",
         *         "description": "任务描述",
         *         "route": {...},
         *         "status": "pending",
         *         "scheduled_time": "2024-12-25T10:00:00Z",
         *         "created_at": "2024-12-24T10:00:00Z",
         *         "updated_at": "2024-12-24T10:00:00Z"
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> getFlightTasks(const http::request<http::string_body> &req);

        /**
         * @brief 创建飞行任务接口
         * POST /api/v1/tasks
         *
         * 请求体格式：
         * {
         *   "name": "string",           // 任务名称（必填）
         *   "description": "string",    // 任务描述（可选）
         *   "route": {                  // 路径信息（可选）
         *     "start": {"lat": 39.904, "lng": 116.407},
         *     "end": {"lat": 39.914, "lng": 116.417},
         *     "waypoints": []
         *   },
         *   "status": "pending",        // 任务状态（可选，默认pending）
         *   "scheduled_time": "2024-12-25T10:00:00Z"  // 计划时间（可选）
         * }
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "任务创建成功",
         *   "data": {
         *     "task": { ... }  // 创建的任务信息
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> createFlightTask(const http::request<http::string_body> &req);

        /**
         * @brief 更新飞行任务接口
         * PUT /api/v1/tasks/{id}
         *
         * 请求体格式（所有字段都是可选的）：
         * {
         *   "name": "string",
         *   "description": "string",
         *   "route": {...},
         *   "status": "pending|ongoing|completed",
         *   "scheduled_time": "2024-12-25T10:00:00Z"
         * }
         *
         * 权限要求：只有任务创建者可以更新任务
         *
         * @param req HTTP请求对象
         * @param task_id 任务ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> updateFlightTask(
            const http::request<http::string_body> &req,
            const std::string &task_id);

        /**
         * @brief 删除飞行任务接口
         * DELETE /api/v1/tasks/{id}
         *
         * 权限要求：只有任务创建者可以删除任务
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "任务删除成功"
         * }
         *
         * @param req HTTP请求对象
         * @param task_id 任务ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> deleteFlightTask(
            const http::request<http::string_body> &req,
            const std::string &task_id);

        /**
         * @brief 获取单个飞行任务详情接口
         * GET /api/v1/tasks/{id}
         *
         * 权限要求：只有任务创建者可以查看任务详情
         *
         * @param req HTTP请求对象
         * @param task_id 任务ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> getFlightTaskById(
            const http::request<http::string_body> &req,
            const std::string &task_id);

        // ========== 辅助方法 ==========

        /**
         * @brief 从请求头中提取Bearer Token
         * @param req HTTP请求对象
         * @return Token字符串，如果不存在返回空字符串
         */
        std::string extractBearerToken(const http::request<http::string_body> &req);

        /**
         * @brief 验证JWT Token并获取用户ID
         * @param req HTTP请求对象
         * @return 用户ID，如果验证失败返回空
         */
        std::optional<int64_t> validateTokenAndGetUserId(const http::request<http::string_body> &req);

        /**
         * @brief 验证创建任务请求参数
         * @param taskData 解析后的JSON数据
         * @return 验证结果和错误信息
         */
        std::pair<bool, std::string> validateCreateTaskRequest(const nlohmann::json &taskData);

        /**
         * @brief 验证更新任务请求参数
         * @param taskData 解析后的JSON数据
         * @return 验证结果和错误信息
         */
        std::pair<bool, std::string> validateUpdateTaskRequest(const nlohmann::json &taskData);

        /**
         * @brief 验证任务状态
         * @param status 状态字符串
         * @return 是否为有效状态
         */
        bool isValidTaskStatus(const std::string &status);

        /**
         * @brief 解析查询参数
         * @param req HTTP请求对象
         * @return 查询参数映射
         */
        std::map<std::string, std::string> parseQueryParams(const http::request<http::string_body> &req);

        /**
         * @brief 记录API调用日志
         * @param user_id 用户ID
         * @param method HTTP方法
         * @param path 请求路径
         * @param success 是否成功
         * @param message 日志消息
         */
        void logApiCall(
            int64_t user_id,
            const std::string &method,
            const std::string &path,
            bool success,
            const std::string &message);

    private:
        /**
         * @brief 检查用户是否拥有指定任务的权限
         * @param task_id 任务ID
         * @param user_id 用户ID
         * @return 是否有权限
         */
        bool checkTaskPermission(int64_t task_id, int64_t user_id);

        /**
         * @brief 将FlightTask对象转换为API响应JSON
         * @param task 任务对象
         * @return JSON对象
         */
        nlohmann::json taskToApiJson(const models::FlightTask &task);

        /**
         * @brief 将JSON对象转换为FlightTask对象
         * @param json JSON对象
         * @param user_id 用户ID
         * @return FlightTask对象
         */
        models::FlightTask jsonToTask(const nlohmann::json &json, int64_t user_id);
    };

} // namespace controllers