#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
#include <memory>
#include <random>
#include <sstream>
#include <iomanip>
#include <optional>

// 直接包含JWT相关的头文件，避免其他依赖
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

/**
 * JWT服务独立测试程序
 * 仅测试JWT核心功能，不依赖数据库和其他组件
 */

// 简化的用户角色枚举
enum class SimpleUserRole {
    USER,
    ADMIN,
    OPERATOR
};

// 简化的用户状态枚举
enum class SimpleUserStatus {
    ACTIVE,
    INACTIVE,
    PENDING,
    LOCKED
};

// 简化的用户类
class SimpleUser {
private:
    int64_t id_;
    std::string username_;
    std::string email_;
    std::string full_name_;
    SimpleUserRole role_;
    SimpleUserStatus status_;

public:
    // Getters
    int64_t getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getFullName() const { return full_name_; }
    SimpleUserRole getRole() const { return role_; }
    SimpleUserStatus getStatus() const { return status_; }

    // Setters
    void setId(int64_t id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setFullName(const std::string& full_name) { full_name_ = full_name; }
    void setRole(SimpleUserRole role) { role_ = role; }
    void setStatus(SimpleUserStatus status) { status_ = status; }

    // 角色转换函数
    static std::string roleToString(SimpleUserRole role) {
        switch (role) {
            case SimpleUserRole::USER: return "USER";
            case SimpleUserRole::ADMIN: return "ADMIN";
            case SimpleUserRole::OPERATOR: return "OPERATOR";
            default: return "UNKNOWN";
        }
    }

    static SimpleUserRole stringToRole(const std::string& role_str) {
        if (role_str == "ADMIN") return SimpleUserRole::ADMIN;
        if (role_str == "OPERATOR") return SimpleUserRole::OPERATOR;
        return SimpleUserRole::USER;
    }

    // 状态转换函数
    static std::string statusToString(SimpleUserStatus status) {
        switch (status) {
            case SimpleUserStatus::ACTIVE: return "ACTIVE";
            case SimpleUserStatus::INACTIVE: return "INACTIVE";
            case SimpleUserStatus::PENDING: return "PENDING";
            case SimpleUserStatus::LOCKED: return "LOCKED";
            default: return "UNKNOWN";
        }
    }
};

// JWT验证结果枚举
enum class JwtValidationResult {
    VALID,
    INVALID_SIGNATURE,
    EXPIRED,
    MALFORMED,
    INVALID_CLAIMS
};

// Token信息结构
struct TokenInfo {
    int64_t user_id;
    std::string username;
    std::string email;
    SimpleUserRole role;
    std::chrono::system_clock::time_point iat;
    std::chrono::system_clock::time_point exp;
    std::string jti;
};

// 简化的JWT服务类
class SimpleJwtService {
private:
    std::string secret_key_;
    std::string issuer_;
    int access_token_expiry_;
    int refresh_token_expiry_;

public:
    SimpleJwtService(const std::string& secret_key,
                    const std::string& issuer = "low-altitude-traffic-system",
                    int access_token_expiry = 900,
                    int refresh_token_expiry = 604800)
        : secret_key_(secret_key)
        , issuer_(issuer)
        , access_token_expiry_(access_token_expiry)
        , refresh_token_expiry_(refresh_token_expiry) {

        if (secret_key_.length() < 32) {
            spdlog::warn("JWT secret key is too short (less than 32 characters), security may be compromised");
        }

        spdlog::debug("SimpleJwtService initialized with issuer: {}", issuer_);
    }

    // 生成Access Token
    std::string generateAccessToken(const SimpleUser& user) {
        return generateToken(user, access_token_expiry_);
    }

    // 生成Refresh Token
    std::string generateRefreshToken(const SimpleUser& user) {
        return generateToken(user, refresh_token_expiry_);
    }

    // 生成自定义过期时间的Token
    std::string generateToken(const SimpleUser& user, int expiry_seconds) {
        try {
            std::string header = generateHeader();
            auto [payload, jti] = generatePayload(user, expiry_seconds);
            std::string signature = generateSignature(header, payload);

            std::string token = header + "." + payload + "." + signature;

            spdlog::debug("Generated JWT token for user: {} (ID: {}), JTI: {}",
                         user.getUsername(), user.getId(), jti);

            return token;
        } catch (const std::exception& e) {
            spdlog::error("Failed to generate JWT token for user {}: {}", user.getUsername(), e.what());
            return "";
        }
    }

    // 验证Token
    JwtValidationResult validateToken(const std::string& token) {
        try {
            if (!isValidTokenFormat(token)) {
                return JwtValidationResult::MALFORMED;
            }

            auto [header, payload, signature] = splitToken(token);

            if (!verifySignature(header, payload, signature)) {
                return JwtValidationResult::INVALID_SIGNATURE;
            }

            std::string decoded_payload = base64UrlDecode(payload);
            nlohmann::json payload_json = nlohmann::json::parse(decoded_payload);

            if (!payload_json.contains("exp")) {
                return JwtValidationResult::INVALID_CLAIMS;
            }

            int64_t exp_timestamp = payload_json["exp"].get<int64_t>();
            auto now = std::chrono::system_clock::now();
            auto exp_time = timestampToTime(exp_timestamp);

            if (now >= exp_time) {
                return JwtValidationResult::EXPIRED;
            }

            return JwtValidationResult::VALID;

        } catch (const std::exception& e) {
            spdlog::error("Token validation error: {}", e.what());
            return JwtValidationResult::MALFORMED;
        }
    }

    // 解析Token
    std::optional<TokenInfo> parseToken(const std::string& token) {
        if (validateToken(token) != JwtValidationResult::VALID) {
            return std::nullopt;
        }

        try {
            auto [header, payload, signature] = splitToken(token);

            std::string decoded_payload = base64UrlDecode(payload);
            nlohmann::json payload_json = nlohmann::json::parse(decoded_payload);

            TokenInfo token_info;
            token_info.user_id = payload_json["sub"].get<int64_t>();
            token_info.username = payload_json["username"].get<std::string>();
            token_info.email = payload_json["email"].get<std::string>();
            token_info.role = SimpleUser::stringToRole(payload_json["role"].get<std::string>());
            token_info.iat = timestampToTime(payload_json["iat"].get<int64_t>());
            token_info.exp = timestampToTime(payload_json["exp"].get<int64_t>());
            token_info.jti = payload_json["jti"].get<std::string>();

            return token_info;

        } catch (const std::exception& e) {
            spdlog::error("Token parsing error: {}", e.what());
            return std::nullopt;
        }
    }

    // 提取用户ID
    int64_t extractUserId(const std::string& token) {
        auto token_info = parseToken(token);
        return token_info.has_value() ? token_info->user_id : 0;
    }

    // 提取JWT ID
    std::string extractJwtId(const std::string& token) {
        auto token_info = parseToken(token);
        return token_info.has_value() ? token_info->jti : "";
    }

    // 获取Token剩余时间
    int getTokenRemainingTime(const std::string& token) {
        auto token_info = parseToken(token);
        if (!token_info.has_value()) {
            return -1;
        }

        auto now = std::chrono::system_clock::now();
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(token_info->exp - now);

        return static_cast<int>(remaining.count());
    }

    // 检查Token是否即将过期
    bool isTokenExpiringSoon(const std::string& token, int threshold_seconds = 300) {
        auto token_info = parseToken(token);
        if (!token_info.has_value()) {
            return true;
        }

        auto now = std::chrono::system_clock::now();
        auto threshold_time = now + std::chrono::seconds(threshold_seconds);

        return token_info->exp <= threshold_time;
    }

private:
    // 生成Header
    std::string generateHeader() {
        nlohmann::json header = {
            {"typ", "JWT"},
            {"alg", "HS256"}
        };
        return base64UrlEncode(header.dump());
    }

    // 生成Payload
    std::pair<std::string, std::string> generatePayload(const SimpleUser& user, int expiry_seconds) {
        auto now = std::chrono::system_clock::now();
        auto exp_time = now + std::chrono::seconds(expiry_seconds);

        std::string jti = generateJwtId();

        nlohmann::json payload = {
            {"iss", issuer_},
            {"sub", user.getId()},
            {"aud", "low-altitude-traffic-system-client"},
            {"exp", timeToTimestamp(exp_time)},
            {"iat", timeToTimestamp(now)},
            {"jti", jti},
            {"username", user.getUsername()},
            {"email", user.getEmail()},
            {"role", SimpleUser::roleToString(user.getRole())},
            {"status", SimpleUser::statusToString(user.getStatus())}
        };

        return {base64UrlEncode(payload.dump()), jti};
    }

    // 生成签名
    std::string generateSignature(const std::string& header, const std::string& payload) {
        std::string message = header + "." + payload;
        return base64UrlEncode(hmacSha256(secret_key_, message));
    }

    // 验证签名
    bool verifySignature(const std::string& header, const std::string& payload, const std::string& signature) {
        std::string expected_signature = generateSignature(header, payload);
        return signature == expected_signature;
    }

    // Base64 URL安全编码
    std::string base64UrlEncode(const std::string& data) {
        BIO* bio, *b64;
        BUF_MEM* bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, data.c_str(), static_cast<int>(data.length()));
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string encoded_data(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);

        // 转换为URL安全的Base64
        std::replace(encoded_data.begin(), encoded_data.end(), '+', '-');
        std::replace(encoded_data.begin(), encoded_data.end(), '/', '_');
        encoded_data.erase(std::remove(encoded_data.begin(), encoded_data.end(), '='), encoded_data.end());

        return encoded_data;
    }

    // Base64 URL安全解码
    std::string base64UrlDecode(const std::string& encoded) {
        std::string padded = encoded;

        // 恢复URL安全字符
        std::replace(padded.begin(), padded.end(), '-', '+');
        std::replace(padded.begin(), padded.end(), '_', '/');

        // 添加填充
        while (padded.length() % 4) {
            padded += "=";
        }

        BIO* bio, *b64;
        char* buffer = new char[padded.length()];

        bio = BIO_new_mem_buf(padded.c_str(), static_cast<int>(padded.length()));
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        int decoded_length = BIO_read(bio, buffer, static_cast<int>(padded.length()));
        BIO_free_all(bio);

        std::string decoded_data(buffer, decoded_length);
        delete[] buffer;

        return decoded_data;
    }

    // 生成JWT ID
    std::string generateJwtId() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);

        std::stringstream ss;
        for (int i = 0; i < 32; ++i) {
            ss << std::hex << dis(gen);
        }

        return ss.str();
    }

    // HMAC SHA256签名
    std::string hmacSha256(const std::string& key, const std::string& message) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;

        HMAC(EVP_sha256(),
             key.c_str(), static_cast<int>(key.length()),
             reinterpret_cast<const unsigned char*>(message.c_str()), message.length(),
             hash, &hash_len);

        return std::string(reinterpret_cast<char*>(hash), hash_len);
    }

    // 时间转换
    int64_t timeToTimestamp(const std::chrono::system_clock::time_point& time_point) {
        return std::chrono::duration_cast<std::chrono::seconds>(time_point.time_since_epoch()).count();
    }

    std::chrono::system_clock::time_point timestampToTime(int64_t timestamp) {
        return std::chrono::system_clock::from_time_t(timestamp);
    }

    // Token格式验证
    bool isValidTokenFormat(const std::string& token) {
        size_t first_dot = token.find('.');
        if (first_dot == std::string::npos) return false;

        size_t second_dot = token.find('.', first_dot + 1);
        if (second_dot == std::string::npos) return false;

        if (token.find('.', second_dot + 1) != std::string::npos) return false;

        if (first_dot == 0 || second_dot == first_dot + 1 || second_dot == token.length() - 1) {
            return false;
        }

        return true;
    }

    // 分割Token
    std::tuple<std::string, std::string, std::string> splitToken(const std::string& token) {
        size_t first_dot = token.find('.');
        size_t second_dot = token.find('.', first_dot + 1);

        std::string header = token.substr(0, first_dot);
        std::string payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
        std::string signature = token.substr(second_dot + 1);

        return {header, payload, signature};
    }
};

// 验证结果转字符串
std::string jwtValidationResultToString(JwtValidationResult result) {
    switch (result) {
        case JwtValidationResult::VALID:
            return "Token is valid";
        case JwtValidationResult::INVALID_SIGNATURE:
            return "Invalid token signature";
        case JwtValidationResult::EXPIRED:
            return "Token has expired";
        case JwtValidationResult::MALFORMED:
            return "Token is malformed";
        case JwtValidationResult::INVALID_CLAIMS:
            return "Token has invalid claims";
        default:
            return "Unknown validation result";
    }
}

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

void printTestResult(const std::string& test_name, bool success) {
    testStats.addTest(test_name, success);
    std::cout << "[" << (success ? "✓" : "✗") << "] " << test_name
              << (success ? " - 通过" : " - 失败") << std::endl;
}

// 创建测试用户
SimpleUser createTestUser() {
    SimpleUser user;
    user.setId(12345);
    user.setUsername("test_user");
    user.setEmail("test@example.com");
    user.setFullName("测试用户");
    user.setRole(SimpleUserRole::USER);
    user.setStatus(SimpleUserStatus::ACTIVE);
    return user;
}

// 创建管理员用户
SimpleUser createAdminUser() {
    SimpleUser user;
    user.setId(99999);
    user.setUsername("admin_user");
    user.setEmail("admin@example.com");
    user.setFullName("管理员用户");
    user.setRole(SimpleUserRole::ADMIN);
    user.setStatus(SimpleUserStatus::ACTIVE);
    return user;
}

int main() {
    std::cout << "🧪 JWT服务独立测试程序" << std::endl;
    std::cout << "测试低空交通系统JWT认证功能（无数据库依赖）" << std::endl;
    std::cout << "包含20项核心测试，确保JWT系统的正确性和安全性" << std::endl;

    try {
        // 设置日志级别为warn，减少测试期间的日志输出
        spdlog::set_level(spdlog::level::warn);

        // 创建测试用户
        SimpleUser testUser = createTestUser();
        SimpleUser adminUser = createAdminUser();

        // 初始化JWT服务
        SimpleJwtService jwtService("test-secret-key-256-bits-minimum-length-required-for-security",
                                   "test-issuer", 900, 604800);

        std::cout << "\n=== 开始JWT核心功能测试 ===" << std::endl;

        // 测试1: 生成Access Token
        std::string accessToken = jwtService.generateAccessToken(testUser);
        bool test1 = !accessToken.empty() && std::count(accessToken.begin(), accessToken.end(), '.') == 2;
        printTestResult("生成Access Token", test1);
        if (test1) {
            std::cout << "Access Token: " << accessToken.substr(0, 50) << "..." << std::endl;
        }

        // 测试2: 生成Refresh Token
        std::string refreshToken = jwtService.generateRefreshToken(testUser);
        bool test2 = !refreshToken.empty() && std::count(refreshToken.begin(), refreshToken.end(), '.') == 2;
        printTestResult("生成Refresh Token", test2);

        // 测试3: 验证有效Token
        JwtValidationResult validationResult = jwtService.validateToken(accessToken);
        bool test3 = (validationResult == JwtValidationResult::VALID);
        printTestResult("验证有效Token", test3);
        std::cout << "验证结果: " << jwtValidationResultToString(validationResult) << std::endl;

        // 测试4: 解析Token信息
        auto tokenInfo = jwtService.parseToken(accessToken);
        bool test4 = tokenInfo.has_value();
        printTestResult("解析Token信息", test4);
        if (test4) {
            std::cout << "用户ID: " << tokenInfo->user_id << ", 用户名: " << tokenInfo->username << std::endl;

            // 测试5: 验证解析出的信息正确性
            bool dataCorrect = (tokenInfo->user_id == testUser.getId()) &&
                              (tokenInfo->username == testUser.getUsername()) &&
                              (tokenInfo->email == testUser.getEmail()) &&
                              (tokenInfo->role == testUser.getRole());
            printTestResult("Token数据正确性", dataCorrect);
        }

        // 测试6: 提取用户ID
        int64_t extractedUserId = jwtService.extractUserId(accessToken);
        bool test6 = (extractedUserId == testUser.getId());
        printTestResult("提取用户ID", test6);

        // 测试7: 提取JWT ID
        std::string jwtId = jwtService.extractJwtId(accessToken);
        bool test7 = !jwtId.empty() && (jwtId.length() == 32);
        printTestResult("提取JWT ID", test7);

        // 测试8: 获取Token剩余时间
        int remainingTime = jwtService.getTokenRemainingTime(accessToken);
        bool test8 = (remainingTime > 0 && remainingTime <= 900);
        printTestResult("获取Token剩余时间", test8);
        std::cout << "剩余时间: " << remainingTime << " 秒" << std::endl;

        // 测试9: 验证无效Token
        JwtValidationResult invalidResult = jwtService.validateToken("invalid.token.signature");
        bool test9 = (invalidResult != JwtValidationResult::VALID);
        printTestResult("验证无效Token", test9);

        // 测试10: 验证篡改的Token
        std::string tamperedToken = accessToken;
        if (tamperedToken.length() > 10) {
            tamperedToken[tamperedToken.length() - 1] = 'X';
        }
        JwtValidationResult tamperedResult = jwtService.validateToken(tamperedToken);
        bool test10 = (tamperedResult == JwtValidationResult::INVALID_SIGNATURE);
        printTestResult("验证篡改的Token", test10);

        // 测试11: 生成短期Token
        std::string shortToken = jwtService.generateToken(testUser, 10);
        bool test11 = !shortToken.empty();
        printTestResult("生成短期Token", test11);
        if (test11) {
            int shortRemaining = jwtService.getTokenRemainingTime(shortToken);
            bool timeCorrect = (shortRemaining > 0 && shortRemaining <= 10);
            printTestResult("短期Token时间正确", timeCorrect);
        }

        // 测试12: Token即将过期检查
        bool notExpiring = jwtService.isTokenExpiringSoon(accessToken, 1000);
        bool expiring = jwtService.isTokenExpiringSoon(shortToken, 60);
        bool test12 = !notExpiring && expiring;
        printTestResult("Token过期检查", test12);

        // 测试13: 管理员角色Token
        std::string adminToken = jwtService.generateAccessToken(adminUser);
        auto adminTokenInfo = jwtService.parseToken(adminToken);
        bool test13 = adminTokenInfo.has_value() && (adminTokenInfo->role == SimpleUserRole::ADMIN);
        printTestResult("管理员角色Token", test13);

        // 测试14: 解析无效Token
        auto invalidTokenInfo = jwtService.parseToken("invalid.token.format");
        printTestResult("解析无效Token应返回空", !invalidTokenInfo.has_value());

        // 测试15: 提取无效Token的用户ID
        int64_t invalidUserId = jwtService.extractUserId("invalid.token");
        printTestResult("从无效Token提取用户ID应返回0", invalidUserId == 0);

        // 测试16: 验证空Token
        JwtValidationResult emptyResult = jwtService.validateToken("");
        printTestResult("验证空Token", emptyResult == JwtValidationResult::MALFORMED);

        // 测试17: Token格式验证（只有一个点）
        JwtValidationResult onePartResult = jwtService.validateToken("invalid.token");
        printTestResult("验证单段Token", onePartResult == JwtValidationResult::MALFORMED);

        // 测试18: 不同用户生成不同Token
        SimpleUser user2 = createTestUser();
        user2.setId(54321);
        user2.setUsername("user2");
        std::string token2 = jwtService.generateAccessToken(user2);
        printTestResult("不同用户生成不同Token", accessToken != token2);

        // 测试19: 相同用户多次生成不同Token
        std::string token3 = jwtService.generateAccessToken(testUser);
        printTestResult("相同用户多次生成不同Token", accessToken != token3);

        // 测试20: Refresh Token有效期更长
        int accessRemaining = jwtService.getTokenRemainingTime(accessToken);
        int refreshRemaining = jwtService.getTokenRemainingTime(refreshToken);
        printTestResult("Refresh Token有效期更长", refreshRemaining > accessRemaining);

        // 输出测试统计
        testStats.printSummary();

        return (testStats.passed == testStats.total) ? 0 : 1;

    } catch (const std::exception& e) {
        std::cout << "\n💥 测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
}