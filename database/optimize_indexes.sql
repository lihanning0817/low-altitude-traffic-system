-- ===================================
-- 数据库索引优化脚本
-- 版本: 1.0
-- 创建时间: 2024-09-24
-- 说明: 为flight_tasks和users表添加复合索引以提升查询性能
-- ===================================

USE low_altitude_traffic_system;

-- ===================================
-- users表复合索引优化
-- ===================================

-- 1. 用户登录查询优化索引 (username + status)
-- 用于: SELECT * FROM users WHERE username = ? AND status = 'active'
CREATE INDEX IF NOT EXISTS idx_users_username_status
ON users(username, status);

-- 2. 用户角色和状态查询索引 (role + status + created_at)
-- 用于: SELECT * FROM users WHERE role = ? AND status = ? ORDER BY created_at DESC
CREATE INDEX IF NOT EXISTS idx_users_role_status_created
ON users(role, status, created_at);

-- 3. 邮箱和状态查询索引 (email + status)
-- 用于: SELECT * FROM users WHERE email = ? AND status = 'active'
CREATE INDEX IF NOT EXISTS idx_users_email_status
ON users(email, status);

-- 4. 最后登录时间查询索引 (status + last_login)
-- 用于: SELECT * FROM users WHERE status = 'active' ORDER BY last_login DESC
CREATE INDEX IF NOT EXISTS idx_users_status_last_login
ON users(status, last_login);

-- ===================================
-- flight_tasks表复合索引优化
-- ===================================

-- 1. 飞行员任务状态查询索引 (pilot_id + status + planned_start_time)
-- 用于: SELECT * FROM flight_tasks WHERE pilot_id = ? AND status IN (?) ORDER BY planned_start_time
CREATE INDEX IF NOT EXISTS idx_flight_tasks_pilot_status_start
ON flight_tasks(pilot_id, status, planned_start_time);

-- 2. 任务类型和状态查询索引 (task_type + status + created_at)
-- 用于: SELECT * FROM flight_tasks WHERE task_type = ? AND status = ? ORDER BY created_at DESC
CREATE INDEX IF NOT EXISTS idx_flight_tasks_type_status_created
ON flight_tasks(task_type, status, created_at);

-- 3. 时间范围查询索引 (status + planned_start_time + planned_end_time)
-- 用于: SELECT * FROM flight_tasks WHERE status = ? AND planned_start_time BETWEEN ? AND ?
CREATE INDEX IF NOT EXISTS idx_flight_tasks_status_time_range
ON flight_tasks(status, planned_start_time, planned_end_time);

-- 4. 优先级和状态查询索引 (priority + status + planned_start_time)
-- 用于: SELECT * FROM flight_tasks WHERE priority = ? AND status IN (?) ORDER BY planned_start_time
CREATE INDEX IF NOT EXISTS idx_flight_tasks_priority_status_start
ON flight_tasks(priority, status, planned_start_time);

-- 5. 创建者查询索引 (created_by + status + created_at)
-- 用于: SELECT * FROM flight_tasks WHERE created_by = ? AND status = ? ORDER BY created_at DESC
CREATE INDEX IF NOT EXISTS idx_flight_tasks_creator_status_created
ON flight_tasks(created_by, status, created_at);

-- 6. 批准者查询索引 (approved_by + status + updated_at)
-- 用于: SELECT * FROM flight_tasks WHERE approved_by = ? AND status = ? ORDER BY updated_at DESC
CREATE INDEX IF NOT EXISTS idx_flight_tasks_approver_status_updated
ON flight_tasks(approved_by, status, updated_at);

-- 7. 地点搜索索引 (departure_location + destination_location)
-- 注意：TEXT字段需要指定前缀长度
CREATE INDEX IF NOT EXISTS idx_flight_tasks_locations
ON flight_tasks(departure_location(50), destination_location(50));

-- 8. 高度和速度查询索引 (flight_altitude + flight_speed + status)
-- 用于统计和分析查询
CREATE INDEX IF NOT EXISTS idx_flight_tasks_altitude_speed_status
ON flight_tasks(flight_altitude, flight_speed, status);

-- ===================================
-- flight_logs表复合索引优化
-- ===================================

-- 1. 任务时间查询索引 (task_id + timestamp)
-- 用于: SELECT * FROM flight_logs WHERE task_id = ? ORDER BY timestamp DESC
CREATE INDEX IF NOT EXISTS idx_flight_logs_task_timestamp
ON flight_logs(task_id, timestamp);

-- 2. 位置查询索引 (task_id + latitude + longitude)
-- 用于地理位置相关查询
CREATE INDEX IF NOT EXISTS idx_flight_logs_task_location
ON flight_logs(task_id, latitude, longitude);

-- 3. 时间范围查询索引 (timestamp + task_id)
-- 用于: SELECT * FROM flight_logs WHERE timestamp BETWEEN ? AND ? AND task_id IN (?)
CREATE INDEX IF NOT EXISTS idx_flight_logs_timestamp_task
ON flight_logs(timestamp, task_id);

-- ===================================
-- system_config表索引优化
-- ===================================

-- 1. 配置键和数据类型查询索引 (config_key + data_type)
CREATE INDEX IF NOT EXISTS idx_system_config_key_type
ON system_config(config_key, data_type);

-- 2. 更新时间查询索引 (updated_at + config_key)
CREATE INDEX IF NOT EXISTS idx_system_config_updated_key
ON system_config(updated_at, config_key);

-- ===================================
-- 查看创建的索引
-- ===================================

-- 查看users表的所有索引
SHOW INDEXES FROM users;

-- 查看flight_tasks表的所有索引
SHOW INDEXES FROM flight_tasks;

-- 查看flight_logs表的所有索引
SHOW INDEXES FROM flight_logs;

-- 查看system_config表的所有索引
SHOW INDEXES FROM system_config;

-- ===================================
-- 索引性能分析查询示例
-- ===================================

-- 1. 分析飞行员的活跃任务
-- EXPLAIN SELECT * FROM flight_tasks
-- WHERE pilot_id = 1 AND status IN ('approved', 'executing')
-- ORDER BY planned_start_time;

-- 2. 分析用户登录查询
-- EXPLAIN SELECT id, username, role FROM users
-- WHERE username = 'pilot001' AND status = 'active';

-- 3. 分析任务类型统计查询
-- EXPLAIN SELECT task_type, COUNT(*) as count
-- FROM flight_tasks
-- WHERE status = 'completed' AND created_at >= '2024-01-01'
-- GROUP BY task_type;

-- 4. 分析飞行日志时间范围查询
-- EXPLAIN SELECT * FROM flight_logs
-- WHERE task_id = 1 AND timestamp BETWEEN '2024-09-24 00:00:00' AND '2024-09-24 23:59:59'
-- ORDER BY timestamp DESC;

-- ===================================
-- 索引维护建议
-- ===================================

-- 定期分析表统计信息
-- ANALYZE TABLE users;
-- ANALYZE TABLE flight_tasks;
-- ANALYZE TABLE flight_logs;
-- ANALYZE TABLE system_config;

-- 检查索引碎片化情况（MySQL 5.7+）
-- SELECT TABLE_NAME, INDEX_NAME, STAT_VALUE as pages
-- FROM mysql.innodb_index_stats
-- WHERE STAT_NAME = 'n_leaf_pages'
-- AND DATABASE_NAME = 'low_altitude_traffic_system'
-- ORDER BY pages DESC;

-- ===================================
-- 注意事项
-- ===================================
-- 1. 复合索引的列顺序很重要，应该按照查询频率和选择性排序
-- 2. 索引会占用存储空间并影响写入性能，需要权衡
-- 3. 定期监控索引使用情况，删除不必要的索引
-- 4. 对于大表，建议在低峰期创建索引
-- 5. 文本字段的索引需要指定合适的前缀长度

-- ===================================
-- 索引创建完成
-- ===================================
SELECT 'Database index optimization completed successfully!' as message;