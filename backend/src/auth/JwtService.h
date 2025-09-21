#pragma once

#include <string>
#include <chrono>
#include <optional>
#include <nlohmann/json.hpp>
#include "models/User.h"

namespace auth {

/**
 * @brief JWT Token信息结构
 * 包含Token的所有相关信息
 */
struct TokenInfo {
    int64_t user_id;                           // 用户ID
    std::string username;                      // 用户名
    std::string email;                         // 邮箱
    models::UserRole role;                     // 用户角色
    std::chrono::system_clock::time_point iat; // 签发时间 (issued at)
    std::chrono::system_clock::time_point exp; // 过期时间 (expiration)
    std::string jti;                           // JWT ID (唯一标识)
};

/**
 * @brief JWT验证结果枚举
 */
enum class JwtValidationResult {
    VALID,              // Token有效
    INVALID_SIGNATURE,  // 签名无效
    EXPIRED,           // Token已过期
    MALFORMED,         // Token格式错误
    INVALID_CLAIMS     // Claims无效
};

/**
 * @brief JWT服务类
 * 负责JWT Token的生成、验证和管理
 *
 * 使用HMAC SHA256算法进行签名
 * 支持Access Token和Refresh Token的生成
 */
class JwtService {
private:
    std::string secret_key_;           // JWT签名密钥
    std::string issuer_;              // 签发者
    int access_token_expiry_;         // Access Token过期时间（秒）
    int refresh_token_expiry_;        // Refresh Token过期时间（秒）

public:
    /**
     * @brief 构造函数
     * @param secret_key JWT签名密钥
     * @param issuer 签发者标识
     * @param access_token_expiry Access Token过期时间（秒，默认15分钟）
     * @param refresh_token_expiry Refresh Token过期时间（秒，默认7天）
     */
    JwtService(
        const std::string& secret_key,
        const std::string& issuer = "low-altitude-traffic-system",
        int access_token_expiry = 900,      // 15分钟
        int refresh_token_expiry = 604800   // 7天
    );

    /**
     * @brief 从配置文件初始化JWT服务
     * @return JwtService实例
     */
    static JwtService fromConfig();

    // ========== Token生成 ==========

    /**
     * @brief 生成Access Token
     * @param user 用户对象
     * @return JWT Token字符串
     */
    std::string generateAccessToken(const models::User& user);

    /**
     * @brief 生成Refresh Token
     * @param user 用户对象
     * @return JWT Token字符串
     */
    std::string generateRefreshToken(const models::User& user);

    /**
     * @brief 生成自定义过期时间的Token
     * @param user 用户对象
     * @param expiry_seconds 过期时间（秒）
     * @return JWT Token字符串
     */
    std::string generateToken(const models::User& user, int expiry_seconds);

    // ========== Token验证 ==========

    /**
     * @brief 验证Token有效性
     * @param token JWT Token字符串
     * @return 验证结果
     */
    JwtValidationResult validateToken(const std::string& token);

    /**
     * @brief 解析Token并提取用户信息
     * @param token JWT Token字符串
     * @return Token信息，如果无效返回空
     */
    std::optional<TokenInfo> parseToken(const std::string& token);

    /**
     * @brief 检查Token是否即将过期
     * @param token JWT Token字符串
     * @param threshold_seconds 提前提醒时间（秒）
     * @return 是否即将过期
     */
    bool isTokenExpiringSoon(const std::string& token, int threshold_seconds = 300);

    // ========== Token工具方法 ==========

    /**
     * @brief 从Token中提取用户ID
     * @param token JWT Token字符串
     * @return 用户ID，如果无效返回0
     */
    int64_t extractUserId(const std::string& token);

    /**
     * @brief 从Token中提取JWT ID
     * @param token JWT Token字符串
     * @return JWT ID，如果无效返回空字符串
     */
    std::string extractJwtId(const std::string& token);

    /**
     * @brief 获取Token剩余有效时间
     * @param token JWT Token字符串
     * @return 剩余秒数，如果无效返回-1
     */
    int getTokenRemainingTime(const std::string& token);

    // ========== 配置管理 ==========

    /**
     * @brief 更新密钥（用于密钥轮换）
     * @param new_secret_key 新密钥
     */
    void updateSecretKey(const std::string& new_secret_key);

    /**
     * @brief 获取当前配置信息
     * @return 配置信息JSON
     */
    nlohmann::json getConfig() const;

private:
    /**
     * @brief 生成JWT Header
     * @return Base64编码的Header
     */
    std::string generateHeader();

    /**
     * @brief 生成JWT Payload
     * @param user 用户对象
     * @param expiry_seconds 过期时间（秒）
     * @return Base64编码的Payload和JWT ID
     */
    std::pair<std::string, std::string> generatePayload(
        const models::User& user,
        int expiry_seconds
    );

    /**
     * @brief 生成JWT签名
     * @param header Base64编码的Header
     * @param payload Base64编码的Payload
     * @return Base64编码的签名
     */
    std::string generateSignature(const std::string& header, const std::string& payload);

    /**
     * @brief 验证JWT签名
     * @param header Base64编码的Header
     * @param payload Base64编码的Payload
     * @param signature Base64编码的签名
     * @return 签名是否有效
     */
    bool verifySignature(
        const std::string& header,
        const std::string& payload,
        const std::string& signature
    );

    /**
     * @brief Base64 URL安全编码
     * @param data 待编码数据
     * @return Base64编码字符串
     */
    std::string base64UrlEncode(const std::string& data);

    /**
     * @brief Base64 URL安全解码
     * @param encoded Base64编码字符串
     * @return 解码后的数据
     */
    std::string base64UrlDecode(const std::string& encoded);

    /**
     * @brief 生成唯一的JWT ID
     * @return 唯一标识符
     */
    std::string generateJwtId();

    /**
     * @brief HMAC SHA256签名
     * @param key 密钥
     * @param message 消息
     * @return 签名结果
     */
    std::string hmacSha256(const std::string& key, const std::string& message);

    /**
     * @brief 时间戳转换为字符串
     * @param time_point 时间点
     * @return Unix时间戳
     */
    int64_t timeToTimestamp(const std::chrono::system_clock::time_point& time_point);

    /**
     * @brief 字符串转换为时间戳
     * @param timestamp Unix时间戳
     * @return 时间点
     */
    std::chrono::system_clock::time_point timestampToTime(int64_t timestamp);

    /**
     * @brief 验证Token格式
     * @param token JWT Token字符串
     * @return 是否格式正确
     */
    bool isValidTokenFormat(const std::string& token);

    /**
     * @brief 分割Token为Header、Payload、Signature
     * @param token JWT Token字符串
     * @return {header, payload, signature}
     */
    std::tuple<std::string, std::string, std::string> splitToken(const std::string& token);
};

/**
 * @brief JWT验证结果转字符串
 * @param result 验证结果
 * @return 结果描述
 */
std::string jwtValidationResultToString(JwtValidationResult result);

} // namespace auth