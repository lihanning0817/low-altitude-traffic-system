#include "repositories/UserRepository.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace repositories {

UserRepository::UserRepository() : BaseRepository() {
    spdlog::debug("UserRepository initialized");
}

// ========== 基础CRUD操作 ==========

int64_t UserRepository::createUser(const models::User& user) {
    // 验证用户数据
    if (!user.validate()) {
        spdlog::error("User validation failed during creation");
        return 0;
    }

    // 检查用户名和邮箱是否已存在
    if (isUsernameExists(user.getUsername())) {
        spdlog::warn("Username already exists: {}", user.getUsername());
        return 0;
    }

    if (isEmailExists(user.getEmail())) {
        spdlog::warn("Email already exists: {}", user.getEmail());
        return 0;
    }

    const std::string query = R"(
        INSERT INTO low_altitude_traffic_system.users (
            username, email, password_hash, full_name, phone,
            role, status, avatar_url, created_at, updated_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())
    )";

    std::vector<mysqlx::Value> params = {
        user.getUsername(),
        user.getEmail(),
        user.getPasswordHash(),
        user.getFullName(),
        user.getPhone().value_or(""),
        models::User::roleToString(user.getRole()),
        models::User::statusToString(user.getStatus()),
        user.getAvatarUrl().value_or("")
    };

    auto& db = database::DatabaseManager::getInstance();
    uint64_t affected_rows = db.executePreparedUpdate(query, params);
    if (affected_rows > 0) {
        // 获取插入的ID - 需要单独查询
        auto result = db.executeQuery("SELECT LAST_INSERT_ID() as id");
        if (result && result->hasData()) {
            auto row = result->fetchRow();
            if (row) {
                int64_t user_id = static_cast<int64_t>(row[0]);
                spdlog::info("User created successfully: {} (ID: {})", user.getUsername(), user_id);
                return user_id;
            }
        }
    }

    spdlog::error("Failed to create user: {}", user.getUsername());
    return 0;
}

std::optional<models::User> UserRepository::getUserById(int64_t user_id) {
    const std::string query = R"(
        SELECT id, username, email, password_hash, full_name, phone,
               role, status, avatar_url, last_login_at, created_at, updated_at
        FROM low_altitude_traffic_system.users
        WHERE id = ?
    )";

    auto result = executeQuerySingle(query, {mysqlx::Value(user_id)});
    if (result) {
        return jsonToUser(result.value());
    }

    return std::nullopt;
}

std::optional<models::User> UserRepository::getUserByUsername(const std::string& username) {
    const std::string query = R"(
        SELECT id, username, email, password_hash, full_name, phone,
               role, status, avatar_url, last_login_at, created_at, updated_at
        FROM low_altitude_traffic_system.users
        WHERE username = ?
    )";

    auto& db = database::DatabaseManager::getInstance();
    auto result = db.executePreparedQuery(query, {mysqlx::Value(username)});

    if (result && result->hasData()) {
        auto row = result->fetchRow();
        if (row) {
            return rowToUser(row);
        }
    }

    return std::nullopt;
}

std::optional<models::User> UserRepository::getUserByEmail(const std::string& email) {
    const std::string query = R"(
        SELECT id, username, email, password_hash, full_name, phone,
               role, status, avatar_url, last_login_at, created_at, updated_at
        FROM low_altitude_traffic_system.users
        WHERE email = ?
    )";

    auto result = executeQuerySingle(query, {mysqlx::Value(email)});
    if (result) {
        return jsonToUser(result.value());
    }

    return std::nullopt;
}

bool UserRepository::updateUser(const models::User& user) {
    // 验证用户数据
    if (!user.validate()) {
        spdlog::error("User validation failed during update");
        return false;
    }

    // 检查用户名和邮箱是否已被其他用户使用
    if (isUsernameExists(user.getUsername(), user.getId())) {
        spdlog::warn("Username already exists: {}", user.getUsername());
        return false;
    }

    if (isEmailExists(user.getEmail(), user.getId())) {
        spdlog::warn("Email already exists: {}", user.getEmail());
        return false;
    }

    const std::string query = R"(
        UPDATE low_altitude_traffic_system.users SET
            username = ?, email = ?, full_name = ?, phone = ?,
            role = ?, status = ?, avatar_url = ?, updated_at = NOW()
        WHERE id = ?
    )";

    std::vector<mysqlx::Value> params = {
        user.getUsername(),
        user.getEmail(),
        user.getFullName(),
        user.getPhone().value_or(""),
        models::User::roleToString(user.getRole()),
        models::User::statusToString(user.getStatus()),
        user.getAvatarUrl().value_or(""),
        user.getId()
    };

    uint64_t affected_rows = executeUpdate(query, params);
    if (affected_rows > 0) {
        spdlog::info("User updated successfully: {} (ID: {})", user.getUsername(), user.getId());
        return true;
    }

    spdlog::error("Failed to update user: {} (ID: {})", user.getUsername(), user.getId());
    return false;
}

bool UserRepository::deleteUser(int64_t user_id) {
    // 软删除：将状态设置为banned
    return updateUserStatus(user_id, models::UserStatus::BANNED);
}

bool UserRepository::hardDeleteUser(int64_t user_id) {
    const std::string query = "DELETE FROM low_altitude_traffic_system.users WHERE id = ?";

    uint64_t affected_rows = executeUpdate(query, {mysqlx::Value(user_id)});
    if (affected_rows > 0) {
        spdlog::info("User hard deleted successfully: ID {}", user_id);
        return true;
    }

    spdlog::error("Failed to hard delete user: ID {}", user_id);
    return false;
}

// ========== 查询操作 ==========

std::vector<models::User> UserRepository::getUsers(
    int offset,
    int limit,
    const std::optional<models::UserStatus>& status,
    const std::optional<models::UserRole>& role) {

    std::stringstream query;
    query << R"(
        SELECT id, username, email, password_hash, full_name, phone,
               role, status, avatar_url, last_login_at, created_at, updated_at
        FROM low_altitude_traffic_system.users
    )";

    // 添加WHERE条件
    std::string where_clause = buildWhereClause(status, role);
    if (!where_clause.empty()) {
        query << " WHERE " << where_clause;
    }

    query << " ORDER BY created_at DESC LIMIT " << limit << " OFFSET " << offset;

    auto results = executeQueryMultiple(query.str(), {});

    std::vector<models::User> users;
    users.reserve(results.size());

    for (const auto& json : results) {
        users.push_back(jsonToUser(json));
    }

    spdlog::debug("Retrieved {} users (offset: {}, limit: {})", users.size(), offset, limit);
    return users;
}

int UserRepository::getUserCount(
    const std::optional<models::UserStatus>& status,
    const std::optional<models::UserRole>& role) {

    std::stringstream query;
    query << "SELECT COUNT(*) as count FROM low_altitude_traffic_system.users";

    // 添加WHERE条件
    std::string where_clause = buildWhereClause(status, role);
    if (!where_clause.empty()) {
        query << " WHERE " << where_clause;
    }

    auto result = executeQuerySingle(query.str(), {});
    if (result && result->contains("count")) {
        return result->at("count").get<int>();
    }

    return 0;
}

std::vector<models::User> UserRepository::searchUsers(
    const std::string& search_term,
    int offset,
    int limit) {

    const std::string query = R"(
        SELECT id, username, email, password_hash, full_name, phone,
               role, status, avatar_url, last_login_at, created_at, updated_at
        FROM low_altitude_traffic_system.users
        WHERE (username LIKE ? OR email LIKE ? OR full_name LIKE ?)
          AND status != 'banned'
        ORDER BY created_at DESC
        LIMIT ? OFFSET ?
    )";

    std::string pattern = "%" + search_term + "%";
    std::vector<mysqlx::Value> params = {
        pattern, pattern, pattern,
        limit,
        offset
    };

    auto results = executeQueryMultiple(query, params);

    std::vector<models::User> users;
    users.reserve(results.size());

    for (const auto& json : results) {
        users.push_back(jsonToUser(json));
    }

    spdlog::debug("Search found {} users for term: '{}'", users.size(), search_term);
    return users;
}

// ========== 认证相关操作 ==========

std::optional<models::User> UserRepository::authenticateUser(
    const std::string& username_or_email,
    const std::string& password) {

    const std::string query = R"(
        SELECT id, username, email, password_hash, full_name, phone,
               role, status, avatar_url, last_login_at, created_at, updated_at
        FROM low_altitude_traffic_system.users
        WHERE (username = ? OR email = ?) AND status = 'active'
    )";

    auto result = executeQuerySingle(query, {mysqlx::Value(username_or_email), mysqlx::Value(username_or_email)});
    if (result) {
        models::User user = jsonToUser(result.value());

        // 验证密码
        if (user.checkPassword(password)) {
            // 更新最后登录时间
            updateLastLogin(user.getId());
            spdlog::info("User authenticated successfully: {}", user.getUsername());
            return user;
        } else {
            spdlog::warn("Invalid password for user: {}", username_or_email);
        }
    } else {
        spdlog::warn("User not found or inactive: {}", username_or_email);
    }

    return std::nullopt;
}

bool UserRepository::updatePassword(int64_t user_id, const std::string& new_password) {
    // 创建临时用户对象来生成密码哈希
    models::User temp_user;
    temp_user.setPassword(new_password);

    const std::string query = R"(
        UPDATE low_altitude_traffic_system.users SET password_hash = ?, updated_at = NOW()
        WHERE id = ?
    )";

    std::vector<mysqlx::Value> params = {
        temp_user.getPasswordHash(),
        user_id
    };

    uint64_t affected_rows = executeUpdate(query, params);
    if (affected_rows > 0) {
        spdlog::info("Password updated successfully for user ID: {}", user_id);
        return true;
    }

    spdlog::error("Failed to update password for user ID: {}", user_id);
    return false;
}

bool UserRepository::updateLastLogin(int64_t user_id) {
    const std::string query = "UPDATE low_altitude_traffic_system.users SET last_login_at = NOW() WHERE id = ?";

    uint64_t affected_rows = executeUpdate(query, {mysqlx::Value(user_id)});
    return affected_rows > 0;
}

// ========== 用户状态管理 ==========

bool UserRepository::updateUserStatus(int64_t user_id, models::UserStatus status) {
    const std::string query = R"(
        UPDATE low_altitude_traffic_system.users SET status = ?, updated_at = NOW()
        WHERE id = ?
    )";

    std::vector<mysqlx::Value> params = {
        models::User::statusToString(status),
        user_id
    };

    uint64_t affected_rows = executeUpdate(query, params);
    if (affected_rows > 0) {
        spdlog::info("User status updated: ID {} -> {}", user_id, models::User::statusToString(status));
        return true;
    }

    return false;
}

bool UserRepository::updateUserRole(int64_t user_id, models::UserRole role) {
    const std::string query = R"(
        UPDATE low_altitude_traffic_system.users SET role = ?, updated_at = NOW()
        WHERE id = ?
    )";

    std::vector<mysqlx::Value> params = {
        models::User::roleToString(role),
        user_id
    };

    uint64_t affected_rows = executeUpdate(query, params);
    if (affected_rows > 0) {
        spdlog::info("User role updated: ID {} -> {}", user_id, models::User::roleToString(role));
        return true;
    }

    return false;
}

bool UserRepository::activateUser(int64_t user_id) {
    return updateUserStatus(user_id, models::UserStatus::ACTIVE);
}

bool UserRepository::deactivateUser(int64_t user_id) {
    return updateUserStatus(user_id, models::UserStatus::INACTIVE);
}

bool UserRepository::banUser(int64_t user_id) {
    return updateUserStatus(user_id, models::UserStatus::BANNED);
}

// ========== 验证操作 ==========

bool UserRepository::isUsernameExists(const std::string& username, int64_t exclude_user_id) {
    std::string query = "SELECT COUNT(*) as count FROM low_altitude_traffic_system.users WHERE username = ?";
    std::vector<mysqlx::Value> params = {username};

    if (exclude_user_id > 0) {
        query += " AND id != ?";
        params.push_back(exclude_user_id);
    }

    auto result = executeQuerySingle(query, params);
    if (result && result->contains("count")) {
        return result->at("count").get<int>() > 0;
    }

    return false;
}

bool UserRepository::isEmailExists(const std::string& email, int64_t exclude_user_id) {
    std::string query = "SELECT COUNT(*) as count FROM low_altitude_traffic_system.users WHERE email = ?";
    std::vector<mysqlx::Value> params = {email};

    if (exclude_user_id > 0) {
        query += " AND id != ?";
        params.push_back(exclude_user_id);
    }

    auto result = executeQuerySingle(query, params);
    if (result && result->contains("count")) {
        return result->at("count").get<int>() > 0;
    }

    return false;
}

bool UserRepository::isUserActive(int64_t user_id) {
    const std::string query = "SELECT status FROM low_altitude_traffic_system.users WHERE id = ?";

    auto result = executeQuerySingle(query, {mysqlx::Value(user_id)});
    if (result && result->contains("status")) {
        std::string status = result->at("status").get<std::string>();
        return status == "active";
    }

    return false;
}

bool UserRepository::hasRole(int64_t user_id, models::UserRole role) {
    const std::string query = "SELECT role FROM low_altitude_traffic_system.users WHERE id = ?";

    auto result = executeQuerySingle(query, {mysqlx::Value(user_id)});
    if (result && result->contains("role")) {
        std::string user_role = result->at("role").get<std::string>();
        return user_role == models::User::roleToString(role);
    }

    return false;
}

// ========== 统计操作 ==========

std::vector<nlohmann::json> UserRepository::getUserRegistrationStats(
    const std::string& start_date,
    const std::string& end_date) {

    const std::string query = R"(
        SELECT DATE(created_at) as date, COUNT(*) as count
        FROM low_altitude_traffic_system.users
        WHERE DATE(created_at) BETWEEN ? AND ?
        GROUP BY DATE(created_at)
        ORDER BY date
    )";

    return executeQueryMultiple(query, {mysqlx::Value(start_date), mysqlx::Value(end_date)});
}

nlohmann::json UserRepository::getUserRoleDistribution() {
    const std::string query = R"(
        SELECT role, COUNT(*) as count
        FROM low_altitude_traffic_system.users
        WHERE status != 'banned'
        GROUP BY role
    )";

    auto results = executeQueryMultiple(query, {});

    nlohmann::json distribution;
    for (const auto& result : results) {
        if (result.contains("role") && result.contains("count")) {
            std::string role = result["role"].get<std::string>();
            int count = result["count"].get<int>();
            distribution[role] = count;
        }
    }

    return distribution;
}

// ========== 私有方法 ==========

models::User UserRepository::jsonToUser(const nlohmann::json& json) {
    models::User user;

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

    if (json.contains("phone") && !json["phone"].is_null() && !json["phone"].get<std::string>().empty()) {
        user.setPhone(json["phone"].get<std::string>());
    }

    if (json.contains("role")) {
        user.setRole(models::User::stringToRole(json["role"].get<std::string>()));
    }

    if (json.contains("status")) {
        user.setStatus(models::User::stringToStatus(json["status"].get<std::string>()));
    }

    if (json.contains("avatar_url") && !json["avatar_url"].is_null() && !json["avatar_url"].get<std::string>().empty()) {
        user.setAvatarUrl(json["avatar_url"].get<std::string>());
    }

    // 处理时间字段
    if (json.contains("created_at") && !json["created_at"].is_null()) {
        // 这里简化处理，实际应该解析时间字符串
        user.setCreatedAt(std::chrono::system_clock::now());
    }

    if (json.contains("updated_at") && !json["updated_at"].is_null()) {
        user.setUpdatedAt(std::chrono::system_clock::now());
    }

    if (json.contains("last_login") && !json["last_login"].is_null()) {
        user.setLastLogin(std::chrono::system_clock::now());
    }

    return user;
}

models::User UserRepository::rowToUser(const mysqlx::Row& row) {
    models::User user;

    if (row.colCount() >= 12) {
        user.setId(static_cast<int64_t>(row[0]));
        user.setUsername(static_cast<std::string>(row[1]));
        user.setEmail(static_cast<std::string>(row[2]));
        user.setPasswordHash(static_cast<std::string>(row[3]));
        user.setFullName(static_cast<std::string>(row[4]));

        // 处理可选字段
        if (!row[5].isNull()) {
            user.setPhone(static_cast<std::string>(row[5]));
        }

        // 角色和状态
        user.setRole(models::User::stringToRole(static_cast<std::string>(row[6])));
        user.setStatus(models::User::stringToStatus(static_cast<std::string>(row[7])));

        // 头像URL
        if (!row[8].isNull()) {
            user.setAvatarUrl(static_cast<std::string>(row[8]));
        }

        // 时间字段 - 简化处理，使用当前时间
        user.setCreatedAt(std::chrono::system_clock::now());
        user.setUpdatedAt(std::chrono::system_clock::now());

        if (!row[9].isNull()) {
            user.setLastLogin(std::chrono::system_clock::now());
        }
    }

    return user;
}

std::string UserRepository::buildWhereClause(
    const std::optional<models::UserStatus>& status,
    const std::optional<models::UserRole>& role) {

    std::vector<std::string> conditions;

    if (status.has_value()) {
        conditions.push_back("status = '" + models::User::statusToString(status.value()) + "'");
    }

    if (role.has_value()) {
        conditions.push_back("role = '" + models::User::roleToString(role.value()) + "'");
    }

    if (conditions.empty()) {
        return "";
    }

    std::stringstream ss;
    for (size_t i = 0; i < conditions.size(); ++i) {
        if (i > 0) {
            ss << " AND ";
        }
        ss << conditions[i];
    }

    return ss.str();
}

} // namespace repositories