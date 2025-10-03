#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace models {

/**
 * @brief 地理围栏类型枚举
 */
enum class GeofenceType {
    CIRCULAR,   // 圆形
    POLYGON,    // 多边形
    CORRIDOR    // 走廊
};

/**
 * @brief 地理围栏用途枚举
 */
enum class GeofencePurpose {
    WARNING,     // 警告区
    RESTRICTED,  // 限制区
    ALLOWED      // 允许区
};

/**
 * @brief 地理围栏状态枚举
 */
enum class GeofenceStatus {
    ACTIVE,      // 激活
    INACTIVE,    // 未激活
    ARCHIVED     // 已归档
};

/**
 * @brief 地理围栏模型类
 */
class Geofence {
private:
    int64_t id_;
    std::string name_;
    std::string description_;
    GeofenceType fenceType_;
    nlohmann::json geometry_;  // 几何数据
    std::optional<double> maxAltitude_;
    std::optional<double> minAltitude_;
    GeofenceStatus status_;
    GeofencePurpose purpose_;
    std::optional<std::chrono::system_clock::time_point> effectiveStartTime_;
    std::optional<std::chrono::system_clock::time_point> effectiveEndTime_;
    int64_t createdBy_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;

public:
    // 构造函数
    Geofence();
    ~Geofence() = default;

    // Getter方法
    int64_t getId() const { return id_; }
    std::string getName() const { return name_; }
    std::string getDescription() const { return description_; }
    GeofenceType getFenceType() const { return fenceType_; }
    nlohmann::json getGeometry() const { return geometry_; }
    std::optional<double> getMaxAltitude() const { return maxAltitude_; }
    std::optional<double> getMinAltitude() const { return minAltitude_; }
    GeofenceStatus getStatus() const { return status_; }
    GeofencePurpose getPurpose() const { return purpose_; }
    std::optional<std::chrono::system_clock::time_point> getEffectiveStartTime() const { return effectiveStartTime_; }
    std::optional<std::chrono::system_clock::time_point> getEffectiveEndTime() const { return effectiveEndTime_; }
    int64_t getCreatedBy() const { return createdBy_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }

    // Setter方法
    void setId(int64_t id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setFenceType(GeofenceType type) { fenceType_ = type; }
    void setGeometry(const nlohmann::json& geometry) { geometry_ = geometry; }
    void setMaxAltitude(std::optional<double> altitude) { maxAltitude_ = altitude; }
    void setMinAltitude(std::optional<double> altitude) { minAltitude_ = altitude; }
    void setStatus(GeofenceStatus status) { status_ = status; }
    void setPurpose(GeofencePurpose purpose) { purpose_ = purpose; }
    void setEffectiveStartTime(std::optional<std::chrono::system_clock::time_point> time) { effectiveStartTime_ = time; }
    void setEffectiveEndTime(std::optional<std::chrono::system_clock::time_point> time) { effectiveEndTime_ = time; }
    void setCreatedBy(int64_t userId) { createdBy_ = userId; }
    void setCreatedAt(std::chrono::system_clock::time_point time) { createdAt_ = time; }
    void setUpdatedAt(std::chrono::system_clock::time_point time) { updatedAt_ = time; }

    // 工具方法
    nlohmann::json toJson() const;
    nlohmann::json toDatabaseJson() const;
    static Geofence fromDatabaseJson(const nlohmann::json& json);

    // 类型转换方法
    static std::string fenceTypeToString(GeofenceType type);
    static GeofenceType stringToFenceType(const std::string& str);
    static std::string purposeToString(GeofencePurpose purpose);
    static GeofencePurpose stringToPurpose(const std::string& str);
    static std::string statusToString(GeofenceStatus status);
    static GeofenceStatus stringToStatus(const std::string& str);

    // 业务方法
    bool isActive() const;
    bool isEffectiveAt(std::chrono::system_clock::time_point time) const;
};

} // namespace models
