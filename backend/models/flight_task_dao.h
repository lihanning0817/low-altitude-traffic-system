#ifndef MODELS_FLIGHT_TASK_DAO_H
#define MODELS_FLIGHT_TASK_DAO_H

#include "flight_task.h"
#include <mysql/mysql.h>
#include <memory>
#include <vector>
#include <optional>

namespace models {
    class FlightTaskDAO {
    private:
        MYSQL* mysql_;

    public:
        explicit FlightTaskDAO(MYSQL* mysql) : mysql_(mysql) {}

        // Create
        std::string create_flight_task(const FlightTask& task);

        // Read
        std::optional<FlightTask> get_flight_task_by_id(const std::string& id);
        std::vector<FlightTask> get_flight_tasks_by_creator(int creator_id);
        std::vector<FlightTask> get_flight_tasks_by_assignee(const std::string& assignee_id);
        std::vector<FlightTask> get_flight_tasks_by_status(FlightTaskStatus status);
        std::vector<FlightTask> get_all_flight_tasks();

        // Update
        bool update_flight_task(const FlightTask& task);
        bool update_flight_task_status(const std::string& task_id, FlightTaskStatus status);
        bool update_flight_task_progress(const std::string& task_id, int progress);

        // Delete
        bool delete_flight_task(const std::string& id);

    private:
        FlightTask result_set_to_flight_task(MYSQL_ROW row);
    };
}

#endif // MODELS_FLIGHT_TASK_DAO_H
