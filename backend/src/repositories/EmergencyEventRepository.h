#pragma once

#include "BaseRepository.h"
#include "models/EmergencyEvent.h"
#include <vector>
#include <optional>

namespace repositories {

/**
 * @brief 紧急事件仓储类
 */
class EmergencyEventRepository : public BaseRepository {
public:
    /**
     * @brief 构造函数
     */
    EmergencyEventRepository();

    /**
     * @brief 创建紧急事件
     * @param event 事件对象
     * @return 创建的事件ID
     */
    int64_t createEvent(const models::EmergencyEvent& event);

    /**
     * @brief 根据ID获取事件
     * @param id 事件ID
     * @return 事件对象（可选）
     */
    std::optional<models::EmergencyEvent> getEventById(int64_t id);

    /**
     * @brief 根据事件编号获取事件
     * @param event_code 事件编号
     * @return 事件对象（可选）
     */
    std::optional<models::EmergencyEvent> getEventByCode(const std::string& event_code);

    /**
     * @brief 获取任务的所有紧急事件
     * @param task_id 任务ID
     * @return 事件列表
     */
    std::vector<models::EmergencyEvent> getEventsByTaskId(int64_t task_id);

    /**
     * @brief 获取所有事件（支持分页和过滤）
     * @param page 页码
     * @param page_size 每页大小
     * @param status 状态过滤（可选）
     * @param severity 严重程度过滤（可选）
     * @return 事件列表
     */
    std::vector<models::EmergencyEvent> getAllEvents(
        int page = 1,
        int page_size = 20,
        std::optional<std::string> status = std::nullopt,
        std::optional<std::string> severity = std::nullopt
    );

    /**
     * @brief 更新事件状态
     * @param id 事件ID
     * @param status 新状态
     * @return 是否成功
     */
    bool updateEventStatus(int64_t id, models::EmergencyStatus status);

    /**
     * @brief 响应事件
     * @param id 事件ID
     * @param response_action 响应措施
     * @param response_notes 响应备注
     * @param user_id 响应人员ID
     * @return 是否成功
     */
    bool respondToEvent(
        int64_t id,
        const std::string& response_action,
        const std::string& response_notes,
        int64_t user_id
    );

    /**
     * @brief 解决事件
     * @param id 事件ID
     * @return 是否成功
     */
    bool resolveEvent(int64_t id);

    /**
     * @brief 取消事件
     * @param id 事件ID
     * @return 是否成功
     */
    bool cancelEvent(int64_t id);

    /**
     * @brief 删除事件
     * @param id 事件ID
     * @return 是否成功
     */
    bool deleteEvent(int64_t id);

    /**
     * @brief 获取激活的紧急事件数量
     * @return 激活事件数量
     */
    int getActiveEventCount();

    /**
     * @brief 获取事件统计信息
     * @return JSON格式的统计信息
     */
    nlohmann::json getEventStatistics();

    /**
     * @brief 生成唯一的事件编号
     * @return 事件编号
     */
    std::string generateEventCode();

private:
    /**
     * @brief 从数据库行创建EmergencyEvent对象
     * @param row 数据库行
     * @return EmergencyEvent对象
     */
    models::EmergencyEvent createEventFromRow(mysqlx::Row& row);
};

} // namespace repositories
