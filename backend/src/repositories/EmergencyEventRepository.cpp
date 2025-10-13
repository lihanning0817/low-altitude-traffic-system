#include "EmergencyEventRepository.h"
#include "database/DatabaseManager.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

namespace repositories {

EmergencyEventRepository::EmergencyEventRepository() : BaseRepository() {
    spdlog::info("[EmergencyEventRepository] Initializing Emergency Event Repository");
}

std::string EmergencyEventRepository::generateEventCode() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << "EMG-" << std::put_time(std::localtime(&time), "%Y%m%d");

    // Add random suffix
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    ss << "-" << dis(gen);

    return ss.str();
}

int64_t EmergencyEventRepository::createEvent(const models::EmergencyEvent& event) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = R"(
            INSERT INTO low_altitude_traffic_system.emergency_events (
                event_code, task_id, drone_id, type, severity, status,
                title, description, location, created_at, updated_at
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())
        )";

        db.executePreparedUpdate(sql, {
            event.event_code,
            static_cast<int64_t>(event.task_id),
            event.drone_id.has_value() ? mysqlx::Value(static_cast<int64_t>(event.drone_id.value())) : mysqlx::Value(),
            models::EmergencyEvent::typeToString(event.type),
            models::EmergencyEvent::severityToString(event.severity),
            models::EmergencyEvent::statusToString(event.status),
            event.title,
            event.description.has_value() ? mysqlx::Value(event.description.value()) : mysqlx::Value(),
            event.location.dump()
        });

        return db.getLastInsertId();
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error creating event: {}", e.what());
        throw;
    }
}

std::optional<models::EmergencyEvent> EmergencyEventRepository::getEventById(int64_t id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT * FROM low_altitude_traffic_system.emergency_events WHERE id = ?";
        auto result = db.executePreparedQuery(sql, {id});

        auto row = result->fetchRow();
        if (!row) {
            return std::nullopt;
        }

        return createEventFromRow(row);
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting event by ID: {}", e.what());
        throw;
    }
}

std::optional<models::EmergencyEvent> EmergencyEventRepository::getEventByCode(const std::string& event_code) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT * FROM low_altitude_traffic_system.emergency_events WHERE event_code = ?";
        auto result = db.executePreparedQuery(sql, {event_code});

        auto row = result->fetchRow();
        if (!row) {
            return std::nullopt;
        }

        return createEventFromRow(row);
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting event by code: {}", e.what());
        throw;
    }
}

std::vector<models::EmergencyEvent> EmergencyEventRepository::getEventsByTaskId(int64_t task_id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT * FROM low_altitude_traffic_system.emergency_events WHERE task_id = ? ORDER BY created_at DESC";
        auto result = db.executePreparedQuery(sql, {task_id});

        std::vector<models::EmergencyEvent> events;
        while (auto row = result->fetchRow()) {
            events.push_back(createEventFromRow(row));
        }

        return events;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting events by task ID: {}", e.what());
        throw;
    }
}

std::vector<models::EmergencyEvent> EmergencyEventRepository::getAllEvents(
    int page,
    int page_size,
    std::optional<std::string> status,
    std::optional<std::string> severity
) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::stringstream sql;
        sql << "SELECT * FROM low_altitude_traffic_system.emergency_events WHERE 1=1";

        std::vector<mysqlx::Value> params;

        if (status.has_value()) {
            sql << " AND status = ?";
            params.push_back(status.value());
        }

        if (severity.has_value()) {
            sql << " AND severity = ?";
            params.push_back(severity.value());
        }

        sql << " ORDER BY created_at DESC";
        sql << " LIMIT ? OFFSET ?";

        params.push_back(page_size);
        params.push_back((page - 1) * page_size);

        auto result = db.executePreparedQuery(sql.str(), params);

        std::vector<models::EmergencyEvent> events;
        while (auto row = result->fetchRow()) {
            events.push_back(createEventFromRow(row));
        }

        return events;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting all events: {}", e.what());
        throw;
    }
}

int64_t EmergencyEventRepository::countEvents(
    std::optional<std::string> status,
    std::optional<std::string> severity
) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::stringstream sql;
        sql << "SELECT COUNT(*) as total FROM low_altitude_traffic_system.emergency_events WHERE 1=1";

        std::vector<mysqlx::Value> params;

        if (status.has_value()) {
            sql << " AND status = ?";
            params.push_back(status.value());
        }

        if (severity.has_value()) {
            sql << " AND severity = ?";
            params.push_back(severity.value());
        }

        auto result = db.executePreparedQuery(sql.str(), params);

        if (auto row = result->fetchRow()) {
            return row[0].get<int64_t>();
        }

        return 0;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error counting events: {}", e.what());
        throw;
    }
}

bool EmergencyEventRepository::updateEventStatus(int64_t id, models::EmergencyStatus status) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "UPDATE low_altitude_traffic_system.emergency_events SET status = ?, updated_at = NOW() WHERE id = ?";
        db.executePreparedUpdate(sql, {models::EmergencyEvent::statusToString(status), id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error updating event status: {}", e.what());
        return false;
    }
}

bool EmergencyEventRepository::respondToEvent(
    int64_t id,
    const std::string& response_action,
    const std::string& response_notes,
    int64_t user_id
) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = R"(
            UPDATE low_altitude_traffic_system.emergency_events
            SET status = 'resolved',
                response_action = ?,
                response_notes = ?,
                responded_by = ?,
                responded_at = NOW(),
                resolved_at = NOW(),
                updated_at = NOW()
            WHERE id = ?
        )";

        db.executePreparedUpdate(sql, {response_action, response_notes, user_id, id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error responding to event: {}", e.what());
        return false;
    }
}

bool EmergencyEventRepository::resolveEvent(int64_t id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = R"(
            UPDATE low_altitude_traffic_system.emergency_events
            SET status = 'resolved',
                resolved_at = NOW(),
                updated_at = NOW()
            WHERE id = ?
        )";

        db.executePreparedUpdate(sql, {id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error resolving event: {}", e.what());
        return false;
    }
}

bool EmergencyEventRepository::cancelEvent(int64_t id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "UPDATE low_altitude_traffic_system.emergency_events SET status = 'cancelled', updated_at = NOW() WHERE id = ?";
        db.executePreparedUpdate(sql, {id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error cancelling event: {}", e.what());
        return false;
    }
}

bool EmergencyEventRepository::deleteEvent(int64_t id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "DELETE FROM low_altitude_traffic_system.emergency_events WHERE id = ?";
        db.executePreparedUpdate(sql, {id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error deleting event: {}", e.what());
        return false;
    }
}

int EmergencyEventRepository::getActiveEventCount() {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT COUNT(*) as count FROM low_altitude_traffic_system.emergency_events WHERE status IN ('active', 'responding')";
        auto result = db.executeQuery(sql);

        auto row = result->fetchRow();
        if (row) {
            return row[0].get<int>();
        }

        return 0;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting active event count: {}", e.what());
        return 0;
    }
}

nlohmann::json EmergencyEventRepository::getEventStatistics() {
    try {
        auto& db = database::DatabaseManager::getInstance();

        nlohmann::json stats;

        // Total events
        auto total_result = db.executeQuery("SELECT COUNT(*) FROM low_altitude_traffic_system.emergency_events");
        auto total_row = total_result->fetchRow();
        stats["total_events"] = total_row ? total_row[0].get<int>() : 0;

        // By status
        auto status_result = db.executeQuery(
            "SELECT status, COUNT(*) as count FROM low_altitude_traffic_system.emergency_events GROUP BY status"
        );
        nlohmann::json by_status = nlohmann::json::object();
        while (auto row = status_result->fetchRow()) {
            by_status[row[0].get<std::string>()] = row[1].get<int>();
        }
        stats["by_status"] = by_status;

        // By severity
        auto severity_result = db.executeQuery(
            "SELECT severity, COUNT(*) as count FROM low_altitude_traffic_system.emergency_events GROUP BY severity"
        );
        nlohmann::json by_severity = nlohmann::json::object();
        while (auto row = severity_result->fetchRow()) {
            by_severity[row[0].get<std::string>()] = row[1].get<int>();
        }
        stats["by_severity"] = by_severity;

        // By type
        auto type_result = db.executeQuery(
            "SELECT type, COUNT(*) as count FROM low_altitude_traffic_system.emergency_events GROUP BY type"
        );
        nlohmann::json by_type = nlohmann::json::object();
        while (auto row = type_result->fetchRow()) {
            by_type[row[0].get<std::string>()] = row[1].get<int>();
        }
        stats["by_type"] = by_type;

        // Active events
        stats["active_events"] = getActiveEventCount();

        return stats;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting event statistics: {}", e.what());
        return nlohmann::json::object();
    }
}

models::EmergencyEvent EmergencyEventRepository::createEventFromRow(mysqlx::Row& row) {
    models::EmergencyEvent event;

    try {
        spdlog::debug("[EmergencyEventRepository] Reading field 0 (id)");
        event.id = row[0].get<int64_t>();

        spdlog::debug("[EmergencyEventRepository] Reading field 1 (event_code)");
        event.event_code = row[1].get<std::string>();

        spdlog::debug("[EmergencyEventRepository] Reading field 2 (task_id)");
        event.task_id = row[2].get<int64_t>();

        spdlog::debug("[EmergencyEventRepository] Reading field 3 (drone_id)");
        if (!row[3].isNull()) {
            event.drone_id = row[3].get<int64_t>();
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 4 (type)");
        event.type = models::EmergencyEvent::stringToType(row[4].get<std::string>());

        spdlog::debug("[EmergencyEventRepository] Reading field 5 (severity)");
        event.severity = models::EmergencyEvent::stringToSeverity(row[5].get<std::string>());

        spdlog::debug("[EmergencyEventRepository] Reading field 6 (status)");
        event.status = models::EmergencyEvent::stringToStatus(row[6].get<std::string>());

        spdlog::debug("[EmergencyEventRepository] Reading field 7 (title)");
        event.title = row[7].get<std::string>();

        spdlog::debug("[EmergencyEventRepository] Reading field 8 (description)");
        if (!row[8].isNull()) {
            event.description = row[8].get<std::string>();
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 9 (location)");
        // MySQL X DevAPI returns JSON fields as mysqlx::DbDoc, need to convert to string first
        try {
            // Try to get as string (works if JSON is stored as string in result)
            std::string location_str = row[9].get<std::string>();
            event.location = nlohmann::json::parse(location_str);
        } catch (const std::exception& e1) {
            try {
                // If that fails, try getting raw bytes and converting
                auto raw_bytes = row[9].getRawBytes();
                std::string location_str(raw_bytes.begin(), raw_bytes.end());
                event.location = nlohmann::json::parse(location_str);
            } catch (const std::exception& e2) {
                spdlog::error("[EmergencyEventRepository] Error parsing location: {}, {}", e1.what(), e2.what());
                // Fallback to empty JSON object
                event.location = nlohmann::json::object();
            }
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 10 (response_action)");
        if (!row[10].isNull()) {
            event.response_action = row[10].get<std::string>();
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 11 (response_notes)");
        if (!row[11].isNull()) {
            event.response_notes = row[11].get<std::string>();
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 12 (responded_by)");
        if (!row[12].isNull()) {
            event.responded_by = row[12].get<int64_t>();
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 13 (responded_at)");
        if (!row[13].isNull()) {
            // 从MySQL TIMESTAMP转换为Unix时间戳（秒）
            uint64_t timestamp = row[13].get<uint64_t>();
            event.responded_at = std::chrono::system_clock::from_time_t(static_cast<time_t>(timestamp));
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 14 (resolved_at)");
        if (!row[14].isNull()) {
            uint64_t timestamp = row[14].get<uint64_t>();
            event.resolved_at = std::chrono::system_clock::from_time_t(static_cast<time_t>(timestamp));
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 15 (created_at)");
        if (!row[15].isNull()) {
            uint64_t timestamp = row[15].get<uint64_t>();
            event.created_at = std::chrono::system_clock::from_time_t(static_cast<time_t>(timestamp));
        } else {
            event.created_at = std::chrono::system_clock::now();
        }

        spdlog::debug("[EmergencyEventRepository] Reading field 16 (updated_at)");
        if (!row[16].isNull()) {
            uint64_t timestamp = row[16].get<uint64_t>();
            event.updated_at = std::chrono::system_clock::from_time_t(static_cast<time_t>(timestamp));
        } else {
            event.updated_at = std::chrono::system_clock::now();
        }

        spdlog::debug("[EmergencyEventRepository] Successfully created event from row");

    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error in createEventFromRow: {}", e.what());
        throw;
    }

    return event;
}

} // namespace repositories
