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
            INSERT INTO emergency_events (
                event_code, task_id, drone_id, type, severity, status,
                title, description, location, created_at, updated_at
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())
        )";

        auto result = db.executeUpdate(sql, {
            event.event_code,
            static_cast<int64_t>(event.task_id),
            event.drone_id.has_value() ? static_cast<int64_t>(event.drone_id.value()) : mysqlx::Value(mysqlx::NullValue()),
            models::EmergencyEvent::typeToString(event.type),
            models::EmergencyEvent::severityToString(event.severity),
            models::EmergencyEvent::statusToString(event.status),
            event.title,
            event.description,
            event.location.dump()
        });

        return result.getAutoIncrementValue();
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error creating event: {}", e.what());
        throw;
    }
}

std::optional<models::EmergencyEvent> EmergencyEventRepository::getEventById(int64_t id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT * FROM emergency_events WHERE id = ?";
        auto result = db.executeQuery(sql, {id});

        auto rows = result.fetchAll();
        if (rows.empty()) {
            return std::nullopt;
        }

        auto row = rows[0];
        return createEventFromRow(row);
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting event by ID: {}", e.what());
        throw;
    }
}

std::optional<models::EmergencyEvent> EmergencyEventRepository::getEventByCode(const std::string& event_code) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT * FROM emergency_events WHERE event_code = ?";
        auto result = db.executeQuery(sql, {event_code});

        auto rows = result.fetchAll();
        if (rows.empty()) {
            return std::nullopt;
        }

        auto row = rows[0];
        return createEventFromRow(row);
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting event by code: {}", e.what());
        throw;
    }
}

std::vector<models::EmergencyEvent> EmergencyEventRepository::getEventsByTaskId(int64_t task_id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT * FROM emergency_events WHERE task_id = ? ORDER BY created_at DESC";
        auto result = db.executeQuery(sql, {task_id});

        std::vector<models::EmergencyEvent> events;
        auto rows = result.fetchAll();

        for (auto& row : rows) {
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
        sql << "SELECT * FROM emergency_events WHERE 1=1";

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

        auto result = db.executeQuery(sql.str(), params);

        std::vector<models::EmergencyEvent> events;
        auto rows = result.fetchAll();

        for (auto& row : rows) {
            events.push_back(createEventFromRow(row));
        }

        return events;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error getting all events: {}", e.what());
        throw;
    }
}

bool EmergencyEventRepository::updateEventStatus(int64_t id, models::EmergencyStatus status) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "UPDATE emergency_events SET status = ?, updated_at = NOW() WHERE id = ?";
        db.executeUpdate(sql, {models::EmergencyEvent::statusToString(status), id});

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
            UPDATE emergency_events
            SET status = 'responding',
                response_action = ?,
                response_notes = ?,
                responded_by = ?,
                responded_at = NOW(),
                updated_at = NOW()
            WHERE id = ?
        )";

        db.executeUpdate(sql, {response_action, response_notes, user_id, id});

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
            UPDATE emergency_events
            SET status = 'resolved',
                resolved_at = NOW(),
                updated_at = NOW()
            WHERE id = ?
        )";

        db.executeUpdate(sql, {id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error resolving event: {}", e.what());
        return false;
    }
}

bool EmergencyEventRepository::cancelEvent(int64_t id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "UPDATE emergency_events SET status = 'cancelled', updated_at = NOW() WHERE id = ?";
        db.executeUpdate(sql, {id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error cancelling event: {}", e.what());
        return false;
    }
}

bool EmergencyEventRepository::deleteEvent(int64_t id) {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "DELETE FROM emergency_events WHERE id = ?";
        db.executeUpdate(sql, {id});

        return true;
    } catch (const std::exception& e) {
        spdlog::error("[EmergencyEventRepository] Error deleting event: {}", e.what());
        return false;
    }
}

int EmergencyEventRepository::getActiveEventCount() {
    try {
        auto& db = database::DatabaseManager::getInstance();

        std::string sql = "SELECT COUNT(*) as count FROM emergency_events WHERE status IN ('active', 'responding')";
        auto result = db.executeQuery(sql);

        auto rows = result.fetchAll();
        if (!rows.empty()) {
            return rows[0][0].get<int>();
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
        auto total_result = db.executeQuery("SELECT COUNT(*) FROM emergency_events");
        auto total_rows = total_result.fetchAll();
        stats["total_events"] = total_rows.empty() ? 0 : total_rows[0][0].get<int>();

        // By status
        auto status_result = db.executeQuery(
            "SELECT status, COUNT(*) as count FROM emergency_events GROUP BY status"
        );
        nlohmann::json by_status = nlohmann::json::object();
        auto status_rows = status_result.fetchAll();
        for (auto& row : status_rows) {
            by_status[row[0].get<std::string>()] = row[1].get<int>();
        }
        stats["by_status"] = by_status;

        // By severity
        auto severity_result = db.executeQuery(
            "SELECT severity, COUNT(*) as count FROM emergency_events GROUP BY severity"
        );
        nlohmann::json by_severity = nlohmann::json::object();
        auto severity_rows = severity_result.fetchAll();
        for (auto& row : severity_rows) {
            by_severity[row[0].get<std::string>()] = row[1].get<int>();
        }
        stats["by_severity"] = by_severity;

        // By type
        auto type_result = db.executeQuery(
            "SELECT type, COUNT(*) as count FROM emergency_events GROUP BY type"
        );
        nlohmann::json by_type = nlohmann::json::object();
        auto type_rows = type_result.fetchAll();
        for (auto& row : type_rows) {
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

    event.id = row[0].get<int64_t>();
    event.event_code = row[1].get<std::string>();
    event.task_id = row[2].get<int64_t>();

    if (!row[3].isNull()) {
        event.drone_id = row[3].get<int64_t>();
    }

    event.type = models::EmergencyEvent::stringToType(row[4].get<std::string>());
    event.severity = models::EmergencyEvent::stringToSeverity(row[5].get<std::string>());
    event.status = models::EmergencyEvent::stringToStatus(row[6].get<std::string>());
    event.title = row[7].get<std::string>();
    event.description = row[8].get<std::string>();

    std::string location_str = row[9].get<std::string>();
    event.location = nlohmann::json::parse(location_str);

    if (!row[10].isNull()) {
        event.response_action = row[10].get<std::string>();
    }

    if (!row[11].isNull()) {
        event.response_notes = row[11].get<std::string>();
    }

    if (!row[12].isNull()) {
        event.responded_by = row[12].get<int64_t>();
    }

    // Timestamps - simplified
    event.created_at = std::chrono::system_clock::now();
    event.updated_at = std::chrono::system_clock::now();

    return event;
}

} // namespace repositories
