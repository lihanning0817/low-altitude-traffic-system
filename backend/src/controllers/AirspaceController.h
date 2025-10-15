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
     * @brief 空域管理控制器
     *
     * 处理空域管理相关的HTTP请求，包括：
     * - 获取空域列表（GET /api/v1/airspaces）
     * - 获取空域详情（GET /api/v1/airspaces/{id}）
     * - 创建空域（POST /api/v1/airspaces）
     * - 更新空域（PUT /api/v1/airspaces/{id}）
     * - 删除空域（DELETE /api/v1/airspaces/{id}）
     *
     * 所有接口都需要JWT认证
     */
    class AirspaceController
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
        AirspaceController(
            std::shared_ptr<mysqlx::Session> dbSession,
            std::shared_ptr<auth::JwtService> jwtService);

        // ========== 空域管理API接口 ==========

        /**
         * @brief 获取空域列表接口
         * GET /api/v1/airspaces
         *
         * 查询参数（可选）：
         * - type: 类型过滤（restricted/controlled/warning/prohibited）
         * - status: 状态过滤（active/inactive/maintenance）
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取空域列表成功",
         *   "data": {
         *     "airspaces": [
         *       {
         *         "id": 1,
         *         "airspace_code": "AS001",
         *         "name": "中心城区限制区",
         *         "type": "restricted",
         *         "status": "active",
         *         "min_altitude": 0.0,
         *         "max_altitude": 500.0,
         *         "geometry": {...}
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> getAirspaces(const http::request<http::string_body> &req);

        /**
         * @brief 获取空域详情接口
         * GET /api/v1/airspaces/{id}
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取空域详情成功",
         *   "data": {
         *     "id": 1,
         *     "airspace_code": "AS001",
         *     "name": "中心城区限制区",
         *     "type": "restricted",
         *     "status": "active",
         *     "min_altitude": 0.0,
         *     "max_altitude": 500.0,
         *     "geometry": {...},
         *     "description": "市中心禁飞区",
         *     "effective_time": "2024-01-01 00:00:00",
         *     "expiry_time": "2025-12-31 23:59:59",
         *     "created_at": "2024-01-01 00:00:00",
         *     "updated_at": "2024-01-01 00:00:00"
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @param airspace_id 空域ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> getAirspaceById(
            const http::request<http::string_body> &req,
            const std::string &airspace_id);

        /**
         * @brief 创建空域接口（管理员）
         * POST /api/v1/airspaces
         *
         * 请求体格式：
         * {
         *   "name": "string",              // 名称（必填）
         *   "type": "restricted",          // 类型（必填）
         *   "status": "active",            // 状态（可选，默认active）
         *   "min_altitude": 0.0,           // 最低高度（必填）
         *   "max_altitude": 500.0,         // 最高高度（必填）
         *   "geometry": {...},             // 几何信息（必填，GeoJSON格式）
         *   "description": "string",       // 描述（可选）
         *   "effective_time": "2024-01-01T00:00:00Z",  // 生效时间（可选）
         *   "expiry_time": "2025-12-31T23:59:59Z"      // 失效时间（可选）
         * }
         *
         * 权限要求：管理员角色
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> createAirspace(const http::request<http::string_body> &req);

        /**
         * @brief 更新空域接口（管理员）
         * PUT /api/v1/airspaces/{id}
         *
         * 请求体格式（所有字段都是可选的）：
         * {
         *   "name": "string",
         *   "type": "restricted",
         *   "status": "active",
         *   "min_altitude": 0.0,
         *   "max_altitude": 500.0,
         *   "description": "string"
         * }
         *
         * 权限要求：管理员角色
         *
         * @param req HTTP请求对象
         * @param airspace_id 空域ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> updateAirspace(
            const http::request<http::string_body> &req,
            const std::string &airspace_id);

        /**
         * @brief 删除空域接口（管理员）
         * DELETE /api/v1/airspaces/{id}
         *
         * 权限要求：管理员角色
         *
         * @param req HTTP请求对象
         * @param airspace_id 空域ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> deleteAirspace(
            const http::request<http::string_body> &req,
            const std::string &airspace_id);

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
         * @brief 生成空域编号
         * @return 空域编号
         */
        std::string generateAirspaceCode();

        /**
         * @brief 解析查询参数
         * @param req HTTP请求对象
         * @return 查询参数映射
         */
        std::map<std::string, std::string> parseQueryParams(const http::request<http::string_body> &req);
    };

} // namespace controllers
