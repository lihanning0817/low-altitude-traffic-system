#ifndef MODELS_EMERGENCY_EVENT_DAO_H
#define MODELS_EMERGENCY_EVENT_DAO_H

#include "emergency_event.h"
#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class EmergencyEventDAO {
    private:
        MYSQL* mysql_;

    public:
        explicit EmergencyEventDAO(MYSQL* mysql) : mysql_(mysql) {}

        // Create
        std::string create_emergency_event(const EmergencyEvent& emergency);

        // Read
        std::optional<EmergencyEvent> get_emergency_event_by_id(const std::string& id);
        std::vector<EmergencyEvent> get_emergency_events_by_status(EmergencyStatus status);
        std::vector<EmergencyEvent> get_emergency_events_by_severity(EmergencySeverity severity);
        std::vector<EmergencyEvent> get_emergency_events_by_type(EmergencyType type);
        std::vector<EmergencyEvent> get_active_emergency_events();
        std::vector<EmergencyEvent> get_all_emergency_events();

        // Update
        bool update_emergency_event(const EmergencyEvent& emergency);
        bool update_emergency_event_status(const std::string& emergency_id,
                                         EmergencyStatus status,
                                         int responder_id = 0);
        bool resolve_emergency_event(const std::string& emergency_id, int resolver_id = 0);

        // Delete
        bool delete_emergency_event(const std::string& id);

    private:
        EmergencyEvent result_set_to_emergency_event(MYSQL_ROW row);
    };
}

#endif // MODELS_EMERGENCY_EVENT_DAO_H
