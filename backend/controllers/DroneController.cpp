#include "DroneController.h"
#include <json/json.h>

namespace controllers {
    httplib::Response DroneController::updateDroneStatus(const httplib::Request& req) {
        Json::Value response;

        // Parse request body
        Json::Reader reader;
        Json::Value requestBody;
        if (!reader.parse(req.body, requestBody)) {
            response["success"] = false;
            response["message"] = "Invalid JSON format";
            return create_json_response(response, 400);
        }

        // Extract drone data (in a real app, you would update database)
        int droneId = requestBody["drone_id"].asInt();
        std::string status = requestBody["status"].asString();
        double battery = requestBody["battery"].asDouble();
        double latitude = requestBody["latitude"].asDouble();
        double longitude = requestBody["longitude"].asDouble();

        // For demo purposes, we'll just echo the data back
        response["success"] = true;
        response["message"] = "Drone status updated successfully";
        response["drone"]["id"] = droneId;
        response["drone"]["status"] = status;
        response["drone"]["battery"] = battery;
        response["drone"]["latitude"] = latitude;
        response["drone"]["longitude"] = longitude;
        response["drone"]["last_update"] = "2023-01-01 00:00:00";

        return create_json_response(response, 200);
    }

    httplib::Response DroneController::getDrone(const httplib::Request& req, const std::string& id) {
        Json::Value response;

        // For demo purposes, we'll return a fake drone
        response["success"] = true;
        response["drone"]["id"] = std::stoi(id);
        response["drone"]["model"] = "DJI Mavic 3";
        response["drone"]["status"] = "available";
        response["drone"]["battery"] = 100.0;
        response["drone"]["latitude"] = 39.9042;
        response["drone"]["longitude"] = 116.4074;
        response["drone"]["last_update"] = "2023-01-01 00:00:00";

        return create_json_response(response, 200);
    }

    void DroneController::setup_routes(Server& app) {
        app.Post("/api/drone/update-status", [this](const httplib::Request& req, httplib::Response& res) {
            res = updateDroneStatus(req);
        });

        app.Get("/api/drone/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = getDrone(req, req.path_params.at("id"));
        });
    }
}
