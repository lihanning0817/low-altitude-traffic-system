#include "EmergencyController.h"
#include "utils/HttpResponse.h"
#include <iostream>
#include <sstream>

namespace controllers {

EmergencyController::EmergencyController(
    std::shared_ptr<repositories::EmergencyEventRepository> eventRepo,
    std::shared_ptr<auth::JwtService> jwtService
) : eventRepo_(eventRepo), jwtService_(jwtService) {
    std::cout << "[EmergencyController] Initializing Emergency Controller" << std::endl;
}

std::string EmergencyController::extractBearerToken(const http::request<http::string_body>& req) {
    auto it = req.find(http::field::authorization);
    if (it != req.end()) {
        std::string auth_header = it->value();
        const std::string bearer_prefix = "Bearer ";
        if (auth_header.substr(0, bearer_prefix.length()) == bearer_prefix) {
            return auth_header.substr(bearer_prefix.length());
        }
    }
    return "";
}

std::optional<int64_t> EmergencyController::validateTokenAndGetUserId(
    const http::request<http::string_body>& req
) {
    std::string token = extractBearerToken(req);
    if (token.empty()) {
        return std::nullopt;
    }

    try {
        auto validationResult = jwtService_->validateToken(token);
        if (validationResult == auth::JwtValidationResult::VALID) {
            auto tokenInfo = jwtService_->parseToken(token);
            if (tokenInfo.has_value()) {
                return tokenInfo->user_id;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] JWT validation error: " << e.what() << std::endl;
    }

    return std::nullopt;
}

std::map<std::string, std::string> EmergencyController::parseQueryParams(
    const http::request<http::string_body>& req
) {
    std::map<std::string, std::string> params;
    std::string target = req.target();
    size_t query_pos = target.find('?');

    if (query_pos != std::string::npos) {
        std::string query = target.substr(query_pos + 1);
        std::stringstream ss(query);
        std::string param;

        while (std::getline(ss, param, '&')) {
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = param.substr(0, eq_pos);
                std::string value = param.substr(eq_pos + 1);
                params[key] = value;
            }
        }
    }

    return params;
}

http::response<http::string_body> EmergencyController::createEvent(
    const http::request<http::string_body>& req
) {
    std::cout << "[EmergencyController] POST /api/v1/emergency/events" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录才能创建紧急事件");
        }

        // 解析请求体
        auto request_data = nlohmann::json::parse(req.body());

        // 创建事件对象
        models::EmergencyEvent event;
        event.event_code = eventRepo_->generateEventCode();
        event.task_id = request_data["task_id"].get<int64_t>();

        if (request_data.contains("drone_id") && !request_data["drone_id"].is_null()) {
            event.drone_id = request_data["drone_id"].get<int64_t>();
        }

        event.type = models::EmergencyEvent::stringToType(
            request_data.value("type", "other")
        );
        event.severity = models::EmergencyEvent::stringToSeverity(
            request_data.value("severity", "medium")
        );
        event.status = models::EmergencyStatus::ACTIVE;
        event.title = request_data["title"].get<std::string>();
        event.description = request_data["description"].get<std::string>();
        event.location = request_data["location"];

        // 验证数据
        if (!event.validate()) {
            return utils::HttpResponse::createErrorResponse("事件数据验证失败");
        }

        // 保存事件
        int64_t event_id = eventRepo_->createEvent(event);

        // 获取创建的事件
        auto created_event = eventRepo_->getEventById(event_id);
        if (!created_event.has_value()) {
            return utils::HttpResponse::createInternalErrorResponse("创建事件失败");
        }

        nlohmann::json response_data;
        response_data["event"] = created_event->toJson();

        return utils::HttpResponse::createSuccessResponse(response_data, "紧急事件创建成功");

    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] Error creating event: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("创建紧急事件失败");
    }
}

http::response<http::string_body> EmergencyController::getAllEvents(
    const http::request<http::string_body>& req
) {
    std::cout << "[EmergencyController] GET /api/v1/emergency/events" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录查看紧急事件");
        }

        // 解析查询参数
        auto params = parseQueryParams(req);

        int page = params.count("page") ? std::stoi(params["page"]) : 1;
        int page_size = params.count("page_size") ? std::stoi(params["page_size"]) : 20;

        std::optional<std::string> status = params.count("status") ?
            std::optional<std::string>(params["status"]) : std::nullopt;
        std::optional<std::string> severity = params.count("severity") ?
            std::optional<std::string>(params["severity"]) : std::nullopt;

        // 获取事件列表
        auto events = eventRepo_->getAllEvents(page, page_size, status, severity);

        nlohmann::json events_json = nlohmann::json::array();
        for (const auto& event : events) {
            events_json.push_back(event.toJson());
        }

        nlohmann::json response_data;
        response_data["events"] = events_json;
        response_data["page"] = page;
        response_data["page_size"] = page_size;
        response_data["total"] = events.size();

        return utils::HttpResponse::createSuccessResponse(response_data, "获取紧急事件列表成功");

    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] Error getting events: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取紧急事件列表失败");
    }
}

http::response<http::string_body> EmergencyController::getEventById(
    const http::request<http::string_body>& req,
    int64_t event_id
) {
    std::cout << "[EmergencyController] GET /api/v1/emergency/events/" << event_id << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录查看紧急事件");
        }

        // 获取事件
        auto event = eventRepo_->getEventById(event_id);
        if (!event.has_value()) {
            return utils::HttpResponse::createNotFoundResponse("紧急事件不存在");
        }

        nlohmann::json response_data;
        response_data["event"] = event->toJson();

        return utils::HttpResponse::createSuccessResponse(response_data, "获取紧急事件成功");

    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] Error getting event: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取紧急事件失败");
    }
}

http::response<http::string_body> EmergencyController::respondToEvent(
    const http::request<http::string_body>& req,
    int64_t event_id
) {
    std::cout << "[EmergencyController] POST /api/v1/emergency/events/" << event_id << "/respond" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录才能响应紧急事件");
        }

        int64_t user_id = user_id_opt.value();

        // 解析请求体
        auto request_data = nlohmann::json::parse(req.body());

        std::string response_action = request_data.value("response_action", "");
        std::string response_notes = request_data.value("response_notes", "");

        if (response_action.empty()) {
            return utils::HttpResponse::createErrorResponse("响应措施不能为空");
        }

        // 响应事件
        bool success = eventRepo_->respondToEvent(event_id, response_action, response_notes, user_id);
        if (!success) {
            return utils::HttpResponse::createInternalErrorResponse("响应紧急事件失败");
        }

        // 获取更新后的事件
        auto event = eventRepo_->getEventById(event_id);
        if (!event.has_value()) {
            return utils::HttpResponse::createNotFoundResponse("紧急事件不存在");
        }

        nlohmann::json response_data;
        response_data["event"] = event->toJson();

        return utils::HttpResponse::createSuccessResponse(response_data, "紧急事件响应成功");

    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] Error responding to event: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("响应紧急事件失败");
    }
}

http::response<http::string_body> EmergencyController::resolveEvent(
    const http::request<http::string_body>& req,
    int64_t event_id
) {
    std::cout << "[EmergencyController] POST /api/v1/emergency/events/" << event_id << "/resolve" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录才能解决紧急事件");
        }

        // 解决事件
        bool success = eventRepo_->resolveEvent(event_id);
        if (!success) {
            return utils::HttpResponse::createInternalErrorResponse("解决紧急事件失败");
        }

        // 获取更新后的事件
        auto event = eventRepo_->getEventById(event_id);
        if (!event.has_value()) {
            return utils::HttpResponse::createNotFoundResponse("紧急事件不存在");
        }

        nlohmann::json response_data;
        response_data["event"] = event->toJson();

        return utils::HttpResponse::createSuccessResponse(response_data, "紧急事件已解决");

    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] Error resolving event: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("解决紧急事件失败");
    }
}

http::response<http::string_body> EmergencyController::cancelEvent(
    const http::request<http::string_body>& req,
    int64_t event_id
) {
    std::cout << "[EmergencyController] POST /api/v1/emergency/events/" << event_id << "/cancel" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录才能取消紧急事件");
        }

        // 取消事件
        bool success = eventRepo_->cancelEvent(event_id);
        if (!success) {
            return utils::HttpResponse::createInternalErrorResponse("取消紧急事件失败");
        }

        // 获取更新后的事件
        auto event = eventRepo_->getEventById(event_id);
        if (!event.has_value()) {
            return utils::HttpResponse::createNotFoundResponse("紧急事件不存在");
        }

        nlohmann::json response_data;
        response_data["event"] = event->toJson();

        return utils::HttpResponse::createSuccessResponse(response_data, "紧急事件已取消");

    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] Error cancelling event: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("取消紧急事件失败");
    }
}

http::response<http::string_body> EmergencyController::getStatistics(
    const http::request<http::string_body>& req
) {
    std::cout << "[EmergencyController] GET /api/v1/emergency/statistics" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录查看统计信息");
        }

        // 获取统计信息
        auto statistics = eventRepo_->getEventStatistics();

        nlohmann::json response_data;
        response_data["statistics"] = statistics;

        return utils::HttpResponse::createSuccessResponse(response_data, "获取统计信息成功");

    } catch (const std::exception& e) {
        std::cerr << "[EmergencyController] Error getting statistics: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取统计信息失败");
    }
}

} // namespace controllers
