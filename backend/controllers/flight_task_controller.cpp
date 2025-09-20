#include "flight_task_controller.h"
#include <json/json.h>

namespace controllers {
    httplib::Response FlightTaskController::create_flight_task(const httplib::Request& req) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        Json::Value root;
        if (!parse_json_body(req, root)) {
            returnDatabaseConnection(dbConn);  // 返回数据库连接
            return create_error_response("Invalid JSON format");
        }

        std::vector<std::string> required_fields = {"name", "description"};
        if (!validate_required_fields(root, required_fields)) {
            returnDatabaseConnection(dbConn);  // 返回数据库连接
            return create_error_response("Missing required fields");
        }

        models::FlightTaskPriority priority = models::FlightTaskPriority::MEDIUM;
        if (root.isMember("priority")) {
            try {
                priority = models::FlightTask::parse_priority_string(root["priority"].asString());
            } catch (const std::exception& e) {
                // Use default priority if parsing fails
            }
        }

        auto task = flight_task_service_->create_flight_task(
            root["name"].asString(),
            root["description"].asString(),
            user_id,
            priority
        );
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);

        if (task.has_value()) {
            Json::Value response;
            response["success"] = true;
            response["message"] = "Flight task created successfully";
            response["task"] = flight_task_to_json(task.value());
            return create_json_response(response, 201);
        } else {
            return create_error_response("Failed to create flight task");
        }
    }

    httplib::Response FlightTaskController::get_flight_task_by_id(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto task = flight_task_service_->get_flight_task_by_id(id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        if (task.has_value()) {
            Json::Value response;
            response["success"] = true;
            response["task"] = flight_task_to_json(task.value());
            return create_json_response(response);
        } else {
            return create_error_response("Flight task not found", 404);
        }
    }

    httplib::Response FlightTaskController::get_flight_tasks_by_creator(const httplib::Request& req) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto tasks = flight_task_service_->get_flight_tasks_by_creator(user_id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        Json::Value response;
        response["success"] = true;
        response["tasks"] = Json::Value(Json::arrayValue);

        for (const auto& task : tasks) {
            response["tasks"].append(flight_task_to_json(task));
        }

        return create_json_response(response);
    }

    httplib::Response FlightTaskController::get_flight_tasks_by_assignee(const httplib::Request& req, const std::string& assignee_id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto tasks = flight_task_service_->get_flight_tasks_by_assignee(assignee_id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        Json::Value response;
        response["success"] = true;
        response["tasks"] = Json::Value(Json::arrayValue);

        for (const auto& task : tasks) {
            response["tasks"].append(flight_task_to_json(task));
        }

        return create_json_response(response);
    }

    httplib::Response FlightTaskController::get_flight_tasks_by_status(const httplib::Request& req, const std::string& status) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        try {
            models::FlightTaskStatus task_status = models::FlightTask::parse_status_string(status);
            auto tasks = flight_task_service_->get_flight_tasks_by_status(task_status);
            
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            
            Json::Value response;
            response["success"] = true;
            response["tasks"] = Json::Value(Json::arrayValue);

            for (const auto& task : tasks) {
                response["tasks"].append(flight_task_to_json(task));
            }

            return create_json_response(response);
        } catch (const std::exception& e) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Invalid status parameter");
        }
    }

    httplib::Response FlightTaskController::get_all_flight_tasks(const httplib::Request& req) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // Only admins can get all flight tasks
        auto current_user = device_service_->get_device_by_id(std::to_string(user_id)); // This is a hack, should use user service
        // In a real implementation, you would check user permissions here

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto tasks = flight_task_service_->get_all_flight_tasks();
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        Json::Value response;
        response["success"] = true;
        response["tasks"] = Json::Value(Json::arrayValue);

        for (const auto& task : tasks) {
            response["tasks"].append(flight_task_to_json(task));
        }

        return create_json_response(response);
    }

    httplib::Response FlightTaskController::update_flight_task(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto task = flight_task_service_->get_flight_task_by_id(id);
        if (!task.has_value()) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Flight task not found", 404);
        }

        // Check if user is the creator or has permission to update
        if (task.value().creator_id != user_id) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Permission denied", 403);
        }

        Json::Value root;
        if (!parse_json_body(req, root)) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Invalid JSON format");
        }

        // Update task fields
        if (root.isMember("name")) task.value().name = root["name"].asString();
        if (root.isMember("description")) task.value().description = root["description"].asString();
        if (root.isMember("priority")) {
            try {
                task.value().priority = models::FlightTask::parse_priority_string(root["priority"].asString());
            } catch (const std::exception& e) {
                // Ignore invalid priority
            }
        }

        if (flight_task_service_->update_flight_task(task.value())) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            
            Json::Value response;
            response["success"] = true;
            response["message"] = "Flight task updated successfully";
            response["task"] = flight_task_to_json(task.value());
            return create_json_response(response);
        } else {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Failed to update flight task");
        }
    }

    httplib::Response FlightTaskController::delete_flight_task(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto task = flight_task_service_->get_flight_task_by_id(id);
        if (!task.has_value()) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Flight task not found", 404);
        }

        // Check if user is the creator
        if (task.value().creator_id != user_id) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Permission denied", 403);
        }

        if (flight_task_service_->delete_flight_task(id)) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_success_response("Flight task deleted successfully");
        } else {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Failed to delete flight task");
        }
    }

    httplib::Response FlightTaskController::assign_flight_task(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        Json::Value root;
        if (!parse_json_body(req, root)) {
            return create_error_response("Invalid JSON format");
        }

        std::vector<std::string> required_fields = {"device_id"};
        if (!validate_required_fields(root, required_fields)) {
            return create_error_response("Missing required fields");
        }

        if (flight_task_service_->assign_flight_task(id, root["device_id"].asString())) {
            return create_success_response("Flight task assigned successfully");
        } else {
            return create_error_response("Failed to assign flight task");
        }
    }

    httplib::Response FlightTaskController::start_flight_task(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        if (flight_task_service_->start_flight_task(id)) {
            return create_success_response("Flight task started successfully");
        } else {
            return create_error_response("Failed to start flight task");
        }
    }

    httplib::Response FlightTaskController::complete_flight_task(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        if (flight_task_service_->complete_flight_task(id)) {
            return create_success_response("Flight task completed successfully");
        } else {
            return create_error_response("Failed to complete flight task");
        }
    }

    httplib::Response FlightTaskController::cancel_flight_task(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        if (flight_task_service_->cancel_flight_task(id)) {
            return create_success_response("Flight task cancelled successfully");
        } else {
            return create_error_response("Failed to cancel flight task");
        }
    }

    httplib::Response FlightTaskController::update_flight_task_progress(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        Json::Value root;
        if (!parse_json_body(req, root)) {
            return create_error_response("Invalid JSON format");
        }

        std::vector<std::string> required_fields = {"progress"};
        if (!validate_required_fields(root, required_fields)) {
            return create_error_response("Missing required fields");
        }

        int progress = root["progress"].asInt();
        // Validate progress range
        if (progress < 0 || progress > 100) {
            return create_error_response("Progress must be between 0 and 100");
        }

        if (flight_task_service_->update_flight_task_progress(id, progress)) {
            return create_success_response("Flight task progress updated successfully");
        } else {
            return create_error_response("Failed to update flight task progress");
        }
    }

    void FlightTaskController::setup_routes(Server& app) {
        app.Post("/api/v1/flight-tasks", [this](const httplib::Request& req, httplib::Response& res) {
            res = create_flight_task(req);
        });

        app.Get("/api/v1/flight-tasks/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_flight_task_by_id(req, req.path_params.at("id"));
        });

        app.Get("/api/v1/flight-tasks/creator", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_flight_tasks_by_creator(req);
        });

        app.Get("/api/v1/flight-tasks/assignee/:assignee_id", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_flight_tasks_by_assignee(req, req.path_params.at("assignee_id"));
        });

        app.Get("/api/v1/flight-tasks/status/:status", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_flight_tasks_by_status(req, req.path_params.at("status"));
        });

        app.Get("/api/v1/flight-tasks", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_all_flight_tasks(req);
        });

        app.Put("/api/v1/flight-tasks/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = update_flight_task(req, req.path_params.at("id"));
        });

        app.Delete("/api/v1/flight-tasks/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = delete_flight_task(req, req.path_params.at("id"));
        });

        app.Post("/api/v1/flight-tasks/:id/assign", [this](const httplib::Request& req, httplib::Response& res) {
            res = assign_flight_task(req, req.path_params.at("id"));
        });

        app.Post("/api/v1/flight-tasks/:id/start", [this](const httplib::Request& req, httplib::Response& res) {
            res = start_flight_task(req, req.path_params.at("id"));
        });

        app.Post("/api/v1/flight-tasks/:id/complete", [this](const httplib::Request& req, httplib::Response& res) {
            res = complete_flight_task(req, req.path_params.at("id"));
        });

        app.Post("/api/v1/flight-tasks/:id/cancel", [this](const httplib::Request& req, httplib::Response& res) {
            res = cancel_flight_task(req, req.path_params.at("id"));
        });

        app.Put("/api/v1/flight-tasks/:id/progress", [this](const httplib::Request& req, httplib::Response& res) {
            res = update_flight_task_progress(req, req.path_params.at("id"));
        });
    }

    Json::Value FlightTaskController::flight_task_to_json(const models::FlightTask& task) {
        Json::Value task_obj;
        task_obj["id"] = task.id;
        task_obj["name"] = task.name;
        task_obj["description"] = task.description;
        task_obj["status"] = task.get_status_string();
        task_obj["priority"] = task.get_priority_string();
        task_obj["progress"] = task.progress;
        task_obj["creator_id"] = task.creator_id;

        if (task.assignee_id.has_value()) {
            task_obj["assignee_id"] = task.assignee_id.value();
        }

        if (task.start_time.has_value()) {
            auto start_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                task.start_time.value().time_since_epoch()).count();
            task_obj["start_time"] = std::to_string(start_time_ms);
        }

        if (task.end_time.has_value()) {
            auto end_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                task.end_time.value().time_since_epoch()).count();
            task_obj["end_time"] = std::to_string(end_time_ms);
        }

        if (task.estimated_duration.has_value()) {
            task_obj["estimated_duration"] = task.estimated_duration.value();
        }

        auto created_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            task.created_at.time_since_epoch()).count();
        task_obj["created_at"] = std::to_string(created_at_ms);

        auto updated_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            task.updated_at.time_since_epoch()).count();
        task_obj["updated_at"] = std::to_string(updated_at_ms);

        return task_obj;
    }
}
