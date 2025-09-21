#include "HttpResponse.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <iomanip>

namespace utils {

http::response<http::string_body> HttpResponse::createSuccessResponse(
    const nlohmann::json& data,
    const std::string& message,
    unsigned int status_code) {

    http::response<http::string_body> response;
    response.result(static_cast<http::status>(status_code));
    response.version(11);

    nlohmann::json response_body = createStandardJsonResponse(true, message, data);
    response.body() = response_body.dump();
    response.set(http::field::content_type, "application/json; charset=utf-8");

    setCommonHeaders(response);
    response.prepare_payload();

    return response;
}

http::response<http::string_body> HttpResponse::createErrorResponse(
    const std::string& message,
    unsigned int status_code,
    const std::string& error_code) {

    http::response<http::string_body> response;
    response.result(static_cast<http::status>(status_code));
    response.version(11);

    nlohmann::json response_body = createStandardJsonResponse(false, message, nlohmann::json::object(), error_code);
    response.body() = response_body.dump();
    response.set(http::field::content_type, "application/json; charset=utf-8");

    setCommonHeaders(response);
    response.prepare_payload();

    // 记录错误日志
    spdlog::warn("HTTP Error Response - Status: {}, Message: {}, Code: {}",
                status_code, message, error_code);

    return response;
}

http::response<http::string_body> HttpResponse::createValidationErrorResponse(
    const std::vector<std::string>& validation_errors,
    const std::string& message) {

    nlohmann::json data;
    data["validation_errors"] = validation_errors;

    http::response<http::string_body> response;
    response.result(static_cast<http::status>(400));
    response.version(11);
    nlohmann::json response_body = createStandardJsonResponse(false, message, data, "VALIDATION_ERROR");
    response.body() = response_body.dump();
    response.set(http::field::content_type, "application/json; charset=utf-8");

    setCommonHeaders(response);
    response.prepare_payload();

    return response;
}

http::response<http::string_body> HttpResponse::createUnauthorizedResponse(
    const std::string& message) {

    http::response<http::string_body> response;
    response.result(static_cast<http::status>(401));
    response.version(11);
    nlohmann::json response_body = createStandardJsonResponse(false, message, nlohmann::json::object(), "UNAUTHORIZED");
    response.body() = response_body.dump();
    response.set(http::field::content_type, "application/json; charset=utf-8");
    response.set(http::field::www_authenticate, "Bearer");

    setCommonHeaders(response);
    response.prepare_payload();

    return response;
}

http::response<http::string_body> HttpResponse::createForbiddenResponse(
    const std::string& message) {

    return createErrorResponse(message, 403, "FORBIDDEN");
}

http::response<http::string_body> HttpResponse::createNotFoundResponse(
    const std::string& message) {

    return createErrorResponse(message, 404, "NOT_FOUND");
}

http::response<http::string_body> HttpResponse::createInternalErrorResponse(
    const std::string& message) {

    // 记录内部错误
    spdlog::error("Internal Server Error: {}", message);

    return createErrorResponse("服务器内部错误", 500, "INTERNAL_ERROR");
}

http::response<http::string_body> HttpResponse::createTextResponse(
    const std::string& content,
    unsigned int status_code,
    const std::string& content_type) {

    http::response<http::string_body> response;
    response.result(static_cast<http::status>(status_code));
    response.version(11);
    response.body() = content;
    response.set(http::field::content_type, content_type + "; charset=utf-8");

    setCommonHeaders(response);
    response.prepare_payload();

    return response;
}

void HttpResponse::setCommonHeaders(http::response<http::string_body>& response) {
    // 设置CORS头
    response.set(http::field::access_control_allow_origin, "*");
    response.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
    response.set(http::field::access_control_allow_headers, "Content-Type, Authorization, X-Requested-With");
    response.set(http::field::access_control_max_age, "86400"); // 24小时

    // 设置安全头
    response.set("X-Content-Type-Options", "nosniff");
    response.set(http::field::x_frame_options, "DENY");
    response.set("X-XSS-Protection", "1; mode=block");

    // 设置服务器标识
    response.set(http::field::server, "Low-Altitude-Traffic-System/1.0");

    // 设置时间戳
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%a, %d %b %Y %H:%M:%S GMT");
    response.set(http::field::date, ss.str());
}

nlohmann::json HttpResponse::createStandardJsonResponse(
    bool success,
    const std::string& message,
    const nlohmann::json& data,
    const std::string& error_code) {

    nlohmann::json response;
    response["success"] = success;
    response["message"] = message;

    if (!data.empty()) {
        response["data"] = data;
    }

    if (!error_code.empty()) {
        response["error_code"] = error_code;
    }

    // 添加时间戳
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    response["timestamp"] = timestamp;

    return response;
}

} // namespace utils