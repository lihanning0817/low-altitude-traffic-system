#include "controllers/AuthController.h"
#include "repositories/UserRepository.h"
#include "auth/JwtService.h"
#include "config/Config.h"
#include "database/DatabaseManager.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <memory>

/**
 * @brief AuthController测试程序
 * 测试用户认证控制器的各项功能
 */

void printTestHeader(const std::string& test_name) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "测试: " << test_name << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void printTestResult(const std::string& test_name, bool success) {
    std::cout << "[" << (success ? "✓" : "✗") << "] " << test_name
              << (success ? " - 通过" : " - 失败") << std::endl;
}

// 创建模拟HTTP请求
http::request<http::string_body> createMockRequest(
    http::verb method,
    const std::string& target,
    const std::string& body = "",
    const std::string& auth_header = "") {

    http::request<http::string_body> req{method, target, 11};
    req.set(http::field::host, "localhost");
    req.set(http::field::content_type, "application/json");

    if (!auth_header.empty()) {
        req.set(http::field::authorization, "Bearer " + auth_header);
    }

    if (!body.empty()) {
        req.body() = body;
        req.prepare_payload();
    }

    return req;
}

int main() {
    try {
        // 设置日志级别
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("=== AuthController测试开始 ===");

        // 加载配置
        auto& config = config::Config::getInstance();
        if (!config.loadFromFile("config/server.json")) {
            spdlog::warn("无法加载配置文件，使用默认配置");
        }

        // 初始化数据库连接
        auto dbManager = std::make_shared<database::DatabaseManager>();
        if (!dbManager->initialize()) {
            spdlog::error("数据库初始化失败");
            return 1;
        }

        // 创建依赖对象
        auto userRepository = std::make_shared<repositories::UserRepository>();
        auto jwtService = std::make_shared<auth::JwtService>(auth::JwtService::fromConfig());
        auto authController = std::make_unique<controllers::AuthController>(userRepository, jwtService);

        bool allTestsPassed = true;

        // ========== 测试1: 用户注册 ==========
        printTestHeader("用户注册功能测试");

        nlohmann::json registerData = {
            {"username", "test_user"},
            {"email", "test@example.com"},
            {"password", "TestPass123"},
            {"full_name", "测试用户"}
        };

        auto registerReq = createMockRequest(http::verb::post, "/api/auth/register", registerData.dump());
        auto registerResp = authController->registerUser(registerReq);

        bool test1_success = (registerResp.result_int() == 201);
        printTestResult("用户注册", test1_success);

        if (test1_success) {
            std::cout << "注册响应: " << registerResp.body() << std::endl;
        } else {
            std::cout << "注册失败响应: " << registerResp.body() << std::endl;
        }
        allTestsPassed = allTestsPassed && test1_success;

        // ========== 测试2: 重复注册（应该失败） ==========
        printTestHeader("重复注册测试");

        auto duplicateRegisterReq = createMockRequest(http::verb::post, "/api/auth/register", registerData.dump());
        auto duplicateRegisterResp = authController->registerUser(duplicateRegisterReq);

        bool test2_success = (duplicateRegisterResp.result_int() == 409); // 冲突
        printTestResult("重复注册检测", test2_success);

        if (test2_success) {
            std::cout << "重复注册正确被拒绝: " << duplicateRegisterResp.body() << std::endl;
        }
        allTestsPassed = allTestsPassed && test2_success;

        // ========== 测试3: 用户登录 ==========
        printTestHeader("用户登录功能测试");

        nlohmann::json loginData = {
            {"username", "test_user"},
            {"password", "TestPass123"}
        };

        auto loginReq = createMockRequest(http::verb::post, "/api/auth/login", loginData.dump());
        auto loginResp = authController->login(loginReq);

        bool test3_success = (loginResp.result_int() == 200);
        printTestResult("用户登录", test3_success);

        std::string accessToken;
        std::string refreshToken;

        if (test3_success) {
            try {
                nlohmann::json loginRespJson = nlohmann::json::parse(loginResp.body());
                if (loginRespJson.contains("data") &&
                    loginRespJson["data"].contains("access_token") &&
                    loginRespJson["data"].contains("refresh_token")) {
                    accessToken = loginRespJson["data"]["access_token"];
                    refreshToken = loginRespJson["data"]["refresh_token"];
                    std::cout << "登录成功，获得Token" << std::endl;
                    std::cout << "Access Token长度: " << accessToken.length() << std::endl;
                    std::cout << "Refresh Token长度: " << refreshToken.length() << std::endl;
                } else {
                    test3_success = false;
                    std::cout << "登录响应格式错误" << std::endl;
                }
            } catch (const std::exception& e) {
                test3_success = false;
                std::cout << "登录响应解析失败: " << e.what() << std::endl;
            }
        } else {
            std::cout << "登录失败响应: " << loginResp.body() << std::endl;
        }
        allTestsPassed = allTestsPassed && test3_success;

        // ========== 测试4: 错误密码登录 ==========
        printTestHeader("错误密码登录测试");

        nlohmann::json wrongLoginData = {
            {"username", "test_user"},
            {"password", "WrongPassword"}
        };

        auto wrongLoginReq = createMockRequest(http::verb::post, "/api/auth/login", wrongLoginData.dump());
        auto wrongLoginResp = authController->login(wrongLoginReq);

        bool test4_success = (wrongLoginResp.result_int() == 401);
        printTestResult("错误密码登录拒绝", test4_success);

        if (test4_success) {
            std::cout << "错误密码正确被拒绝" << std::endl;
        }
        allTestsPassed = allTestsPassed && test4_success;

        // ========== 测试5: 获取当前用户信息 ==========
        if (!accessToken.empty()) {
            printTestHeader("获取当前用户信息测试");

            auto getUserReq = createMockRequest(http::verb::get, "/api/auth/me", "", accessToken);
            auto getUserResp = authController->getCurrentUser(getUserReq);

            bool test5_success = (getUserResp.result_int() == 200);
            printTestResult("获取用户信息", test5_success);

            if (test5_success) {
                std::cout << "用户信息响应: " << getUserResp.body() << std::endl;
            } else {
                std::cout << "获取用户信息失败: " << getUserResp.body() << std::endl;
            }
            allTestsPassed = allTestsPassed && test5_success;
        }

        // ========== 测试6: Token刷新 ==========
        if (!refreshToken.empty()) {
            printTestHeader("Token刷新测试");

            nlohmann::json refreshData = {
                {"refresh_token", refreshToken}
            };

            auto refreshReq = createMockRequest(http::verb::post, "/api/auth/refresh", refreshData.dump());
            auto refreshResp = authController->refreshToken(refreshReq);

            bool test6_success = (refreshResp.result_int() == 200);
            printTestResult("Token刷新", test6_success);

            if (test6_success) {
                try {
                    nlohmann::json refreshRespJson = nlohmann::json::parse(refreshResp.body());
                    if (refreshRespJson.contains("data") &&
                        refreshRespJson["data"].contains("access_token")) {
                        std::string newAccessToken = refreshRespJson["data"]["access_token"];
                        std::cout << "Token刷新成功，新Token长度: " << newAccessToken.length() << std::endl;
                        accessToken = newAccessToken; // 更新token用于后续测试
                    } else {
                        test6_success = false;
                        std::cout << "Token刷新响应格式错误" << std::endl;
                    }
                } catch (const std::exception& e) {
                    test6_success = false;
                    std::cout << "Token刷新响应解析失败: " << e.what() << std::endl;
                }
            } else {
                std::cout << "Token刷新失败: " << refreshResp.body() << std::endl;
            }
            allTestsPassed = allTestsPassed && test6_success;
        }

        // ========== 测试7: 修改密码 ==========
        if (!accessToken.empty()) {
            printTestHeader("修改密码测试");

            nlohmann::json passwordData = {
                {"old_password", "TestPass123"},
                {"new_password", "NewPass456"}
            };

            auto changePasswordReq = createMockRequest(http::verb::put, "/api/auth/password", passwordData.dump(), accessToken);
            auto changePasswordResp = authController->changePassword(changePasswordReq);

            bool test7_success = (changePasswordResp.result_int() == 200);
            printTestResult("修改密码", test7_success);

            if (test7_success) {
                std::cout << "密码修改成功" << std::endl;
            } else {
                std::cout << "密码修改失败: " << changePasswordResp.body() << std::endl;
            }
            allTestsPassed = allTestsPassed && test7_success;
        }

        // ========== 测试8: 使用新密码登录 ==========
        printTestHeader("新密码登录测试");

        nlohmann::json newLoginData = {
            {"username", "test_user"},
            {"password", "NewPass456"}
        };

        auto newLoginReq = createMockRequest(http::verb::post, "/api/auth/login", newLoginData.dump());
        auto newLoginResp = authController->login(newLoginReq);

        bool test8_success = (newLoginResp.result_int() == 200);
        printTestResult("新密码登录", test8_success);

        if (test8_success) {
            std::cout << "新密码登录成功" << std::endl;
        }
        allTestsPassed = allTestsPassed && test8_success;

        // ========== 测试9: 登出 ==========
        if (!accessToken.empty()) {
            printTestHeader("用户登出测试");

            auto logoutReq = createMockRequest(http::verb::post, "/api/auth/logout", "", accessToken);
            auto logoutResp = authController->logout(logoutReq);

            bool test9_success = (logoutResp.result_int() == 200);
            printTestResult("用户登出", test9_success);

            if (test9_success) {
                std::cout << "登出成功" << std::endl;
            }
            allTestsPassed = allTestsPassed && test9_success;
        }

        // ========== 测试10: 参数验证测试 ==========
        printTestHeader("参数验证测试");

        // 测试缺少参数的注册请求
        nlohmann::json invalidRegisterData = {
            {"username", "test"}  // 缺少email和password
        };

        auto invalidRegisterReq = createMockRequest(http::verb::post, "/api/auth/register", invalidRegisterData.dump());
        auto invalidRegisterResp = authController->registerUser(invalidRegisterReq);

        bool test10_success = (invalidRegisterResp.result_int() == 400);
        printTestResult("参数验证", test10_success);

        if (test10_success) {
            std::cout << "参数验证正确工作" << std::endl;
        }
        allTestsPassed = allTestsPassed && test10_success;

        // ========== 最终结果 ==========
        std::cout << "\n" << std::string(70, '=') << std::endl;
        if (allTestsPassed) {
            std::cout << "🎉 所有AuthController测试通过！认证系统功能正常" << std::endl;
            spdlog::info("All AuthController tests passed successfully");
        } else {
            std::cout << "❌ 部分AuthController测试失败！请检查实现" << std::endl;
            spdlog::error("Some AuthController tests failed");
        }
        std::cout << std::string(70, '=') << std::endl;

        return allTestsPassed ? 0 : 1;

    } catch (const std::exception& e) {
        spdlog::error("AuthController测试过程中发生异常: {}", e.what());
        std::cout << "测试异常: " << e.what() << std::endl;
        return 1;
    }
}