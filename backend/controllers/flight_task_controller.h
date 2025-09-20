#ifndef CONTROLLERS_FLIGHT_TASK_CONTROLLER_H
#define CONTROLLERS_FLIGHT_TASK_CONTROLLER_H

#include "base_controller.h"
#include "../services/flight_task_service.h"
#include "../services/device_service.h"
#include <memory>
#include "server.h"

namespace controllers {
    class FlightTaskController : public BaseController {
    private:
        std::shared_ptr<services::FlightTaskService> flight_task_service_;
        std::shared_ptr<services::DeviceService> device_service_;

    public:
        explicit FlightTaskController(std::shared_ptr<services::FlightTaskService> flight_task_service,
                                    std::shared_ptr<services::DeviceService> device_service)
            : flight_task_service_(flight_task_service), device_service_(device_service) {}

        // Flight task endpoints
        httplib::Response create_flight_task(const httplib::Request& req);
        httplib::Response get_flight_task_by_id(const httplib::Request& req, const std::string& id);
        httplib::Response get_flight_tasks_by_creator(const httplib::Request& req);
        httplib::Response get_flight_tasks_by_assignee(const httplib::Request& req, const std::string& assignee_id);
        httplib::Response get_flight_tasks_by_status(const httplib::Request& req, const std::string& status);
        httplib::Response get_all_flight_tasks(const httplib::Request& req);
        httplib::Response update_flight_task(const httplib::Request& req, const std::string& id);
        httplib::Response delete_flight_task(const httplib::Request& req, const std::string& id);

        // Flight task action endpoints
        httplib::Response assign_flight_task(const httplib::Request& req, const std::string& id);
        httplib::Response start_flight_task(const httplib::Request& req, const std::string& id);
        httplib::Response complete_flight_task(const httplib::Request& req, const std::string& id);
        httplib::Response cancel_flight_task(const httplib::Request& req, const std::string& id);
        httplib::Response update_flight_task_progress(const httplib::Request& req, const std::string& id);

        // Helper methods
        void setup_routes(Server& app);
        nlohmann::json flight_task_to_json(const FlightTask& task);
    };
}

#endif // CONTROLLERS_FLIGHT_TASK_CONTROLLER_H
