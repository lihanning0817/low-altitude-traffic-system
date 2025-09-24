#pragma once

#include "BaseRepository.h"
#include "models/FlightTask.h"
#include <vector>
#include <optional>
#include <string>

namespace repositories {

/**
 * @brief 飞行任务Repository类
 * 负责飞行任务的数据库操作
 */
class FlightTaskRepository : public BaseRepository {
private:
    static const std::string TABLE_NAME;

public:
    /**
     * @brief 构造函数
     */
    FlightTaskRepository();

    /**
     * @brief 析构函数
     */
    ~FlightTaskRepository() = default;

    // ========== 基础CRUD操作 ==========

    /**
     * @brief 创建飞行任务
     * @param task 飞行任务对象
     * @return 创建的任务ID，失败返回0
     */
    int64_t create(const models::FlightTask& task);

    /**
     * @brief 根据ID查找飞行任务
     * @param id 任务ID
     * @return 飞行任务对象，未找到返回空
     */
    std::optional<models::FlightTask> findById(int64_t id);

    /**
     * @brief 更新飞行任务
     * @param id 任务ID
     * @param task 更新的任务数据
     * @return 是否更新成功
     */
    bool update(int64_t id, const models::FlightTask& task);

    /**
     * @brief 删除飞行任务
     * @param id 任务ID
     * @return 是否删除成功
     */
    bool deleteById(int64_t id);

    // ========== 查询操作 ==========

    /**
     * @brief 获取用户的所有飞行任务
     * @param user_id 用户ID
     * @return 飞行任务列表
     */
    std::vector<models::FlightTask> findByUserId(int64_t user_id);

    /**
     * @brief 根据状态查找飞行任务
     * @param status 任务状态
     * @param user_id 用户ID（可选，如果指定则只查找该用户的任务）
     * @return 飞行任务列表
     */
    std::vector<models::FlightTask> findByStatus(
        models::FlightTaskStatus status,
        std::optional<int64_t> user_id = std::nullopt
    );

    /**
     * @brief 获取所有飞行任务（分页）
     * @param offset 偏移量
     * @param limit 限制数量
     * @param user_id 用户ID（可选，如果指定则只查找该用户的任务）
     * @return 飞行任务列表
     */
    std::vector<models::FlightTask> findAll(
        int offset = 0,
        int limit = 100,
        std::optional<int64_t> user_id = std::nullopt
    );

    /**
     * @brief 统计用户的任务数量
     * @param user_id 用户ID
     * @return 任务数量
     */
    int64_t countByUserId(int64_t user_id);

    /**
     * @brief 根据状态统计任务数量
     * @param status 任务状态
     * @param user_id 用户ID（可选）
     * @return 任务数量
     */
    int64_t countByStatus(
        models::FlightTaskStatus status,
        std::optional<int64_t> user_id = std::nullopt
    );

    // ========== 业务方法 ==========

    /**
     * @brief 更新任务状态
     * @param id 任务ID
     * @param status 新状态
     * @return 是否更新成功
     */
    bool updateStatus(int64_t id, models::FlightTaskStatus status);

    /**
     * @brief 检查用户是否拥有该任务
     * @param task_id 任务ID
     * @param user_id 用户ID
     * @return 是否拥有
     */
    bool isTaskOwnedByUser(int64_t task_id, int64_t user_id);

    /**
     * @brief 获取任务的基本信息（不包含详细路径）
     * @param id 任务ID
     * @return 任务基本信息的JSON
     */
    std::optional<nlohmann::json> getTaskSummary(int64_t id);

    /**
     * @brief 批量更新任务状态
     * @param task_ids 任务ID列表
     * @param status 新状态
     * @param user_id 用户ID（用于权限检查）
     * @return 成功更新的任务数量
     */
    int updateStatusBatch(
        const std::vector<int64_t>& task_ids,
        models::FlightTaskStatus status,
        int64_t user_id
    );

    // ========== 搜索方法 ==========

    /**
     * @brief 按名称搜索任务
     * @param name_pattern 名称模式（支持模糊搜索）
     * @param user_id 用户ID（可选）
     * @return 匹配的任务列表
     */
    std::vector<models::FlightTask> searchByName(
        const std::string& name_pattern,
        std::optional<int64_t> user_id = std::nullopt
    );

    /**
     * @brief 按时间范围查找任务
     * @param start_time 开始时间
     * @param end_time 结束时间
     * @param user_id 用户ID（可选）
     * @return 匹配的任务列表
     */
    std::vector<models::FlightTask> findByTimeRange(
        const std::chrono::system_clock::time_point& start_time,
        const std::chrono::system_clock::time_point& end_time,
        std::optional<int64_t> user_id = std::nullopt
    );

private:
    /**
     * @brief 将数据库行转换为FlightTask对象
     * @param row_json 数据库行JSON
     * @return FlightTask对象
     */
    models::FlightTask jsonToFlightTask(const nlohmann::json& row_json);

    /**
     * @brief 构建查询条件
     * @param user_id 用户ID（可选）
     * @return 查询条件映射
     */
    std::map<std::string, mysqlx::Value> buildUserCondition(
        std::optional<int64_t> user_id
    );
};

} // namespace repositories