-- 本地数据库初始化脚本
-- 此文件不会被提交到仓库（.gitignore 中已排除 .local 目录）

-- 创建数据库
CREATE DATABASE IF NOT EXISTS low_altitude_traffic_system
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE low_altitude_traffic_system;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    full_name VARCHAR(100),
    phone VARCHAR(20),
    role VARCHAR(20) DEFAULT 'user',
    status VARCHAR(20) DEFAULT 'active',
    failed_login_attempts INT DEFAULT 0,
    locked_until TIMESTAMP NULL,
    last_login TIMESTAMP NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_username (username),
    INDEX idx_email (email),
    INDEX idx_role (role)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 任务表
CREATE TABLE IF NOT EXISTS tasks (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    task_code VARCHAR(50) UNIQUE NOT NULL,
    title VARCHAR(200) NOT NULL,
    description TEXT,
    status VARCHAR(20) DEFAULT 'pending',
    priority VARCHAR(20) DEFAULT 'medium',
    user_id BIGINT NOT NULL,
    route JSON,
    start_location JSON,
    end_location JSON,
    estimated_duration INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    scheduled_time TIMESTAMP NULL,
    completed_time TIMESTAMP NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_task_code (task_code),
    INDEX idx_status (status),
    INDEX idx_user_id (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 无人机表
CREATE TABLE IF NOT EXISTS drones (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    drone_code VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    model VARCHAR(50),
    status VARCHAR(20) DEFAULT 'idle',
    battery_level DECIMAL(5,2) DEFAULT 100.00,
    current_location JSON,
    max_speed DECIMAL(10,2),
    max_altitude DECIMAL(10,2),
    max_range DECIMAL(10,2),
    payload_capacity DECIMAL(10,2),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    last_maintenance TIMESTAMP NULL,
    INDEX idx_drone_code (drone_code),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 应急事件表
CREATE TABLE IF NOT EXISTS emergency_events (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    event_code VARCHAR(50) UNIQUE NOT NULL,
    task_id BIGINT NOT NULL,
    drone_id BIGINT,
    type VARCHAR(50) NOT NULL,
    severity VARCHAR(20) DEFAULT 'medium',
    status VARCHAR(20) DEFAULT 'active',
    title VARCHAR(200) NOT NULL,
    description TEXT,
    location JSON,
    response_action TEXT,
    response_notes TEXT,
    responded_by BIGINT,
    responded_at TIMESTAMP NULL,
    resolved_at TIMESTAMP NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE,
    FOREIGN KEY (drone_id) REFERENCES drones(id) ON DELETE SET NULL,
    FOREIGN KEY (responded_by) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_event_code (event_code),
    INDEX idx_task_id (task_id),
    INDEX idx_status (status),
    INDEX idx_severity (severity)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 权限表
CREATE TABLE IF NOT EXISTS permissions (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) UNIQUE NOT NULL,
    description TEXT,
    resource VARCHAR(100) NOT NULL,
    action VARCHAR(50) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_resource (resource)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 角色表
CREATE TABLE IF NOT EXISTS roles (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 角色-权限关联表
CREATE TABLE IF NOT EXISTS role_permissions (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    role_id BIGINT NOT NULL,
    permission_id BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
    FOREIGN KEY (permission_id) REFERENCES permissions(id) ON DELETE CASCADE,
    UNIQUE KEY uk_role_permission (role_id, permission_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 用户-角色关联表
CREATE TABLE IF NOT EXISTS user_roles (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id BIGINT NOT NULL,
    role_id BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
    UNIQUE KEY uk_user_role (user_id, role_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 插入默认角色
INSERT IGNORE INTO roles (name, description) VALUES
('admin', '系统管理员，拥有所有权限'),
('operator', '操作员，可以管理任务和无人机'),
('viewer', '查看者，只能查看数据');

-- 插入默认权限
INSERT IGNORE INTO permissions (name, description, resource, action) VALUES
('users.read', '查看用户信息', 'users', 'read'),
('users.create', '创建用户', 'users', 'create'),
('users.update', '更新用户信息', 'users', 'update'),
('users.delete', '删除用户', 'users', 'delete'),
('tasks.read', '查看任务', 'tasks', 'read'),
('tasks.create', '创建任务', 'tasks', 'create'),
('tasks.update', '更新任务', 'tasks', 'update'),
('tasks.delete', '删除任务', 'tasks', 'delete'),
('drones.read', '查看无人机', 'drones', 'read'),
('drones.create', '创建无人机', 'drones', 'create'),
('drones.update', '更新无人机', 'drones', 'update'),
('drones.delete', '删除无人机', 'drones', 'delete'),
('emergency.read', '查看应急事件', 'emergency', 'read'),
('emergency.respond', '响应应急事件', 'emergency', 'respond'),
('emergency.resolve', '解决应急事件', 'emergency', 'resolve');

-- 为 admin 角色分配所有权限
INSERT IGNORE INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p WHERE r.name = 'admin';

-- 为 operator 角色分配操作权限（除了用户管理）
INSERT IGNORE INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p
WHERE r.name = 'operator' AND p.resource IN ('tasks', 'drones', 'emergency');

-- 为 viewer 角色分配只读权限
INSERT IGNORE INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p
WHERE r.name = 'viewer' AND p.action = 'read';

-- 创建默认管理员用户（密码：admin123，需要在后端加密后更新）
-- 注意：这个密码哈希需要通过后端的密码加密算法生成
-- 临时使用明文，首次登录后请立即修改
INSERT IGNORE INTO users (username, email, password_hash, full_name, role) VALUES
('admin', 'admin@example.com', '$2a$10$placeholder', '系统管理员', 'admin');

SELECT 'Database initialization completed!' as status;
