-- 用户表
CREATE TABLE users (
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role ENUM('admin', 'user') DEFAULT 'user',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 设备表
CREATE TABLE devices (
    id VARCHAR(50) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    type VARCHAR(50) NOT NULL,
    status VARCHAR(50) DEFAULT 'online',
    `signal` INT DEFAULT 80,
    battery INT DEFAULT 100,
    latitude DECIMAL(10, 8) NOT NULL,
    longitude DECIMAL(11, 8) NOT NULL,
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 飞行任务表
CREATE TABLE flights (
    id VARCHAR(50) PRIMARY KEY,
    device_id VARCHAR(50) NOT NULL,
    start_point JSON NOT NULL,
    end_point JSON NOT NULL,
    path JSON,
    status VARCHAR(50) DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(id)
);

-- 应急事件表
CREATE TABLE emergencies (
    id VARCHAR(50) PRIMARY KEY,
    type VARCHAR(50) NOT NULL,
    severity VARCHAR(50) NOT NULL,
    description TEXT,
    location JSON NOT NULL,
    status VARCHAR(50) DEFAULT 'reported',
    reported_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    resolved_at TIMESTAMP NULL
);

-- 空域表
CREATE TABLE airspaces (
    id VARCHAR(50) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    type VARCHAR(50) NOT NULL,
    boundaries JSON NOT NULL,
    status VARCHAR(50) DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 飞行许可表
CREATE TABLE flight_permits (
    id VARCHAR(50) PRIMARY KEY,
    user_id INT NOT NULL,
    airspace_id VARCHAR(50) NOT NULL,
    flight_id VARCHAR(50) NOT NULL,
    status VARCHAR(50) DEFAULT 'pending',
    application_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    approval_time TIMESTAMP NULL,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (airspace_id) REFERENCES airspaces(id),
    FOREIGN KEY (flight_id) REFERENCES flights(id)
);

-- 气象数据表
CREATE TABLE weather_data (
    id INT PRIMARY KEY AUTO_INCREMENT,
    location JSON NOT NULL,
    temperature DECIMAL(5,2),
    humidity INT,
    wind_speed DECIMAL(5,2),
    wind_direction VARCHAR(50),
    visibility DECIMAL(5,2),
    precipitation DECIMAL(5,2),
    recorded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 紧急降落点表
CREATE TABLE emergency_landing_points (
    id VARCHAR(50) PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    location JSON NOT NULL,
    status VARCHAR(50) DEFAULT 'available',
    capacity INT DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 插入测试数据
-- 用户数据
INSERT INTO users (username, password_hash, role) VALUES 
('admin', '$2b$10$EwB0v5q9Y7Z6X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R', 'admin'),
('user1', '$2b$10$EwB0v5q9Y7Z6X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R', 'user');

-- 设备数据
INSERT INTO devices (id, name, type, status, `signal`, battery, latitude, longitude) VALUES 
('UAV001', '巡逻无人机-01', 'drone', 'cruising', 85, 78, 39.90923000, 116.39742800),
('CAM002', '监控摄像头-02', 'camera', 'normal', 92, 100, 39.91923000, 116.40742800),
('RAD003', '雷达设备-03', 'radar', 'operating', 78, 95, 39.89923000, 116.38742800);
