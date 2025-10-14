#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <mysqlx/xdevapi.h>

#include "auth/JwtService.h"
#include "utils/HttpResponse.h"

namespace beast = boost::beast;
namespace http = beast::http;

namespace controllers
{

    /**
     * @brief 飞行许可控制器
     *
     * 处理飞行许可相关的HTTP请求，包括：
     * - 申请飞行许可（POST /api/v1/flight-permits）
     * - 获取飞行许可列表（GET /api/v1/flight-permits）
     * - 审批飞行许可（POST /api/v1/flight-permits/{id}/approve）
     * - 拒绝飞行许可（POST /api/v1/flight-permits/{id}/reject）
     *
     * 所有接口都需要JWT认证
     */
    class FlightPermitController
    {
    private:
        std::shared_ptr<mysqlx::Session> dbSession_;
        std::shared_ptr<auth::JwtService> jwtService_;

    public:
        /**
         * @brief 构造函数
         * @param dbSession MySQL X DevAPI会话对象
         * @param jwtService JWT服务对象
         */
        FlightPermitController(
            std::shared_ptr<mysqlx::Session> dbSession,
            std::shared_ptr<auth::JwtService> jwtService);

        // ========== 飞行许可API接口 ==========

        /**
         * @brief 申请飞行许可接口
         * POST /api/v1/flight-permits
         *
         * 请求体格式：
         * {
         *   "task_id": 1,                 // 任务ID（必填）
         *   "airspace_id": 2,             // 空域ID（必填）
         *   "start_time": "2024-12-25T10:00:00Z",  // 开始时间（必填）
         *   "end_time": "2024-12-25T12:00:00Z",    // 结束时间（必填）
         *   "remarks": "string"           // 备注（可选）
         * }
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "飞行许可申请成功",
         *   "data": {
         *     "permit_id": 1,
         *     "permit_code": "FP20241225001",
         *     "status": "pending"
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> applyFlightPermit(const http::request<http::string_body> &req);

        /**
         * @brief 获取飞行许可列表接口
         * GET /api/v1/flight-permits
         *
         * 查询参数（可选）：
         * - status: 许可状态过滤（pending/approved/rejected/expired）
         * - applicant_id: 申请人ID过滤
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取飞行许可列表成功",
         *   "data": {
         *     "permits": [
         *       {
         *         "id": 1,
         *         "permit_code": "FP20241225001",
         *         "task_id": 1,
         *         "airspace_id": 2,
         *         "applicant_id": 3,
         *         "status": "pending",
         *         "application_time": "2024-12-25T10:00:00Z",
         *         "start_time": "2024-12-25T10:00:00Z",
         *         "end_time": "2024-12-25T12:00:00Z"
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> getFlightPermits(const http::request<http::string_body> &req);

        /**
         * @brief 审批飞行许可接口（管理员）
         * POST /api/v1/flight-permits/{id}/approve
         *
         * 权限要求：管理员角色
         *
         * @param req HTTP请求对象
         * @param permit_id 许可ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> approveFlightPermit(
            const http::request<http::string_body> &req,
            const std::string &permit_id);

        /**
         * @brief 拒绝飞行许可接口（管理员）
         * POST /api/v1/flight-permits/{id}/reject
         *
         * 权限要求：管理员角色
         *
         * @param req HTTP请求对象
         * @param permit_id 许可ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> rejectFlightPermit(
            const http::request<http::string_body> &req,
            const std::string &permit_id);

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
         * @return 用户ID，如果验证失败返回-1
         */
        int64_t validateTokenAndGetUserId(const http::request<http::string_body> &req);

        /**
         * @brief 验证用户是否为管理员
         * @param user_id 用户ID
         * @return 是否为管理员
         */
        bool isAdmin(int64_t user_id);

        /**
         * @brief 生成飞行许可编号
         * @return 许可编号
         */
        std::string generatePermitCode();

        /**
         * @brief 解析查询参数
         * @param req HTTP请求对象
         * @return 查询参数映射
         */
        std::map<std::string, std::string> parseQueryParams(const http::request<http::string_body> &req);
    };

} // namespace controllers
