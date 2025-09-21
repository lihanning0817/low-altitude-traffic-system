#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "config/Config.h"
#include "database/DatabaseManager.h"
#include "repositories/UserRepository.h"
#include "controllers/AuthController.h"
#include "auth/JwtService.h"
#include "utils/HttpResponse.h"

// 模拟HTTP请求
class MockRequest {
public:
    std::string body_;
    std::map<std::string, std::string> headers_;

    MockRequest(const std::string& body) : body_(body) {}

    std::string body() const { return body_; }

    auto find(const std::string& field) const {
        return headers_.find(field);
    }

    auto end() const {
        return headers_.end();
    }

    void setHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }
};

void testAuthController() {
    try {
        std::cout << "\n=== 初始化测试环境 ===\n";

        // 初始化日志
        auto console = spdlog::stdout_color_mt("console");
        spdlog::set_default_logger(console);
        spdlog::set_level(spdlog::level::info);

        // 加载配置
        auto& config = config::Config::getInstance();
        config.loadFromFile("config/server.json");

        // 初始化数据库
        auto& db_manager = database::DatabaseManager::getInstance();
        database::DatabaseConfig db_config{
            config.getString("database.host", "localhost"),
            config.getInt("database.port", 33060),
            config.getString("database.database", "low_altitude_traffic_system"),
            config.getString("database.username", "root"),
            config.getString("database.password", "Aabc123456+"),
            config.getInt("database.connection_timeout", 10),
            config.getBool("database.use_ssl", false)
        };

        if (!db_manager.initialize(db_config)) {
            std::cerr << "数据库连接失败！\n";
            return;
        }
        std::cout << "数据库连接成功\n";

        // 创建控制器
        auto userRepo = std::make_shared<repositories::UserRepository>();
        auto jwtService = std::make_shared<auth::JwtService>(auth::JwtService::fromConfig());
        auto authController = std::make_shared<controllers::AuthController>(userRepo, jwtService);

        std::cout << "\n=== 测试1: 用户注册 ===\n";

        // 构造注册请求
        nlohmann::json registerData = {
            {"username", "testuser001"},
            {"email", "testuser001@example.com"},
            {"password", "Test123456"},
            {"full_name", "测试用户001"}
        };

        MockRequest registerReq(registerData.dump());

        std::cout << "请求数据: " << registerReq.body() << "\n";

        // 调用注册接口（这里需要转换类型）
        // 由于实际的HTTP类型复杂，我们直接调用内部逻辑

        std::cout << "直接测试 UserRepository 功能:\n";

        // 检查用户名是否存在
        bool exists = userRepo->isUsernameExists("testuser001");
        std::cout << "用户名 testuser001 是否存在: " << (exists ? "是" : "否") << "\n";

        if (!exists) {
            // 创建用户对象
            models::User newUser;
            newUser.setUsername("testuser001");
            newUser.setEmail("testuser001@example.com");
            newUser.setFullName("测试用户001");
            newUser.setRole(models::UserRole::USER);
            newUser.setStatus(models::UserStatus::ACTIVE);

            // 设置密码哈希
            std::string passwordHash = models::User::hashPassword("Test123456");
            newUser.setPasswordHash(passwordHash);

            // 创建用户
            int64_t userId = userRepo->createUser(newUser);
            if (userId > 0) {
                std::cout << "✅ 用户创建成功！ID: " << userId << "\n";
                newUser.setId(userId);

                std::cout << "\n=== 测试2: 用户登录 ===\n";

                // 验证用户登录
                auto userOpt = userRepo->authenticateUser("testuser001", "Test123456");
                if (userOpt.has_value()) {
                    std::cout << "✅ 用户验证成功！\n";

                    models::User user = userOpt.value();

                    // 生成JWT令牌
                    std::string accessToken = jwtService->generateAccessToken(user);
                    std::string refreshToken = jwtService->generateRefreshToken(user);

                    if (!accessToken.empty() && !refreshToken.empty()) {
                        std::cout << "✅ JWT令牌生成成功\n";
                        std::cout << "Access Token: " << accessToken.substr(0, 50) << "...\n";
                        std::cout << "Refresh Token: " << refreshToken.substr(0, 50) << "...\n";

                        std::cout << "\n=== 测试3: 令牌验证 ===\n";

                        // 验证令牌
                        auto validationResult = jwtService->validateToken(accessToken);
                        if (validationResult == auth::JwtValidationResult::VALID) {
                            std::cout << "✅ Access Token 验证成功\n";

                            // 提取用户ID
                            int64_t tokenUserId = jwtService->extractUserId(accessToken);
                            std::cout << "令牌中的用户ID: " << tokenUserId << "\n";

                            std::cout << "\n=== 测试4: 获取用户信息 ===\n";

                            // 根据ID获取用户
                            auto userByIdOpt = userRepo->getUserById(tokenUserId);
                            if (userByIdOpt.has_value()) {
                                std::cout << "✅ 根据ID获取用户成功\n";
                                models::User userById = userByIdOpt.value();
                                std::cout << "用户信息: " << userById.getUsername() << " (" << userById.getEmail() << ")\n";

                                std::cout << "\n=== 测试5: 修改密码 ===\n";

                                // 验证旧密码
                                if (models::User::verifyPassword("Test123456", userById.getPasswordHash())) {
                                    std::cout << "✅ 旧密码验证成功\n";

                                    // 更新密码
                                    std::string newPasswordHash = models::User::hashPassword("NewTest123456");
                                    bool passwordUpdated = userRepo->updatePassword(userById.getId(), newPasswordHash);
                                    if (passwordUpdated) {
                                        std::cout << "✅ 密码更新成功\n";

                                        // 验证新密码
                                        auto newLoginOpt = userRepo->authenticateUser("testuser001", "NewTest123456");
                                        if (newLoginOpt.has_value()) {
                                            std::cout << "✅ 新密码验证成功\n";
                                        } else {
                                            std::cout << "❌ 新密码验证失败\n";
                                        }
                                    } else {
                                        std::cout << "❌ 密码更新失败\n";
                                    }
                                } else {
                                    std::cout << "❌ 旧密码验证失败\n";
                                }
                            } else {
                                std::cout << "❌ 根据ID获取用户失败\n";
                            }
                        } else {
                            std::cout << "❌ Access Token 验证失败\n";
                        }
                    } else {
                        std::cout << "❌ JWT令牌生成失败\n";
                    }
                } else {
                    std::cout << "❌ 用户验证失败\n";
                }
            } else {
                std::cout << "❌ 用户创建失败\n";
            }
        } else {
            std::cout << "用户已存在，跳过创建\n";

            std::cout << "\n=== 测试现有用户登录 ===\n";
            auto userOpt = userRepo->authenticateUser("testuser001", "Test123456");
            if (userOpt.has_value()) {
                std::cout << "✅ 现有用户验证成功！\n";
            } else {
                // 尝试新密码
                userOpt = userRepo->authenticateUser("testuser001", "NewTest123456");
                if (userOpt.has_value()) {
                    std::cout << "✅ 现有用户（新密码）验证成功！\n";
                } else {
                    std::cout << "❌ 现有用户验证失败\n";
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "测试异常: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "=== AuthController 手动测试 ===\n";
    testAuthController();
    std::cout << "\n=== 测试完成 ===\n";
    return 0;
}