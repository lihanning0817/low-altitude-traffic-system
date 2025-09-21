#pragma once

#include "BaseRepository.h"
#include "models/User.h"
#include <mysqlx/xdevapi.h>
#include <vector>
#include <optional>
#include <string>

namespace repositories {

/**
 * @brief 用户数据访问类
 * 负责所有用户相关的数据库操作，包括CRUD、认证、权限管理等
 */
class UserRepository : public BaseRepository {
public:
    /**
     * @brief 构造函数
     */
    UserRepository();

    /**
     * @brief 析构函数
     */
    ~UserRepository() override = default;

    // ========== 基础CRUD操作 ==========

    /**
     * @brief 创建新用户
     * @param user 用户对象
     * @return 创建成功返回用户ID，失败返回0
     */
    int64_t createUser(const models::User& user);

    /**
     * @brief 根据ID获取用户
     * @param user_id 用户ID
     * @return 用户对象，如果不存在返回空
     */
    std::optional<models::User> getUserById(int64_t user_id);

    /**
     * @brief 根据用户名获取用户
     * @param username 用户名
     * @return 用户对象，如果不存在返回空
     */
    std::optional<models::User> getUserByUsername(const std::string& username);

    /**
     * @brief 根据邮箱获取用户
     * @param email 邮箱地址
     * @return 用户对象，如果不存在返回空
     */
    std::optional<models::User> getUserByEmail(const std::string& email);

    /**
     * @brief 更新用户信息
     * @param user 用户对象
     * @return 是否更新成功
     */
    bool updateUser(const models::User& user);

    /**
     * @brief 删除用户（软删除，更改状态为banned）
     * @param user_id 用户ID
     * @return 是否删除成功
     */
    bool deleteUser(int64_t user_id);

    /**
     * @brief 物理删除用户（真正从数据库中删除）
     * @param user_id 用户ID
     * @return 是否删除成功
     */
    bool hardDeleteUser(int64_t user_id);

    // ========== 查询操作 ==========

    /**
     * @brief 获取所有用户列表（分页）
     * @param offset 偏移量
     * @param limit 限制数量
     * @param status 用户状态过滤（可选）
     * @param role 用户角色过滤（可选）
     * @return 用户列表
     */
    std::vector<models::User> getUsers(
        int offset = 0,
        int limit = 50,
        const std::optional<models::UserStatus>& status = std::nullopt,
        const std::optional<models::UserRole>& role = std::nullopt
    );

    /**
     * @brief 获取用户总数
     * @param status 用户状态过滤（可选）
     * @param role 用户角色过滤（可选）
     * @return 用户总数
     */
    int getUserCount(
        const std::optional<models::UserStatus>& status = std::nullopt,
        const std::optional<models::UserRole>& role = std::nullopt
    );

    /**
     * @brief 搜索用户（根据用户名、邮箱、真实姓名）
     * @param search_term 搜索关键词
     * @param offset 偏移量
     * @param limit 限制数量
     * @return 匹配的用户列表
     */
    std::vector<models::User> searchUsers(
        const std::string& search_term,
        int offset = 0,
        int limit = 50
    );

    // ========== 认证相关操作 ==========

    /**
     * @brief 验证用户登录
     * @param username_or_email 用户名或邮箱
     * @param password 密码
     * @return 验证成功返回用户对象，失败返回空
     */
    std::optional<models::User> authenticateUser(
        const std::string& username_or_email,
        const std::string& password
    );

    /**
     * @brief 更新用户密码
     * @param user_id 用户ID
     * @param new_password 新密码
     * @return 是否更新成功
     */
    bool updatePassword(int64_t user_id, const std::string& new_password);

    /**
     * @brief 更新最后登录时间
     * @param user_id 用户ID
     * @return 是否更新成功
     */
    bool updateLastLogin(int64_t user_id);

    // ========== 用户状态管理 ==========

    /**
     * @brief 更新用户状态
     * @param user_id 用户ID
     * @param status 新状态
     * @return 是否更新成功
     */
    bool updateUserStatus(int64_t user_id, models::UserStatus status);

    /**
     * @brief 更新用户角色
     * @param user_id 用户ID
     * @param role 新角色
     * @return 是否更新成功
     */
    bool updateUserRole(int64_t user_id, models::UserRole role);

    /**
     * @brief 激活用户
     * @param user_id 用户ID
     * @return 是否激活成功
     */
    bool activateUser(int64_t user_id);

    /**
     * @brief 禁用用户
     * @param user_id 用户ID
     * @return 是否禁用成功
     */
    bool deactivateUser(int64_t user_id);

    /**
     * @brief 封禁用户
     * @param user_id 用户ID
     * @return 是否封禁成功
     */
    bool banUser(int64_t user_id);

    // ========== 验证操作 ==========

    /**
     * @brief 检查用户名是否已存在
     * @param username 用户名
     * @param exclude_user_id 排除的用户ID（用于更新时检查）
     * @return 是否已存在
     */
    bool isUsernameExists(const std::string& username, int64_t exclude_user_id = 0);

    /**
     * @brief 检查邮箱是否已存在
     * @param email 邮箱地址
     * @param exclude_user_id 排除的用户ID（用于更新时检查）
     * @return 是否已存在
     */
    bool isEmailExists(const std::string& email, int64_t exclude_user_id = 0);

    /**
     * @brief 检查用户是否处于活跃状态
     * @param user_id 用户ID
     * @return 是否活跃
     */
    bool isUserActive(int64_t user_id);

    /**
     * @brief 检查用户是否有指定角色
     * @param user_id 用户ID
     * @param role 角色
     * @return 是否有该角色
     */
    bool hasRole(int64_t user_id, models::UserRole role);

    // ========== 统计操作 ==========

    /**
     * @brief 获取用户注册统计（按日期）
     * @param start_date 开始日期（YYYY-MM-DD格式）
     * @param end_date 结束日期（YYYY-MM-DD格式）
     * @return 统计结果JSON数组
     */
    std::vector<nlohmann::json> getUserRegistrationStats(
        const std::string& start_date,
        const std::string& end_date
    );

    /**
     * @brief 获取用户角色分布统计
     * @return 角色分布JSON对象
     */
    nlohmann::json getUserRoleDistribution();

private:
    /**
     * @brief 将JSON对象转换为User模型
     * @param json JSON对象
     * @return User对象
     */
    models::User jsonToUser(const nlohmann::json& json);

    /**
     * @brief 将MySQL X DevAPI行转换为User模型
     * @param row 数据库行
     * @return User对象
     */
    models::User rowToUser(const mysqlx::Row& row);

    // 表名常量
    static constexpr const char* TABLE_NAME = "low_altitude_traffic_system.users";

    /**
     * @brief 构建WHERE条件字符串
     * @param status 状态过滤
     * @param role 角色过滤
     * @return WHERE条件字符串
     */
    std::string buildWhereClause(
        const std::optional<models::UserStatus>& status,
        const std::optional<models::UserRole>& role
    );
};

} // namespace repositories