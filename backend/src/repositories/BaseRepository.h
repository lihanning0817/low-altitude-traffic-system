#pragma once

#include "database/DatabaseManager.h"
#include <mysqlx/xdevapi.h>
#include <memory>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace repositories {

/**
 * @brief 基础Repository类
 * 提供通用的数据库操作方法，基于MySQL X DevAPI
 * 所有具体Repository类继承此类
 */
class BaseRepository {
protected:
    database::DatabaseManager& db_manager_;

public:
    /**
     * @brief 构造函数
     */
    BaseRepository();

    /**
     * @brief 虚析构函数
     */
    virtual ~BaseRepository() = default;

protected:
    // ========== 查询方法 ==========

    /**
     * @brief 执行查询并返回第一行结果
     * @param query SQL查询语句
     * @param params 查询参数
     * @return 查询结果的JSON对象，如果没有结果返回空
     */
    std::optional<nlohmann::json> executeQuerySingle(
        const std::string& query,
        const std::vector<mysqlx::Value>& params = {}
    );

    /**
     * @brief 执行查询并返回多行结果
     * @param query SQL查询语句
     * @param params 查询参数
     * @return 查询结果的JSON数组
     */
    std::vector<nlohmann::json> executeQueryMultiple(
        const std::string& query,
        const std::vector<mysqlx::Value>& params = {}
    );

    /**
     * @brief 执行查询并返回原始Row结果（第一行）
     * @param query SQL查询语句
     * @param params 查询参数
     * @return 数据库行结果，如果没有结果返回空Row
     */
    std::optional<mysqlx::Row> executeQuerySingleRow(
        const std::string& query,
        const std::vector<mysqlx::Value>& params = {}
    );

    /**
     * @brief 执行查询并返回所有行结果
     * @param query SQL查询语句
     * @param params 查询参数
     * @return 数据库行结果列表
     */
    std::vector<mysqlx::Row> executeQueryMultipleRows(
        const std::string& query,
        const std::vector<mysqlx::Value>& params = {}
    );

    // ========== 更新方法 ==========

    /**
     * @brief 执行更新操作（INSERT, UPDATE, DELETE）
     * @param query SQL语句
     * @param params 参数
     * @return 受影响的行数
     */
    uint64_t executeUpdate(
        const std::string& query,
        const std::vector<mysqlx::Value>& params = {}
    );

    /**
     * @brief 执行插入操作并返回插入的ID
     * @param query SQL插入语句
     * @param params 参数
     * @return 插入的ID，失败返回0
     */
    uint64_t executeInsert(
        const std::string& query,
        const std::vector<mysqlx::Value>& params = {}
    );

    // ========== 事务方法 ==========

    /**
     * @brief 开始事务
     * @return 是否成功
     */
    bool beginTransaction();

    /**
     * @brief 提交事务
     * @return 是否成功
     */
    bool commitTransaction();

    /**
     * @brief 回滚事务
     * @return 是否成功
     */
    bool rollbackTransaction();

    // ========== 表操作方法 ==========

    /**
     * @brief 获取表对象（用于NoSQL风格操作）
     * @param table_name 表名
     * @return 表对象
     */
    mysqlx::Table getTable(const std::string& table_name);

    // ========== 工具方法 ==========

    /**
     * @brief 获取最后插入的ID
     * @return 最后插入的ID
     */
    uint64_t getLastInsertId();

    /**
     * @brief 将Row转换为JSON对象
     * @param row 数据库行
     * @param column_names 列名列表（可选，用于指定列名）
     * @return JSON对象
     */
    nlohmann::json rowToJson(
        const mysqlx::Row& row,
        const std::vector<std::string>& column_names = {}
    );

    /**
     * @brief 将多个Row转换为JSON数组
     * @param rows 数据库行列表
     * @param column_names 列名列表（可选）
     * @return JSON数组
     */
    nlohmann::json rowsToJsonArray(
        const std::vector<mysqlx::Row>& rows,
        const std::vector<std::string>& column_names = {}
    );

    /**
     * @brief 检查数据库连接是否可用
     * @return 是否连接正常
     */
    bool isDatabaseAvailable();

    /**
     * @brief 构建WHERE子句
     * @param conditions 条件映射（列名 -> 值）
     * @param operator_type 操作符类型（AND/OR）
     * @return WHERE子句字符串
     */
    std::string buildWhereClause(
        const std::map<std::string, mysqlx::Value>& conditions,
        const std::string& operator_type = "AND"
    );

    /**
     * @brief 构建INSERT语句
     * @param table_name 表名
     * @param data 数据映射（列名 -> 值）
     * @return INSERT语句和参数
     */
    std::pair<std::string, std::vector<mysqlx::Value>> buildInsertQuery(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& data
    );

    /**
     * @brief 构建UPDATE语句
     * @param table_name 表名
     * @param data 更新数据（列名 -> 值）
     * @param conditions 条件（列名 -> 值）
     * @return UPDATE语句和参数
     */
    std::pair<std::string, std::vector<mysqlx::Value>> buildUpdateQuery(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& data,
        const std::map<std::string, mysqlx::Value>& conditions
    );

    // ========== 通用CRUD模板方法 ==========

    /**
     * @brief 通用查找方法
     * @param table_name 表名
     * @param conditions 查询条件
     * @param columns 要查询的列（空表示所有列）
     * @return 查询结果
     */
    std::vector<nlohmann::json> findBy(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& conditions,
        const std::vector<std::string>& columns = {}
    );

    /**
     * @brief 通用查找单个记录方法
     * @param table_name 表名
     * @param conditions 查询条件
     * @param columns 要查询的列（空表示所有列）
     * @return 查询结果，如果没有找到返回空
     */
    std::optional<nlohmann::json> findOneBy(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& conditions,
        const std::vector<std::string>& columns = {}
    );

    /**
     * @brief 通用根据ID查找方法
     * @param table_name 表名
     * @param id ID值
     * @param id_column ID列名（默认为"id"）
     * @return 查询结果，如果没有找到返回空
     */
    std::optional<nlohmann::json> findById(
        const std::string& table_name,
        const mysqlx::Value& id,
        const std::string& id_column = "id"
    );

    /**
     * @brief 通用保存方法
     * @param table_name 表名
     * @param data 数据
     * @return 插入的ID，失败返回0
     */
    uint64_t save(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& data
    );

    /**
     * @brief 通用更新方法
     * @param table_name 表名
     * @param data 更新数据
     * @param conditions 更新条件
     * @return 受影响的行数
     */
    uint64_t update(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& data,
        const std::map<std::string, mysqlx::Value>& conditions
    );

    /**
     * @brief 通用删除方法
     * @param table_name 表名
     * @param conditions 删除条件
     * @return 受影响的行数
     */
    uint64_t remove(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& conditions
    );

    /**
     * @brief 计数方法
     * @param table_name 表名
     * @param conditions 条件（可选）
     * @return 记录数量
     */
    uint64_t count(
        const std::string& table_name,
        const std::map<std::string, mysqlx::Value>& conditions = {}
    );
};

} // namespace repositories