#include "JwtService.h"
#include "config/Config.h"
#include <spdlog/spdlog.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace auth {

JwtService::JwtService(
    const std::string& secret_key,
    const std::string& issuer,
    int access_token_expiry,
    int refresh_token_expiry)
    : secret_key_(secret_key)
    , issuer_(issuer)
    , access_token_expiry_(access_token_expiry)
    , refresh_token_expiry_(refresh_token_expiry) {

    // 验证密钥长度（至少32字符）
    if (secret_key_.length() < 32) {
        spdlog::warn("JWT secret key is too short (less than 32 characters), security may be compromised");
    }

    spdlog::debug("JwtService initialized with issuer: {}", issuer_);
}

JwtService JwtService::fromConfig() {
    auto& config = config::Config::getInstance();

    std::string secret_key = config.getString("security.jwt_secret", "default-secret-key-change-in-production");
    std::string issuer = config.getString("security.jwt_issuer", "low-altitude-traffic-system");
    int access_expiry = config.getInt("security.jwt_access_token_expiry", 900);      // 15分钟
    int refresh_expiry = config.getInt("security.jwt_refresh_token_expiry", 604800); // 7天

    return JwtService(secret_key, issuer, access_expiry, refresh_expiry);
}

// ========== Token生成 ==========

std::string JwtService::generateAccessToken(const models::User& user) {
    return generateToken(user, access_token_expiry_);
}

std::string JwtService::generateRefreshToken(const models::User& user) {
    return generateToken(user, refresh_token_expiry_);
}

std::string JwtService::generateToken(const models::User& user, int expiry_seconds) {
    try {
        // 生成Header
        std::string header = generateHeader();

        // 生成Payload
        auto [payload, jti] = generatePayload(user, expiry_seconds);

        // 生成签名
        std::string signature = generateSignature(header, payload);

        // 组装完整Token
        std::string token = header + "." + payload + "." + signature;

        spdlog::debug("Generated JWT token for user: {} (ID: {}), JTI: {}",
                     user.getUsername(), user.getId(), jti);

        return token;

    } catch (const std::exception& e) {
        spdlog::error("Failed to generate JWT token for user {}: {}", user.getUsername(), e.what());
        return "";
    }
}

// ========== Token验证 ==========

JwtValidationResult JwtService::validateToken(const std::string& token) {
    try {
        // 检查Token格式
        if (!isValidTokenFormat(token)) {
            return JwtValidationResult::MALFORMED;
        }

        // 分割Token
        auto [header, payload, signature] = splitToken(token);

        // 验证签名
        if (!verifySignature(header, payload, signature)) {
            return JwtValidationResult::INVALID_SIGNATURE;
        }

        // 解析Payload检查过期时间
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

std::optional<TokenInfo> JwtService::parseToken(const std::string& token) {
    // 首先验证Token
    if (validateToken(token) != JwtValidationResult::VALID) {
        return std::nullopt;
    }

    try {
        // 分割Token
        auto [header, payload, signature] = splitToken(token);

        // 解析Payload
        std::string decoded_payload = base64UrlDecode(payload);
        nlohmann::json payload_json = nlohmann::json::parse(decoded_payload);

        // 提取Token信息
        TokenInfo token_info;
        token_info.user_id = payload_json["sub"].get<int64_t>();
        token_info.username = payload_json["username"].get<std::string>();
        token_info.email = payload_json["email"].get<std::string>();
        token_info.role = models::User::stringToRole(payload_json["role"].get<std::string>());
        token_info.iat = timestampToTime(payload_json["iat"].get<int64_t>());
        token_info.exp = timestampToTime(payload_json["exp"].get<int64_t>());
        token_info.jti = payload_json["jti"].get<std::string>();

        return token_info;

    } catch (const std::exception& e) {
        spdlog::error("Token parsing error: {}", e.what());
        return std::nullopt;
    }
}

bool JwtService::isTokenExpiringSoon(const std::string& token, int threshold_seconds) {
    auto token_info = parseToken(token);
    if (!token_info.has_value()) {
        return true; // 无效Token当作即将过期
    }

    auto now = std::chrono::system_clock::now();
    auto threshold_time = now + std::chrono::seconds(threshold_seconds);

    return token_info->exp <= threshold_time;
}

// ========== Token工具方法 ==========

int64_t JwtService::extractUserId(const std::string& token) {
    auto token_info = parseToken(token);
    return token_info.has_value() ? token_info->user_id : 0;
}

std::string JwtService::extractJwtId(const std::string& token) {
    auto token_info = parseToken(token);
    return token_info.has_value() ? token_info->jti : "";
}

int JwtService::getTokenRemainingTime(const std::string& token) {
    auto token_info = parseToken(token);
    if (!token_info.has_value()) {
        return -1;
    }

    auto now = std::chrono::system_clock::now();
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(token_info->exp - now);

    return static_cast<int>(remaining.count());
}

// ========== 配置管理 ==========

void JwtService::updateSecretKey(const std::string& new_secret_key) {
    if (new_secret_key.length() < 32) {
        spdlog::warn("New JWT secret key is too short, update ignored");
        return;
    }

    secret_key_ = new_secret_key;
    spdlog::info("JWT secret key updated successfully");
}

nlohmann::json JwtService::getConfig() const {
    return nlohmann::json{
        {"issuer", issuer_},
        {"access_token_expiry", access_token_expiry_},
        {"refresh_token_expiry", refresh_token_expiry_},
        {"secret_key_length", secret_key_.length()}
    };
}

// ========== 私有方法实现 ==========

std::string JwtService::generateHeader() {
    nlohmann::json header = {
        {"typ", "JWT"},
        {"alg", "HS256"}
    };

    return base64UrlEncode(header.dump());
}

std::pair<std::string, std::string> JwtService::generatePayload(
    const models::User& user,
    int expiry_seconds) {

    auto now = std::chrono::system_clock::now();
    auto exp_time = now + std::chrono::seconds(expiry_seconds);

    std::string jti = generateJwtId();

    nlohmann::json payload = {
        {"iss", issuer_},                                    // 签发者
        {"sub", user.getId()},                              // 主题（用户ID）
        {"aud", "low-altitude-traffic-system-client"},      // 受众
        {"exp", timeToTimestamp(exp_time)},                 // 过期时间
        {"iat", timeToTimestamp(now)},                      // 签发时间
        {"jti", jti},                                       // JWT ID
        {"username", user.getUsername()},                    // 用户名
        {"email", user.getEmail()},                         // 邮箱
        {"role", models::User::roleToString(user.getRole())}, // 角色
        {"status", models::User::statusToString(user.getStatus())} // 状态
    };

    return {base64UrlEncode(payload.dump()), jti};
}

std::string JwtService::generateSignature(const std::string& header, const std::string& payload) {
    std::string message = header + "." + payload;
    return base64UrlEncode(hmacSha256(secret_key_, message));
}

bool JwtService::verifySignature(
    const std::string& header,
    const std::string& payload,
    const std::string& signature) {

    std::string expected_signature = generateSignature(header, payload);
    return signature == expected_signature;
}

std::string JwtService::base64UrlEncode(const std::string& data) {
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

    // 转换为URL安全的Base64（替换字符并移除填充）
    std::replace(encoded_data.begin(), encoded_data.end(), '+', '-');
    std::replace(encoded_data.begin(), encoded_data.end(), '/', '_');
    encoded_data.erase(std::remove(encoded_data.begin(), encoded_data.end(), '='), encoded_data.end());

    return encoded_data;
}

std::string JwtService::base64UrlDecode(const std::string& encoded) {
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

std::string JwtService::generateJwtId() {
    // 生成32字符的随机ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }

    return ss.str();
}

std::string JwtService::hmacSha256(const std::string& key, const std::string& message) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    HMAC(EVP_sha256(),
         key.c_str(), static_cast<int>(key.length()),
         reinterpret_cast<const unsigned char*>(message.c_str()), message.length(),
         hash, &hash_len);

    return std::string(reinterpret_cast<char*>(hash), hash_len);
}

int64_t JwtService::timeToTimestamp(const std::chrono::system_clock::time_point& time_point) {
    return std::chrono::duration_cast<std::chrono::seconds>(time_point.time_since_epoch()).count();
}

std::chrono::system_clock::time_point JwtService::timestampToTime(int64_t timestamp) {
    return std::chrono::system_clock::from_time_t(timestamp);
}

bool JwtService::isValidTokenFormat(const std::string& token) {
    // JWT应该有3个部分，用'.'分隔
    size_t first_dot = token.find('.');
    if (first_dot == std::string::npos) return false;

    size_t second_dot = token.find('.', first_dot + 1);
    if (second_dot == std::string::npos) return false;

    // 确保没有第三个'.'
    if (token.find('.', second_dot + 1) != std::string::npos) return false;

    // 检查三个部分都不为空
    if (first_dot == 0 || second_dot == first_dot + 1 || second_dot == token.length() - 1) {
        return false;
    }

    return true;
}

std::tuple<std::string, std::string, std::string> JwtService::splitToken(const std::string& token) {
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);

    std::string header = token.substr(0, first_dot);
    std::string payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    std::string signature = token.substr(second_dot + 1);

    return {header, payload, signature};
}

// ========== 工具函数 ==========

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

} // namespace auth