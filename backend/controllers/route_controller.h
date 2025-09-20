#ifndef CONTROLLERS_ROUTE_CONTROLLER_H
#define CONTROLLERS_ROUTE_CONTROLLER_H

#include "base_controller.h"
#include "../services/route_service.h"
#include "../services/flight_task_service.h"
#include <memory>
#include "server.h"

namespace controllers {
    class RouteController : public BaseController {
    private:
        std::shared_ptr<services::RouteService> route_service_;
        std::shared_ptr<services::FlightTaskService> flight_task_service_;

    public:
        explicit RouteController(std::shared_ptr<services::RouteService> route_service,
                               std::shared_ptr<services::FlightTaskService> flight_task_service)
            : route_service_(route_service), flight_task_service_(flight_task_service) {}

        // Route endpoints
        httplib::Response plan_route(const httplib::Request& req);
        httplib::Response get_route_by_id(const httplib::Request& req, const std::string& id);
        httplib::Response get_routes_by_creator(const httplib::Request& req);
        httplib::Response get_routes_by_flight_task(const httplib::Request& req, const std::string& flight_task_id);
        httplib::Response get_all_routes(const httplib::Request& req);
        httplib::Response update_route(const httplib::Request& req, const std::string& id);
        httplib::Response delete_route(const httplib::Request& req, const std::string& id);

        // Helper methods
        void setup_routes(Server& app);
        Json::Value route_to_json(const models::Route& route);
    };
}

#endif // CONTROLLERS_ROUTE_CONTROLLER_H
