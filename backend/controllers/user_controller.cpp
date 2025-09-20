#include "user_controller.h"
#include "../utils/jwt.h"
#include <json/json.h>

namespace controllers {
    httplib::Response UserController::register_user(const httplib::Request& req) {
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

        std::vector<std::string> required_fields = {"username", "email", "password", "first_name", "last_name"};
        if (!validate_required_fields(root, required_fields)) {
            returnDatabaseConnection(dbConn);  // 返回数据库连接
            return create_error_response("Missing required fields");
        }

        try {
            auto user = user_service_->create_user(
                root["username"].asString(),
                root["email"].asString(),
                root["password"].asString(),
                root["first_name"].asString(),
                root["last_name"].asString(),
                root.get("phone", "").asString()
            );

            // 返回数据库连接
            returnDatabaseConnection(dbConn);

            if (user.has_value()) {
                Json::Value response;
                response["success"] = true;
                response["message"] = "User registered successfully";
                response["user"]["id"] = user.value().id;
                response["user"]["username"] = user.value().username;
                response["user"]["email"] = user.value().email;
                response["user"]["first_name"] = user.value().first_name;
                response["user"]["last_name"] = user.value().last_name;
                response["user"]["phone"] = user.value().phone;
                response["user"]["role"] = user.value().get_role_string();
                return create_json_response(response, 201);
            } else {
                return create_error_response("Failed to register user");
            }
        } catch (const std::exception& e) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response(e.what());
        }
    }

    httplib::Response UserController::login_user(const httplib::Request& req) {
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

        std::vector<std::string> required_fields = {"username", "password"};
        if (!validate_required_fields(root, required_fields)) {
            returnDatabaseConnection(dbConn);  // 返回数据库连接
            return create_error_response("Missing required fields");
        }

        auto user = user_service_->authenticate_user(
            root["username"].asString(),
            root["password"].asString()
        );

        // 返回数据库连接
        returnDatabaseConnection(dbConn);

        if (user.has_value()) {
            // Update last login
            user_service_->update_last_login(user.value().id);

            // Generate JWT token
            utils::JWTManager jwt_manager("low_altitude_traffic_secret_key", 24);
            std::string token = jwt_manager.generate_token(
                user.value().id,
                user.value().username,
                user.value().get_role_string()
            );

            Json::Value response;
            response["success"] = true;
            response["message"] = "Login successful";
            response["token"] = token;
            response["user"]["id"] = user.value().id;
            response["user"]["username"] = user.value().username;
            response["user"]["email"] = user.value().email;
            response["user"]["first_name"] = user.value().first_name;
            response["user"]["last_name"] = user.value().last_name;
            response["user"]["role"] = user.value().get_role_string();

            auto res = create_json_response(response);
            res.set_header("Authorization", "Bearer " + token);
            return res;
        } else {
            return create_error_response("Invalid username or password", 401);
        }
    }

    httplib::Response UserController::logout_user(const httplib::Request& req) {
        // In a stateless JWT system, logout is handled on the frontend
        // We'll just return a success message
        return create_success_response("Logout successful");
    }

    httplib::Response UserController::get_current_user(const httplib::Request& req) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto user = user_service_->get_user_by_id(user_id);
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);

        if (user.has_value()) {
            Json::Value response;
            response["success"] = true;
            response["user"]["id"] = user.value().id;
            response["user"]["username"] = user.value().username;
            response["user"]["email"] = user.value().email;
            response["user"]["first_name"] = user.value().first_name;
            response["user"]["last_name"] = user.value().last_name;
            response["user"]["phone"] = user.value().phone;
            response["user"]["role"] = user.value().get_role_string();
            return create_json_response(response);
        } else {
            return create_error_response("User not found", 404);
        }
    }

    httplib::Response UserController::get_user_by_id(const httplib::Request& req, const std::string& id) {
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
            int target_user_id = std::stoi(id);
            auto user = user_service_->get_user_by_id(target_user_id);
            
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            
            if (user.has_value()) {
                Json::Value response;
                response["success"] = true;
                response["user"]["id"] = user.value().id;
                response["user"]["username"] = user.value().username;
                response["user"]["email"] = user.value().email;
                response["user"]["first_name"] = user.value().first_name;
                response["user"]["last_name"] = user.value().last_name;
                response["user"]["phone"] = user.value().phone;
                response["user"]["role"] = user.value().get_role_string();
                return create_json_response(response);
            } else {
                return create_error_response("User not found", 404);
            }
        } catch (const std::exception& e) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Invalid user ID");
        }
    }

    httplib::Response UserController::get_all_users(const httplib::Request& req) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto users = user_service_->get_all_users();
        
        // 返回数据库连接
        returnDatabaseConnection(dbConn);
        
        Json::Value response;
        response["success"] = true;
        response["users"] = Json::Value(Json::arrayValue);

        for (const auto& user : users) {
            Json::Value user_obj;
            user_obj["id"] = user.id;
            user_obj["username"] = user.username;
            user_obj["email"] = user.email;
            user_obj["first_name"] = user.first_name;
            user_obj["last_name"] = user.last_name;
            user_obj["phone"] = user.phone;
            user_obj["role"] = user.get_role_string();
            response["users"].append(user_obj);
        }

        return create_json_response(response);
    }

    httplib::Response UserController::update_user(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // Check if user is updating their own profile or is an admin
        int target_user_id;
        try {
            target_user_id = std::stoi(id);
        } catch (const std::exception& e) {
            return create_error_response("Invalid user ID");
        }

        if (user_id != target_user_id) {
            // Check if current user is admin
            // 获取数据库连接
            auto dbConn = getDatabaseConnection();
            if (!dbConn) {
                return create_error_response("Database connection failed");
            }
            
            auto current_user = user_service_->get_user_by_id(user_id);
            
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            
            if (!current_user.has_value() || current_user.value().role != models::UserRole::ADMIN) {
                return create_error_response("Permission denied", 403);
            }
        }

        Json::Value root;
        if (!parse_json_body(req, root)) {
            return create_error_response("Invalid JSON format");
        }

        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto user = user_service_->get_user_by_id(target_user_id);
        if (!user.has_value()) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("User not found", 404);
        }

        // Update user fields
        if (root.isMember("first_name")) user.value().first_name = root["first_name"].asString();
        if (root.isMember("last_name")) user.value().last_name = root["last_name"].asString();
        if (root.isMember("phone")) user.value().phone = root["phone"].asString();
        if (root.isMember("email")) user.value().email = root["email"].asString();

        if (user_service_->update_user(user.value())) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            
            Json::Value response;
            response["success"] = true;
            response["message"] = "User updated successfully";
            response["user"]["id"] = user.value().id;
            response["user"]["username"] = user.value().username;
            response["user"]["email"] = user.value().email;
            response["user"]["first_name"] = user.value().first_name;
            response["user"]["last_name"] = user.value().last_name;
            response["user"]["phone"] = user.value().phone;
            response["user"]["role"] = user.value().get_role_string();
            return create_json_response(response);
        } else {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Failed to update user");
        }
    }

    httplib::Response UserController::delete_user(const httplib::Request& req, const std::string& id) {
        int user_id;
        if (!validate_jwt_token(req, user_id)) {
            return create_error_response("Invalid or missing token", 401);
        }

        // Check if current user is admin
        // 获取数据库连接
        auto dbConn = getDatabaseConnection();
        if (!dbConn) {
            return create_error_response("Database connection failed");
        }
        
        auto current_user = user_service_->get_user_by_id(user_id);
        if (!current_user.has_value() || current_user.value().role != models::UserRole::ADMIN) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Permission denied", 403);
        }

        try {
            int target_user_id = std::stoi(id);
            if (user_service_->delete_user(target_user_id)) {
                // 返回数据库连接
                returnDatabaseConnection(dbConn);
                return create_success_response("User deleted successfully");
            } else {
                // 返回数据库连接
                returnDatabaseConnection(dbConn);
                return create_error_response("Failed to delete user", 404);
            }
        } catch (const std::exception& e) {
            // 返回数据库连接
            returnDatabaseConnection(dbConn);
            return create_error_response("Invalid user ID");
        }
    }

    void UserController::setup_routes(Server& app) {
        app.Post("/api/v1/users/register", [this](const httplib::Request& req, httplib::Response& res) {
            res = register_user(req);
        });

        app.Post("/api/v1/users/login", [this](const httplib::Request& req, httplib::Response& res) {
            res = login_user(req);
        });

        app.Post("/api/v1/users/logout", [this](const httplib::Request& req, httplib::Response& res) {
            res = logout_user(req);
        });

        app.Get("/api/v1/users/me", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_current_user(req);
        });

        app.Get("/api/v1/users/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_user_by_id(req, req.path_params.at("id"));
        });

        app.Get("/api/v1/users", [this](const httplib::Request& req, httplib::Response& res) {
            res = get_all_users(req);
        });

        app.Put("/api/v1/users/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = update_user(req, req.path_params.at("id"));
        });

        app.Delete("/api/v1/users/:id", [this](const httplib::Request& req, httplib::Response& res) {
            res = delete_user(req, req.path_params.at("id"));
        });
    }
}
