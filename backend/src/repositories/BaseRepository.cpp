#include "repositories/BaseRepository.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sstream>

namespace repositories {

BaseRepository::BaseRepository()
    : db_manager_(database::DatabaseManager::getInstance()) {
}

// ========== 查询方法 ==========

std::optional<nlohmann::json> BaseRepository::executeQuerySingle(
    const std::string& query,
    const std::vector<mysqlx::Value>& params) {

    try {
        auto result = db_manager_.executePreparedQuery(query, params);
        if (!result || !result->hasData()) {
            return std::nullopt;
        }

        auto row = result->fetchRow();
        if (!row || row.isNull()) {
            return std::nullopt;
        }

        return rowToJson(row, result->getColumnNames());
    } catch (const std::exception& e) {
        spdlog::error("执行单行查询失败: {}", e.what());
        return std::nullopt;
    }
}

std::vector<nlohmann::json> BaseRepository::executeQueryMultiple(
    const std::string& query,
    const std::vector<mysqlx::Value>& params) {

    std::vector<nlohmann::json> results;

    try {
        auto result = db_manager_.executePreparedQuery(query, params);
        if (!result || !result->hasData()) {
            return results;
        }

        auto columnNames = result->getColumnNames();
        auto row = result->fetchRow();
        while (row && !row.isNull()) {
            results.push_back(rowToJson(row, columnNames));
            row = result->fetchRow();
        }
    } catch (const std::exception& e) {
        spdlog::error("执行多行查询失败: {}", e.what());
    }

    return results;
}

std::optional<mysqlx::Row> BaseRepository::executeQuerySingleRow(
    const std::string& query,
    const std::vector<mysqlx::Value>& params) {

    try {
        auto result = db_manager_.executePreparedQuery(query, params);
        if (!result || !result->hasData()) {
            return std::nullopt;
        }

        return result->fetchRow();
    } catch (const std::exception& e) {
        spdlog::error("执行单行查询失败: {}", e.what());
        return std::nullopt;
    }
}

std::vector<mysqlx::Row> BaseRepository::executeQueryMultipleRows(
    const std::string& query,
    const std::vector<mysqlx::Value>& params) {

    std::vector<mysqlx::Row> rows;

    try {
        auto result = db_manager_.executePreparedQuery(query, params);
        if (!result || !result->hasData()) {
            return rows;
        }

        auto row = result->fetchRow();
        while (row && !row.isNull()) {
            rows.push_back(row);
            row = result->fetchRow();
        }
    } catch (const std::exception& e) {
        spdlog::error("执行多行查询失败: {}", e.what());
    }

    return rows;
}

// ========== 更新方法 ==========

uint64_t BaseRepository::executeUpdate(
    const std::string& query,
    const std::vector<mysqlx::Value>& params) {

    try {
        return db_manager_.executePreparedUpdate(query, params);
    } catch (const std::exception& e) {
        spdlog::error("执行更新操作失败: {}", e.what());
        return 0;
    }
}

uint64_t BaseRepository::executeInsert(
    const std::string& query,
    const std::vector<mysqlx::Value>& params) {

    try {
        auto affectedRows = db_manager_.executePreparedUpdate(query, params);
        if (affectedRows > 0) {
            return db_manager_.getLastInsertId();
        }
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("执行插入操作失败: {}", e.what());
        return 0;
    }
}

// ========== 事务方法 ==========

bool BaseRepository::beginTransaction() {
    try {
        return db_manager_.startTransaction();
    } catch (const std::exception& e) {
        spdlog::error("开始事务失败: {}", e.what());
        return false;
    }
}

bool BaseRepository::commitTransaction() {
    try {
        return db_manager_.commitTransaction();
    } catch (const std::exception& e) {
        spdlog::error("提交事务失败: {}", e.what());
        return false;
    }
}

bool BaseRepository::rollbackTransaction() {
    try {
        return db_manager_.rollbackTransaction();
    } catch (const std::exception& e) {
        spdlog::error("回滚事务失败: {}", e.what());
        return false;
    }
}

// ========== 表操作方法 ==========

mysqlx::Table BaseRepository::getTable(const std::string& table_name) {
    return db_manager_.getTable(table_name);
}

// ========== 工具方法 ==========

uint64_t BaseRepository::getLastInsertId() {
    return db_manager_.getLastInsertId();
}

nlohmann::json BaseRepository::rowToJson(
    const mysqlx::Row& row,
    const std::vector<std::string>& column_names) {

    nlohmann::json json_obj;

    try {
        for (size_t i = 0; i < row.colCount(); ++i) {
            std::string col_name;
            if (i < column_names.size()) {
                col_name = column_names[i];
            } else {
                col_name = "col_" + std::to_string(i);
            }

            // 处理NULL值
            if (row[i].isNull()) {
                json_obj[col_name] = nullptr;
                continue;
            }

            // 根据类型转换值
            try {
                switch (row[i].getType()) {
                    case mysqlx::Value::BOOL:
                        json_obj[col_name] = static_cast<bool>(row[i]);
                        break;
                    case mysqlx::Value::UINT64:
                        json_obj[col_name] = static_cast<uint64_t>(row[i]);
                        break;
                    case mysqlx::Value::INT64:
                        json_obj[col_name] = static_cast<int64_t>(row[i]);
                        break;
                    case mysqlx::Value::FLOAT:
                        json_obj[col_name] = static_cast<float>(row[i]);
                        break;
                    case mysqlx::Value::DOUBLE:
                        json_obj[col_name] = static_cast<double>(row[i]);
                        break;
                    case mysqlx::Value::STRING:
                    case mysqlx::Value::RAW:
                    default:
                        json_obj[col_name] = static_cast<std::string>(row[i]);
                        break;
                }
            } catch (const std::exception&) {
                // 如果类型转换失败，尝试作为字符串
                json_obj[col_name] = static_cast<std::string>(row[i]);
            }
        }
    } catch (const std::exception& e) {
        spdlog::error("行转JSON失败: {}", e.what());
    }

    return json_obj;
}

nlohmann::json BaseRepository::rowsToJsonArray(
    const std::vector<mysqlx::Row>& rows,
    const std::vector<std::string>& column_names) {

    nlohmann::json json_array = nlohmann::json::array();

    for (const auto& row : rows) {
        json_array.push_back(rowToJson(row, column_names));
    }

    return json_array;
}

bool BaseRepository::isDatabaseAvailable() {
    try {
        auto result = db_manager_.executeQuery("SELECT 1");
        return result && result->hasData();
    } catch (const std::exception& e) {
        spdlog::error("检查数据库可用性失败: {}", e.what());
        return false;
    }
}

std::string BaseRepository::buildWhereClause(
    const std::map<std::string, mysqlx::Value>& conditions,
    const std::string& operator_type) {

    if (conditions.empty()) {
        return "";
    }

    std::ostringstream where_clause;
    where_clause << " WHERE ";

    bool first = true;
    for (const auto& [column, value] : conditions) {
        if (!first) {
            where_clause << " " << operator_type << " ";
        }
        where_clause << column << " = ?";
        first = false;
    }

    return where_clause.str();
}

std::pair<std::string, std::vector<mysqlx::Value>> BaseRepository::buildInsertQuery(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& data) {

    if (data.empty()) {
        throw std::invalid_argument("插入数据不能为空");
    }

    std::ostringstream query;
    std::vector<mysqlx::Value> params;

    query << "INSERT INTO " << table_name << " (";

    // 构建列名部分
    bool first = true;
    for (const auto& [column, value] : data) {
        if (!first) {
            query << ", ";
        }
        query << column;
        first = false;
    }

    query << ") VALUES (";

    // 构建值部分
    first = true;
    for (const auto& [column, value] : data) {
        if (!first) {
            query << ", ";
        }
        query << "?";
        params.push_back(value);
        first = false;
    }

    query << ")";

    return {query.str(), params};
}

std::pair<std::string, std::vector<mysqlx::Value>> BaseRepository::buildUpdateQuery(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& data,
    const std::map<std::string, mysqlx::Value>& conditions) {

    if (data.empty()) {
        throw std::invalid_argument("更新数据不能为空");
    }

    if (conditions.empty()) {
        throw std::invalid_argument("更新条件不能为空");
    }

    std::ostringstream query;
    std::vector<mysqlx::Value> params;

    query << "UPDATE " << table_name << " SET ";

    // 构建SET部分
    bool first = true;
    for (const auto& [column, value] : data) {
        if (!first) {
            query << ", ";
        }
        query << column << " = ?";
        params.push_back(value);
        first = false;
    }

    // 构建WHERE部分
    query << " WHERE ";
    first = true;
    for (const auto& [column, value] : conditions) {
        if (!first) {
            query << " AND ";
        }
        query << column << " = ?";
        params.push_back(value);
        first = false;
    }

    return {query.str(), params};
}

// ========== 通用CRUD模板方法 ==========

std::vector<nlohmann::json> BaseRepository::findBy(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& conditions,
    const std::vector<std::string>& columns) {

    std::ostringstream query;
    std::vector<mysqlx::Value> params;

    // 构建SELECT部分
    query << "SELECT ";
    if (columns.empty()) {
        query << "*";
    } else {
        bool first = true;
        for (const auto& col : columns) {
            if (!first) {
                query << ", ";
            }
            query << col;
            first = false;
        }
    }

    query << " FROM " << table_name;

    // 构建WHERE部分
    if (!conditions.empty()) {
        query << " WHERE ";
        bool first = true;
        for (const auto& [column, value] : conditions) {
            if (!first) {
                query << " AND ";
            }
            query << column << " = ?";
            params.push_back(value);
            first = false;
        }
    }

    return executeQueryMultiple(query.str(), params);
}

std::optional<nlohmann::json> BaseRepository::findOneBy(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& conditions,
    const std::vector<std::string>& columns) {

    auto results = findBy(table_name, conditions, columns);
    return results.empty() ? std::nullopt : std::make_optional(results[0]);
}

std::optional<nlohmann::json> BaseRepository::findById(
    const std::string& table_name,
    const mysqlx::Value& id,
    const std::string& id_column) {

    std::map<std::string, mysqlx::Value> conditions{{id_column, id}};
    return findOneBy(table_name, conditions);
}

uint64_t BaseRepository::save(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& data) {

    auto [query, params] = buildInsertQuery(table_name, data);
    return executeInsert(query, params);
}

uint64_t BaseRepository::update(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& data,
    const std::map<std::string, mysqlx::Value>& conditions) {

    auto [query, params] = buildUpdateQuery(table_name, data, conditions);
    return executeUpdate(query, params);
}

uint64_t BaseRepository::remove(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& conditions) {

    if (conditions.empty()) {
        throw std::invalid_argument("删除条件不能为空");
    }

    std::ostringstream query;
    std::vector<mysqlx::Value> params;

    query << "DELETE FROM " << table_name << " WHERE ";

    bool first = true;
    for (const auto& [column, value] : conditions) {
        if (!first) {
            query << " AND ";
        }
        query << column << " = ?";
        params.push_back(value);
        first = false;
    }

    return executeUpdate(query.str(), params);
}

uint64_t BaseRepository::count(
    const std::string& table_name,
    const std::map<std::string, mysqlx::Value>& conditions) {

    std::ostringstream query;
    std::vector<mysqlx::Value> params;

    query << "SELECT COUNT(*) as count FROM " << table_name;

    if (!conditions.empty()) {
        query << " WHERE ";
        bool first = true;
        for (const auto& [column, value] : conditions) {
            if (!first) {
                query << " AND ";
            }
            query << column << " = ?";
            params.push_back(value);
            first = false;
        }
    }

    auto result = executeQuerySingle(query.str(), params);
    if (result && result->contains("count")) {
        return (*result)["count"].get<uint64_t>();
    }

    return 0;
}

} // namespace repositories