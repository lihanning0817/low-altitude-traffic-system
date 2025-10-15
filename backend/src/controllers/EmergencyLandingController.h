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
     * @brief 紧急降落点控制器
     *
     * 处理紧急降落点相关的HTTP请求，包括：
     * - 获取紧急降落点列表（GET /api/v1/emergency-landing-points）
     * - 添加紧急降落点（POST /api/v1/emergency-landing-points）
     * - 查找最近的紧急降落点（GET /api/v1/emergency-landing-points/nearest）
     * - 更新紧急降落点（PUT /api/v1/emergency-landing-points/{id}）
     *
     * 所有接口都需要JWT认证
     */
    class EmergencyLandingController
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
        EmergencyLandingController(
            std::shared_ptr<mysqlx::Session> dbSession,
            std::shared_ptr<auth::JwtService> jwtService);

        // ========== 紧急降落点API接口 ==========

        /**
         * @brief 获取紧急降落点列表接口
         * GET /api/v1/emergency-landing-points
         *
         * 查询参数（可选）：
         * - type: 类型过滤（hospital/park/field/building_roof/other）
         * - status: 状态过滤（active/inactive/maintenance）
         * - safety_rating: 安全等级过滤（A/B/C/D）
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取紧急降落点列表成功",
         *   "data": {
         *     "landing_points": [
         *       {
         *         "id": 1,
         *         "point_code": "ELP001",
         *         "name": "中心医院停机坪",
         *         "type": "hospital",
         *         "lat": 39.904,
         *         "lng": 116.407,
         *         "altitude": 100.5,
         *         "capacity": 2,
         *         "status": "active",
         *         "safety_rating": "A"
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> getEmergencyLandingPoints(const http::request<http::string_body> &req);

        /**
         * @brief 获取紧急降落点详情接口
         * GET /api/v1/emergency-landing-points/{id}
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取紧急降落点详情成功",
         *   "data": {
         *     "id": 1,
         *     "point_code": "ELP001",
         *     "name": "中心医院停机坪",
         *     "type": "hospital",
         *     "lat": 39.904,
         *     "lng": 116.407,
         *     "altitude": 100.5,
         *     "capacity": 2,
         *     "status": "active",
         *     "safety_rating": "A",
         *     "weather_protected": true,
         *     "accessible_24h": true,
         *     "contact_name": "张三",
         *     "contact_phone": "13800138000",
         *     "address": "北京市朝阳区...",
         *     "description": "大型三甲医院停机坪",
         *     "facilities": "照明,充电,维修",
         *     "created_at": "2024-01-01 00:00:00",
         *     "updated_at": "2024-10-01 12:00:00"
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @param point_id 降落点ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> getEmergencyLandingPointById(
            const http::request<http::string_body> &req,
            const std::string &point_id);

        /**
         * @brief 添加紧急降落点接口（管理员）
         * POST /api/v1/emergency-landing-points
         *
         * 请求体格式：
         * {
         *   "name": "string",              // 名称（必填）
         *   "type": "hospital",            // 类型（必填）
         *   "lat": 39.904,                 // 纬度（必填）
         *   "lng": 116.407,                // 经度（必填）
         *   "altitude": 100.5,             // 海拔（可选）
         *   "capacity": 2,                 // 容量（必填，默认1）
         *   "safety_rating": "A",          // 安全等级（可选）
         *   "weather_protected": true,     // 是否有气象保护（可选）
         *   "accessible_24h": true         // 是否24小时可用（可选）
         * }
         *
         * 权限要求：管理员角色
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> addEmergencyLandingPoint(const http::request<http::string_body> &req);

        /**
         * @brief 查找最近的紧急降落点接口
         * GET /api/v1/emergency-landing-points/nearest?lat=39.904&lng=116.407&limit=5
         *
         * 查询参数：
         * - lat: 当前纬度（必填）
         * - lng: 当前经度（必填）
         * - limit: 返回数量限制（可选，默认5）
         * - radius: 搜索半径（米，可选，默认10000）
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "查找最近的紧急降落点成功",
         *   "data": {
         *     "landing_points": [
         *       {
         *         "id": 1,
         *         "name": "中心医院停机坪",
         *         "type": "hospital",
         *         "lat": 39.904,
         *         "lng": 116.407,
         *         "distance": 1234.56,  // 距离（米）
         *         "safety_rating": "A"
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> findNearestLandingPoints(const http::request<http::string_body> &req);

        /**
         * @brief 更新紧急降落点接口（管理员）
         * PUT /api/v1/emergency-landing-points/{id}
         *
         * 请求体格式（所有字段都是可选的）：
         * {
         *   "name": "string",
         *   "status": "active|inactive|maintenance",
         *   "capacity": 2,
         *   "safety_rating": "A"
         * }
         *
         * 权限要求：管理员角色
         *
         * @param req HTTP请求对象
         * @param point_id 降落点ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> updateEmergencyLandingPoint(
            const http::request<http::string_body> &req,
            const std::string &point_id);

        /**
         * @brief 删除紧急降落点接口（管理员）
         * DELETE /api/v1/emergency-landing-points/{id}
         *
         * 权限要求：管理员角色
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "紧急降落点删除成功"
         * }
         *
         * @param req HTTP请求对象
         * @param point_id 降落点ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> deleteEmergencyLandingPoint(
            const http::request<http::string_body> &req,
            const std::string &point_id);

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
         * @brief 生成降落点编号
         * @return 降落点编号
         */
        std::string generatePointCode();

        /**
         * @brief 解析查询参数
         * @param req HTTP请求对象
         * @return 查询参数映射
         */
        std::map<std::string, std::string> parseQueryParams(const http::request<http::string_body> &req);

        /**
         * @brief 计算两个坐标点之间的距离（米）
         * @param lat1 纬度1
         * @param lng1 经度1
         * @param lat2 纬度2
         * @param lng2 经度2
         * @return 距离（米）
         */
        double calculateDistance(double lat1, double lng1, double lat2, double lng2);
    };

} // namespace controllers
