#ifndef CONTROLLERS_USER_CONTROLLER_H
#define CONTROLLERS_USER_CONTROLLER_H

#include "base_controller.h"
#include "../services/user_service.h"
#include <memory>
#include "server.h"

namespace controllers {
    class UserController : public BaseController {
    private:
        std::shared_ptr<services::UserService> user_service_;

    public:
        explicit UserController(std::shared_ptr<services::UserService> user_service)
            : user_service_(user_service) {}

        // User authentication endpoints
        httplib::Response register_user(const httplib::Request& req);
        httplib::Response login_user(const httplib::Request& req);
        httplib::Response logout_user(const httplib::Request& req);
        httplib::Response get_current_user(const httplib::Request& req);

        // User management endpoints
        httplib::Response get_user_by_id(const httplib::Request& req, const std::string& id);
        httplib::Response get_all_users(const httplib::Request& req);
        httplib::Response update_user(const httplib::Request& req, const std::string& id);
        httplib::Response delete_user(const httplib::Request& req, const std::string& id);

        // Helper methods
        void setup_routes(Server& app);
    };
}

#endif // CONTROLLERS_USER_CONTROLLER_H
