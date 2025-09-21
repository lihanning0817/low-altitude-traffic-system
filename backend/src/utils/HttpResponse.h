#pragma once

#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;

namespace utils {

/**
 * @brief HTTP响应工具类
 * 提供统一的HTTP响应格式和创建方法
 */
class HttpResponse {
public:
    /**
     * @brief 创建JSON成功响应
     * @param data 响应数据
     * @param message 成功消息
     * @param status_code HTTP状态码（默认200）
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createSuccessResponse(
        const nlohmann::json& data = nlohmann::json::object(),
        const std::string& message = "Success",
        unsigned int status_code = 200
    );

    /**
     * @brief 创建JSON错误响应
     * @param message 错误消息
     * @param status_code HTTP状态码（默认400）
     * @param error_code 错误代码（可选）
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createErrorResponse(
        const std::string& message,
        unsigned int status_code = 400,
        const std::string& error_code = ""
    );

    /**
     * @brief 创建验证错误响应
     * @param validation_errors 验证错误列表
     * @param message 主要错误消息
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createValidationErrorResponse(
        const std::vector<std::string>& validation_errors,
        const std::string& message = "Validation failed"
    );

    /**
     * @brief 创建认证错误响应
     * @param message 错误消息
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createUnauthorizedResponse(
        const std::string& message = "Unauthorized"
    );

    /**
     * @brief 创建权限不足响应
     * @param message 错误消息
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createForbiddenResponse(
        const std::string& message = "Forbidden"
    );

    /**
     * @brief 创建资源未找到响应
     * @param message 错误消息
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createNotFoundResponse(
        const std::string& message = "Resource not found"
    );

    /**
     * @brief 创建服务器内部错误响应
     * @param message 错误消息
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createInternalErrorResponse(
        const std::string& message = "Internal server error"
    );

    /**
     * @brief 创建纯文本响应
     * @param content 文本内容
     * @param status_code HTTP状态码
     * @param content_type 内容类型
     * @return HTTP响应对象
     */
    static http::response<http::string_body> createTextResponse(
        const std::string& content,
        unsigned int status_code = 200,
        const std::string& content_type = "text/plain"
    );

private:
    /**
     * @brief 设置通用的响应头
     * @param response HTTP响应对象
     */
    static void setCommonHeaders(http::response<http::string_body>& response);

    /**
     * @brief 创建标准响应格式的JSON
     * @param success 是否成功
     * @param message 消息
     * @param data 数据（可选）
     * @param error_code 错误代码（可选）
     * @return 格式化的JSON对象
     */
    static nlohmann::json createStandardJsonResponse(
        bool success,
        const std::string& message,
        const nlohmann::json& data = nlohmann::json::object(),
        const std::string& error_code = ""
    );
};

} // namespace utils