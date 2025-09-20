#pragma once

#include "base_controller.h"
#include "../services/device_service.h"
#include <memory>
#include "server.h"

namespace controllers {
    class DroneController : public BaseController {
    private:
        std::shared_ptr<services::DeviceService> device_service_;

    public:
        explicit DroneController(std::shared_ptr<services::DeviceService> device_service)
            : device_service_(device_service) {}

        httplib::Response updateDroneStatus(const httplib::Request& req);
        httplib::Response getDrone(const httplib::Request& req, const std::string& id);
        void setup_routes(Server& app);
    };
}
