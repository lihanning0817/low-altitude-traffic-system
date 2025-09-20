#include "base_controller.h"
#include "../utils/jwt.h"
#include <jwt-cpp/jwt.h>
#include <sstream>
#include <iostream>

namespace controllers {
    // 初始化静态成员
    std::shared_ptr<Server> BaseController::serverInstance = nullptr;
    
    void BaseController::setServerInstance(std::shared_ptr<Server> server) {
        serverInstance = server;
    }
    
    std::shared_ptr<Server> BaseController::getServerInstance() {
        return serverInstance;
    }
    
    httplib::Response BaseController::create_json_response(const Json::Value& data, int status) {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = ""; // Compact JSON
        std::string json_str = Json::writeString(builder, data);

        httplib::Response res;
        res.status = status;
        res.set_header("Content-Type", "application/json");
        res.body = json_str;
        return res;
    }

    httplib::Response BaseController::create_error_response(const std::string& message, int status) {
        Json::Value error;
        error["success"] = false;
        error["error"] = message;
        error["timestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

        return create_json_response(error, status);
    }

    httplib::Response BaseController::create_success_response(const std::string& message, int status) {
        Json::Value success;
        success["success"] = true;
        success["message"] = message;
        success["timestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

        return create_json_response(success, status);
    }

    bool BaseController::parse_json_body(const httplib::Request& req, Json::Value& root) {
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        std::string errors;

        return reader->parse(req.body.c_str(), req.body.c_str() + req.body.length(), &root, &errors);
    }

    bool BaseController::validate_required_fields(const Json::Value& root, const std::vector<std::string>& required_fields) {
        for (const auto& field : required_fields) {
            if (!root.isMember(field) || root[field].isNull()) {
                return false;
            }
        }
        return true;
    }

    bool BaseController::validate_jwt_token(const httplib::Request& req, int& user_id) {
        std::string token = get_bearer_token(req);
        if (token.empty()) {
            return false;
        }

        // In a real implementation, you would verify the token with your JWT manager
        // For now, we'll just check if it exists
        try {
            auto decoded = jwt::decode(token);
            // Extract user_id from token
            user_id = std::stoi(decoded.get_payload_claim("user_id").as_string());
            return true;
        } catch (const std::exception& e) {
            std::cerr << "JWT validation failed: " << e.what() << std::endl;
            return false;
        }
    }

    std::string BaseController::get_bearer_token(const httplib::Request& req) {
        auto it = req.headers.find("Authorization");
        if (it != req.headers.end()) {
            const std::string& auth_header = it->second;
            if (auth_header.substr(0, 7) == "Bearer ") {
                return auth_header.substr(7);
            }
        }
        return "";
    }
}
