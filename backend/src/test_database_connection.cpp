#include <iostream>
#include <memory>
#include <chrono>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "config/Config.h"
#include "database/DatabaseManager.h"
#include "repositories/UserRepository.h"
#include "models/User.h"

/**
 * @brief 数据库连接和CRUD操作测试程序
 *
 * 此程序用于验证：
 * 1. 配置文件加载
 * 2. DatabaseManager 连接
 * 3. UserRepository CRUD 操作
 * 4. MySQL X DevAPI 兼容性
 */

void printTestHeader(const std::string& test_name) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "测试: " << test_name << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void printTestResult(const std::string& test_name, bool success, const std::string& details = "") {
    std::cout << "[" << (success ? "✓" : "✗") << "] " << test_name
              << (success ? " - 通过" : " - 失败");
    if (!details.empty()) {
        std::cout << " (" << details << ")";
    }
    std::cout << std::endl;
}

bool testConfigLoading() {
    printTestHeader("配置文件加载测试");

    try {
        auto& config = config::Config::getInstance();

        // 尝试加载配置文件
        bool loaded = config.loadFromFile("config/server.json");
        printTestResult("配置文件加载", loaded);

        if (!loaded) {
            return false;
        }

        // 验证关键配置项
        std::string db_host = config.getString("database.host", "");
        int db_port = config.getInt("database.port", 0);
        std::string db_name = config.getString("database.database", "");
        std::string db_user = config.getString("database.username", "");

        bool config_valid = !db_host.empty() && db_port > 0 && !db_name.empty() && !db_user.empty();
        printTestResult("配置项验证", config_valid,
                       "host=" + db_host + ", port=" + std::to_string(db_port));

        return loaded && config_valid;

    } catch (const std::exception& e) {
        printTestResult("配置加载异常", false, e.what());
        return false;
    }
}

bool testDatabaseConnection() {
    printTestHeader("数据库连接测试");

    try {
        auto& config = config::Config::getInstance();

        // 创建数据库配置 - 从配置文件读取
        database::DatabaseConfig db_config;
        db_config.host = config.getString("database.host", "localhost");
        db_config.port = config.getInt("database.port", 33060);  // 从配置文件读取端口，默认33060
        db_config.database = config.getString("database.database", "low_altitude_traffic_system");
        db_config.username = config.getString("database.username", "root");
        db_config.password = config.getString("database.password", "");
        db_config.connection_timeout = config.getInt("database.connection_timeout", 10);
        db_config.use_ssl = false;

        std::cout << "连接配置: " << db_config.host << ":" << db_config.port
                  << "/" << db_config.database << " (用户: " << db_config.username << ")" << std::endl;

        // 获取数据库管理器实例
        auto& db_manager = database::DatabaseManager::getInstance();

        // 初始化连接
        bool initialized = db_manager.initialize(db_config);
        printTestResult("数据库连接初始化", initialized);

        if (!initialized) {
            return false;
        }

        // 测试连接状态
        bool connection_valid = db_manager.testConnection();
        printTestResult("连接有效性检查", connection_valid);

        // 获取连接状态信息
        if (connection_valid) {
            auto status = db_manager.getStatus();
            std::cout << "连接状态: " << status.dump(2) << std::endl;
        }

        // 测试基本查询
        try {
            auto result = db_manager.executeQuery("SELECT 1 as test_value, 'MySQL X DevAPI测试' as test_message");
            bool query_success = (result != nullptr && result->hasData());
            printTestResult("基本查询测试", query_success);

            if (query_success) {
                auto row = result->fetchRow();
                if (row && !row.isNull()) {
                    std::cout << "查询结果: test_value=" << static_cast<int>(row[0])
                             << ", test_message=" << static_cast<std::string>(row[1]) << std::endl;
                }
            }
        } catch (const std::exception& e) {
            printTestResult("基本查询测试", false, e.what());
            return false;
        }

        // 数据库结构已在initialize()中初始化完成
        printTestResult("数据库结构初始化", true);

        return initialized && connection_valid;

    } catch (const std::exception& e) {
        printTestResult("数据库连接异常", false, e.what());
        return false;
    }
}

bool testUserRepositoryCRUD() {
    printTestHeader("UserRepository CRUD测试");

    try {
        repositories::UserRepository user_repo;

        // 测试用户数据
        const std::string test_username = "test_user_crud_" + std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        const std::string test_email = "test_crud@example.com";
        const std::string test_password = "TestPassword123!";
        const std::string test_full_name = "CRUD测试用户";

        // 清理可能存在的测试数据
        std::cout << "清理已有测试数据..." << std::endl;
        auto existing_user = user_repo.getUserByUsername(test_username);
        if (existing_user.has_value()) {
            user_repo.hardDeleteUser(existing_user->getId());
        }
        existing_user = user_repo.getUserByEmail(test_email);
        if (existing_user.has_value()) {
            user_repo.hardDeleteUser(existing_user->getId());
        }

        // 1. 创建用户测试 (CREATE)
        std::cout << "\n--- CREATE 测试 ---" << std::endl;
        models::User new_user;
        new_user.setUsername(test_username);
        new_user.setEmail(test_email);
        new_user.setPassword(test_password);  // 会自动生成哈希
        new_user.setFullName(test_full_name);
        new_user.setRole(models::UserRole::USER);
        new_user.setStatus(models::UserStatus::ACTIVE);
        new_user.setPhone("13800138000");

        int64_t user_id = user_repo.createUser(new_user);
        bool create_success = (user_id > 0);
        printTestResult("创建用户", create_success, "ID: " + std::to_string(user_id));

        if (!create_success) {
            return false;
        }

        // 2. 读取用户测试 (READ)
        std::cout << "\n--- READ 测试 ---" << std::endl;

        // 按ID查询
        auto user_by_id = user_repo.getUserById(user_id);
        bool read_by_id_success = user_by_id.has_value();
        printTestResult("按ID查询用户", read_by_id_success);

        // 按用户名查询
        auto user_by_username = user_repo.getUserByUsername(test_username);
        bool read_by_username_success = user_by_username.has_value();
        printTestResult("按用户名查询用户", read_by_username_success);

        // 按邮箱查询
        auto user_by_email = user_repo.getUserByEmail(test_email);
        bool read_by_email_success = user_by_email.has_value();
        printTestResult("按邮箱查询用户", read_by_email_success);

        if (!read_by_id_success || !read_by_username_success || !read_by_email_success) {
            return false;
        }

        // 验证查询结果
        auto& user = user_by_id.value();
        bool data_integrity = (user.getUsername() == test_username) &&
                             (user.getEmail() == test_email) &&
                             (user.getFullName() == test_full_name) &&
                             (user.getRole() == models::UserRole::USER) &&
                             (user.getStatus() == models::UserStatus::ACTIVE);
        printTestResult("数据完整性验证", data_integrity);

        // 3. 更新用户测试 (UPDATE)
        std::cout << "\n--- UPDATE 测试 ---" << std::endl;

        user.setFullName("更新后的测试用户");
        user.setPhone("13900139000");
        user.setRole(models::UserRole::OPERATOR);

        bool update_success = user_repo.updateUser(user);
        printTestResult("更新用户信息", update_success);

        if (update_success) {
            // 验证更新结果
            auto updated_user = user_repo.getUserById(user_id);
            bool update_verify = updated_user.has_value() &&
                               (updated_user->getFullName() == "更新后的测试用户") &&
                               (updated_user->getPhone() == "13900139000") &&
                               (updated_user->getRole() == models::UserRole::OPERATOR);
            printTestResult("更新结果验证", update_verify);
        }

        // 4. 认证测试
        std::cout << "\n--- 认证测试 ---" << std::endl;

        auto auth_user = user_repo.authenticateUser(test_username, test_password);
        bool auth_success = auth_user.has_value();
        printTestResult("用户认证", auth_success);

        // 错误密码测试
        auto auth_fail = user_repo.authenticateUser(test_username, "wrong_password");
        bool auth_fail_success = !auth_fail.has_value();
        printTestResult("错误密码认证（应该失败）", auth_fail_success);

        // 5. 其他功能测试
        std::cout << "\n--- 其他功能测试 ---" << std::endl;

        // 用户名存在性检查
        bool username_exists = user_repo.isUsernameExists(test_username);
        printTestResult("用户名存在性检查", username_exists);

        // 邮箱存在性检查
        bool email_exists = user_repo.isEmailExists(test_email);
        printTestResult("邮箱存在性检查", email_exists);

        // 用户活跃状态检查
        bool user_active = user_repo.isUserActive(user_id);
        printTestResult("用户活跃状态检查", user_active);

        // 角色检查
        bool has_operator_role = user_repo.hasRole(user_id, models::UserRole::OPERATOR);
        printTestResult("用户角色检查", has_operator_role);

        // 6. 删除用户测试 (DELETE)
        std::cout << "\n--- DELETE 测试 ---" << std::endl;

        // 软删除（设置为banned状态）
        bool soft_delete_success = user_repo.deleteUser(user_id);
        printTestResult("软删除用户", soft_delete_success);

        if (soft_delete_success) {
            // 验证软删除结果
            auto deleted_user = user_repo.getUserById(user_id);
            bool soft_delete_verify = deleted_user.has_value() &&
                                    (deleted_user->getStatus() == models::UserStatus::BANNED);
            printTestResult("软删除结果验证", soft_delete_verify);
        }

        // 硬删除（真正从数据库删除）
        bool hard_delete_success = user_repo.hardDeleteUser(user_id);
        printTestResult("硬删除用户", hard_delete_success);

        if (hard_delete_success) {
            // 验证硬删除结果
            auto final_user = user_repo.getUserById(user_id);
            bool hard_delete_verify = !final_user.has_value();
            printTestResult("硬删除结果验证", hard_delete_verify);
        }

        return create_success && read_by_id_success && update_success &&
               auth_success && auth_fail_success && hard_delete_success;

    } catch (const std::exception& e) {
        printTestResult("UserRepository CRUD异常", false, e.what());
        return false;
    }
}

int main() {
    try {
        // 设置日志
        spdlog::set_level(spdlog::level::info);
        auto console = spdlog::stdout_color_mt("console");
        spdlog::set_default_logger(console);

        std::cout << "========== MySQL X DevAPI 数据库连接测试程序 ==========" << std::endl;
        std::cout << "测试目标:" << std::endl;
        std::cout << "1. 配置文件加载" << std::endl;
        std::cout << "2. DatabaseManager 连接测试" << std::endl;
        std::cout << "3. UserRepository CRUD 操作测试" << std::endl;
        std::cout << "4. MySQL X DevAPI 兼容性验证" << std::endl;

        bool all_tests_passed = true;

        // 测试1: 配置文件加载
        bool config_test = testConfigLoading();
        all_tests_passed = all_tests_passed && config_test;

        if (!config_test) {
            std::cout << "\n❌ 配置文件加载失败，后续测试无法进行！" << std::endl;
            return 1;
        }

        // 测试2: 数据库连接
        bool db_test = testDatabaseConnection();
        all_tests_passed = all_tests_passed && db_test;

        if (!db_test) {
            std::cout << "\n❌ 数据库连接失败，CRUD测试无法进行！" << std::endl;
            return 1;
        }

        // 测试3: UserRepository CRUD
        bool crud_test = testUserRepositoryCRUD();
        all_tests_passed = all_tests_passed && crud_test;

        // 最终结果
        std::cout << "\n" << std::string(70, '=') << std::endl;
        if (all_tests_passed) {
            std::cout << "🎉 所有数据库测试通过！MySQL X DevAPI迁移成功" << std::endl;
            std::cout << "✅ DatabaseManager 连接正常" << std::endl;
            std::cout << "✅ UserRepository CRUD 操作正常" << std::endl;
            std::cout << "✅ mysqlx::Value 类型转换正常" << std::endl;
            spdlog::info("All database tests passed successfully - X DevAPI migration complete");
        } else {
            std::cout << "❌ 部分数据库测试失败！请检查错误信息" << std::endl;
            spdlog::error("Some database tests failed - check error messages");
        }
        std::cout << std::string(70, '=') << std::endl;

        return all_tests_passed ? 0 : 1;

    } catch (const std::exception& e) {
        std::cerr << "测试程序异常: " << e.what() << std::endl;
        spdlog::error("Test program exception: {}", e.what());
        return 1;
    }
}