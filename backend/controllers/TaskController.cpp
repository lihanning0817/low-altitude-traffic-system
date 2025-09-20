#include "TaskController.h"
#include <json/json.h>

namespace controllers {
    httplib::Response TaskController::createTask(const httplib::Request& req) {
        Json::Value response;

        // Parse request body
        Json::Reader reader;
        Json::Value requestBody;
        if (!reader.parse(req.body, requestBody)) {
            response["success"] = false;
            response["message"] = "Invalid JSON format";
            return create_json_response(response, 400);
        }

        // Extract task data (in a real app, you would save to database)
        int userId = requestBody["user_id"].asInt();
        std::string type = requestBody["type"].asString();
        double startLat = requestBody["start_lat"].asDouble();
        double startLng = requestBody["start_lng"].asDouble();
        double endLat = requestBody["end_lat"].asDouble();
        double endLng = requestBody["end_lng"].asDouble();

        // For demo purposes, we'll just echo the data back with a fake ID
        response["success"] = true;
        response["message"] = "Task created successfully";
        response["task"]["id"] = 1;
        response["task"]["user_id"] = userId;
        response["task"]["type"] = type;
        response["task"]["status"] = "pending";
        response["task"]["start_lat"] = startLat;
        response["task"]["start_lng"] = startLng;
        response["task"]["end_lat"] = endLat;
        response["task"]["end_lng"] = endLng;
        response["task"]["created_at"] = "2023-01-01 00:00:00";

        return create_json_response(response, 200);
    }

    httplib::Response TaskController::getTask(const httplib::Request& req, const std::string& id) {
        Json::Value response;

        // For demo purposes, we'll return a fake task
        response["success"] = true;
        response["task"]["id"] = std::stoi(id);
        response["task"]["user_id"] = 1;
        response["task"]["type"] = "logistics";
        response["task"]["status"] = "in_progress";
        response["task"]["start_lat"] = 39.9042;
        response["task"]["start_lng"] = 116.4074;
        response["task"]["end_lat"] = 31.2304;
        response["task"]["end_lng"] = 121.4737;
        response["task"]["assigned_drone_id"] = 1;
        response["task"]["created_at"] = "2023-01-01 00:00:00";

        return create_json_response(response, 200);
    }

    httplib::Response TaskController::listTasks(const httplib::Request& req) {
        Json::Value response;

        // For demo purposes, we'll return a fake list of tasks
        response["success"] = true;
        response["tasks"][0]["id"] = 1;
        response["tasks"][0]["user_id"] = 1;
        response["tasks"][0]["type"] = "logistics";
        response["tasks"][0]["status"] = "in_progress";
        response["tasks"][0]["created_at"] = "2023-01-01 00:00:00";

        response["tasks"][1]["id"] = 2;
        response["tasks"][1]["user_id"] = 1;
        response["tasks"][1]["type"] = "agriculture";
        response["tasks"][1]["status"] = "pending";
        response["tasks"][1]["created_at"] = "2023-01-02 00:00:00";

        return create_json_response(response, 200);
    }

    void TaskController::setup_routes(Server& app) {
        app.Post("/api/task/create", [this](const httplib::Request& req, httplib::Response& res) {
            res = createTask(req);
        });

        app.Get("/api/task/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = getTask(req, req.path_params.at("id"));
        });

        app.Get("/api/task/list", [this](const httplib::Request& req, httplib::Response& res) {
            res = listTasks(req);
        });
    }
}
