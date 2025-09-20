#include "flight_conflict_dao.h"
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <stdexcept>
#include <iostream>
#include <json/json.h>

namespace models {
    std::string FlightConflictDAO::create_flight_conflict(const FlightConflict& conflict) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "INSERT INTO flight_conflicts (id, flight1_id, flight2_id, conflict_time, severity, "
                    "distance, resolution_status, resolution_action) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
                )
            );

            pstmt->setString(1, conflict.id);
            pstmt->setString(2, conflict.flight1_id);
            pstmt->setString(3, conflict.flight2_id);
            auto conflict_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                conflict.conflict_time.time_since_epoch()).count();
            pstmt->setTimestamp(4, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(conflict_time_ts)));

            pstmt->setString(5, conflict.get_severity_string());
            pstmt->setDouble(6, conflict.distance);
            pstmt->setString(7, conflict.get_resolution_status_string());
            pstmt->setString(8, resolution_action_to_json(conflict.resolution_action));

            int rows_affected = pstmt->executeUpdate();
            if (rows_affected > 0) {
                return conflict.id;
            }
            return "";
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in create_flight_conflict: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<FlightConflict> FlightConflictDAO::get_flight_conflict_by_id(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_conflicts WHERE id = ?")
            );
            pstmt->setString(1, id);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            if (rs->next()) {
                return result_set_to_flight_conflict(rs.get());
            }
            return std::nullopt;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_conflict_by_id: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<FlightConflict> FlightConflictDAO::get_flight_conflicts_by_flight(const std::string& flight_id) {
        std::vector<FlightConflict> conflicts;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_conflicts WHERE flight1_id = ? OR flight2_id = ?")
            );
            pstmt->setString(1, flight_id);
            pstmt->setString(2, flight_id);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                conflicts.push_back(result_set_to_flight_conflict(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_conflicts_by_flight: " << e.what() << std::endl;
        }
        return conflicts;
    }

    std::vector<FlightConflict> FlightConflictDAO::get_flight_conflicts_by_severity(ConflictSeverity severity) {
        std::vector<FlightConflict> conflicts;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_conflicts WHERE severity = ?")
            );
            pstmt->setString(1, FlightConflict::get_severity_string());

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                conflicts.push_back(result_set_to_flight_conflict(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_conflicts_by_severity: " << e.what() << std::endl;
        }
        return conflicts;
    }

    std::vector<FlightConflict> FlightConflictDAO::get_flight_conflicts_by_resolution_status(ResolutionStatus status) {
        std::vector<FlightConflict> conflicts;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_conflicts WHERE resolution_status = ?")
            );
            pstmt->setString(1, FlightConflict::get_resolution_status_string());

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                conflicts.push_back(result_set_to_flight_conflict(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_conflicts_by_resolution_status: " << e.what() << std::endl;
        }
        return conflicts;
    }

    std::vector<FlightConflict> FlightConflictDAO::get_unresolved_flight_conflicts() {
        std::vector<FlightConflict> conflicts;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_conflicts WHERE resolution_status = 'pending'")
            );

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                conflicts.push_back(result_set_to_flight_conflict(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_unresolved_flight_conflicts: " << e.what() << std::endl;
        }
        return conflicts;
    }

    std::vector<FlightConflict> FlightConflictDAO::get_all_flight_conflicts() {
        std::vector<FlightConflict> conflicts;
        try {
            std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
            std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT * FROM flight_conflicts"));

            while (rs->next()) {
                conflicts.push_back(result_set_to_flight_conflict(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_all_flight_conflicts: " << e.what() << std::endl;
        }
        return conflicts;
    }

    bool FlightConflictDAO::update_flight_conflict(const FlightConflict& conflict) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "UPDATE flight_conflicts SET flight1_id = ?, flight2_id = ?, conflict_time = ?, "
                    "severity = ?, distance = ?, resolution_status = ?, resolution_action = ?, "
                    "resolved_at = ?, created_at = NOW() WHERE id = ?"
                )
            );

            pstmt->setString(1, conflict.flight1_id);
            pstmt->setString(2, conflict.flight2_id);
            auto conflict_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                conflict.conflict_time.time_since_epoch()).count();
            pstmt->setTimestamp(3, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(conflict_time_ts)));

            pstmt->setString(4, conflict.get_severity_string());
            pstmt->setDouble(5, conflict.distance);
            pstmt->setString(6, conflict.get_resolution_status_string());
            pstmt->setString(7, resolution_action_to_json(conflict.resolution_action));

            if (conflict.resolved_at.has_value()) {
                auto resolved_at_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                    conflict.resolved_at.value().time_since_epoch()).count();
                pstmt->setTimestamp(8, std::unique_ptr<sql::Timestamp>(
                    new sql::Timestamp(resolved_at_ts)));
            } else {
                pstmt->setNull(8, 0);
            }

            pstmt->setString(9, conflict.id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in update_flight_conflict: " << e.what() << std::endl;
            return false;
        }
    }

    bool FlightConflictDAO::update_flight_conflict_resolution(const std::string& conflict_id,
                                                            ResolutionStatus status,
                                                            const std::optional<ResolutionAction>& action) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "UPDATE flight_conflicts SET resolution_status = ?, resolution_action = ?, resolved_at = NOW() WHERE id = ?"
                )
            );

            pstmt->setString(1, FlightConflict::get_resolution_status_string());
            pstmt->setString(2, resolution_action_to_json(action));
            pstmt->setString(3, conflict_id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in update_flight_conflict_resolution: " << e.what() << std::endl;
            return false;
        }
    }

    bool FlightConflictDAO::delete_flight_conflict(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("DELETE FROM flight_conflicts WHERE id = ?")
            );
            pstmt->setString(1, id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in delete_flight_conflict: " << e.what() << std::endl;
            return false;
        }
    }

    FlightConflict FlightConflictDAO::result_set_to_flight_conflict(sql::ResultSet* rs) {
        FlightConflict conflict;
        conflict.id = rs->getString("id");
        conflict.flight1_id = rs->getString("flight1_id");
        conflict.flight2_id = rs->getString("flight2_id");

        // Handle timestamps
        auto conflict_time_ts = rs->getTimestamp("conflict_time");
        conflict.conflict_time = std::chrono::system_clock::from_time_t(conflict_time_ts->getTime() / 1000);

        conflict.severity = FlightConflict::parse_severity_string(rs->getString("severity"));
        conflict.distance = rs->getDouble("distance");
        conflict.resolution_status = FlightConflict::parse_resolution_status_string(rs->getString("resolution_status"));

        if (!rs->isNull("resolution_action")) {
            conflict.resolution_action = json_to_resolution_action(rs->getString("resolution_action"));
        }

        if (!rs->isNull("resolved_at")) {
            auto resolved_at_ts = rs->getTimestamp("resolved_at");
            conflict.resolved_at = std::chrono::system_clock::from_time_t(resolved_at_ts->getTime() / 1000);
        }

        auto created_at_ts = rs->getTimestamp("created_at");
        conflict.created_at = std::chrono::system_clock::from_time_t(created_at_ts->getTime() / 1000);

        return conflict;
    }

    std::string FlightConflictDAO::resolution_action_to_json(const std::optional<ResolutionAction>& action) {
        if (!action.has_value()) {
            return "";
        }

        Json::Value root;
        root["type"] = action.value().type;

        Json::Value actions_array(Json::arrayValue);
        for (const auto& act : action.value().actions) {
            actions_array.append(act);
        }
        root["actions"] = actions_array;

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, root);
    }

    std::optional<ResolutionAction> FlightConflictDAO::json_to_resolution_action(const std::string& json_str) {
        if (json_str.empty()) {
            return std::nullopt;
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        std::string errors;

        if (reader->parse(json_str.c_str(), json_str.c_str() + json_str.length(), &root, &errors)) {
            ResolutionAction action;
            action.type = root["type"].asString();

            const Json::Value& actions_array = root["actions"];
            for (const auto& act_value : actions_array) {
                action.actions.push_back(act_value.asString());
            }

            return action;
        }

        return std::nullopt;
    }
}