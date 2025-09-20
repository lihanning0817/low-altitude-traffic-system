#include "PaymentController.h"
#include <json/json.h>

namespace controllers {
    httplib::Response PaymentController::createPayment(const httplib::Request& req) {
        Json::Value response;

        // Parse request body
        Json::Reader reader;
        Json::Value requestBody;
        if (!reader.parse(req.body, requestBody)) {
            response["success"] = false;
            response["message"] = "Invalid JSON format";
            return create_json_response(response, 400);
        }

        // Extract payment data (in a real app, you would save to database)
        int userId = requestBody["user_id"].asInt();
        int taskId = requestBody["task_id"].asInt();
        double amount = requestBody["amount"].asDouble();

        // For demo purposes, we'll just echo the data back with a fake ID
        response["success"] = true;
        response["message"] = "Payment created successfully";
        response["payment"]["id"] = 1;
        response["payment"]["user_id"] = userId;
        response["payment"]["task_id"] = taskId;
        response["payment"]["amount"] = amount;
        response["payment"]["status"] = "pending";
        response["payment"]["created_at"] = "2023-01-01 00:00:00";

        return create_json_response(response, 200);
    }

    httplib::Response PaymentController::getPayment(const httplib::Request& req, const std::string& id) {
        Json::Value response;

        // For demo purposes, we'll return a fake payment
        response["success"] = true;
        response["payment"]["id"] = std::stoi(id);
        response["payment"]["user_id"] = 1;
        response["payment"]["task_id"] = 1;
        response["payment"]["amount"] = 99.99;
        response["payment"]["status"] = "completed";
        response["payment"]["created_at"] = "2023-01-01 00:00:00";

        return create_json_response(response, 200);
    }

    void PaymentController::setup_routes(Server& app) {
        app.Post("/api/payment/create", [this](const httplib::Request& req, httplib::Response& res) {
            res = createPayment(req);
        });

        app.Get("/api/payment/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = getPayment(req, req.path_params.at("id"));
        });
    }
}
