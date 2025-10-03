#include "Geofence.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace models {

Geofence::Geofence()
    : id_(0)
    , name_("")
    , description_("")
    , fenceType_(GeofenceType::POLYGON)
    , geometry_(nlohmann::json::object())
    , maxAltitude_(std::nullopt)
    , minAltitude_(std::nullopt)
    , status_(GeofenceStatus::ACTIVE)
    , purpose_(GeofencePurpose::WARNING)
    , effectiveStartTime_(std::nullopt)
    , effectiveEndTime_(std::nullopt)
    , createdBy_(0)
    , createdAt_(std::chrono::system_clock::now())
    , updatedAt_(std::chrono::system_clock::now())
{
}

nlohmann::json Geofence::toJson() const {
    nlohmann::json j;
    j["id"] = id_;
    j["name"] = name_;
    j["description"] = description_;
    j["fence_type"] = fenceTypeToString(fenceType_);
    j["geometry"] = geometry_;

    if (maxAltitude_.has_value()) {
        j["max_altitude"] = maxAltitude_.value();
    } else {
        j["max_altitude"] = nullptr;
    }

    if (minAltitude_.has_value()) {
        j["min_altitude"] = minAltitude_.value();
    } else {
        j["min_altitude"] = nullptr;
    }

    j["status"] = statusToString(status_);
    j["purpose"] = purposeToString(purpose_);

    // 时间字段格式化
    auto timeToString = [](std::chrono::system_clock::time_point tp) -> std::string {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    };

    if (effectiveStartTime_.has_value()) {
        j["effective_start_time"] = timeToString(effectiveStartTime_.value());
    } else {
        j["effective_start_time"] = nullptr;
    }

    if (effectiveEndTime_.has_value()) {
        j["effective_end_time"] = timeToString(effectiveEndTime_.value());
    } else {
        j["effective_end_time"] = nullptr;
    }

    j["created_by"] = createdBy_;
    j["created_at"] = timeToString(createdAt_);
    j["updated_at"] = timeToString(updatedAt_);

    return j;
}

nlohmann::json Geofence::toDatabaseJson() const {
    nlohmann::json j;
    j["name"] = name_;
    j["description"] = description_;
    j["fence_type"] = fenceTypeToString(fenceType_);
    j["geometry"] = geometry_.dump();  // 数据库存储为字符串

    if (maxAltitude_.has_value()) {
        j["max_altitude"] = maxAltitude_.value();
    }

    if (minAltitude_.has_value()) {
        j["min_altitude"] = minAltitude_.value();
    }

    j["status"] = statusToString(status_);
    j["purpose"] = purposeToString(purpose_);
    j["created_by"] = createdBy_;

    return j;
}

Geofence Geofence::fromDatabaseJson(const nlohmann::json& json) {
    Geofence fence;

    if (json.contains("id")) {
        fence.setId(json["id"].get<int64_t>());
    }

    if (json.contains("name")) {
        fence.setName(json["name"].get<std::string>());
    }

    if (json.contains("description") && !json["description"].is_null()) {
        fence.setDescription(json["description"].get<std::string>());
    }

    if (json.contains("fence_type")) {
        fence.setFenceType(stringToFenceType(json["fence_type"].get<std::string>()));
    }

    if (json.contains("geometry")) {
        if (json["geometry"].is_string()) {
            fence.setGeometry(nlohmann::json::parse(json["geometry"].get<std::string>()));
        } else {
            fence.setGeometry(json["geometry"]);
        }
    }

    if (json.contains("max_altitude") && !json["max_altitude"].is_null()) {
        fence.setMaxAltitude(json["max_altitude"].get<double>());
    }

    if (json.contains("min_altitude") && !json["min_altitude"].is_null()) {
        fence.setMinAltitude(json["min_altitude"].get<double>());
    }

    if (json.contains("status")) {
        fence.setStatus(stringToStatus(json["status"].get<std::string>()));
    }

    if (json.contains("purpose")) {
        fence.setPurpose(stringToPurpose(json["purpose"].get<std::string>()));
    }

    if (json.contains("created_by")) {
        fence.setCreatedBy(json["created_by"].get<int64_t>());
    }

    return fence;
}

std::string Geofence::fenceTypeToString(GeofenceType type) {
    switch (type) {
        case GeofenceType::CIRCULAR: return "circular";
        case GeofenceType::POLYGON: return "polygon";
        case GeofenceType::CORRIDOR: return "corridor";
        default: return "polygon";
    }
}

GeofenceType Geofence::stringToFenceType(const std::string& str) {
    if (str == "circular") return GeofenceType::CIRCULAR;
    if (str == "polygon") return GeofenceType::POLYGON;
    if (str == "corridor") return GeofenceType::CORRIDOR;
    return GeofenceType::POLYGON;
}

std::string Geofence::purposeToString(GeofencePurpose purpose) {
    switch (purpose) {
        case GeofencePurpose::WARNING: return "warning";
        case GeofencePurpose::RESTRICTED: return "restricted";
        case GeofencePurpose::ALLOWED: return "allowed";
        default: return "warning";
    }
}

GeofencePurpose Geofence::stringToPurpose(const std::string& str) {
    if (str == "warning") return GeofencePurpose::WARNING;
    if (str == "restricted") return GeofencePurpose::RESTRICTED;
    if (str == "allowed") return GeofencePurpose::ALLOWED;
    return GeofencePurpose::WARNING;
}

std::string Geofence::statusToString(GeofenceStatus status) {
    switch (status) {
        case GeofenceStatus::ACTIVE: return "active";
        case GeofenceStatus::INACTIVE: return "inactive";
        case GeofenceStatus::ARCHIVED: return "archived";
        default: return "active";
    }
}

GeofenceStatus Geofence::stringToStatus(const std::string& str) {
    if (str == "active") return GeofenceStatus::ACTIVE;
    if (str == "inactive") return GeofenceStatus::INACTIVE;
    if (str == "archived") return GeofenceStatus::ARCHIVED;
    return GeofenceStatus::ACTIVE;
}

bool Geofence::isActive() const {
    return status_ == GeofenceStatus::ACTIVE;
}

bool Geofence::isEffectiveAt(std::chrono::system_clock::time_point time) const {
    if (!isActive()) {
        return false;
    }

    if (effectiveStartTime_.has_value() && time < effectiveStartTime_.value()) {
        return false;
    }

    if (effectiveEndTime_.has_value() && time > effectiveEndTime_.value()) {
        return false;
    }

    return true;
}

} // namespace models
