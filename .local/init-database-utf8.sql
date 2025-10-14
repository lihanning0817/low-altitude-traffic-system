-- Local Database Initialization Script (UTF-8 Compatible)
-- This file will not be committed to the repository (.local directory is ignored)

-- Create database
CREATE DATABASE IF NOT EXISTS low_altitude_traffic_system
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE low_altitude_traffic_system;

-- Users table
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

-- Tasks table
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

-- Drones table
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

-- Emergency events table
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

-- Permissions table
CREATE TABLE IF NOT EXISTS permissions (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) UNIQUE NOT NULL,
    description TEXT,
    resource VARCHAR(100) NOT NULL,
    action VARCHAR(50) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_resource (resource)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Roles table
CREATE TABLE IF NOT EXISTS roles (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Role permissions association table
CREATE TABLE IF NOT EXISTS role_permissions (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    role_id BIGINT NOT NULL,
    permission_id BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
    FOREIGN KEY (permission_id) REFERENCES permissions(id) ON DELETE CASCADE,
    UNIQUE KEY uk_role_permission (role_id, permission_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- User roles association table
CREATE TABLE IF NOT EXISTS user_roles (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id BIGINT NOT NULL,
    role_id BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
    UNIQUE KEY uk_user_role (user_id, role_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Insert default roles
INSERT IGNORE INTO roles (name, description) VALUES
('admin', 'System administrator with full permissions'),
('operator', 'Operator can manage tasks and drones'),
('viewer', 'Viewer can only read data');

-- Insert default permissions
INSERT IGNORE INTO permissions (name, description, resource, action) VALUES
('users.read', 'View user information', 'users', 'read'),
('users.create', 'Create user', 'users', 'create'),
('users.update', 'Update user information', 'users', 'update'),
('users.delete', 'Delete user', 'users', 'delete'),
('tasks.read', 'View tasks', 'tasks', 'read'),
('tasks.create', 'Create task', 'tasks', 'create'),
('tasks.update', 'Update task', 'tasks', 'update'),
('tasks.delete', 'Delete task', 'tasks', 'delete'),
('drones.read', 'View drones', 'drones', 'read'),
('drones.create', 'Create drone', 'drones', 'create'),
('drones.update', 'Update drone', 'drones', 'update'),
('drones.delete', 'Delete drone', 'drones', 'delete'),
('emergency.read', 'View emergency events', 'emergency', 'read'),
('emergency.respond', 'Respond to emergency events', 'emergency', 'respond'),
('emergency.resolve', 'Resolve emergency events', 'emergency', 'resolve');

-- Assign all permissions to admin role
INSERT IGNORE INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p WHERE r.name = 'admin';

-- Assign operation permissions to operator role (except user management)
INSERT IGNORE INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p
WHERE r.name = 'operator' AND p.resource IN ('tasks', 'drones', 'emergency');

-- Assign read-only permissions to viewer role
INSERT IGNORE INTO role_permissions (role_id, permission_id)
SELECT r.id, p.id FROM roles r, permissions p
WHERE r.name = 'viewer' AND p.action = 'read';

-- Create default admin user
-- Note: Password hash is placeholder, needs to be updated via backend
INSERT IGNORE INTO users (username, email, password_hash, full_name, role) VALUES
('admin', 'admin@example.com', '$2a$10$placeholder', 'System Administrator', 'admin');

SELECT 'Database initialization completed successfully!' as status;
