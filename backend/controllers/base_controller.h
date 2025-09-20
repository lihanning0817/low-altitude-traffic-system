#ifndef CONTROLLERS_BASE_CONTROLLER_H
#define CONTROLLERS_BASE_CONTROLLER_H

#include <nlohmann/json.hpp>
#include <memory>
#include "httplib.h"
#include "server.h"

namespace controllers {
    class BaseController {
    protected:
        // Helper methods for JSON responses
        httplib::Response create_json_response(const nlohmann::json& data, int status = 200);
        httplib::Response create_error_response(const std::string& message, int status = 400);
        httplib::Response create_success_response(const std::string& message, int status = 200);

        // Helper methods for JSON parsing
        bool parse_json_body(const httplib::Request& req, nlohmann::json& root);
        bool validate_required_fields(const nlohmann::json& root, const std::vector<std::string>& required_fields);

        // Helper methods for authentication
        bool validate_jwt_token(const httplib::Request& req, int& user_id);
        std::string get_bearer_token(const httplib::Request& req);
        
        // 获取服务器实例
        static std::shared_ptr<Server> getServerInstance();
        
        // 服务器实例引用
        static std::shared_ptr<Server> serverInstance;
        
    public:
        // 设置服务器实例
        static void setServerInstance(std::shared_ptr<Server> server);
    };
}

#endif // CONTROLLERS_BASE_CONTROLLER_H
