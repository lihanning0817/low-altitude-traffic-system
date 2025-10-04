#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <string>
#include "repositories/EmergencyEventRepository.h"
#include "auth/JwtService.h"

namespace beast = boost::beast;
namespace http = beast::http;

namespace controllers {

/**
 * @brief 紧急事件控制器类
 * 处理紧急事件相关的HTTP请求
 */
class EmergencyController {
private:
    std::shared_ptr<repositories::EmergencyEventRepository> eventRepo_;
    std::shared_ptr<auth::JwtService> jwtService_;

public:
    /**
     * @brief 构造函数
     * @param eventRepo 紧急事件仓储
     * @param jwtService JWT服务
     */
    EmergencyController(
        std::shared_ptr<repositories::EmergencyEventRepository> eventRepo,
        std::shared_ptr<auth::JwtService> jwtService
    );

    /**
     * @brief 创建紧急事件
     * POST /api/v1/emergency/events
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> createEvent(
        const http::request<http::string_body>& req
    );

    /**
     * @brief 获取所有紧急事件（支持过滤）
     * GET /api/v1/emergency/events
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getAllEvents(
        const http::request<http::string_body>& req
    );

    /**
     * @brief 根据ID获取紧急事件
     * GET /api/v1/emergency/events/{id}
     *
     * @param req HTTP请求对象
     * @param event_id 事件ID
     * @return HTTP响应对象
     */
    http::response<http::string_body> getEventById(
        const http::request<http::string_body>& req,
        int64_t event_id
    );

    /**
     * @brief 响应紧急事件
     * POST /api/v1/emergency/events/{id}/respond
     *
     * @param req HTTP请求对象
     * @param event_id 事件ID
     * @return HTTP响应对象
     */
    http::response<http::string_body> respondToEvent(
        const http::request<http::string_body>& req,
        int64_t event_id
    );

    /**
     * @brief 解决紧急事件
     * POST /api/v1/emergency/events/{id}/resolve
     *
     * @param req HTTP请求对象
     * @param event_id 事件ID
     * @return HTTP响应对象
     */
    http::response<http::string_body> resolveEvent(
        const http::request<http::string_body>& req,
        int64_t event_id
    );

    /**
     * @brief 取消紧急事件
     * POST /api/v1/emergency/events/{id}/cancel
     *
     * @param req HTTP请求对象
     * @param event_id 事件ID
     * @return HTTP响应对象
     */
    http::response<http::string_body> cancelEvent(
        const http::request<http::string_body>& req,
        int64_t event_id
    );

    /**
     * @brief 获取紧急事件统计
     * GET /api/v1/emergency/statistics
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getStatistics(
        const http::request<http::string_body>& req
    );

private:
    /**
     * @brief 从请求中提取Bearer Token
     * @param req HTTP请求对象
     * @return Token字符串
     */
    std::string extractBearerToken(const http::request<http::string_body>& req);

    /**
     * @brief 验证JWT Token并获取用户ID
     * @param req HTTP请求对象
     * @return 用户ID（可选）
     */
    std::optional<int64_t> validateTokenAndGetUserId(
        const http::request<http::string_body>& req
    );

    /**
     * @brief 解析URL查询参数
     * @param req HTTP请求对象
     * @return 查询参数映射
     */
    std::map<std::string, std::string> parseQueryParams(
        const http::request<http::string_body>& req
    );
};

} // namespace controllers
