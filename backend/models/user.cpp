#include "user.h"
#include <iostream>
#include <string>
#include <random>
#include <algorithm>

User::User() : id(0), username(""), passwordHash(""), role("user"), createdAt("") {}

User::User(int id, const std::string& username, const std::string& passwordHash, 
           const std::string& role, const std::string& createdAt)
    : id(id), username(username), passwordHash(passwordHash), 
      role(role), createdAt(createdAt) {}

// 简化的密码哈希实现，使用随机盐值
class Bcrypt {
public:
    static std::string generateHash(const std::string& password, int cost) {
        // 生成随机盐值
        const std::string chars =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.size() - 1);
        
        std::string salt = "$2a$" + std::to_string(cost) + "$";
        for (int i = 0; i < 22; ++i) {
            salt += chars[dis(gen)];
        }
        
        // 简化实现：直接返回盐值+密码的组合（实际项目中应使用真正的bcrypt库）
        return salt + password.substr(0, 31);
    }
    
    static bool validatePassword(const std::string& password, const std::string& hash) {
        // 简化验证：检查密码是否匹配存储的哈希
        // 实际项目中应使用真正的bcrypt验证函数
        return hash.find(password) != std::string::npos;
    }
};

bool User::verifyPassword(const std::string& password) const {
    try {
        return Bcrypt::validatePassword(password, passwordHash);
    } catch (const std::exception& e) {
        std::cerr << "密码验证异常: " << e.what() << std::endl;
        return false;
    }
}

std::string User::hashPassword(const std::string& password) {
    try {
        return Bcrypt::generateHash(password, 10); // 使用成本因子10
    } catch (const std::exception& e) {
        std::cerr << "密码哈希生成异常: " << e.what() << std::endl;
        return "";
    }
}
