#pragma once

#include "base_controller.h"
#include "server.h"

namespace controllers {
    class PaymentController : public BaseController {
    public:
        httplib::Response createPayment(const httplib::Request& req);
        httplib::Response getPayment(const httplib::Request& req, const std::string& id);
        void setup_routes(Server& app);
    };
}
