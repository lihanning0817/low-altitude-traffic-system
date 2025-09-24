-- ===================================
-- 数据库迁移脚本 - 更新任务状态枚举
-- 版本: 001
-- 创建时间: 2024-09-24
-- 说明: 将flight_tasks表的status字段从in_progress更新为executing
-- ===================================

USE low_altitude_traffic_system;

-- 开始事务
START TRANSACTION;

-- 1. 备份当前状态数据（可选）
CREATE TABLE IF NOT EXISTS flight_tasks_status_backup_20240924 AS
SELECT id, status, updated_at
FROM flight_tasks
WHERE status = 'in_progress';

-- 2. 更新现有的in_progress状态为executing
UPDATE flight_tasks
SET status = 'executing', updated_at = CURRENT_TIMESTAMP
WHERE status = 'in_progress';

-- 3. 修改表结构，更新ENUM定义
ALTER TABLE flight_tasks
MODIFY COLUMN status ENUM('pending', 'approved', 'executing', 'completed', 'cancelled', 'failed') DEFAULT 'pending' COMMENT '任务状态';

-- 4. 验证更新结果
SELECT
    status,
    COUNT(*) as count
FROM flight_tasks
GROUP BY status;

-- 提交事务
COMMIT;

-- 记录迁移完成
INSERT INTO system_config (config_key, config_value, description, data_type)
VALUES (
    'migration.001.completed',
    UNIX_TIMESTAMP(),
    'Migration 001: Updated flight_tasks status enum from in_progress to executing',
    'number'
) ON DUPLICATE KEY UPDATE
    config_value = UNIX_TIMESTAMP(),
    updated_at = CURRENT_TIMESTAMP;

SELECT 'Migration 001: Status update completed successfully!' as message;

-- ===================================
-- 验证查询
-- ===================================
-- 检查是否还有in_progress状态的记录
SELECT COUNT(*) as remaining_in_progress
FROM flight_tasks
WHERE status = 'in_progress';

-- 查看所有状态分布
SELECT status, COUNT(*) as count
FROM flight_tasks
GROUP BY status
ORDER BY count DESC;