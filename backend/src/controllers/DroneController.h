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
     * @brief 无人机管理控制器
     *
     * 处理无人机信息相关的HTTP请求，包括：
     * - 获取无人机列表（GET /api/v1/drones）
     * - 获取无人机详情（GET /api/v1/drones/{id}）
     *
     * 所有接口都需要JWT认证
     */
    class DroneController
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
        DroneController(
            std::shared_ptr<mysqlx::Session> dbSession,
            std::shared_ptr<auth::JwtService> jwtService);

        // ========== 无人机管理API接口 ==========

        /**
         * @brief 获取无人机列表接口
         * GET /api/v1/drones
         *
         * 查询参数（可选）：
         * - status: 状态过滤（active/inactive/maintenance/retired）
         * - owner_id: 所有者ID过滤
         * - type: 类型过滤（fixed_wing/multi_rotor/hybrid/other）
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取无人机列表成功",
         *   "data": {
         *     "drones": [
         *       {
         *         "id": 1,
         *         "drone_code": "DR001",
         *         "name": "巡检无人机001",
         *         "model": "DJI Phantom 4",
         *         "type": "multi_rotor",
         *         "status": "active",
         *         "owner_id": 1,
         *         "max_altitude": 500.0,
         *         "max_speed": 20.0,
         *         "battery_capacity": 5870
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> getDrones(const http::request<http::string_body> &req);

        /**
         * @brief 获取无人机详情接口
         * GET /api/v1/drones/{id}
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取无人机详情成功",
         *   "data": {
         *     "id": 1,
         *     "drone_code": "DR001",
         *     "name": "巡检无人机001",
         *     "model": "DJI Phantom 4",
         *     "manufacturer": "DJI",
         *     "type": "multi_rotor",
         *     "status": "active",
         *     "owner_id": 1,
         *     "registration_number": "REG001",
         *     "serial_number": "SN123456",
         *     "max_altitude": 500.0,
         *     "max_speed": 20.0,
         *     "max_flight_time": 30,
         *     "battery_capacity": 5870,
         *     "weight": 1.38,
         *     "purchase_date": "2024-01-01",
         *     "last_maintenance_date": "2024-10-01",
         *     "next_maintenance_date": "2025-01-01",
         *     "certifications": "...",
         *     "description": "...",
         *     "created_at": "2024-01-01 00:00:00",
         *     "updated_at": "2024-10-01 12:00:00"
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @param drone_id 无人机ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> getDroneById(
            const http::request<http::string_body> &req,
            const std::string &drone_id);

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
         * @brief 解析查询参数
         * @param req HTTP请求对象
         * @return 查询参数映射
         */
        std::map<std::string, std::string> parseQueryParams(const http::request<http::string_body> &req);
    };

} // namespace controllers
