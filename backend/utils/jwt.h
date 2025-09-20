#pragma once

#include <string>
#include <unordered_map>
#include <chrono>

/**
 * @brief JWT工具类
 * 
 * 提供JWT令牌的生成、验证和解析功能
 */
class JWTUtil {
private:
    static const std::string SECRET_KEY;
    static const int EXPIRATION_HOURS = 24;

public:
    /**
     * @brief 生成JWT令牌
     * @param userId 用户ID
     * @param username 用户名
     * @param role 用户角色
     * @return JWT令牌字符串
     */
    static std::string generateToken(int userId, const std::string& username, 
                                   const std::string& role);
    
    /**
     * @brief 验证JWT令牌
     * @param token JWT令牌
     * @return 验证通过返回true，否则返回false
     */
    static bool validateToken(const std::string& token);
    
    /**
     * @brief 解析JWT令牌获取用户信息
     * @param token JWT令牌
     * @return 包含用户信息的map
     */
    static std::unordered_map<std::string, std::string> parseToken(
        const std::string& token);
    
    /**
     * @brief 检查令牌是否过期
     * @param token JWT令牌
     * @return 过期返回true，否则返回false
     */
    static bool isExpired(const std::string& token);
};
