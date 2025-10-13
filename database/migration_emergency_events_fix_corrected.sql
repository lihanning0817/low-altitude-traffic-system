-- ============================================================================
-- 数据库迁移脚本: 修复 emergency_events 表以匹配后端代码要求（修正版）
-- 创建时间: 2025-10-13
-- 说明: 此脚本将修改 emergency_events 表结构以适配后端 EmergencyEventRepository
-- 修正: 将BIGINT改为INT以匹配现有表结构
-- ============================================================================

USE low_altitude_traffic_system;

-- 1. 删除旧表（如果存在）
DROP TABLE IF EXISTS emergency_events;

-- 2. 创建符合后端要求的新表结构
CREATE TABLE emergency_events (
    -- 主键和基本字段
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '事件ID',
    event_code VARCHAR(50) NOT NULL UNIQUE COMMENT '事件编号（格式: EMG-YYYYMMDD-XXXX）',
    task_id INT NOT NULL COMMENT '关联任务ID',
    drone_id INT NULL COMMENT '关联无人机ID（可选）',

    -- 事件信息
    type ENUM(
        'equipment_failure',    -- 设备故障
        'weather_emergency',    -- 恶劣天气
        'collision_risk',       -- 碰撞风险
        'signal_loss',          -- 信号丢失
        'battery_low',          -- 电量低
        'geofence_violation',   -- 电子围栏违规
        'manual_emergency',     -- 手动紧急
        'other'                 -- 其他
    ) NOT NULL COMMENT '事件类型',

    severity ENUM(
        'low',      -- 低
        'medium',   -- 中
        'high',     -- 高
        'critical'  -- 严重
    ) NOT NULL COMMENT '严重程度',

    status ENUM(
        'active',      -- 激活中
        'responding',  -- 响应中
        'resolved',    -- 已解决
        'cancelled'    -- 已取消
    ) DEFAULT 'active' NOT NULL COMMENT '事件状态',

    title VARCHAR(255) NOT NULL COMMENT '事件标题',
    description TEXT NULL COMMENT '事件描述（可选）',

    -- 位置信息（JSON格式: {"lat": 39.9042, "lon": 116.4074}）
    location JSON NOT NULL COMMENT '事件位置（JSON格式）',

    -- 响应信息
    response_action TEXT NULL COMMENT '响应措施（可选）',
    response_notes TEXT NULL COMMENT '响应备注（可选）',
    responded_by INT NULL COMMENT '响应人员ID（可选）',
    responded_at TIMESTAMP NULL COMMENT '响应时间（可选）',
    resolved_at TIMESTAMP NULL COMMENT '解决时间（可选）',

    -- 时间戳
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    -- 外键约束
    FOREIGN KEY (task_id) REFERENCES flight_tasks(id) ON DELETE CASCADE,
    FOREIGN KEY (drone_id) REFERENCES drones(id) ON DELETE SET NULL,
    FOREIGN KEY (responded_by) REFERENCES users(id) ON DELETE SET NULL,

    -- 索引
    INDEX idx_event_code (event_code),
    INDEX idx_task_id (task_id),
    INDEX idx_drone_id (drone_id),
    INDEX idx_type (type),
    INDEX idx_severity (severity),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='应急事件表（已适配后端代码）';

-- 3. 插入测试数据（可选）
INSERT INTO emergency_events (event_code, task_id, type, severity, status, title, description, location)
VALUES
('EMG-20251013-0001', 1, 'equipment_failure', 'high', 'active', '设备故障测试', '这是一个测试应急事件', '{"lat": 39.9042, "lon": 116.4074}'),
('EMG-20251013-0002', 1, 'weather_emergency', 'critical', 'active', '恶劣天气警告', '强风暴来袭', '{"lat": 31.2304, "lon": 121.4737}');

-- ============================================================================
-- 迁移完成
-- ============================================================================
