#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <optional>

namespace models {

/**
 * @brief 飞行任务状态枚举
 */
enum class FlightTaskStatus {
    PENDING,    // 待执行
    ONGOING,    // 进行中
    COMPLETED   // 已完成
};

/**
 * @brief 飞行任务模型类
 * 对应前端API的flight_tasks接口
 */
class FlightTask {
private:
    int64_t id_;
    std::string name_;
    std::string description_;
    nlohmann::json route_;
    FlightTaskStatus status_;
    std::optional<std::chrono::system_clock::time_point> scheduled_time_;
    int64_t user_id_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;

public:
    /**
     * @brief 默认构造函数
     */
    FlightTask();

    /**
     * @brief 构造函数
     */
    FlightTask(int64_t id, const std::string& name, const std::string& description,
               const nlohmann::json& route, FlightTaskStatus status, int64_t user_id);

    // Getters
    int64_t getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const nlohmann::json& getRoute() const { return route_; }
    FlightTaskStatus getStatus() const { return status_; }
    const std::optional<std::chrono::system_clock::time_point>& getScheduledTime() const { return scheduled_time_; }
    int64_t getUserId() const { return user_id_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int64_t id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setRoute(const nlohmann::json& route) { route_ = route; }
    void setStatus(FlightTaskStatus status) { status_ = status; }
    void setScheduledTime(const std::chrono::system_clock::time_point& scheduled_time) {
        scheduled_time_ = scheduled_time;
    }
    void setUserId(int64_t user_id) { user_id_ = user_id; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }

    /**
     * @brief 转换为JSON
     * @return JSON对象
     */
    nlohmann::json toJson() const;

    /**
     * @brief 从JSON创建飞行任务对象
     * @param json JSON对象
     * @return 飞行任务对象
     */
    static FlightTask fromJson(const nlohmann::json& json);

    /**
     * @brief 验证飞行任务数据
     * @return 是否有效
     */
    bool validate() const;

    /**
     * @brief 状态枚举转字符串
     */
    static std::string statusToString(FlightTaskStatus status);

    /**
     * @brief 字符串转状态枚举
     */
    static FlightTaskStatus stringToStatus(const std::string& status_str);

    /**
     * @brief 创建用于数据库插入的JSON
     * @return 数据库插入用的JSON对象
     */
    nlohmann::json toDatabaseJson() const;

    /**
     * @brief 从数据库行创建FlightTask对象
     * @param row_json 数据库行的JSON表示
     * @return FlightTask对象
     */
    static FlightTask fromDatabaseJson(const nlohmann::json& row_json);
};

} // namespace models