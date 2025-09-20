#pragma once

#include "base_controller.h"
#include "../models/user.h"
#include "../utils/jwt.h"
#include "../models/user_dao.h"
#include "server.h"

namespace controllers {
    /**
     * @brief 认证控制器
     * 
     * 处理用户登录、注册等认证相关请求
     */
    class AuthController : public BaseController
    {
    public:
        /**
         * @brief 用户登录接口
         * @param req HTTP请求对象
         */
        httplib::Response login(const httplib::Request& req);
        
        /**
         * @brief 用户注册接口
         * @param req HTTP请求对象
         */
        httplib::Response registerUser(const httplib::Request& req);
        
        /**
         * @brief 用户登出接口
         * @param req HTTP请求对象
         */
        httplib::Response logout(const httplib::Request& req);
        
        /**
         * @brief 设置路由
         * @param app 服务器对象
         */
        void setup_routes(Server& app);
    };
}
