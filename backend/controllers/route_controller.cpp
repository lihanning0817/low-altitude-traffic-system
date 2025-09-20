#include "route_controller.h"
#include <json/json.h>

namespace controllers {
    httplib::Response RouteController::plan_route(const httplib::Request& req) {
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

        std::vector<std::string> required_fields = {"start_lat", "start_lng", "end_lat", "end_lng"};
        if (!validate_required_fields(root, required_fields)) {
            returnDatabaseConnection(dbConn);  // 返回数据库连接
            return create_error_response("Missing required fields");
        }

        double start_lat = root["start_lat"].asDouble();
        double start_lng = root["start_lng"].asDouble();
        double end_lat = root["end_lat"].asDouble();
        double end_lng = root["end_lng"].asDouble();

        std::string flight_task_id = "";
        if (root.isMember("flight_task_id")) {
            flight_task_id = root["flight_task_id"].asString();
        }

        auto route = route_service_->plan_route(start_lat, start_lng, end_lat, end_lng, user_id, flight_task_id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);

        if (route.has_value()) {
            Json::Value response;
            response["success"] = true;
            response["message"] = "Route planned successfully";
            response["route"] = route_to_json(route.value());
            return create_json_response(response, 201);
        } else {
            return create_error_response("Failed to plan route");
        }
    }

    httplib::Response RouteController::get_route_by_id(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto route = route_service_->get_route_by_id(id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        if (route.has_value()) {
            Json::Value response;
            response["success"] = true;
            response["route"] = route_to_json(route.value());
            return create_json_response(response);
        } else {
            return create_error_response("Route not found", 404);
        }
    }

    httplib::Response RouteController::get_routes_by_creator(const httplib::Request& req) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto routes = route_service_->get_routes_by_creator(user_id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        Json::Value response;
        response["success"] = true;
        response["routes"] = Json::Value(Json::arrayValue);

        for (const auto& route : routes) {
            response["routes"].append(route_to_json(route));
        }

        return create_json_response(response);
    }

    httplib::Response RouteController::get_routes_by_flight_task(const httplib::Request& req, const std::string& flight_task_id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto routes = route_service_->get_routes_by_flight_task(flight_task_id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        Json::Value response;
        response["success"] = true;
        response["routes"] = Json::Value(Json::arrayValue);

        for (const auto& route : routes) {
            response["routes"].append(route_to_json(route));
        }

        return create_json_response(response);
    }

    httplib::Response RouteController::get_all_routes(const httplib::Request& req) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // Only admins can get all routes
        // In a real implementation, you would check user permissions here

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto routes = route_service_->get_all_routes();
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        Json::Value response;
        response["success"] = true;
        response["routes"] = Json::Value(Json::arrayValue);

        for (const auto& route : routes) {
            response["routes"].append(route_to_json(route));
        }

        return create_json_response(response);
    }

    httplib::Response RouteController::update_route(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto route = route_service_->get_route_by_id(id);
        if (!route.has_value()) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Route not found", 404);
        }

        // Check if user is the creator
        if (route.value().creator_id != user_id) {
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

        // Update route fields
        if (root.isMember("name")) route.value().name = root["name"].asString();

        if (route_service_->update_route(route.value())) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            
            Json::Value response;
            response["success"] = true;
            response["message"] = "Route updated successfully";
            response["route"] = route_to_json(route.value());
            return create_json_response(response);
        } else {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Failed to update route");
        }
    }

    httplib::Response RouteController::delete_route(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto route = route_service_->get_route_by_id(id);
        if (!route.has_value()) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Route not found", 404);
        }

        // Check if user is the creator
        if (route.value().creator_id != user_id) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Permission denied", 403);
        }

        if (route_service_->delete_route(id)) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_success_response("Route deleted successfully");
        } else {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Failed to delete route");
        }
    }

    void RouteController::setup_routes(Server& app) {
        app.Post("/api/v1/routes/plan", [this](const httplib::Request& req, httplib::Response& res) {
            res = plan_route(req);
        });

        app.Get("/api/v1/routes/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_route_by_id(req, req.path_params.at("id"));
        });

        app.Get("/api/v1/routes/creator", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_routes_by_creator(req);
        });

        app.Get("/api/v1/routes/flight-task/:flight_task_id", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_routes_by_flight_task(req, req.path_params.at("flight_task_id"));
        });

        app.Get("/api/v1/routes", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_all_routes(req);
        });

        app.Put("/api/v1/routes/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = update_route(req, req.path_params.at("id"));
        });

        app.Delete("/api/v1/routes/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = delete_route(req, req.path_params.at("id"));
        });
    }

    Json::Value RouteController::route_to_json(const models::Route& route) {
        Json::Value route_obj;
        route_obj["id"] = route.id;
        route_obj["name"] = route.name;
        route_obj["total_distance"] = route.total_distance;
        route_obj["total_time"] = route.total_time;
        route_obj["creator_id"] = route.creator_id;
        route_obj["flight_task_id"] = route.flight_task_id;

        // Start point
        Json::Value start_point;
        start_point["latitude"] = route.start_point.latitude;
        start_point["longitude"] = route.start_point.longitude;
        route_obj["start_point"] = start_point;

        // End point
        Json::Value end_point;
        end_point["latitude"] = route.end_point.latitude;
        end_point["longitude"] = route.end_point.longitude;
        route_obj["end_point"] = end_point;

        // Waypoints
        route_obj["waypoints"] = Json::Value(Json::arrayValue);
        for (const auto& waypoint : route.waypoints) {
            Json::Value wp;
            wp["latitude"] = waypoint.latitude;
            wp["longitude"] = waypoint.longitude;
            wp["is_main_road"] = waypoint.is_main_road;
            route_obj["waypoints"].append(wp);
        }

        // Segments
        route_obj["segments"] = Json::Value(Json::arrayValue);
        for (const auto& segment : route.segments) {
            Json::Value seg;
            seg["distance"] = segment.distance;

            Json::Value from;
            from["latitude"] = segment.from.latitude;
            from["longitude"] = segment.from.longitude;
            seg["from"] = from;

            Json::Value to;
            to["latitude"] = segment.to.latitude;
            to["longitude"] = segment.to.longitude;
            seg["to"] = to;

            route_obj["segments"].append(seg);
        }

        auto created_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            route.created_at.time_since_epoch()).count();
        route_obj["created_at"] = std::to_string(created_at_ms);

        return route_obj;
    }
}
