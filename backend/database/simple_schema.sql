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
