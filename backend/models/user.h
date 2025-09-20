#pragma once

#include <string>
#include <memory>

/**
 * @brief 用户实体类
 * 
 * 对应数据库users表，封装用户信息和操作
 */
class User {
private:
    int id;
    std::string username;
    std::string passwordHash;
    std::string role;
    std::string createdAt;

public:
    // 构造函数
    User();
    User(int id, const std::string& username, const std::string& passwordHash, 
         const std::string& role, const std::string& createdAt);
    
    // Getter方法
    int getId() const { return id; }
    const std::string& getUsername() const { return username; }
    const std::string& getPasswordHash() const { return passwordHash; }
    const std::string& getRole() const { return role; }
    const std::string& getCreatedAt() const { return createdAt; }
    
    // Setter方法
    void setId(int newId) { this->id = newId; }
    void setUsername(const std::string& newUsername) { this->username = newUsername; }
    void setPasswordHash(const std::string& newPasswordHash) { this->passwordHash = newPasswordHash; }
    void setRole(const std::string& newRole) { this->role = newRole; }
    void setCreatedAt(const std::string& newCreatedAt) { this->createdAt = newCreatedAt; }
    
    /**
     * @brief 验证密码
     * @param password 用户输入的密码
     * @return 密码正确返回true，否则返回false
     */
    bool verifyPassword(const std::string& password) const;
    
    /**
     * @brief 生成密码哈希
     * @param password 用户密码
     * @return 密码哈希值
     */
    static std::string hashPassword(const std::string& password);
};
