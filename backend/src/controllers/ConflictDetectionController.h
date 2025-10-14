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
     * @brief 冲突检测控制器
     *
     * 处理飞行冲突检测相关的HTTP请求，包括：
     * - 飞行注册（POST /api/v1/flights）
     * - 获取飞行冲突列表（GET /api/v1/flight-conflicts）
     * - 解决飞行冲突（POST /api/v1/flight-conflicts/{id}/resolve）
     *
     * 所有接口都需要JWT认证
     */
    class ConflictDetectionController
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
        ConflictDetectionController(
            std::shared_ptr<mysqlx::Session> dbSession,
            std::shared_ptr<auth::JwtService> jwtService);

        // ========== 飞行注册与冲突检测API接口 ==========

        /**
         * @brief 飞行注册接口（自动进行冲突检测）
         * POST /api/v1/flights
         *
         * 请求体格式：
         * {
         *   "task_id": 1,                      // 任务ID（必填）
         *   "start_time": "2024-12-25T10:00:00Z",  // 开始时间（必填）
         *   "end_time": "2024-12-25T12:00:00Z",    // 结束时间（必填）
         *   "route": {                         // 路径信息（必填）
         *     "start": {"lat": 39.904, "lng": 116.407},
         *     "end": {"lat": 39.914, "lng": 116.417},
         *     "waypoints": []
         *   },
         *   "altitude": 100.5                  // 飞行高度（必填）
         * }
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "飞行注册成功",
         *   "data": {
         *     "flight_id": 1,
         *     "conflicts": [  // 如果检测到冲突
         *       {
         *         "conflict_id": 1,
         *         "conflict_type": "space",
         *         "severity": "high",
         *         "conflicting_task_id": 2
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> registerFlight(const http::request<http::string_body> &req);

        /**
         * @brief 获取飞行冲突列表接口
         * GET /api/v1/flight-conflicts
         *
         * 查询参数（可选）：
         * - status: 冲突状态过滤（active/resolved/ignored）
         * - severity: 严重程度过滤（low/medium/high/critical）
         * - task_id: 任务ID过滤
         *
         * 响应格式：
         * {
         *   "success": true,
         *   "message": "获取飞行冲突列表成功",
         *   "data": {
         *     "conflicts": [
         *       {
         *         "id": 1,
         *         "conflict_code": "FC20241225001",
         *         "task_id_1": 1,
         *         "task_id_2": 2,
         *         "conflict_type": "space",
         *         "severity": "high",
         *         "status": "active",
         *         "conflict_time": "2024-12-25T10:00:00Z",
         *         "distance": 50.5
         *       }
         *     ]
         *   }
         * }
         *
         * @param req HTTP请求对象
         * @return HTTP响应对象
         */
        http::response<http::string_body> getFlightConflicts(const http::request<http::string_body> &req);

        /**
         * @brief 解决飞行冲突接口
         * POST /api/v1/flight-conflicts/{id}/resolve
         *
         * 请求体格式：
         * {
         *   "resolution_notes": "string"  // 解决方案说明（可选）
         * }
         *
         * @param req HTTP请求对象
         * @param conflict_id 冲突ID
         * @return HTTP响应对象
         */
        http::response<http::string_body> resolveConflict(
            const http::request<http::string_body> &req,
            const std::string &conflict_id);

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
         * @brief 生成冲突编号
         * @return 冲突编号
         */
        std::string generateConflictCode();

        /**
         * @brief 解析查询参数
         * @param req HTTP请求对象
         * @return 查询参数映射
         */
        std::map<std::string, std::string> parseQueryParams(const http::request<http::string_body> &req);

        /**
         * @brief 检测飞行冲突
         * @param task_id 任务ID
         * @param start_time 开始时间
         * @param end_time 结束时间
         * @param route 路径信息（JSON格式）
         * @param altitude 飞行高度
         * @return 冲突ID列表
         */
        std::vector<int64_t> detectConflicts(
            int64_t task_id,
            const std::string &start_time,
            const std::string &end_time,
            const nlohmann::json &route,
            double altitude);

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
