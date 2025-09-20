#include "flight_task_dao.h"
#include <iostream>
#include <sstream>

namespace models {
    std::string FlightTaskDAO::create_flight_task(const FlightTask& task) {
        try {
            // 构建SQL查询
            std::string sql = "INSERT INTO flight_tasks (id, name, description, status, priority, progress, "
                              "start_time, end_time, estimated_duration, creator_id, assignee_id) "
                              "VALUES ('" +
                              mysql_real_escape_string(mysql_, new char[task.id.length() * 2 + 1], task.id.c_str(), task.id.length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[task.name.length() * 2 + 1], task.name.c_str(), task.name.length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[task.description.length() * 2 + 1], task.description.c_str(), task.description.length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[task.get_status_string().length() * 2 + 1], task.get_status_string().c_str(), task.get_status_string().length()) + "', '" +
                              mysql_real_escape_string(mysql_, new char[task.get_priority_string().length() * 2 + 1], task.get_priority_string().c_str(), task.get_priority_string().length()) + "', " +
                              std::to_string(task.progress) + ", ";
            
            // 处理 start_time
            if (task.start_time.has_value()) {
                auto start_time_time_t = std::chrono::system_clock::to_time_t(task.start_time.value());
                struct tm* tm_info = localtime(&start_time_time_t);
                char buffer[20];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
                sql += "'" + std::string(buffer) + "', ";
            } else {
                sql += "NULL, ";
            }
            
            // 处理 end_time
            if (task.end_time.has_value()) {
                auto end_time_time_t = std::chrono::system_clock::to_time_t(task.end_time.value());
                struct tm* tm_info = localtime(&end_time_time_t);
                char buffer[20];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
                sql += "'" + std::string(buffer) + "', ";
            } else {
                sql += "NULL, ";
            }
            
            // 处理 estimated_duration
            if (task.estimated_duration.has_value()) {
                sql += std::to_string(task.estimated_duration.value()) + ", ";
            } else {
                sql += "NULL, ";
            }
            
            // 添加 creator_id 和 assignee_id
            sql += std::to_string(task.creator_id) + ", ";
            
            if (task.assignee_id.has_value()) {
                sql += "'" + mysql_real_escape_string(mysql_, new char[task.assignee_id.value().length() * 2 + 1], task.assignee_id.value().c_str(), task.assignee_id.value().length()) + "'";
            } else {
                sql += "NULL";
            }
            
            sql += ")";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "插入飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return "";
            }

            // 检查是否插入成功
            if (mysql_affected_rows(mysql_) > 0) {
                return task.id;
            }
            return "";
        } catch (const std::exception& e) {
            std::cerr << "插入飞行任务失败: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<FlightTask> FlightTaskDAO::get_flight_task_by_id(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM flight_tasks WHERE id = '" + 
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据ID查询飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return std::nullopt;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return std::nullopt;
            }
            
            // 检查是否有结果
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                std::optional<FlightTask> task = result_set_to_flight_task(row);
                
                // 释放结果集
                mysql_free_result(result);
                return task;
            }
            
            // 释放结果集
            mysql_free_result(result);
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "根据ID查询飞行任务失败: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<FlightTask> FlightTaskDAO::get_flight_tasks_by_creator(int creator_id) {
        std::vector<FlightTask> tasks;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM flight_tasks WHERE creator_id = " + std::to_string(creator_id);
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据创建者查询飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                tasks.push_back(result_set_to_flight_task(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据创建者查询飞行任务失败: " << e.what() << std::endl;
        }
        return tasks;
    }

    std::vector<FlightTask> FlightTaskDAO::get_flight_tasks_by_assignee(const std::string& assignee_id) {
        std::vector<FlightTask> tasks;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM flight_tasks WHERE assignee_id = '" + 
                              mysql_real_escape_string(mysql_, new char[assignee_id.length() * 2 + 1], assignee_id.c_str(), assignee_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据分配者查询飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                tasks.push_back(result_set_to_flight_task(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据分配者查询飞行任务失败: " << e.what() << std::endl;
        }
        return tasks;
    }

    std::vector<FlightTask> FlightTaskDAO::get_flight_tasks_by_status(FlightTaskStatus status) {
        std::vector<FlightTask> tasks;
        try {
            // 构建SQL查询
            std::string sql = "SELECT * FROM flight_tasks WHERE status = '" +
                              mysql_real_escape_string(mysql_, new char[FlightTask::get_status_string().length() * 2 + 1], FlightTask::get_status_string().c_str(), FlightTask::get_status_string().length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "根据状态查询飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                tasks.push_back(result_set_to_flight_task(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "根据状态查询飞行任务失败: " << e.what() << std::endl;
        }
        return tasks;
    }

    std::vector<FlightTask> FlightTaskDAO::get_all_flight_tasks() {
        std::vector<FlightTask> tasks;
        try {
            // 执行查询
            if (mysql_query(mysql_, "SELECT * FROM flight_tasks")) {
                std::cerr << "查询所有飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 获取结果集
            MYSQL_RES* result = mysql_store_result(mysql_);
            if (!result) {
                std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
                return tasks;
            }
            
            // 遍历结果集
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                tasks.push_back(result_set_to_flight_task(row));
            }
            
            // 释放结果集
            mysql_free_result(result);
        } catch (const std::exception& e) {
            std::cerr << "查询所有飞行任务失败: " << e.what() << std::endl;
        }
        return tasks;
    }

    bool FlightTaskDAO::update_flight_task(const FlightTask& task) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE flight_tasks SET name = '" +
                              mysql_real_escape_string(mysql_, new char[task.name.length() * 2 + 1], task.name.c_str(), task.name.length()) + "', " +
                              "description = '" +
                              mysql_real_escape_string(mysql_, new char[task.description.length() * 2 + 1], task.description.c_str(), task.description.length()) + "', " +
                              "status = '" +
                              mysql_real_escape_string(mysql_, new char[task.get_status_string().length() * 2 + 1], task.get_status_string().c_str(), task.get_status_string().length()) + "', " +
                              "priority = '" +
                              mysql_real_escape_string(mysql_, new char[task.get_priority_string().length() * 2 + 1], task.get_priority_string().c_str(), task.get_priority_string().length()) + "', " +
                              "progress = " + std::to_string(task.progress) + ", ";
            
            // 处理 start_time
            if (task.start_time.has_value()) {
                auto start_time_time_t = std::chrono::system_clock::to_time_t(task.start_time.value());
                struct tm* tm_info = localtime(&start_time_time_t);
                char buffer[20];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
                sql += "start_time = '" + std::string(buffer) + "', ";
            } else {
                sql += "start_time = NULL, ";
            }
            
            // 处理 end_time
            if (task.end_time.has_value()) {
                auto end_time_time_t = std::chrono::system_clock::to_time_t(task.end_time.value());
                struct tm* tm_info = localtime(&end_time_time_t);
                char buffer[20];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
                sql += "end_time = '" + std::string(buffer) + "', ";
            } else {
                sql += "end_time = NULL, ";
            }
            
            // 处理 estimated_duration
            if (task.estimated_duration.has_value()) {
                sql += "estimated_duration = " + std::to_string(task.estimated_duration.value()) + ", ";
            } else {
                sql += "estimated_duration = NULL, ";
            }
            
            // 添加 creator_id 和 assignee_id
            sql += "creator_id = " + std::to_string(task.creator_id) + ", ";
            
            if (task.assignee_id.has_value()) {
                sql += "assignee_id = '" + 
                       mysql_real_escape_string(mysql_, new char[task.assignee_id.value().length() * 2 + 1], task.assignee_id.value().c_str(), task.assignee_id.value().length()) + "', ";
            } else {
                sql += "assignee_id = NULL, ";
            }
            
            // 更新时间
            sql += "updated_at = NOW() WHERE id = '" +
                   mysql_real_escape_string(mysql_, new char[task.id.length() * 2 + 1], task.id.c_str(), task.id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新飞行任务失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool FlightTaskDAO::update_flight_task_status(const std::string& task_id, FlightTaskStatus status) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE flight_tasks SET status = '" +
                              mysql_real_escape_string(mysql_, new char[FlightTask::get_status_string().length() * 2 + 1], FlightTask::get_status_string().c_str(), FlightTask::get_status_string().length()) + "', " +
                              "updated_at = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[task_id.length() * 2 + 1], task_id.c_str(), task_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新飞行任务状态失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新飞行任务状态失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool FlightTaskDAO::update_flight_task_progress(const std::string& task_id, int progress) {
        try {
            // 构建SQL查询
            std::string sql = "UPDATE flight_tasks SET progress = " + std::to_string(progress) +
                              ", updated_at = NOW() WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[task_id.length() * 2 + 1], task_id.c_str(), task_id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "更新飞行任务进度失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否更新成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "更新飞行任务进度失败: " << e.what() << std::endl;
            return false;
        }
    }

    bool FlightTaskDAO::delete_flight_task(const std::string& id) {
        try {
            // 构建SQL查询
            std::string sql = "DELETE FROM flight_tasks WHERE id = '" +
                              mysql_real_escape_string(mysql_, new char[id.length() * 2 + 1], id.c_str(), id.length()) + "'";
            
            // 执行查询
            if (mysql_query(mysql_, sql.c_str())) {
                std::cerr << "删除飞行任务失败: " << mysql_error(mysql_) << std::endl;
                return false;
            }
            
            // 检查是否删除成功
            return mysql_affected_rows(mysql_) > 0;
        } catch (const std::exception& e) {
            std::cerr << "删除飞行任务失败: " << e.what() << std::endl;
            return false;
        }
    }

    FlightTask FlightTaskDAO::result_set_to_flight_task(MYSQL_ROW row) {
        FlightTask task;
        task.id = row[0] ? row[0] : "";
        task.name = row[1] ? row[1] : "";
        task.description = row[2] ? row[2] : "";
        task.status = FlightTask::parse_status_string(row[3] ? row[3] : "");
        task.priority = FlightTask::parse_priority_string(row[4] ? row[4] : "");
        task.progress = row[5] ? std::stoi(row[5]) : 0;

        // Handle timestamps
        if (row[6]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[6], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            task.created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[7]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[7], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            task.updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[8]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[8], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            task.start_time = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[9]) {
            // Convert MySQL datetime string to time_t
            struct tm tm = {};
            sscanf(row[9], "%d-%d-%d %d:%d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            task.end_time = std::chrono::system_clock::from_time_t(mktime(&tm));
        }

        if (row[10]) {
            task.estimated_duration = std::stoi(row[10]);
        }

        task.creator_id = row[11] ? std::stoi(row[11]) : 0;

        if (row[12]) {
            task.assignee_id = row[12];
        }

        return task;
    }
}
