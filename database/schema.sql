-- 城市智能低空交通系统数据库设计
-- 创建数据库
CREATE DATABASE IF NOT EXISTS low_altitude_traffic_system
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE low_altitude_traffic_system;

-- 1. 用户表
CREATE TABLE users (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE COMMENT '用户名',
    email VARCHAR(100) NOT NULL UNIQUE COMMENT '邮箱',
    password_hash VARCHAR(255) NOT NULL COMMENT '密码哈希',
    full_name VARCHAR(100) NOT NULL COMMENT '真实姓名',
    phone VARCHAR(20) COMMENT '电话号码',
    role ENUM('admin', 'operator', 'user') DEFAULT 'user' COMMENT '用户角色',
    status ENUM('active', 'inactive', 'banned') DEFAULT 'active' COMMENT '用户状态',
    avatar_url VARCHAR(255) COMMENT '头像URL',
    last_login TIMESTAMP NULL COMMENT '最后登录时间',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    INDEX idx_username (username),
    INDEX idx_email (email),
    INDEX idx_role (role),
    INDEX idx_status (status)
) ENGINE=InnoDB COMMENT='用户表';

-- 2. 无人机设备表
CREATE TABLE drones (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    drone_id VARCHAR(50) NOT NULL UNIQUE COMMENT '设备ID',
    name VARCHAR(100) NOT NULL COMMENT '设备名称',
    model VARCHAR(50) NOT NULL COMMENT '设备型号',
    manufacturer VARCHAR(50) COMMENT '制造商',
    owner_id BIGINT NOT NULL COMMENT '所有者ID',
    status ENUM('active', 'maintenance', 'offline', 'retired') DEFAULT 'active' COMMENT '设备状态',
    max_speed DECIMAL(8,2) COMMENT '最大速度(km/h)',
    max_altitude DECIMAL(8,2) COMMENT '最大飞行高度(m)',
    max_range DECIMAL(10,2) COMMENT '最大航程(km)',
    battery_capacity DECIMAL(8,2) COMMENT '电池容量(mAh)',
    weight DECIMAL(8,2) COMMENT '设备重量(kg)',
    payload_capacity DECIMAL(8,2) COMMENT '载荷能力(kg)',
    registration_number VARCHAR(50) COMMENT '注册编号',
    certification_status ENUM('certified', 'pending', 'expired') DEFAULT 'pending' COMMENT '认证状态',
    last_maintenance DATE COMMENT '最后维护日期',
    next_maintenance DATE COMMENT '下次维护日期',
    location_lat DECIMAL(10,6) COMMENT '当前纬度',
    location_lng DECIMAL(10,6) COMMENT '当前经度',
    location_updated_at TIMESTAMP NULL COMMENT '位置更新时间',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_drone_id (drone_id),
    INDEX idx_owner (owner_id),
    INDEX idx_status (status),
    INDEX idx_location (location_lat, location_lng)
) ENGINE=InnoDB COMMENT='无人机设备表';

-- 3. 飞行任务表
CREATE TABLE flight_tasks (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    task_id VARCHAR(50) NOT NULL UNIQUE COMMENT '任务ID',
    user_id BIGINT NOT NULL COMMENT '创建用户ID',
    drone_id BIGINT NULL COMMENT '执行设备ID',
    task_name VARCHAR(100) NOT NULL COMMENT '任务名称',
    description TEXT COMMENT '任务描述',
    task_type ENUM('delivery', 'surveillance', 'inspection', 'emergency', 'transport') NOT NULL COMMENT '任务类型',
    priority ENUM('low', 'medium', 'high', 'urgent') DEFAULT 'medium' COMMENT '优先级',
    status ENUM('pending', 'approved', 'executing', 'completed', 'cancelled', 'failed') DEFAULT 'pending' COMMENT '任务状态',

    -- 起点信息
    start_lat DECIMAL(10,6) NOT NULL COMMENT '起点纬度',
    start_lng DECIMAL(10,6) NOT NULL COMMENT '起点经度',
    start_altitude DECIMAL(8,2) DEFAULT 100 COMMENT '起点高度(m)',
    start_address VARCHAR(255) COMMENT '起点地址',

    -- 终点信息
    end_lat DECIMAL(10,6) NOT NULL COMMENT '终点纬度',
    end_lng DECIMAL(10,6) NOT NULL COMMENT '终点经度',
    end_altitude DECIMAL(8,2) DEFAULT 100 COMMENT '终点高度(m)',
    end_address VARCHAR(255) COMMENT '终点地址',

    -- 飞行参数
    planned_speed DECIMAL(8,2) COMMENT '计划速度(km/h)',
    planned_altitude DECIMAL(8,2) COMMENT '计划飞行高度(m)',
    estimated_duration INT COMMENT '预计时长(分钟)',
    estimated_distance DECIMAL(10,2) COMMENT '预计距离(km)',

    -- 时间安排
    scheduled_start_time TIMESTAMP NULL COMMENT '计划开始时间',
    scheduled_end_time TIMESTAMP NULL COMMENT '计划结束时间',
    actual_start_time TIMESTAMP NULL COMMENT '实际开始时间',
    actual_end_time TIMESTAMP NULL COMMENT '实际结束时间',

    -- 审批信息
    approved_by BIGINT NULL COMMENT '审批人ID',
    approved_at TIMESTAMP NULL COMMENT '审批时间',
    approval_notes TEXT COMMENT '审批备注',

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (drone_id) REFERENCES drones(id) ON DELETE SET NULL,
    FOREIGN KEY (approved_by) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_task_id (task_id),
    INDEX idx_user (user_id),
    INDEX idx_drone (drone_id),
    INDEX idx_status (status),
    INDEX idx_type (task_type),
    INDEX idx_scheduled_time (scheduled_start_time),
    INDEX idx_location_start (start_lat, start_lng),
    INDEX idx_location_end (end_lat, end_lng)
) ENGINE=InnoDB COMMENT='飞行任务表';

-- 4. 路径规划表
CREATE TABLE flight_routes (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    route_id VARCHAR(50) NOT NULL UNIQUE COMMENT '路径ID',
    task_id BIGINT NULL COMMENT '关联任务ID',
    user_id BIGINT NOT NULL COMMENT '创建用户ID',
    route_name VARCHAR(100) COMMENT '路径名称',
    route_type ENUM('shortest', 'fastest', 'safest', 'custom') DEFAULT 'shortest' COMMENT '路径类型',

    -- 起终点
    start_lat DECIMAL(10,6) NOT NULL COMMENT '起点纬度',
    start_lng DECIMAL(10,6) NOT NULL COMMENT '起点经度',
    end_lat DECIMAL(10,6) NOT NULL COMMENT '终点纬度',
    end_lng DECIMAL(10,6) NOT NULL COMMENT '终点经度',

    -- 路径数据
    waypoints JSON COMMENT '路径点数据(JSON格式)',
    total_distance DECIMAL(10,2) COMMENT '总距离(km)',
    estimated_time DECIMAL(8,2) COMMENT '预计时间(分钟)',
    max_altitude DECIMAL(8,2) COMMENT '最大飞行高度(m)',
    min_altitude DECIMAL(8,2) COMMENT '最小飞行高度(m)',

    -- 路径状态
    status ENUM('draft', 'active', 'archived') DEFAULT 'draft' COMMENT '路径状态',
    is_approved BOOLEAN DEFAULT FALSE COMMENT '是否已审批',
    approved_by BIGINT NULL COMMENT '审批人ID',
    approved_at TIMESTAMP NULL COMMENT '审批时间',

    -- 安全信息
    risk_level ENUM('low', 'medium', 'high') DEFAULT 'medium' COMMENT '风险等级',
    safety_notes TEXT COMMENT '安全备注',

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    FOREIGN KEY (task_id) REFERENCES flight_tasks(id) ON DELETE SET NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (approved_by) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_route_id (route_id),
    INDEX idx_task (task_id),
    INDEX idx_user (user_id),
    INDEX idx_status (status),
    INDEX idx_type (route_type),
    INDEX idx_location (start_lat, start_lng, end_lat, end_lng)
) ENGINE=InnoDB COMMENT='飞行路径表';

-- 5. 飞行记录表
CREATE TABLE flight_records (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    record_id VARCHAR(50) NOT NULL UNIQUE COMMENT '记录ID',
    task_id BIGINT NOT NULL COMMENT '任务ID',
    drone_id BIGINT NOT NULL COMMENT '设备ID',
    route_id BIGINT NULL COMMENT '路径ID',

    -- 飞行数据
    start_time TIMESTAMP NOT NULL COMMENT '开始时间',
    end_time TIMESTAMP NULL COMMENT '结束时间',
    duration INT NULL COMMENT '飞行时长(秒)',
    actual_distance DECIMAL(10,2) NULL COMMENT '实际距离(km)',
    max_speed DECIMAL(8,2) NULL COMMENT '最大速度(km/h)',
    avg_speed DECIMAL(8,2) NULL COMMENT '平均速度(km/h)',
    max_altitude DECIMAL(8,2) NULL COMMENT '最大高度(m)',
    avg_altitude DECIMAL(8,2) NULL COMMENT '平均高度(m)',

    -- 状态信息
    flight_status ENUM('in_progress', 'completed', 'aborted', 'emergency_landed') NOT NULL COMMENT '飞行状态',
    battery_start DECIMAL(5,2) COMMENT '起始电量(%)',
    battery_end DECIMAL(5,2) COMMENT '结束电量(%)',
    weather_condition VARCHAR(100) COMMENT '天气条件',

    -- 轨迹数据
    trajectory JSON COMMENT '飞行轨迹(JSON格式)',
    telemetry_data JSON COMMENT '遥测数据(JSON格式)',

    -- 异常信息
    incidents JSON COMMENT '异常事件(JSON格式)',
    emergency_events JSON COMMENT '紧急事件(JSON格式)',

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    FOREIGN KEY (task_id) REFERENCES flight_tasks(id) ON DELETE CASCADE,
    FOREIGN KEY (drone_id) REFERENCES drones(id) ON DELETE CASCADE,
    FOREIGN KEY (route_id) REFERENCES flight_routes(id) ON DELETE SET NULL,
    INDEX idx_record_id (record_id),
    INDEX idx_task (task_id),
    INDEX idx_drone (drone_id),
    INDEX idx_route (route_id),
    INDEX idx_time (start_time, end_time),
    INDEX idx_status (flight_status)
) ENGINE=InnoDB COMMENT='飞行记录表';

-- 6. 空域管理表
CREATE TABLE airspaces (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    airspace_id VARCHAR(50) NOT NULL UNIQUE COMMENT '空域ID',
    name VARCHAR(100) NOT NULL COMMENT '空域名称',
    type ENUM('restricted', 'controlled', 'uncontrolled', 'temporary') NOT NULL COMMENT '空域类型',
    description TEXT COMMENT '空域描述',

    -- 空域边界(简化为矩形区域)
    north_lat DECIMAL(10,6) NOT NULL COMMENT '北纬度',
    south_lat DECIMAL(10,6) NOT NULL COMMENT '南纬度',
    east_lng DECIMAL(10,6) NOT NULL COMMENT '东经度',
    west_lng DECIMAL(10,6) NOT NULL COMMENT '西经度',

    -- 高度范围
    min_altitude DECIMAL(8,2) DEFAULT 0 COMMENT '最小高度(m)',
    max_altitude DECIMAL(8,2) DEFAULT 300 COMMENT '最大高度(m)',

    -- 时间限制
    active_start_time TIME COMMENT '生效开始时间',
    active_end_time TIME COMMENT '生效结束时间',
    active_days VARCHAR(20) COMMENT '生效日期(1234567表示周一到周日)',

    -- 状态信息
    status ENUM('active', 'inactive', 'temporary') DEFAULT 'active' COMMENT '空域状态',
    authority VARCHAR(100) COMMENT '管理机构',
    contact_info VARCHAR(255) COMMENT '联系信息',

    -- 限制信息
    max_concurrent_flights INT DEFAULT 10 COMMENT '最大同时飞行数量',
    required_permissions JSON COMMENT '需要的权限(JSON格式)',
    restrictions TEXT COMMENT '其他限制说明',

    valid_from DATE COMMENT '生效开始日期',
    valid_to DATE COMMENT '生效结束日期',

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    INDEX idx_airspace_id (airspace_id),
    INDEX idx_type (type),
    INDEX idx_status (status),
    INDEX idx_boundary (north_lat, south_lat, east_lng, west_lng),
    INDEX idx_validity (valid_from, valid_to)
) ENGINE=InnoDB COMMENT='空域管理表';

-- 7. 应急事件表
CREATE TABLE emergency_events (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    event_id VARCHAR(50) NOT NULL UNIQUE COMMENT '事件ID',
    task_id BIGINT NULL COMMENT '关联任务ID',
    drone_id BIGINT NULL COMMENT '关联设备ID',
    user_id BIGINT NOT NULL COMMENT '报告人ID',

    -- 事件信息
    event_type ENUM('mechanical_failure', 'weather_emergency', 'collision_risk', 'unauthorized_flight', 'lost_control', 'battery_critical', 'other') NOT NULL COMMENT '事件类型',
    severity ENUM('low', 'medium', 'high', 'critical') NOT NULL COMMENT '严重程度',
    title VARCHAR(200) NOT NULL COMMENT '事件标题',
    description TEXT NOT NULL COMMENT '事件描述',

    -- 位置信息
    location_lat DECIMAL(10,6) COMMENT '事件纬度',
    location_lng DECIMAL(10,6) COMMENT '事件经度',
    location_address VARCHAR(255) COMMENT '事件地址',

    -- 时间信息
    occurred_at TIMESTAMP NOT NULL COMMENT '发生时间',
    reported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '报告时间',
    resolved_at TIMESTAMP NULL COMMENT '解决时间',

    -- 处理信息
    status ENUM('reported', 'investigating', 'responding', 'resolved', 'closed') DEFAULT 'reported' COMMENT '处理状态',
    assigned_to BIGINT NULL COMMENT '分配处理人ID',
    response_team VARCHAR(100) COMMENT '响应团队',

    -- 处理结果
    resolution TEXT COMMENT '解决方案',
    lessons_learned TEXT COMMENT '经验教训',
    follow_up_actions TEXT COMMENT '后续行动',

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    FOREIGN KEY (task_id) REFERENCES flight_tasks(id) ON DELETE SET NULL,
    FOREIGN KEY (drone_id) REFERENCES drones(id) ON DELETE SET NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (assigned_to) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_event_id (event_id),
    INDEX idx_task (task_id),
    INDEX idx_drone (drone_id),
    INDEX idx_user (user_id),
    INDEX idx_type (event_type),
    INDEX idx_severity (severity),
    INDEX idx_status (status),
    INDEX idx_occurred_time (occurred_at),
    INDEX idx_location (location_lat, location_lng)
) ENGINE=InnoDB COMMENT='应急事件表';

-- 8. 系统日志表
CREATE TABLE system_logs (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    log_id VARCHAR(50) NOT NULL UNIQUE COMMENT '日志ID',
    user_id BIGINT NULL COMMENT '用户ID',
    session_id VARCHAR(100) COMMENT '会话ID',

    -- 日志信息
    level ENUM('debug', 'info', 'warning', 'error', 'critical') NOT NULL COMMENT '日志级别',
    category VARCHAR(50) NOT NULL COMMENT '日志分类',
    action VARCHAR(100) NOT NULL COMMENT '操作动作',
    message TEXT NOT NULL COMMENT '日志消息',

    -- 请求信息
    request_method VARCHAR(10) COMMENT '请求方法',
    request_url VARCHAR(500) COMMENT '请求URL',
    request_ip VARCHAR(45) COMMENT '请求IP',
    user_agent TEXT COMMENT '用户代理',

    -- 响应信息
    response_status INT COMMENT '响应状态码',
    response_time INT COMMENT '响应时间(ms)',

    -- 额外数据
    metadata JSON COMMENT '元数据(JSON格式)',
    stack_trace TEXT COMMENT '错误堆栈',

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',

    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_log_id (log_id),
    INDEX idx_user (user_id),
    INDEX idx_level (level),
    INDEX idx_category (category),
    INDEX idx_action (action),
    INDEX idx_created_time (created_at),
    INDEX idx_request_info (request_method, response_status)
) ENGINE=InnoDB COMMENT='系统日志表';

-- 9. 系统配置表
CREATE TABLE system_config (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    config_key VARCHAR(100) NOT NULL UNIQUE COMMENT '配置键',
    config_value TEXT COMMENT '配置值',
    config_type ENUM('string', 'number', 'boolean', 'json') DEFAULT 'string' COMMENT '配置类型',
    category VARCHAR(50) DEFAULT 'general' COMMENT '配置分类',
    description TEXT COMMENT '配置描述',
    is_public BOOLEAN DEFAULT FALSE COMMENT '是否公开配置',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    INDEX idx_key (config_key),
    INDEX idx_category (category),
    INDEX idx_public (is_public)
) ENGINE=InnoDB COMMENT='系统配置表';

-- 10. 地图标记点表
CREATE TABLE map_markers (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    lng DECIMAL(10,6) NOT NULL COMMENT '经度',
    lat DECIMAL(10,6) NOT NULL COMMENT '纬度',
    title VARCHAR(255) NOT NULL COMMENT '标记点标题',
    type ENUM('marker', 'start', 'end', 'waypoint') DEFAULT 'marker' COMMENT '标记点类型',
    description TEXT COMMENT '标记点描述',
    user_id BIGINT NULL COMMENT '创建用户ID',
    task_id BIGINT NULL COMMENT '关联任务ID',
    is_active BOOLEAN DEFAULT TRUE COMMENT '是否激活',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL,
    FOREIGN KEY (task_id) REFERENCES flight_tasks(id) ON DELETE SET NULL,
    INDEX idx_location (lng, lat),
    INDEX idx_type (type),
    INDEX idx_user (user_id),
    INDEX idx_task (task_id),
    INDEX idx_active (is_active),
    INDEX idx_created_time (created_at)
) ENGINE=InnoDB COMMENT='地图标记点表';

-- 11. 路径线段表
CREATE TABLE map_routes (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    route_name VARCHAR(255) NOT NULL COMMENT '路径名称',
    start_lng DECIMAL(10,6) NOT NULL COMMENT '起点经度',
    start_lat DECIMAL(10,6) NOT NULL COMMENT '起点纬度',
    end_lng DECIMAL(10,6) NOT NULL COMMENT '终点经度',
    end_lat DECIMAL(10,6) NOT NULL COMMENT '终点纬度',
    path_coordinates JSON COMMENT '路径坐标数组',
    distance DECIMAL(10,2) COMMENT '路径距离(km)',
    duration DECIMAL(8,2) COMMENT '预计时长(分钟)',
    route_type ENUM('driving', 'walking', 'flight') DEFAULT 'flight' COMMENT '路径类型',
    user_id BIGINT NULL COMMENT '创建用户ID',
    task_id BIGINT NULL COMMENT '关联任务ID',
    is_active BOOLEAN DEFAULT TRUE COMMENT '是否激活',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL,
    FOREIGN KEY (task_id) REFERENCES flight_tasks(id) ON DELETE SET NULL,
    INDEX idx_start_location (start_lng, start_lat),
    INDEX idx_end_location (end_lng, end_lat),
    INDEX idx_type (route_type),
    INDEX idx_user (user_id),
    INDEX idx_task (task_id),
    INDEX idx_active (is_active),
    INDEX idx_created_time (created_at)
) ENGINE=InnoDB COMMENT='地图路径表';

-- 插入初始管理员用户（密码：admin123）
INSERT INTO users (username, email, password_hash, full_name, role, status)
VALUES ('admin', 'admin@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewKZDKlMHe8w8Hjy', '系统管理员', 'admin', 'active');

-- 插入示例用户
INSERT INTO users (username, email, password_hash, full_name, role, status) VALUES
('user1', 'user1@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewKZDKlMHe8w8Hjy', '普通用户1', 'user', 'active'),
('operator1', 'operator1@example.com', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewKZDKlMHe8w8Hjy', '操作员1', 'operator', 'active');

-- 插入示例无人机
INSERT INTO drones (drone_id, name, model, manufacturer, owner_id, status, max_speed, max_altitude, max_range, location_lat, location_lng) VALUES
('DJI-001', 'Phantom 4 Pro', 'Phantom 4 Pro', 'DJI', 1, 'active', 72, 300, 30, 39.904200, 116.407396),
('DJI-002', 'Mavic 3', 'Mavic 3', 'DJI', 1, 'active', 68, 300, 46, 39.914200, 116.417396),
('XF-001', 'Wing Loong II', 'Wing Loong II', '翔丰', 1, 'maintenance', 280, 9000, 4000, 39.924200, 116.427396);

-- 插入初始系统配置
INSERT INTO system_config (config_key, config_value, config_type, category, description, is_public) VALUES
('system.name', '城市智能低空交通系统', 'string', 'system', '系统名称', true),
('system.version', '1.0.0', 'string', 'system', '系统版本', true),
('flight.max_altitude', '300', 'number', 'flight', '最大飞行高度(米)', true),
('flight.min_altitude', '50', 'number', 'flight', '最小飞行高度(米)', true),
('flight.max_speed', '80', 'number', 'flight', '最大飞行速度(km/h)', true),
('security.session_timeout', '3600', 'number', 'security', '会话超时时间(秒)', false),
('security.password_min_length', '8', 'number', 'security', '密码最小长度', false);