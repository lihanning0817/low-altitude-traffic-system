-- ========================================
-- 地理围栏和禁飞区表迁移脚本
-- 版本: 002
-- 创建时间: 2025-01-03
-- 描述: 添加地理围栏(geofences)和禁飞区(no_fly_zones)表
-- ========================================

USE low_altitude_traffic_system;

-- ========================================
-- 1. 地理围栏表 (geofences)
-- ========================================

CREATE TABLE IF NOT EXISTS geofences (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '地理围栏ID',
    name VARCHAR(255) NOT NULL COMMENT '围栏名称',
    description TEXT COMMENT '围栏描述',

    -- 围栏类型：circular(圆形), polygon(多边形), corridor(走廊)
    fence_type ENUM('circular', 'polygon', 'corridor') NOT NULL DEFAULT 'polygon' COMMENT '围栏类型',

    -- 几何数据（JSON格式存储）
    -- circular: {"center": {"lat": 39.9, "lng": 116.4}, "radius": 1000}
    -- polygon: {"points": [{"lat": 39.9, "lng": 116.4}, ...]}
    -- corridor: {"path": [{"lat": 39.9, "lng": 116.4}, ...], "width": 500}
    geometry JSON NOT NULL COMMENT '围栏几何数据',

    -- 围栏属性
    max_altitude DECIMAL(10, 2) DEFAULT NULL COMMENT '最大飞行高度(米)',
    min_altitude DECIMAL(10, 2) DEFAULT NULL COMMENT '最小飞行高度(米)',

    -- 围栏状态
    status ENUM('active', 'inactive', 'archived') NOT NULL DEFAULT 'active' COMMENT '围栏状态',

    -- 围栏用途：warning(警告区), restricted(限制区), allowed(允许区)
    purpose ENUM('warning', 'restricted', 'allowed') NOT NULL DEFAULT 'warning' COMMENT '围栏用途',

    -- 时间限制（可选）
    effective_start_time DATETIME DEFAULT NULL COMMENT '生效开始时间',
    effective_end_time DATETIME DEFAULT NULL COMMENT '生效结束时间',

    -- 创建者信息
    created_by BIGINT NOT NULL COMMENT '创建者用户ID',

    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    -- 外键约束
    FOREIGN KEY (created_by) REFERENCES users(id) ON DELETE CASCADE,

    -- 索引
    INDEX idx_status (status),
    INDEX idx_fence_type (fence_type),
    INDEX idx_purpose (purpose),
    INDEX idx_created_by (created_by),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='地理围栏表';

-- ========================================
-- 2. 禁飞区表 (no_fly_zones)
-- ========================================

CREATE TABLE IF NOT EXISTS no_fly_zones (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '禁飞区ID',
    name VARCHAR(255) NOT NULL COMMENT '禁飞区名称',
    description TEXT COMMENT '禁飞区描述',

    -- 禁飞区类型：permanent(永久), temporary(临时), emergency(紧急)
    zone_type ENUM('permanent', 'temporary', 'emergency') NOT NULL DEFAULT 'permanent' COMMENT '禁飞区类型',

    -- 几何数据（JSON格式存储，支持多边形）
    -- {"points": [{"lat": 39.9, "lng": 116.4}, {"lat": 39.91, "lng": 116.41}, ...]}
    geometry JSON NOT NULL COMMENT '禁飞区几何数据',

    -- 高度限制
    max_altitude DECIMAL(10, 2) DEFAULT NULL COMMENT '最大飞行高度(米)，NULL表示全高度禁飞',

    -- 禁飞区状态
    status ENUM('active', 'inactive', 'archived') NOT NULL DEFAULT 'active' COMMENT '禁飞区状态',

    -- 严重程度：critical(严重), high(高), medium(中), low(低)
    severity ENUM('critical', 'high', 'medium', 'low') NOT NULL DEFAULT 'high' COMMENT '严重程度',

    -- 禁飞原因
    reason TEXT COMMENT '禁飞原因',

    -- 时间限制（临时禁飞区必填）
    effective_start_time DATETIME DEFAULT NULL COMMENT '生效开始时间',
    effective_end_time DATETIME DEFAULT NULL COMMENT '生效结束时间',

    -- 创建者信息
    created_by BIGINT NOT NULL COMMENT '创建者用户ID',

    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    -- 外键约束
    FOREIGN KEY (created_by) REFERENCES users(id) ON DELETE CASCADE,

    -- 索引
    INDEX idx_zone_type (zone_type),
    INDEX idx_status (status),
    INDEX idx_severity (severity),
    INDEX idx_created_by (created_by),
    INDEX idx_effective_time (effective_start_time, effective_end_time),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='禁飞区表';

-- ========================================
-- 3. 飞行任务与地理围栏关联表 (可选)
-- ========================================

CREATE TABLE IF NOT EXISTS task_geofence_relations (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '关联ID',
    task_id BIGINT NOT NULL COMMENT '飞行任务ID',
    geofence_id BIGINT NOT NULL COMMENT '地理围栏ID',

    -- 关联类型：planned(计划通过), violated(违反)
    relation_type ENUM('planned', 'violated') NOT NULL DEFAULT 'planned' COMMENT '关联类型',

    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',

    -- 外键约束
    FOREIGN KEY (task_id) REFERENCES api_flight_tasks(id) ON DELETE CASCADE,
    FOREIGN KEY (geofence_id) REFERENCES geofences(id) ON DELETE CASCADE,

    -- 唯一索引，防止重复关联
    UNIQUE INDEX idx_task_geofence (task_id, geofence_id),
    INDEX idx_task_id (task_id),
    INDEX idx_geofence_id (geofence_id),
    INDEX idx_relation_type (relation_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='飞行任务与地理围栏关联表';

-- ========================================
-- 4. 插入示例数据
-- ========================================

-- 插入示例地理围栏（需要先有用户数据）
INSERT INTO geofences (name, description, fence_type, geometry, max_altitude, status, purpose, created_by)
SELECT
    '北京市中心安全区',
    '北京市中心核心区域安全围栏',
    'polygon',
    JSON_OBJECT(
        'points', JSON_ARRAY(
            JSON_OBJECT('lat', 39.904200, 'lng', 116.407396),
            JSON_OBJECT('lat', 39.914200, 'lng', 116.407396),
            JSON_OBJECT('lat', 39.914200, 'lng', 116.417396),
            JSON_OBJECT('lat', 39.904200, 'lng', 116.417396)
        )
    ),
    500.00,
    'active',
    'restricted',
    id
FROM users WHERE username = 'admin' LIMIT 1;

-- 插入示例禁飞区
INSERT INTO no_fly_zones (name, description, zone_type, geometry, max_altitude, status, severity, reason, created_by)
SELECT
    '天安门广场禁飞区',
    '天安门广场及周边区域永久禁飞',
    'permanent',
    JSON_OBJECT(
        'points', JSON_ARRAY(
            JSON_OBJECT('lat', 39.903, 'lng', 116.397),
            JSON_OBJECT('lat', 39.909, 'lng', 116.397),
            JSON_OBJECT('lat', 39.909, 'lng', 116.407),
            JSON_OBJECT('lat', 39.903, 'lng', 116.407)
        )
    ),
    NULL,
    'active',
    'critical',
    '重要政治区域，全面禁飞',
    id
FROM users WHERE username = 'admin' LIMIT 1;

-- ========================================
-- 5. 权限设置
-- ========================================

-- 确保用户有适当的权限访问这些表
-- GRANT SELECT, INSERT, UPDATE, DELETE ON low_altitude_traffic_system.geofences TO 'your_app_user'@'localhost';
-- GRANT SELECT, INSERT, UPDATE, DELETE ON low_altitude_traffic_system.no_fly_zones TO 'your_app_user'@'localhost';
-- GRANT SELECT, INSERT, UPDATE, DELETE ON low_altitude_traffic_system.task_geofence_relations TO 'your_app_user'@'localhost';

-- ========================================
-- 迁移完成
-- ========================================

SELECT 'Migration 002 completed successfully: Geofence and No-Fly Zone tables created' AS status;
