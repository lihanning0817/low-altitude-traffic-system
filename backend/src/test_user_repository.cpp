#include "repositories/UserRepository.h"
#include "database/DatabaseManager.h"
#include "config/Config.h"
#include <spdlog/spdlog.h>
#include <iostream>

/**
 * @brief UserRepository功能测试程序
 * 用于验证UserRepository的各项功能是否正常工作
 */

void printUser(const models::User& user) {
    std::cout << "用户信息:" << std::endl;
    std::cout << "  ID: " << user.getId() << std::endl;
    std::cout << "  用户名: " << user.getUsername() << std::endl;
    std::cout << "  邮箱: " << user.getEmail() << std::endl;
    std::cout << "  真实姓名: " << user.getFullName() << std::endl;
    std::cout << "  角色: " << models::User::roleToString(user.getRole()) << std::endl;
    std::cout << "  状态: " << models::User::statusToString(user.getStatus()) << std::endl;
    if (user.getPhone().has_value()) {
        std::cout << "  电话: " << user.getPhone().value() << std::endl;
    }
    std::cout << "------------------------" << std::endl;
}

int main() {
    try {
        // 设置日志级别
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("=== UserRepository 功能测试开始 ===");

        // 加载配置
        auto& config = config::Config::getInstance();
        if (!config.loadFromFile("config/server.json")) {
            spdlog::warn("无法加载配置文件，使用默认配置");
        }

        // 初始化数据库
        auto& db_manager = database::DatabaseManager::getInstance();
        database::DatabaseConfig db_config{
            config.getString("database.host", "localhost"),
            config.getInt("database.port", 33060),
            config.getString("database.database", "low_altitude_traffic_system"),
            config.getString("database.username", "root"),
            config.getString("database.password", ""),
            config.getInt("database.connection_pool_size", 10),
            config.getInt("database.connection_timeout", 10)
        };

        if (!db_manager.initialize(db_config)) {
            spdlog::error("数据库初始化失败！");
            return 1;
        }

        spdlog::info("数据库连接成功！");

        // 创建UserRepository实例
        repositories::UserRepository userRepo;

        // 测试1: 创建新用户
        spdlog::info("=== 测试1: 创建新用户 ===");
        models::User newUser;
        newUser.setUsername("test_user_" + std::to_string(std::time(nullptr)));
        newUser.setEmail("test@example.com");
        newUser.setFullName("测试用户");
        newUser.setPassword("test123456");  // 这会自动生成哈希
        newUser.setRole(models::UserRole::USER);
        newUser.setStatus(models::UserStatus::ACTIVE);
        newUser.setPhone("13800138000");

        int64_t userId = userRepo.createUser(newUser);
        if (userId > 0) {
            spdlog::info("用户创建成功，ID: {}", userId);
        } else {
            spdlog::error("用户创建失败！");
            return 1;
        }

        // 测试2: 根据ID获取用户
        spdlog::info("=== 测试2: 根据ID获取用户 ===");
        auto userById = userRepo.getUserById(userId);
        if (userById.has_value()) {
            spdlog::info("成功获取用户:");
            printUser(userById.value());
        } else {
            spdlog::error("根据ID获取用户失败！");
        }

        // 测试3: 根据用户名获取用户
        spdlog::info("=== 测试3: 根据用户名获取用户 ===");
        auto userByUsername = userRepo.getUserByUsername(newUser.getUsername());
        if (userByUsername.has_value()) {
            spdlog::info("成功根据用户名获取用户:");
            printUser(userByUsername.value());
        } else {
            spdlog::error("根据用户名获取用户失败！");
        }

        // 测试4: 用户认证
        spdlog::info("=== 测试4: 用户认证 ===");
        auto authenticatedUser = userRepo.authenticateUser(newUser.getUsername(), "test123456");
        if (authenticatedUser.has_value()) {
            spdlog::info("用户认证成功:");
            printUser(authenticatedUser.value());
        } else {
            spdlog::error("用户认证失败！");
        }

        // 测试错误密码
        auto failedAuth = userRepo.authenticateUser(newUser.getUsername(), "wrong_password");
        if (!failedAuth.has_value()) {
            spdlog::info("错误密码认证正确被拒绝");
        } else {
            spdlog::error("错误密码认证不应该成功！");
        }

        // 测试5: 更新用户信息
        spdlog::info("=== 测试5: 更新用户信息 ===");
        if (userById.has_value()) {
            models::User updateUser = userById.value();
            updateUser.setFullName("更新后的测试用户");
            updateUser.setPhone("13900139000");

            if (userRepo.updateUser(updateUser)) {
                spdlog::info("用户信息更新成功");

                // 重新获取验证更新
                auto updatedUser = userRepo.getUserById(userId);
                if (updatedUser.has_value()) {
                    spdlog::info("验证更新结果:");
                    printUser(updatedUser.value());
                }
            } else {
                spdlog::error("用户信息更新失败！");
            }
        }

        // 测试6: 用户状态管理
        spdlog::info("=== 测试6: 用户状态管理 ===");
        if (userRepo.deactivateUser(userId)) {
            spdlog::info("用户禁用成功");
        }

        if (userRepo.activateUser(userId)) {
            spdlog::info("用户激活成功");
        }

        // 测试7: 获取用户列表
        spdlog::info("=== 测试7: 获取用户列表 ===");
        auto users = userRepo.getUsers(0, 10);
        spdlog::info("获取到 {} 个用户:", users.size());
        for (const auto& user : users) {
            printUser(user);
        }

        // 测试8: 获取用户总数
        spdlog::info("=== 测试8: 获取用户总数 ===");
        int totalUsers = userRepo.getUserCount();
        spdlog::info("系统中总共有 {} 个用户", totalUsers);

        // 测试9: 搜索用户
        spdlog::info("=== 测试9: 搜索用户 ===");
        auto searchResults = userRepo.searchUsers("测试", 0, 10);
        spdlog::info("搜索'测试'找到 {} 个用户:", searchResults.size());
        for (const auto& user : searchResults) {
            printUser(user);
        }

        // 测试10: 用户名和邮箱存在性检查
        spdlog::info("=== 测试10: 用户名和邮箱存在性检查 ===");
        bool usernameExists = userRepo.isUsernameExists(newUser.getUsername());
        bool emailExists = userRepo.isEmailExists(newUser.getEmail());
        spdlog::info("用户名存在: {}, 邮箱存在: {}", usernameExists, emailExists);

        // 测试11: 角色分布统计
        spdlog::info("=== 测试11: 用户角色分布统计 ===");
        auto roleDistribution = userRepo.getUserRoleDistribution();
        spdlog::info("用户角色分布: {}", roleDistribution.dump(2));

        // 清理测试数据（可选）
        spdlog::info("=== 清理测试数据 ===");
        if (userRepo.hardDeleteUser(userId)) {
            spdlog::info("测试用户删除成功");
        } else {
            spdlog::warn("测试用户删除失败，请手动清理");
        }

        spdlog::info("=== UserRepository 功能测试完成 ===");

    } catch (const std::exception& e) {
        spdlog::error("测试过程中发生异常: {}", e.what());
        return 1;
    }

    return 0;
}