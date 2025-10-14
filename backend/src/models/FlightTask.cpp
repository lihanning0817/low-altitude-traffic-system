#include "FlightTask.h"
#include "../utils/ParamParser.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace models {

FlightTask::FlightTask() : id_(0), user_id_(0), status_(FlightTaskStatus::PENDING) {
    auto now = std::chrono::system_clock::now();
    created_at_ = now;
    updated_at_ = now;
}

FlightTask::FlightTask(int64_t id, const std::string& name, const std::string& description,
                       const nlohmann::json& route, FlightTaskStatus status, int64_t user_id)
    : id_(id), name_(name), description_(description), route_(route),
      status_(status), user_id_(user_id) {
    auto now = std::chrono::system_clock::now();
    created_at_ = now;
    updated_at_ = now;
}

nlohmann::json FlightTask::toJson() const {
    nlohmann::json json;
    json["id"] = std::to_string(id_);
    json["name"] = name_;
    json["description"] = description_;
    json["route"] = route_;
    json["status"] = statusToString(status_);
    json["user_id"] = user_id_;

    // 时间转换为ISO8601格式
    auto to_iso_string = [](const std::chrono::system_clock::time_point& tp) -> std::string {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    };

    if (scheduled_time_.has_value()) {
        json["scheduled_time"] = to_iso_string(scheduled_time_.value());
    } else {
        json["scheduled_time"] = nullptr;
    }

    json["created_at"] = to_iso_string(created_at_);
    json["updated_at"] = to_iso_string(updated_at_);

    return json;
}

FlightTask FlightTask::fromJson(const nlohmann::json& json) {
    FlightTask task;

    if (json.contains("id") && !json["id"].is_null()) {
        if (json["id"].is_string()) {
            // 使用ParamParser安全解析ID
            task.id_ = utils::ParamParser::parseLongLong(json["id"].get<std::string>(), 0, 1, std::nullopt);
        } else {
            task.id_ = json["id"].get<int64_t>();
        }
    }

    if (json.contains("name")) {
        task.name_ = json["name"].get<std::string>();
    }

    if (json.contains("description")) {
        task.description_ = json["description"].get<std::string>();
    }

    if (json.contains("route") && !json["route"].is_null()) {
        task.route_ = json["route"];
    }

    if (json.contains("status")) {
        task.status_ = stringToStatus(json["status"].get<std::string>());
    }

    if (json.contains("user_id")) {
        task.user_id_ = json["user_id"].get<int64_t>();
    }

    // 时间解析（这里简化处理，实际项目中可能需要更精确的时间解析）
    if (json.contains("scheduled_time") && !json["scheduled_time"].is_null()) {
        // 简化：假设时间字符串是可以直接解析的
        // 实际实现中应该使用更严格的ISO8601解析
        task.scheduled_time_ = std::chrono::system_clock::now();
    }

    return task;
}

bool FlightTask::validate() const {
    if (name_.empty()) {
        return false;
    }
    if (user_id_ <= 0) {
        return false;
    }
    // 路径验证（简化）
    if (!route_.is_null() && !route_.is_object()) {
        return false;
    }
    return true;
}

std::string FlightTask::statusToString(FlightTaskStatus status) {
    switch (status) {
        case FlightTaskStatus::PENDING:
            return "pending";
        case FlightTaskStatus::ONGOING:
            return "ongoing";
        case FlightTaskStatus::COMPLETED:
            return "completed";
        default:
            return "pending";
    }
}

FlightTaskStatus FlightTask::stringToStatus(const std::string& status_str) {
    if (status_str == "pending") {
        return FlightTaskStatus::PENDING;
    } else if (status_str == "ongoing") {
        return FlightTaskStatus::ONGOING;
    } else if (status_str == "completed") {
        return FlightTaskStatus::COMPLETED;
    } else {
        return FlightTaskStatus::PENDING;
    }
}

nlohmann::json FlightTask::toDatabaseJson() const {
    nlohmann::json json;
    json["name"] = name_;
    json["description"] = description_;
    json["route"] = route_.dump(); // 将JSON对象转换为字符串存储
    json["status"] = statusToString(status_);
    json["user_id"] = user_id_;

    if (scheduled_time_.has_value()) {
        auto time_t = std::chrono::system_clock::to_time_t(scheduled_time_.value());
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
        json["scheduled_time"] = ss.str();
    }

    return json;
}

FlightTask FlightTask::fromDatabaseJson(const nlohmann::json& row_json) {
    FlightTask task;

    if (row_json.contains("id")) {
        task.id_ = row_json["id"].get<int64_t>();
    }

    if (row_json.contains("name")) {
        task.name_ = row_json["name"].get<std::string>();
    }

    if (row_json.contains("description") && !row_json["description"].is_null()) {
        task.description_ = row_json["description"].get<std::string>();
    }

    if (row_json.contains("route") && !row_json["route"].is_null()) {
        std::string route_str = row_json["route"].get<std::string>();
        if (!route_str.empty()) {
            try {
                task.route_ = nlohmann::json::parse(route_str);
            } catch (const std::exception&) {
                task.route_ = nlohmann::json::object();
            }
        }
    }

    if (row_json.contains("status")) {
        task.status_ = stringToStatus(row_json["status"].get<std::string>());
    }

    if (row_json.contains("user_id")) {
        task.user_id_ = row_json["user_id"].get<int64_t>();
    }

    // 时间字段处理（简化）
    if (row_json.contains("scheduled_time") && !row_json["scheduled_time"].is_null()) {
        // 这里应该实现从数据库时间戳到chrono time_point的转换
        // 简化处理
        task.scheduled_time_ = std::chrono::system_clock::now();
    }

    if (row_json.contains("created_at")) {
        // 简化处理
        task.created_at_ = std::chrono::system_clock::now();
    }

    if (row_json.contains("updated_at")) {
        // 简化处理
        task.updated_at_ = std::chrono::system_clock::now();
    }

    return task;
}

} // namespace models