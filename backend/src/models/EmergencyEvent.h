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
 */
class EmergencyEvent {
public:
    int64_t id;                                    // 事件ID
    std::string event_code;                        // 事件编号
    int64_t task_id;                               // 关联任务ID
    std::optional<int64_t> drone_id;               // 关联无人机ID（可选）
    EmergencyType type;                            // 事件类型
    EmergencySeverity severity;                    // 严重程度
    EmergencyStatus status;                        // 事件状态
    std::string title;                             // 事件标题
    std::string description;                       // 事件描述
    nlohmann::json location;                       // 事件位置 {lat, lon}
    std::optional<std::string> response_action;    // 响应措施
    std::optional<std::string> response_notes;     // 响应备注
    std::optional<int64_t> responded_by;           // 响应人员ID
    std::optional<std::chrono::system_clock::time_point> responded_at;  // 响应时间
    std::optional<std::chrono::system_clock::time_point> resolved_at;   // 解决时间
    std::chrono::system_clock::time_point created_at;  // 创建时间
    std::chrono::system_clock::time_point updated_at;  // 更新时间

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
