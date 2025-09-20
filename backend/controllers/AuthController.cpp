#include "AuthController.h"
#include <json/json.h>
#include <memory>

namespace controllers {
    httplib::Response AuthController::login(const httplib::Request& req)
    {
        // 解析JSON请求体
        Json::Value root;
        if (!parse_json_body(req, root) || !root["username"].isString() || !root["password"].isString()) {
            // 返回错误响应
            return create_error_response("请求参数错误", 400);
        }
        
        std::string username = root["username"].asString();
        std::string password = root["password"].asString();
        
        try {
            // 查询用户
            UserDAO userDAO(nullptr); // 这里需要传入数据库连接，暂时传入nullptr
            std::shared_ptr<User> user = userDAO.findByUsername(username);
            
            if (!user || !user->verifyPassword(password)) {
                // 用户不存在或密码错误
                return create_error_response("用户名或密码错误", 401);
            }
            
            // 生成JWT令牌
            std::string token = JWTUtil::generateToken(user->getId(), user->getUsername(), user->getRole());
            
            // 返回成功响应
            Json::Value result;
            result["success"] = true;
            result["message"] = "登录成功";
            result["data"]["token"] = token;
            result["data"]["userId"] = user->getId();
            result["data"]["username"] = user->getUsername();
            result["data"]["role"] = user->getRole();
            
            return create_json_response(result, 200);
            
        } catch (const std::exception& e) {
            // 服务器内部错误
            return create_error_response("服务器内部错误", 500);
        }
    }
    
    httplib::Response AuthController::registerUser(const httplib::Request& req)
    {
        // 解析JSON请求体
        Json::Value root;
        if (!parse_json_body(req, root) || !root["username"].isString() || !root["password"].isString()) {
            // 返回错误响应
            return create_error_response("请求参数错误", 400);
        }
        
        std::string username = root["username"].asString();
        std::string password = root["password"].asString();
        
        try {
            // 检查用户名是否已存在
            UserDAO userDAO(nullptr); // 这里需要传入数据库连接，暂时传入nullptr
            std::shared_ptr<User> existingUser = userDAO.findByUsername(username);
            if (existingUser) {
                return create_error_response("用户名已存在", 409);
            }
            
            // 创建新用户
            std::string passwordHash = User::hashPassword(password);
            std::shared_ptr<User> newUser = std::make_shared<User>(0, username, passwordHash, "user", "");
            bool success = userDAO.insert(newUser);
            
            if (success) {
                // 注册成功
                return create_success_response("注册成功", 200);
            } else {
                // 注册失败
                return create_error_response("注册失败", 500);
            }
            
        } catch (const std::exception& e) {
            // 服务器内部错误
            return create_error_response("服务器内部错误", 500);
        }
    }
    
    httplib::Response AuthController::logout(const httplib::Request& req)
    {
        // 登出操作（实际项目中可能需要实现令牌黑名单等机制）
        return create_success_response("登出成功", 200);
    }
    
    void AuthController::setup_routes(Server& app)
    {
        app.Post("/api/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
            res = login(req);
        });
        
        app.Post("/api/auth/register", [this](const httplib::Request& req, httplib::Response& res) {
            res = registerUser(req);
        });
        
        app.Post("/api/auth/logout", [this](const httplib::Request& req, httplib::Response& res) {
            res = logout(req);
        });
    }
}
