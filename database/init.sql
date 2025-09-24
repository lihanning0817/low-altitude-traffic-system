-- ===================================
-- 低空交通管理系统数据库初始化脚本
-- 版本: 1.0
-- 创建时间: 2024-09-24
-- 说明: 本脚本用于初始化项目的核心数据库表结构
-- ===================================

-- 创建数据库（可选，如果数据库不存在）
-- CREATE DATABASE IF NOT EXISTS low_altitude_traffic_system
-- CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 使用数据库
-- USE low_altitude_traffic_system;

-- ===================================
-- 用户表 (users)
-- ===================================
DROP TABLE IF EXISTS users;

CREATE TABLE users (
    id INT PRIMARY KEY AUTO_INCREMENT COMMENT '用户ID',
    username VARCHAR(50) NOT NULL UNIQUE COMMENT '用户名',
    password_hash VARCHAR(255) NOT NULL COMMENT '密码哈希',
    email VARCHAR(100) UNIQUE COMMENT '邮箱',
    phone VARCHAR(20) COMMENT '电话',
    role ENUM('admin', 'pilot', 'controller', 'viewer') DEFAULT 'viewer' COMMENT '用户角色',
    status ENUM('active', 'inactive', 'suspended') DEFAULT 'active' COMMENT '用户状态',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    last_login TIMESTAMP NULL COMMENT '最后登录时间',
    INDEX idx_username (username),
    INDEX idx_email (email),
    INDEX idx_role (role)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- ===================================
-- 飞行任务表 (flight_tasks)
-- ===================================
DROP TABLE IF EXISTS flight_tasks;

CREATE TABLE flight_tasks (
    id INT PRIMARY KEY AUTO_INCREMENT COMMENT '任务ID',
    task_name VARCHAR(100) NOT NULL COMMENT '任务名称',
    pilot_id INT NOT NULL COMMENT '飞行员ID',
    aircraft_id VARCHAR(50) COMMENT '飞机标识',
    task_type ENUM('survey', 'transport', 'patrol', 'emergency', 'training') NOT NULL COMMENT '任务类型',
    priority ENUM('low', 'medium', 'high', 'critical') DEFAULT 'medium' COMMENT '优先级',
    status ENUM('pending', 'approved', 'executing', 'completed', 'cancelled', 'failed') DEFAULT 'pending' COMMENT '任务状态',
    planned_start_time DATETIME NOT NULL COMMENT '计划开始时间',
    planned_end_time DATETIME NOT NULL COMMENT '计划结束时间',
    actual_start_time DATETIME NULL COMMENT '实际开始时间',
    actual_end_time DATETIME NULL COMMENT '实际结束时间',
    departure_location VARCHAR(200) NOT NULL COMMENT '出发地点',
    destination_location VARCHAR(200) NOT NULL COMMENT '目的地',
    flight_altitude INT COMMENT '飞行高度(米)',
    flight_speed DECIMAL(8,2) COMMENT '飞行速度(km/h)',
    route_data JSON COMMENT '航线数据(JSON格式)',
    weather_conditions TEXT COMMENT '天气条件',
    notes TEXT COMMENT '备注信息',
    created_by INT NOT NULL COMMENT '创建人ID',
    approved_by INT NULL COMMENT '批准人ID',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    FOREIGN KEY (pilot_id) REFERENCES users(id) ON DELETE RESTRICT,
    FOREIGN KEY (created_by) REFERENCES users(id) ON DELETE RESTRICT,
    FOREIGN KEY (approved_by) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_pilot_id (pilot_id),
    INDEX idx_status (status),
    INDEX idx_task_type (task_type),
    INDEX idx_planned_start (planned_start_time),
    INDEX idx_created_by (created_by)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='飞行任务表';

-- ===================================
-- 飞行日志表 (flight_logs)
-- ===================================
DROP TABLE IF EXISTS flight_logs;

CREATE TABLE flight_logs (
    id INT PRIMARY KEY AUTO_INCREMENT COMMENT '日志ID',
    task_id INT NOT NULL COMMENT '关联任务ID',
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '记录时间',
    latitude DECIMAL(10, 8) COMMENT '纬度',
    longitude DECIMAL(11, 8) COMMENT '经度',
    altitude DECIMAL(8, 2) COMMENT '高度(米)',
    speed DECIMAL(8, 2) COMMENT '速度(km/h)',
    heading DECIMAL(5, 2) COMMENT '航向角(度)',
    battery_level DECIMAL(5, 2) COMMENT '电池电量(%)',
    signal_strength INT COMMENT '信号强度',
    status_message TEXT COMMENT '状态信息',
    FOREIGN KEY (task_id) REFERENCES flight_tasks(id) ON DELETE CASCADE,
    INDEX idx_task_id (task_id),
    INDEX idx_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='飞行日志表';

-- ===================================
-- 系统配置表 (system_config)
-- ===================================
DROP TABLE IF EXISTS system_config;

CREATE TABLE system_config (
    id INT PRIMARY KEY AUTO_INCREMENT COMMENT '配置ID',
    config_key VARCHAR(100) NOT NULL UNIQUE COMMENT '配置键',
    config_value TEXT COMMENT '配置值',
    description TEXT COMMENT '配置描述',
    data_type ENUM('string', 'number', 'boolean', 'json') DEFAULT 'string' COMMENT '数据类型',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_config_key (config_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='系统配置表';

-- ===================================
-- 插入默认管理员用户（可选）
-- 注意: 在生产环境中应该删除或修改默认密码
-- ===================================
-- INSERT INTO users (username, password_hash, email, role) VALUES
-- ('admin', '$2b$10$example_hash_here', 'admin@example.com', 'admin');

-- ===================================
-- 插入默认系统配置（可选）
-- ===================================
-- INSERT INTO system_config (config_key, config_value, description, data_type) VALUES
-- ('max_flight_altitude', '500', '最大飞行高度限制(米)', 'number'),
-- ('max_concurrent_flights', '10', '最大并发飞行任务数', 'number'),
-- ('default_flight_duration', '120', '默认飞行时长(分钟)', 'number');

-- ===================================
-- 初始化完成
-- ===================================
-- 脚本执行完成，数据库表结构初始化成功