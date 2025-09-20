#include "flight_permit_dao.h"
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <stdexcept>
#include <iostream>

namespace models {
    std::string FlightPermitDAO::create_flight_permit(const FlightPermit& permit) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "INSERT INTO flight_permits (id, flight_task_id, airspace_id, status, applicant_id, "
                    "application_time, start_time, end_time, remarks) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
                )
            );

            pstmt->setString(1, permit.id);
            pstmt->setString(2, permit.flight_task_id);
            pstmt->setString(3, permit.airspace_id);
            pstmt->setString(4, permit.get_status_string());
            pstmt->setInt(5, permit.applicant_id);

            auto application_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                permit.application_time.time_since_epoch()).count();
            pstmt->setTimestamp(6, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(application_time_ts)));

            auto start_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                permit.start_time.time_since_epoch()).count();
            pstmt->setTimestamp(7, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(start_time_ts)));

            auto end_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                permit.end_time.time_since_epoch()).count();
            pstmt->setTimestamp(8, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(end_time_ts)));

            pstmt->setString(9, permit.remarks);

            int rows_affected = pstmt->executeUpdate();
            if (rows_affected > 0) {
                return permit.id;
            }
            return "";
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in create_flight_permit: " << e.what() << std::endl;
            return "";
        }
    }

    std::optional<FlightPermit> FlightPermitDAO::get_flight_permit_by_id(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_permits WHERE id = ?")
            );
            pstmt->setString(1, id);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            if (rs->next()) {
                return result_set_to_flight_permit(rs.get());
            }
            return std::nullopt;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_permit_by_id: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<FlightPermit> FlightPermitDAO::get_flight_permits_by_applicant(int applicant_id) {
        std::vector<FlightPermit> permits;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_permits WHERE applicant_id = ?")
            );
            pstmt->setInt(1, applicant_id);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                permits.push_back(result_set_to_flight_permit(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_permits_by_applicant: " << e.what() << std::endl;
        }
        return permits;
    }

    std::vector<FlightPermit> FlightPermitDAO::get_flight_permits_by_airspace(const std::string& airspace_id) {
        std::vector<FlightPermit> permits;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_permits WHERE airspace_id = ?")
            );
            pstmt->setString(1, airspace_id);

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                permits.push_back(result_set_to_flight_permit(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_permits_by_airspace: " << e.what() << std::endl;
        }
        return permits;
    }

    std::vector<FlightPermit> FlightPermitDAO::get_flight_permits_by_status(PermitStatus status) {
        std::vector<FlightPermit> permits;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_permits WHERE status = ?")
            );
            pstmt->setString(1, FlightPermit::get_status_string());

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                permits.push_back(result_set_to_flight_permit(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_flight_permits_by_status: " << e.what() << std::endl;
        }
        return permits;
    }

    std::vector<FlightPermit> FlightPermitDAO::get_pending_flight_permits() {
        std::vector<FlightPermit> permits;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("SELECT * FROM flight_permits WHERE status = 'pending'")
            );

            std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
            while (rs->next()) {
                permits.push_back(result_set_to_flight_permit(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_pending_flight_permits: " << e.what() << std::endl;
        }
        return permits;
    }

    std::vector<FlightPermit> FlightPermitDAO::get_all_flight_permits() {
        std::vector<FlightPermit> permits;
        try {
            std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
            std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT * FROM flight_permits"));

            while (rs->next()) {
                permits.push_back(result_set_to_flight_permit(rs.get()));
            }
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in get_all_flight_permits: " << e.what() << std::endl;
        }
        return permits;
    }

    bool FlightPermitDAO::update_flight_permit(const FlightPermit& permit) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "UPDATE flight_permits SET flight_task_id = ?, airspace_id = ?, status = ?, applicant_id = ?, "
                    "approver_id = ?, application_time = ?, approval_time = ?, start_time = ?, "
                    "end_time = ?, remarks = ? WHERE id = ?"
                )
            );

            pstmt->setString(1, permit.flight_task_id);
            pstmt->setString(2, permit.airspace_id);
            pstmt->setString(3, permit.get_status_string());
            pstmt->setInt(4, permit.applicant_id);

            if (permit.approver_id.has_value()) {
                pstmt->setInt(5, permit.approver_id.value());
            } else {
                pstmt->setNull(5, 0);
            }

            auto application_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                permit.application_time.time_since_epoch()).count();
            pstmt->setTimestamp(6, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(application_time_ts)));

            if (permit.approval_time.has_value()) {
                auto approval_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                    permit.approval_time.value().time_since_epoch()).count();
                pstmt->setTimestamp(7, std::unique_ptr<sql::Timestamp>(
                    new sql::Timestamp(approval_time_ts)));
            } else {
                pstmt->setNull(7, 0);
            }

            auto start_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                permit.start_time.time_since_epoch()).count();
            pstmt->setTimestamp(8, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(start_time_ts)));

            auto end_time_ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                permit.end_time.time_since_epoch()).count();
            pstmt->setTimestamp(9, std::unique_ptr<sql::Timestamp>(
                new sql::Timestamp(end_time_ts)));

            pstmt->setString(10, permit.remarks);
            pstmt->setString(11, permit.id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in update_flight_permit: " << e.what() << std::endl;
            return false;
        }
    }

    bool FlightPermitDAO::update_flight_permit_status(const std::string& permit_id, PermitStatus status,
                                                    int approver_id, const std::string& remarks) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement(
                    "UPDATE flight_permits SET status = ?, approver_id = ?, approval_time = NOW(), remarks = ? WHERE id = ?"
                )
            );

            pstmt->setString(1, FlightPermit::get_status_string());
            if (approver_id > 0) {
                pstmt->setInt(2, approver_id);
            } else {
                pstmt->setNull(2, 0);
            }
            pstmt->setString(3, remarks);
            pstmt->setString(4, permit_id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in update_flight_permit_status: " << e.what() << std::endl;
            return false;
        }
    }

    bool FlightPermitDAO::delete_flight_permit(const std::string& id) {
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(
                conn_->prepareStatement("DELETE FROM flight_permits WHERE id = ?")
            );
            pstmt->setString(1, id);

            int rows_affected = pstmt->executeUpdate();
            return rows_affected > 0;
        } catch (sql::SQLException& e) {
            std::cerr << "SQLException in delete_flight_permit: " << e.what() << std::endl;
            return false;
        }
    }

    FlightPermit FlightPermitDAO::result_set_to_flight_permit(sql::ResultSet* rs) {
        FlightPermit permit;
        permit.id = rs->getString("id");
        permit.flight_task_id = rs->getString("flight_task_id");
        permit.airspace_id = rs->getString("airspace_id");
        permit.status = FlightPermit::parse_status_string(rs->getString("status"));
        permit.applicant_id = rs->getInt("applicant_id");

        if (!rs->isNull("approver_id")) {
            permit.approver_id = rs->getInt("approver_id");
        }

        // Handle timestamps
        auto application_time_ts = rs->getTimestamp("application_time");
        permit.application_time = std::chrono::system_clock::from_time_t(application_time_ts->getTime() / 1000);

        if (!rs->isNull("approval_time")) {
            auto approval_time_ts = rs->getTimestamp("approval_time");
            permit.approval_time = std::chrono::system_clock::from_time_t(approval_time_ts->getTime() / 1000);
        }

        auto start_time_ts = rs->getTimestamp("start_time");
        permit.start_time = std::chrono::system_clock::from_time_t(start_time_ts->getTime() / 1000);

        auto end_time_ts = rs->getTimestamp("end_time");
        permit.end_time = std::chrono::system_clock::from_time_t(end_time_ts->getTime() / 1000);

        permit.remarks = rs->getString("remarks");

        return permit;
    }
}