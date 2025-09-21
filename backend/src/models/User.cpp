#include "User.h"
#include <spdlog/spdlog.h>
#include <regex>
#include <iomanip>
#include <sstream>
#include <random>

// 简化的密码哈希实现（生产环境应使用bcrypt等安全库）
#include <openssl/sha.h>
#include <openssl/rand.h>

namespace models {

User::User()
    : id_(0)
    , role_(UserRole::USER)
    , status_(UserStatus::ACTIVE)
    , created_at_(std::chrono::system_clock::now())
    , updated_at_(std::chrono::system_clock::now()) {
}

User::User(int64_t id, const std::string& username, const std::string& email,
           const std::string& password_hash, const std::string& full_name,
           UserRole role, UserStatus status)
    : id_(id)
    , username_(username)
    , email_(email)
    , password_hash_(password_hash)
    , full_name_(full_name)
    , role_(role)
    , status_(status)
    , created_at_(std::chrono::system_clock::now())
    , updated_at_(std::chrono::system_clock::now()) {
}

nlohmann::json User::toJson(bool include_sensitive) const {
    nlohmann::json json = {
        {"id", id_},
        {"username", username_},
        {"email", email_},
        {"full_name", full_name_},
        {"role", roleToString(role_)},
        {"status", statusToString(status_)},
        {"created_at", std::chrono::duration_cast<std::chrono::seconds>(
            created_at_.time_since_epoch()).count()},
        {"updated_at", std::chrono::duration_cast<std::chrono::seconds>(
            updated_at_.time_since_epoch()).count()}
    };

    if (phone_.has_value()) {
        json["phone"] = phone_.value();
    }

    if (avatar_url_.has_value()) {
        json["avatar_url"] = avatar_url_.value();
    }

    if (last_login_.has_value()) {
        json["last_login"] = std::chrono::duration_cast<std::chrono::seconds>(
            last_login_.value().time_since_epoch()).count();
    }

    if (include_sensitive) {
        json["password_hash"] = password_hash_;
    }

    return json;
}

User User::fromJson(const nlohmann::json& json) {
    User user;

    if (json.contains("id")) {
        user.setId(json["id"].get<int64_t>());
    }

    if (json.contains("username")) {
        user.setUsername(json["username"].get<std::string>());
    }

    if (json.contains("email")) {
        user.setEmail(json["email"].get<std::string>());
    }

    if (json.contains("password_hash")) {
        user.setPasswordHash(json["password_hash"].get<std::string>());
    }

    if (json.contains("full_name")) {
        user.setFullName(json["full_name"].get<std::string>());
    }

    if (json.contains("phone") && !json["phone"].is_null()) {
        user.setPhone(json["phone"].get<std::string>());
    }

    if (json.contains("role")) {
        user.setRole(stringToRole(json["role"].get<std::string>()));
    }

    if (json.contains("status")) {
        user.setStatus(stringToStatus(json["status"].get<std::string>()));
    }

    if (json.contains("avatar_url") && !json["avatar_url"].is_null()) {
        user.setAvatarUrl(json["avatar_url"].get<std::string>());
    }

    if (json.contains("created_at")) {
        auto timestamp = json["created_at"].get<int64_t>();
        user.setCreatedAt(std::chrono::system_clock::from_time_t(timestamp));
    }

    if (json.contains("updated_at")) {
        auto timestamp = json["updated_at"].get<int64_t>();
        user.setUpdatedAt(std::chrono::system_clock::from_time_t(timestamp));
    }

    if (json.contains("last_login") && !json["last_login"].is_null()) {
        auto timestamp = json["last_login"].get<int64_t>();
        user.setLastLogin(std::chrono::system_clock::from_time_t(timestamp));
    }

    return user;
}

bool User::validate() const {
    // 验证用户名
    if (username_.empty() || username_.length() < 3 || username_.length() > 50) {
        return false;
    }

    // 验证邮箱格式
    std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!std::regex_match(email_, email_regex)) {
        return false;
    }

    // 验证全名
    if (full_name_.empty() || full_name_.length() > 100) {
        return false;
    }

    // 验证密码哈希
    if (password_hash_.empty()) {
        return false;
    }

    return true;
}

bool User::checkPassword(const std::string& password) const {
    return verifyPassword(password, password_hash_);
}

void User::setPassword(const std::string& password) {
    password_hash_ = hashPassword(password);
    updated_at_ = std::chrono::system_clock::now();
}

std::string User::roleToString(UserRole role) {
    switch (role) {
        case UserRole::ADMIN: return "admin";
        case UserRole::OPERATOR: return "operator";
        case UserRole::USER: return "user";
        default: return "user";
    }
}

UserRole User::stringToRole(const std::string& role_str) {
    if (role_str == "admin") return UserRole::ADMIN;
    if (role_str == "operator") return UserRole::OPERATOR;
    return UserRole::USER;
}

std::string User::statusToString(UserStatus status) {
    switch (status) {
        case UserStatus::ACTIVE: return "active";
        case UserStatus::INACTIVE: return "inactive";
        case UserStatus::BANNED: return "banned";
        default: return "active";
    }
}

UserStatus User::stringToStatus(const std::string& status_str) {
    if (status_str == "inactive") return UserStatus::INACTIVE;
    if (status_str == "banned") return UserStatus::BANNED;
    return UserStatus::ACTIVE;
}

std::string User::hashPassword(const std::string& password) {
    // 生成盐值
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        spdlog::error("Failed to generate salt for password hashing");
        return "";
    }

    // 将密码和盐值组合
    std::string salted_password = password + std::string(reinterpret_cast<char*>(salt), sizeof(salt));

    // 计算SHA256哈希
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salted_password.c_str(), salted_password.length());
    SHA256_Final(hash, &sha256);

    // 将盐值和哈希组合并转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(salt[i]);
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}

bool User::verifyPassword(const std::string& password, const std::string& hash) {
    if (hash.length() != 96) { // 32字节盐值 + 32字节哈希 = 64字节 = 128个十六进制字符
        return false;
    }

    // 提取盐值
    std::string salt_hex = hash.substr(0, 32);
    std::string stored_hash = hash.substr(32);

    // 将十六进制盐值转换为字节
    std::string salt;
    for (size_t i = 0; i < salt_hex.length(); i += 2) {
        std::string byte_string = salt_hex.substr(i, 2);
        char byte = static_cast<char>(std::strtol(byte_string.c_str(), nullptr, 16));
        salt.push_back(byte);
    }

    // 重新计算哈希
    std::string salted_password = password + salt;
    unsigned char computed_hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salted_password.c_str(), salted_password.length());
    SHA256_Final(computed_hash, &sha256);

    // 转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(computed_hash[i]);
    }

    return ss.str() == stored_hash;
}

} // namespace models