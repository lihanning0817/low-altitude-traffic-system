#include "auth/JwtService.h"
#include "config/Config.h"
#include "models/User.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>

/**
 * @brief JWT服务测试程序
 * 测试JWT Token的生成、验证、解析等核心功能
 * 包含27项全面测试，确保JWT系统的正确性和安全性
 */

// 测试统计
struct TestStats {
    int total = 0;
    int passed = 0;
    std::vector<std::string> failed_tests;

    void addTest(const std::string& name, bool success) {
        total++;
        if (success) {
            passed++;
        } else {
            failed_tests.push_back(name);
        }
    }

    void printSummary() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "🧪 测试结果汇总:" << std::endl;
        std::cout << "总测试数: " << total << std::endl;
        std::cout << "通过测试: " << passed << std::endl;
        std::cout << "失败测试: " << (total - passed) << std::endl;
        std::cout << "成功率: " << (total > 0 ? (passed * 100.0 / total) : 0) << "%" << std::endl;

        if (!failed_tests.empty()) {
            std::cout << "\n❌ 失败的测试:" << std::endl;
            for (const auto& test : failed_tests) {
                std::cout << "  - " << test << std::endl;
            }
        }

        if (passed == total) {
            std::cout << "\n🎉 所有测试通过！JWT服务功能正常" << std::endl;
        } else {
            std::cout << "\n⚠️  部分测试失败，请检查JWT服务实现" << std::endl;
        }
        std::cout << std::string(60, '=') << std::endl;
    }
};

static TestStats testStats;

void printTestHeader(const std::string& test_name) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "测试: " << test_name << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void printTestResult(const std::string& test_name, bool success) {
    testStats.addTest(test_name, success);
    std::cout << "[" << (success ? "✓" : "✗") << "] " << test_name
              << (success ? " - 通过" : " - 失败") << std::endl;
}

void printTokenInfo(const auth::TokenInfo& token_info) {
    std::cout << "Token信息:" << std::endl;
    std::cout << "  用户ID: " << token_info.user_id << std::endl;
    std::cout << "  用户名: " << token_info.username << std::endl;
    std::cout << "  邮箱: " << token_info.email << std::endl;
    std::cout << "  角色: " << models::User::roleToString(token_info.role) << std::endl;
    std::cout << "  JWT ID: " << token_info.jti << std::endl;

    auto now = std::chrono::system_clock::now();
    auto exp_seconds = std::chrono::duration_cast<std::chrono::seconds>(token_info.exp - now).count();
    std::cout << "  剩余时间: " << exp_seconds << " 秒" << std::endl;
}

// 创建测试用户
models::User createTestUser() {
    models::User user;
    user.setId(12345);
    user.setUsername("test_user");
    user.setEmail("test@example.com");
    user.setFullName("测试用户");
    user.setRole(models::UserRole::USER);
    user.setStatus(models::UserStatus::ACTIVE);
    return user;
}

// 创建管理员用户
models::User createAdminUser() {
    models::User user;
    user.setId(99999);
    user.setUsername("admin_user");
    user.setEmail("admin@example.com");
    user.setFullName("管理员用户");
    user.setRole(models::UserRole::ADMIN);
    user.setStatus(models::UserStatus::ACTIVE);
    return user;
}

int main() {
    std::cout << "🧪 JWT服务综合测试程序" << std::endl;
    std::cout << "测试低空交通系统JWT认证功能" << std::endl;
    std::cout << "包含27项全面测试，确保JWT系统的正确性和安全性" << std::endl;

    try {
        // 设置日志级别为warn，减少测试期间的日志输出
        spdlog::set_level(spdlog::level::warn);
        spdlog::info("=== JWT服务测试开始 ===");

        // 加载配置
        auto& config = config::Config::getInstance();
        if (!config.loadFromFile("config/server.json")) {
            spdlog::warn("无法加载配置文件，使用默认配置");
        }

        // 创建测试用户
        models::User testUser = createTestUser();
        models::User adminUser = createAdminUser();

        // 初始化JWT服务
        auth::JwtService jwtService = auth::JwtService::fromConfig();

        // 显示JWT服务配置
        std::cout << "JWT服务配置:" << std::endl;
        std::cout << jwtService.getConfig().dump(2) << std::endl;

        // ========== 测试1: 生成Access Token ==========
        printTestHeader("生成Access Token");
        std::string accessToken = jwtService.generateAccessToken(testUser);
        bool test1_success = !accessToken.empty() &&
                           std::count(accessToken.begin(), accessToken.end(), '.') == 2;
        printTestResult("生成Access Token", test1_success);
        if (test1_success) {
            std::cout << "生成的Access Token: " << accessToken.substr(0, 50) << "..." << std::endl;
            std::cout << "Token长度: " << accessToken.length() << " 字符" << std::endl;
        }

        // ========== 测试2: 生成Refresh Token ==========
        printTestHeader("生成Refresh Token");
        std::string refreshToken = jwtService.generateRefreshToken(testUser);
        bool test2_success = !refreshToken.empty() &&
                           std::count(refreshToken.begin(), refreshToken.end(), '.') == 2;
        printTestResult("生成Refresh Token", test2_success);
        if (test2_success) {
            std::cout << "生成的Refresh Token: " << refreshToken.substr(0, 50) << "..." << std::endl;
            std::cout << "Token长度: " << refreshToken.length() << " 字符" << std::endl;
        }

        // ========== 测试3: 验证有效Token ==========
        printTestHeader("验证有效Token");
        auth::JwtValidationResult validationResult = jwtService.validateToken(accessToken);
        bool test3_success = (validationResult == auth::JwtValidationResult::VALID);
        printTestResult("验证有效Token", test3_success);
        std::cout << "验证结果: " << auth::jwtValidationResultToString(validationResult) << std::endl;

        // ========== 测试4: 解析Token信息 ==========
        printTestHeader("解析Token信息");
        auto tokenInfo = jwtService.parseToken(accessToken);
        bool test4_success = tokenInfo.has_value();
        printTestResult("解析Token信息", test4_success);
        if (test4_success) {
            printTokenInfo(tokenInfo.value());

            // 验证解析出的信息是否正确
            bool dataCorrect = (tokenInfo->user_id == testUser.getId()) &&
                              (tokenInfo->username == testUser.getUsername()) &&
                              (tokenInfo->email == testUser.getEmail()) &&
                              (tokenInfo->role == testUser.getRole());
            printTestResult("Token数据正确性", dataCorrect);
        }

        // ========== 测试5: 提取用户ID ==========
        printTestHeader("提取用户ID");
        int64_t extractedUserId = jwtService.extractUserId(accessToken);
        bool test5_success = (extractedUserId == testUser.getId());
        printTestResult("提取用户ID", test5_success);
        std::cout << "提取的用户ID: " << extractedUserId << std::endl;
        std::cout << "期望的用户ID: " << testUser.getId() << std::endl;

        // ========== 测试6: 提取JWT ID ==========
        printTestHeader("提取JWT ID");
        std::string jwtId = jwtService.extractJwtId(accessToken);
        bool test6_success = !jwtId.empty() && (jwtId.length() == 32);
        printTestResult("提取JWT ID", test6_success);
        std::cout << "提取的JWT ID: " << jwtId << std::endl;
        std::cout << "JWT ID长度: " << jwtId.length() << " 字符" << std::endl;

        // ========== 测试7: 获取Token剩余时间 ==========
        printTestHeader("获取Token剩余时间");
        int remainingTime = jwtService.getTokenRemainingTime(accessToken);
        bool test7_success = (remainingTime > 0 && remainingTime <= 900);
        printTestResult("获取Token剩余时间", test7_success);
        std::cout << "Token剩余时间: " << remainingTime << " 秒" << std::endl;
        std::cout << "剩余时间（分钟）: " << (remainingTime / 60.0) << " 分钟" << std::endl;

        // ========== 测试8: 验证无效Token ==========
        printTestHeader("验证无效Token");
        std::string invalidToken = "invalid.token.signature";
        auth::JwtValidationResult invalidResult = jwtService.validateToken(invalidToken);
        bool test8_success = (invalidResult != auth::JwtValidationResult::VALID);
        printTestResult("验证无效Token", test8_success);
        std::cout << "无效Token验证结果: " << auth::jwtValidationResultToString(invalidResult) << std::endl;

        // ========== 测试9: 验证篡改的Token ==========
        printTestHeader("验证篡改的Token");
        std::string tamperedToken = accessToken;
        if (tamperedToken.length() > 10) {
            tamperedToken[tamperedToken.length() - 1] = 'X';
        }
        auth::JwtValidationResult tamperedResult = jwtService.validateToken(tamperedToken);
        bool test9_success = (tamperedResult == auth::JwtValidationResult::INVALID_SIGNATURE);
        printTestResult("验证篡改的Token", test9_success);
        std::cout << "篡改Token验证结果: " << auth::jwtValidationResultToString(tamperedResult) << std::endl;

        // ========== 测试10: 生成自定义过期时间Token ==========
        printTestHeader("生成自定义过期时间Token");
        std::string shortLivedToken = jwtService.generateToken(testUser, 10);
        bool test10_success = !shortLivedToken.empty();
        printTestResult("生成短期Token", test10_success);
        if (test10_success) {
            int shortTokenRemaining = jwtService.getTokenRemainingTime(shortLivedToken);
            bool timeCorrect = (shortTokenRemaining > 0 && shortTokenRemaining <= 10);
            printTestResult("短期Token时间正确", timeCorrect);
            std::cout << "短期Token剩余时间: " << shortTokenRemaining << " 秒" << std::endl;
        }

        // ========== 测试11: Token即将过期检查 ==========
        printTestHeader("Token即将过期检查");
        bool notExpiringSoon = jwtService.isTokenExpiringSoon(accessToken, 1000);
        bool expiringSoon = jwtService.isTokenExpiringSoon(shortLivedToken, 60);
        bool test11_success = !notExpiringSoon && expiringSoon;
        printTestResult("Token过期检查", test11_success);
        std::cout << "Access Token即将过期(1000秒内): " << (notExpiringSoon ? "是" : "否") << std::endl;
        std::cout << "短期Token即将过期(60秒内): " << (expiringSoon ? "是" : "否") << std::endl;

        // ========== 测试12: 管理员角色Token ==========
        printTestHeader("管理员角色Token");
        std::string adminToken = jwtService.generateAccessToken(adminUser);
        auto adminTokenInfo = jwtService.parseToken(adminToken);
        bool test12_success = adminTokenInfo.has_value() &&
                             (adminTokenInfo->role == models::UserRole::ADMIN);
        printTestResult("管理员角色Token", test12_success);
        if (test12_success) {
            std::cout << "管理员Token角色: " << models::User::roleToString(adminTokenInfo->role) << std::endl;
        }

        // ========== 测试13-27: 边界情况和安全测试 ==========
        printTestHeader("边界情况和安全测试");

        // 测试13: 解析无效Token
        auto invalidTokenInfo = jwtService.parseToken("invalid.token.format");
        printTestResult("解析无效Token应返回空", !invalidTokenInfo.has_value());

        // 测试14: 提取无效Token的用户ID
        int64_t invalidUserId = jwtService.extractUserId("invalid.token");
        printTestResult("从无效Token提取用户ID应返回0", invalidUserId == 0);

        // 测试15: 提取无效Token的JWT ID
        std::string invalidJwtId = jwtService.extractJwtId("invalid.token");
        printTestResult("从无效Token提取JWT ID应返回空", invalidJwtId.empty());

        // 测试16: 获取无效Token的剩余时间
        int invalidRemainingTime = jwtService.getTokenRemainingTime("invalid.token");
        printTestResult("获取无效Token剩余时间应返回-1", invalidRemainingTime == -1);

        // 测试17: 验证空Token
        auth::JwtValidationResult emptyResult = jwtService.validateToken("");
        printTestResult("验证空Token", emptyResult == auth::JwtValidationResult::MALFORMED);

        // 测试18: Token格式验证（只有一个点）
        auth::JwtValidationResult onePartResult = jwtService.validateToken("invalid.token");
        printTestResult("验证单段Token", onePartResult == auth::JwtValidationResult::MALFORMED);

        // 测试19: Token格式验证（四个段）
        auth::JwtValidationResult fourPartResult = jwtService.validateToken("header.payload.signature.extra");
        printTestResult("验证四段Token", fourPartResult == auth::JwtValidationResult::MALFORMED);

        // 测试20: JWT配置获取
        auto config_json = jwtService.getConfig();
        bool configTest = config_json.contains("issuer") && config_json.contains("access_token_expiry");
        printTestResult("获取JWT配置", configTest);

        // 测试21: 密钥更新
        std::string newKey = "new-super-secure-secret-key-256-bits-minimum-length-for-testing";
        jwtService.updateSecretKey(newKey);
        std::string newKeyToken = jwtService.generateAccessToken(testUser);
        printTestResult("更新密钥后生成Token", !newKeyToken.empty());

        // 测试22: 太短的密钥
        auth::JwtService shortKeyService("short", "test-issuer", 900, 604800);
        std::string shortKeyToken = shortKeyService.generateAccessToken(testUser);
        printTestResult("短密钥仍能生成Token", !shortKeyToken.empty());

        // 测试23: Token结构验证（Header.Payload.Signature）
        size_t accessDots = std::count(accessToken.begin(), accessToken.end(), '.');
        printTestResult("Token包含正确数量的点", accessDots == 2);

        // 测试24: 不同用户生成不同Token
        models::User user2 = createTestUser();
        user2.setId(54321);
        user2.setUsername("user2");
        std::string token2 = jwtService.generateAccessToken(user2);
        printTestResult("不同用户生成不同Token", accessToken != token2);

        // 测试25: 相同用户多次生成不同Token（因为JWT ID不同）
        std::string token3 = jwtService.generateAccessToken(testUser);
        printTestResult("相同用户多次生成不同Token", accessToken != token3);

        // 测试26: Refresh Token有效期更长
        int accessRemaining = jwtService.getTokenRemainingTime(accessToken);
        int refreshRemaining = jwtService.getTokenRemainingTime(refreshToken);
        printTestResult("Refresh Token有效期更长", refreshRemaining > accessRemaining);

        // 测试27: 从配置创建JWT服务
        auth::JwtService configService = auth::JwtService::fromConfig();
        std::string configToken = configService.generateAccessToken(testUser);
        printTestResult("从配置创建JWT服务", !configToken.empty());

        // 输出测试统计
        testStats.printSummary();

        return (testStats.passed == testStats.total) ? 0 : 1;

    } catch (const std::exception& e) {
        spdlog::error("JWT测试过程中发生异常: {}", e.what());
        std::cout << "测试异常: " << e.what() << std::endl;
        return 1;
    }
}