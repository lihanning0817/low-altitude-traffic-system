#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <optional>

namespace models {

/**
 * @brief 用户角色枚举
 */
enum class UserRole {
    ADMIN,
    OPERATOR,
    USER
};

/**
 * @brief 用户状态枚举
 */
enum class UserStatus {
    ACTIVE,
    INACTIVE,
    BANNED
};

/**
 * @brief 用户模型类
 */
class User {
private:
    int64_t id_;
    std::string username_;
    std::string email_;
    std::string password_hash_;
    std::string full_name_;
    std::optional<std::string> phone_;
    UserRole role_;
    UserStatus status_;
    std::optional<std::string> avatar_url_;
    std::optional<std::chrono::system_clock::time_point> last_login_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;

public:
    /**
     * @brief 默认构造函数
     */
    User();

    /**
     * @brief 构造函数
     */
    User(int64_t id, const std::string& username, const std::string& email,
         const std::string& password_hash, const std::string& full_name,
         UserRole role = UserRole::USER, UserStatus status = UserStatus::ACTIVE);

    // Getters
    int64_t getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getPasswordHash() const { return password_hash_; }
    const std::string& getFullName() const { return full_name_; }
    const std::optional<std::string>& getPhone() const { return phone_; }
    UserRole getRole() const { return role_; }
    UserStatus getStatus() const { return status_; }
    const std::optional<std::string>& getAvatarUrl() const { return avatar_url_; }
    const std::optional<std::chrono::system_clock::time_point>& getLastLogin() const { return last_login_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int64_t id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
    void setFullName(const std::string& full_name) { full_name_ = full_name; }
    void setPhone(const std::string& phone) { phone_ = phone; }
    void setRole(UserRole role) { role_ = role; }
    void setStatus(UserStatus status) { status_ = status; }
    void setAvatarUrl(const std::string& avatar_url) { avatar_url_ = avatar_url; }
    void setLastLogin(const std::chrono::system_clock::time_point& last_login) { last_login_ = last_login; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }

    /**
     * @brief 转换为JSON
     * @param include_sensitive 是否包含敏感信息（如密码哈希）
     * @return JSON对象
     */
    nlohmann::json toJson(bool include_sensitive = false) const;

    /**
     * @brief 从JSON创建用户对象
     * @param json JSON对象
     * @return 用户对象
     */
    static User fromJson(const nlohmann::json& json);

    /**
     * @brief 验证用户数据
     * @return 是否有效
     */
    bool validate() const;

    /**
     * @brief 检查密码
     * @param password 明文密码
     * @return 是否匹配
     */
    bool checkPassword(const std::string& password) const;

    /**
     * @brief 设置密码（自动哈希）
     * @param password 明文密码
     */
    void setPassword(const std::string& password);

    /**
     * @brief 角色枚举转字符串
     */
    static std::string roleToString(UserRole role);

    /**
     * @brief 字符串转角色枚举
     */
    static UserRole stringToRole(const std::string& role_str);

    /**
     * @brief 状态枚举转字符串
     */
    static std::string statusToString(UserStatus status);

    /**
     * @brief 字符串转状态枚举
     */
    static UserStatus stringToStatus(const std::string& status_str);

    /**
     * @brief 生成密码哈希
     * @param password 明文密码
     * @return 密码哈希
     */
    static std::string hashPassword(const std::string& password);

    /**
     * @brief 验证密码哈希
     * @param password 明文密码
     * @param hash 密码哈希
     * @return 是否匹配
     */
    static bool verifyPassword(const std::string& password, const std::string& hash);

private:
    // 内部成员变量...
};

} // namespace models