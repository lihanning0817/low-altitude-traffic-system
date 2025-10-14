#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <optional>
#include <chrono>

namespace models {

/**
 * @brief 紧急事件类型枚举
 */
enum class EmergencyType {
    EQUIPMENT_FAILURE,    // 设备故障
    WEATHER_EMERGENCY,    // 恶劣天气
    COLLISION_RISK,       // 碰撞风险
    SIGNAL_LOSS,          // 信号丢失
    BATTERY_LOW,          // 电量低
    GEOFENCE_VIOLATION,   // 电子围栏违规
    MANUAL_EMERGENCY,     // 手动紧急
    OTHER                 // 其他
};

/**
 * @brief 紧急事件严重程度枚举
 */
enum class EmergencySeverity {
    LOW,       // 低
    MEDIUM,    // 中
    HIGH,      // 高
    CRITICAL   // 严重
};

/**
 * @brief 紧急事件状态枚举
 */
enum class EmergencyStatus {
    ACTIVE,      // 激活中
    RESPONDING,  // 响应中
    RESOLVED,    // 已解决
    CANCELLED    // 已取消
};

/**
 * @brief 紧急事件模型类
 *
 * 成员按大小排序以优化内存对齐：
 * 1. 8字节成员 (int64_t)
 * 2. 时间戳 (time_point)
 * 3. 字符串和JSON (通常32字节)
 * 4. optional<int64_t> (16字节)
 * 5. optional<string> (40字节)
 * 6. optional<time_point> (16字节)
 * 7. 枚举 (4字节)
 */
class EmergencyEvent {
public:
    // 8字节成员
    int64_t id;                                    // 事件ID
    int64_t task_id;                               // 关联任务ID

    // 时间戳 (通常8字节)
    std::chrono::system_clock::time_point created_at;  // 创建时间
    std::chrono::system_clock::time_point updated_at;  // 更新时间

    // 字符串 (通常32字节)
    std::string event_code;                        // 事件编号
    std::string title;                             // 事件标题

    // JSON (通常16-24字节)
    nlohmann::json location;                       // 事件位置 {lat, lon}

    // optional<int64_t> (16字节)
    std::optional<int64_t> drone_id;               // 关联无人机ID（可选）
    std::optional<int64_t> responded_by;           // 响应人员ID

    // optional<time_point> (16字节)
    std::optional<std::chrono::system_clock::time_point> responded_at;  // 响应时间
    std::optional<std::chrono::system_clock::time_point> resolved_at;   // 解决时间

    // optional<string> (40字节)
    std::optional<std::string> description;        // 事件描述（可选）
    std::optional<std::string> response_action;    // 响应措施
    std::optional<std::string> response_notes;     // 响应备注

    // 枚举 (4字节)
    EmergencyType type;                            // 事件类型
    EmergencySeverity severity;                    // 严重程度
    EmergencyStatus status;                        // 事件状态

    /**
     * @brief 默认构造函数
     */
    EmergencyEvent() = default;

    /**
     * @brief 转换为JSON对象
     * @return JSON对象
     */
    nlohmann::json toJson() const;

    /**
     * @brief 从数据库JSON创建对象
     * @param json 数据库返回的JSON
     * @return EmergencyEvent对象
     */
    static EmergencyEvent fromDatabaseJson(const nlohmann::json& json);

    /**
     * @brief 事件类型转字符串
     */
    static std::string typeToString(EmergencyType type);

    /**
     * @brief 字符串转事件类型
     */
    static EmergencyType stringToType(const std::string& str);

    /**
     * @brief 严重程度转字符串
     */
    static std::string severityToString(EmergencySeverity severity);

    /**
     * @brief 字符串转严重程度
     */
    static EmergencySeverity stringToSeverity(const std::string& str);

    /**
     * @brief 状态转字符串
     */
    static std::string statusToString(EmergencyStatus status);

    /**
     * @brief 字符串转状态
     */
    static EmergencyStatus stringToStatus(const std::string& str);

    /**
     * @brief 验证事件数据
     * @return 如果数据有效返回true
     */
    bool validate() const;
};

} // namespace models
