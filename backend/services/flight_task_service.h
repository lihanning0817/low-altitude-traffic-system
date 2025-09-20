#ifndef SERVICES_FLIGHT_TASK_SERVICE_H
#define SERVICES_FLIGHT_TASK_SERVICE_H

#include "../models/flight_task.h"
#include "../models/flight_task_dao.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class FlightTaskService {
    private:
        std::shared_ptr<models::FlightTaskDAO> flight_task_dao_;

    public:
        explicit FlightTaskService(std::shared_ptr<models::FlightTaskDAO> flight_task_dao)
            : flight_task_dao_(flight_task_dao) {}

        // Flight task management
        std::optional<models::FlightTask> create_flight_task(const std::string& name,
                                                           const std::string& description,
                                                           int creator_id,
                                                           models::FlightTaskPriority priority = models::FlightTaskPriority::MEDIUM);

        std::optional<models::FlightTask> get_flight_task_by_id(const std::string& id);
        std::vector<models::FlightTask> get_flight_tasks_by_creator(int creator_id);
        std::vector<models::FlightTask> get_flight_tasks_by_assignee(const std::string& assignee_id);
        std::vector<models::FlightTask> get_flight_tasks_by_status(models::FlightTaskStatus status);
        std::vector<models::FlightTask> get_all_flight_tasks();

        bool update_flight_task(const models::FlightTask& task);
        bool update_flight_task_status(const std::string& task_id, models::FlightTaskStatus status);
        bool update_flight_task_progress(const std::string& task_id, int progress);
        bool assign_flight_task(const std::string& task_id, const std::string& device_id);
        bool start_flight_task(const std::string& task_id);
        bool complete_flight_task(const std::string& task_id);
        bool cancel_flight_task(const std::string& task_id);

        bool delete_flight_task(const std::string& id);

        // Helper methods
        std::string generate_task_id();
    };
}

#endif // SERVICES_FLIGHT_TASK_SERVICE_H