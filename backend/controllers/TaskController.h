#pragma once

#include "base_controller.h"
#include "server.h"

namespace controllers {
    class TaskController : public BaseController {
    public:
        httplib::Response createTask(const httplib::Request& req);
        httplib::Response getTask(const httplib::Request& req, const std::string& id);
        httplib::Response listTasks(const httplib::Request& req);
        void setup_routes(Server& app);
    };
}
