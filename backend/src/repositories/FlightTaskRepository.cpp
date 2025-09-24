#include "FlightTaskRepository.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace repositories {

const std::string FlightTaskRepository::TABLE_NAME = "low_altitude_traffic_system.flight_tasks";

FlightTaskRepository::FlightTaskRepository() : BaseRepository() {
    std::cout << "[FlightTaskRepository] Initializing FlightTask Repository" << std::endl;
}

int64_t FlightTaskRepository::create(const models::FlightTask& task) {
    try {
        std::cout << "[FlightTaskRepository] Creating new flight task: " << task.getName() << std::endl;

        // 构建插入数据
        auto task_data = task.toDatabaseJson();

        std::map<std::string, mysqlx::Value> data;
        // 映射到主表字段名
        data["task_name"] = task_data["name"].get<std::string>();
        data["description"] = task_data["description"].get<std::string>();
        data["task_type"] = "delivery"; // 默认任务类型
        data["status"] = task_data["status"].get<std::string>();
        data["user_id"] = task_data["user_id"].get<int64_t>();

        // 设置默认位置信息 (可以从route JSON中提取)
        data["start_lat"] = 39.904200;  // 默认北京坐标
        data["start_lng"] = 116.407396;
        data["end_lat"] = 39.914200;
        data["end_lng"] = 116.417396;
        data["start_altitude"] = 100.0;
        data["end_altitude"] = 100.0;

        if (task_data.contains("scheduled_time") && !task_data["scheduled_time"].is_null()) {
            data["scheduled_start_time"] = task_data["scheduled_time"].get<std::string>();
        }

        // 执行插入
        int64_t task_id = save(TABLE_NAME, data);

        if (task_id > 0) {
            std::cout << "[FlightTaskRepository] Successfully created flight task with ID: " << task_id << std::endl;
        } else {
            std::cerr << "[FlightTaskRepository] Failed to create flight task" << std::endl;
        }

        return task_id;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error creating flight task: " << e.what() << std::endl;
        return 0;
    }
}

std::optional<models::FlightTask> FlightTaskRepository::findById(int64_t id) {
    try {
        std::cout << "[FlightTaskRepository] Finding flight task by ID: " << id << std::endl;

        auto result = BaseRepository::findById(TABLE_NAME, id);
        if (result.has_value()) {
            auto task = jsonToFlightTask(result.value());
            std::cout << "[FlightTaskRepository] Found flight task: " << task.getName() << std::endl;
            return task;
        } else {
            std::cout << "[FlightTaskRepository] Flight task not found with ID: " << id << std::endl;
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error finding flight task by ID " << id << ": " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool FlightTaskRepository::update(int64_t id, const models::FlightTask& task) {
    try {
        std::cout << "[FlightTaskRepository] Updating flight task ID: " << id << std::endl;

        // 构建更新数据
        auto task_data = task.toDatabaseJson();

        std::map<std::string, mysqlx::Value> data;
        data["name"] = task_data["name"].get<std::string>();
        data["description"] = task_data["description"].get<std::string>();
        data["route"] = task_data["route"].get<std::string>();
        data["status"] = task_data["status"].get<std::string>();

        if (task_data.contains("scheduled_time") && !task_data["scheduled_time"].is_null()) {
            data["scheduled_time"] = task_data["scheduled_time"].get<std::string>();
        }

        // 构建条件
        std::map<std::string, mysqlx::Value> conditions;
        conditions["id"] = id;

        // 执行更新
        uint64_t affected_rows = BaseRepository::update(TABLE_NAME, data, conditions);

        bool success = affected_rows > 0;
        if (success) {
            std::cout << "[FlightTaskRepository] Successfully updated flight task ID: " << id << std::endl;
        } else {
            std::cout << "[FlightTaskRepository] No rows affected when updating flight task ID: " << id << std::endl;
        }

        return success;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error updating flight task ID " << id << ": " << e.what() << std::endl;
        return false;
    }
}

bool FlightTaskRepository::deleteById(int64_t id) {
    try {
        std::cout << "[FlightTaskRepository] Deleting flight task ID: " << id << std::endl;

        std::map<std::string, mysqlx::Value> conditions;
        conditions["id"] = id;

        uint64_t affected_rows = remove(TABLE_NAME, conditions);

        bool success = affected_rows > 0;
        if (success) {
            std::cout << "[FlightTaskRepository] Successfully deleted flight task ID: " << id << std::endl;
        } else {
            std::cout << "[FlightTaskRepository] No rows affected when deleting flight task ID: " << id << std::endl;
        }

        return success;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error deleting flight task ID " << id << ": " << e.what() << std::endl;
        return false;
    }
}

std::vector<models::FlightTask> FlightTaskRepository::findByUserId(int64_t user_id) {
    try {
        std::cout << "[FlightTaskRepository] Finding flight tasks for user ID: " << user_id << std::endl;

        std::map<std::string, mysqlx::Value> conditions;
        conditions["user_id"] = user_id;

        auto results = findBy(TABLE_NAME, conditions);

        std::vector<models::FlightTask> tasks;
        for (const auto& row_json : results) {
            tasks.push_back(jsonToFlightTask(row_json));
        }

        std::cout << "[FlightTaskRepository] Found " << tasks.size() << " flight tasks for user ID: " << user_id << std::endl;
        return tasks;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error finding flight tasks for user ID " << user_id << ": " << e.what() << std::endl;
        return {};
    }
}

std::vector<models::FlightTask> FlightTaskRepository::findByStatus(
    models::FlightTaskStatus status,
    std::optional<int64_t> user_id
) {
    try {
        std::string status_str = models::FlightTask::statusToString(status);
        std::cout << "[FlightTaskRepository] Finding flight tasks with status: " << status_str;
        if (user_id.has_value()) {
            std::cout << " for user ID: " << user_id.value();
        }
        std::cout << std::endl;

        std::map<std::string, mysqlx::Value> conditions;
        conditions["status"] = status_str;

        if (user_id.has_value()) {
            conditions["user_id"] = user_id.value();
        }

        auto results = findBy(TABLE_NAME, conditions);

        std::vector<models::FlightTask> tasks;
        for (const auto& row_json : results) {
            tasks.push_back(jsonToFlightTask(row_json));
        }

        std::cout << "[FlightTaskRepository] Found " << tasks.size() << " flight tasks with status: " << status_str << std::endl;
        return tasks;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error finding flight tasks by status: " << e.what() << std::endl;
        return {};
    }
}

std::vector<models::FlightTask> FlightTaskRepository::findAll(
    int offset,
    int limit,
    std::optional<int64_t> user_id
) {
    try {
        std::cout << "[FlightTaskRepository] Finding all flight tasks with offset: " << offset << ", limit: " << limit;
        if (user_id.has_value()) {
            std::cout << " for user ID: " << user_id.value();
        }
        std::cout << std::endl;

        // 构建查询
        std::string query = "SELECT * FROM " + TABLE_NAME;
        std::vector<mysqlx::Value> params;

        if (user_id.has_value()) {
            query += " WHERE user_id = ?";
            params.push_back(user_id.value());
        }

        query += " ORDER BY created_at DESC LIMIT ? OFFSET ?";
        params.push_back(limit);
        params.push_back(offset);

        auto results = executeQueryMultiple(query, params);

        std::vector<models::FlightTask> tasks;
        for (const auto& row_json : results) {
            tasks.push_back(jsonToFlightTask(row_json));
        }

        std::cout << "[FlightTaskRepository] Found " << tasks.size() << " flight tasks" << std::endl;
        return tasks;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error finding all flight tasks: " << e.what() << std::endl;
        return {};
    }
}

int64_t FlightTaskRepository::countByUserId(int64_t user_id) {
    try {
        std::map<std::string, mysqlx::Value> conditions;
        conditions["user_id"] = user_id;

        uint64_t count_result = count(TABLE_NAME, conditions);
        std::cout << "[FlightTaskRepository] User " << user_id << " has " << count_result << " flight tasks" << std::endl;

        return static_cast<int64_t>(count_result);
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error counting flight tasks for user " << user_id << ": " << e.what() << std::endl;
        return 0;
    }
}

int64_t FlightTaskRepository::countByStatus(
    models::FlightTaskStatus status,
    std::optional<int64_t> user_id
) {
    try {
        std::map<std::string, mysqlx::Value> conditions;
        conditions["status"] = models::FlightTask::statusToString(status);

        if (user_id.has_value()) {
            conditions["user_id"] = user_id.value();
        }

        uint64_t count_result = count(TABLE_NAME, conditions);
        std::cout << "[FlightTaskRepository] Found " << count_result << " flight tasks with status: "
                  << models::FlightTask::statusToString(status) << std::endl;

        return static_cast<int64_t>(count_result);
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error counting flight tasks by status: " << e.what() << std::endl;
        return 0;
    }
}

bool FlightTaskRepository::updateStatus(int64_t id, models::FlightTaskStatus status) {
    try {
        std::string status_str = models::FlightTask::statusToString(status);
        std::cout << "[FlightTaskRepository] Updating status of flight task " << id << " to: " << status_str << std::endl;

        std::map<std::string, mysqlx::Value> data;
        data["status"] = status_str;

        std::map<std::string, mysqlx::Value> conditions;
        conditions["id"] = id;

        uint64_t affected_rows = BaseRepository::update(TABLE_NAME, data, conditions);

        bool success = affected_rows > 0;
        if (success) {
            std::cout << "[FlightTaskRepository] Successfully updated status of flight task " << id << std::endl;
        } else {
            std::cout << "[FlightTaskRepository] No rows affected when updating status of flight task " << id << std::endl;
        }

        return success;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error updating status of flight task " << id << ": " << e.what() << std::endl;
        return false;
    }
}

bool FlightTaskRepository::isTaskOwnedByUser(int64_t task_id, int64_t user_id) {
    try {
        std::map<std::string, mysqlx::Value> conditions;
        conditions["id"] = task_id;
        conditions["user_id"] = user_id;

        auto result = findOneBy(TABLE_NAME, conditions, {"id"});
        bool owned = result.has_value();

        std::cout << "[FlightTaskRepository] Task " << task_id << " "
                  << (owned ? "is" : "is not") << " owned by user " << user_id << std::endl;

        return owned;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error checking task ownership: " << e.what() << std::endl;
        return false;
    }
}

std::optional<nlohmann::json> FlightTaskRepository::getTaskSummary(int64_t id) {
    try {
        std::vector<std::string> columns = {"id", "name", "status", "user_id", "created_at", "updated_at"};

        std::map<std::string, mysqlx::Value> conditions;
        conditions["id"] = id;

        auto result = findOneBy(TABLE_NAME, conditions, columns);
        if (result.has_value()) {
            std::cout << "[FlightTaskRepository] Retrieved summary for flight task " << id << std::endl;
            return result.value();
        } else {
            std::cout << "[FlightTaskRepository] Flight task " << id << " not found for summary" << std::endl;
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error getting task summary: " << e.what() << std::endl;
        return std::nullopt;
    }
}

int FlightTaskRepository::updateStatusBatch(
    const std::vector<int64_t>& task_ids,
    models::FlightTaskStatus status,
    int64_t user_id
) {
    try {
        std::string status_str = models::FlightTask::statusToString(status);
        std::cout << "[FlightTaskRepository] Batch updating " << task_ids.size()
                  << " tasks to status: " << status_str << " for user: " << user_id << std::endl;

        int updated_count = 0;

        for (int64_t task_id : task_ids) {
            // 检查任务是否属于该用户
            if (isTaskOwnedByUser(task_id, user_id)) {
                if (updateStatus(task_id, status)) {
                    updated_count++;
                }
            } else {
                std::cout << "[FlightTaskRepository] Skipping task " << task_id
                          << " - not owned by user " << user_id << std::endl;
            }
        }

        std::cout << "[FlightTaskRepository] Successfully updated " << updated_count
                  << " out of " << task_ids.size() << " tasks" << std::endl;

        return updated_count;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error in batch status update: " << e.what() << std::endl;
        return 0;
    }
}

std::vector<models::FlightTask> FlightTaskRepository::searchByName(
    const std::string& name_pattern,
    std::optional<int64_t> user_id
) {
    try {
        std::cout << "[FlightTaskRepository] Searching flight tasks by name pattern: " << name_pattern;
        if (user_id.has_value()) {
            std::cout << " for user ID: " << user_id.value();
        }
        std::cout << std::endl;

        std::string query = "SELECT * FROM " + TABLE_NAME + " WHERE name LIKE ?";
        std::vector<mysqlx::Value> params;
        params.push_back("%" + name_pattern + "%");

        if (user_id.has_value()) {
            query += " AND user_id = ?";
            params.push_back(user_id.value());
        }

        query += " ORDER BY created_at DESC";

        auto results = executeQueryMultiple(query, params);

        std::vector<models::FlightTask> tasks;
        for (const auto& row_json : results) {
            tasks.push_back(jsonToFlightTask(row_json));
        }

        std::cout << "[FlightTaskRepository] Found " << tasks.size()
                  << " flight tasks matching pattern: " << name_pattern << std::endl;
        return tasks;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error searching flight tasks by name: " << e.what() << std::endl;
        return {};
    }
}

std::vector<models::FlightTask> FlightTaskRepository::findByTimeRange(
    const std::chrono::system_clock::time_point& start_time,
    const std::chrono::system_clock::time_point& end_time,
    std::optional<int64_t> user_id
) {
    try {
        std::cout << "[FlightTaskRepository] Finding flight tasks in time range";
        if (user_id.has_value()) {
            std::cout << " for user ID: " << user_id.value();
        }
        std::cout << std::endl;

        // 转换时间格式
        auto to_mysql_time = [](const std::chrono::system_clock::time_point& tp) -> std::string {
            auto time_t = std::chrono::system_clock::to_time_t(tp);
            std::stringstream ss;
            ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        };

        std::string query = "SELECT * FROM " + TABLE_NAME +
                           " WHERE created_at BETWEEN ? AND ?";
        std::vector<mysqlx::Value> params;
        params.push_back(to_mysql_time(start_time));
        params.push_back(to_mysql_time(end_time));

        if (user_id.has_value()) {
            query += " AND user_id = ?";
            params.push_back(user_id.value());
        }

        query += " ORDER BY created_at DESC";

        auto results = executeQueryMultiple(query, params);

        std::vector<models::FlightTask> tasks;
        for (const auto& row_json : results) {
            tasks.push_back(jsonToFlightTask(row_json));
        }

        std::cout << "[FlightTaskRepository] Found " << tasks.size()
                  << " flight tasks in specified time range" << std::endl;
        return tasks;
    } catch (const std::exception& e) {
        std::cerr << "[FlightTaskRepository] Error finding flight tasks by time range: " << e.what() << std::endl;
        return {};
    }
}

models::FlightTask FlightTaskRepository::jsonToFlightTask(const nlohmann::json& row_json) {
    return models::FlightTask::fromDatabaseJson(row_json);
}

std::map<std::string, mysqlx::Value> FlightTaskRepository::buildUserCondition(
    std::optional<int64_t> user_id
) {
    std::map<std::string, mysqlx::Value> conditions;
    if (user_id.has_value()) {
        conditions["user_id"] = user_id.value();
    }
    return conditions;
}

} // namespace repositories