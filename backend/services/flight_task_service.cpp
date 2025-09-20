#include "flight_task_service.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

namespace services {
    std::optional<models::FlightTask> FlightTaskService::create_flight_task(const std::string& name,
                                                                         const std::string& description,
                                                                         int creator_id,
                                                                         models::FlightTaskPriority priority) {
        // Create flight task object
        models::FlightTask task;
        task.id = generate_task_id();
        task.name = name;
        task.description = description;
        task.creator_id = creator_id;
        task.priority = priority;
        task.status = models::FlightTaskStatus::PENDING;
        task.progress = 0;
        task.created_at = std::chrono::system_clock::now();
        task.updated_at = std::chrono::system_clock::now();

        // Save to database
        std::string task_id = flight_task_dao_->create_flight_task(task);
        if (!task_id.empty()) {
            task.id = task_id;
            return task;
        }

        return std::nullopt;
    }

    std::optional<models::FlightTask> FlightTaskService::get_flight_task_by_id(const std::string& id) {
        return flight_task_dao_->get_flight_task_by_id(id);
    }

    std::vector<models::FlightTask> FlightTaskService::get_flight_tasks_by_creator(int creator_id) {
        return flight_task_dao_->get_flight_tasks_by_creator(creator_id);
    }

    std::vector<models::FlightTask> FlightTaskService::get_flight_tasks_by_assignee(const std::string& assignee_id) {
        return flight_task_dao_->get_flight_tasks_by_assignee(assignee_id);
    }

    std::vector<models::FlightTask> FlightTaskService::get_flight_tasks_by_status(models::FlightTaskStatus status) {
        return flight_task_dao_->get_flight_tasks_by_status(status);
    }

    std::vector<models::FlightTask> FlightTaskService::get_all_flight_tasks() {
        return flight_task_dao_->get_all_flight_tasks();
    }

    bool FlightTaskService::update_flight_task(const models::FlightTask& task) {
        return flight_task_dao_->update_flight_task(task);
    }

    bool FlightTaskService::update_flight_task_status(const std::string& task_id, models::FlightTaskStatus status) {
        return flight_task_dao_->update_flight_task_status(task_id, status);
    }

    bool FlightTaskService::update_flight_task_progress(const std::string& task_id, int progress) {
        return flight_task_dao_->update_flight_task_progress(task_id, progress);
    }

    bool FlightTaskService::assign_flight_task(const std::string& task_id, const std::string& device_id) {
        auto task = flight_task_dao_->get_flight_task_by_id(task_id);
        if (task.has_value()) {
            task.value().assignee_id = device_id;
            task.value().updated_at = std::chrono::system_clock::now();
            return flight_task_dao_->update_flight_task(task.value());
        }
        return false;
    }

    bool FlightTaskService::start_flight_task(const std::string& task_id) {
        auto task = flight_task_dao_->get_flight_task_by_id(task_id);
        if (task.has_value()) {
            task.value().status = models::FlightTaskStatus::IN_PROGRESS;
            task.value().start_time = std::chrono::system_clock::now();
            task.value().progress = 10; // Set initial progress
            task.value().updated_at = std::chrono::system_clock::now();
            return flight_task_dao_->update_flight_task(task.value());
        }
        return false;
    }

    bool FlightTaskService::complete_flight_task(const std::string& task_id) {
        auto task = flight_task_dao_->get_flight_task_by_id(task_id);
        if (task.has_value()) {
            task.value().status = models::FlightTaskStatus::COMPLETED;
            task.value().end_time = std::chrono::system_clock::now();
            task.value().progress = 100;
            task.value().updated_at = std::chrono::system_clock::now();
            return flight_task_dao_->update_flight_task(task.value());
        }
        return false;
    }

    bool FlightTaskService::cancel_flight_task(const std::string& task_id) {
        auto task = flight_task_dao_->get_flight_task_by_id(task_id);
        if (task.has_value()) {
            task.value().status = models::FlightTaskStatus::CANCELLED;
            task.value().end_time = std::chrono::system_clock::now();
            task.value().updated_at = std::chrono::system_clock::now();
            return flight_task_dao_->update_flight_task(task.value());
        }
        return false;
    }

    bool FlightTaskService::delete_flight_task(const std::string& id) {
        return flight_task_dao_->delete_flight_task(id);
    }

    std::string FlightTaskService::generate_task_id() {
        // Generate a unique task ID
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::stringstream ss;
        ss << "FT" << std::setfill('0') << std::setw(10) << (millis % 10000000000ULL);

        return ss.str();
    }
}