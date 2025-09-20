-- MySQL 8.x Database Schema

-- Users table
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    phone VARCHAR(20) UNIQUE NOT NULL,
    role ENUM('user', 'admin') DEFAULT 'user',
    vip_flag BOOLEAN DEFAULT FALSE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Drones table
CREATE TABLE drones (
    id INT AUTO_INCREMENT PRIMARY KEY,
    model VARCHAR(100) NOT NULL,
    status ENUM('available', 'busy', 'maintenance', 'offline') DEFAULT 'available',
    battery DECIMAL(5,2) DEFAULT 100.00,
    latitude DECIMAL(10, 8) NOT NULL,
    longitude DECIMAL(11, 8) NOT NULL,
    last_update TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Tasks table
CREATE TABLE tasks (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    type ENUM('logistics', 'agriculture', 'inspection', 'rescue') NOT NULL,
    status ENUM('pending', 'assigned', 'in_progress', 'completed', 'cancelled') DEFAULT 'pending',
    start_lat DECIMAL(10, 8) NOT NULL,
    start_lng DECIMAL(11, 8) NOT NULL,
    end_lat DECIMAL(10, 8) NOT NULL,
    end_lng DECIMAL(11, 8) NOT NULL,
    assigned_drone_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (assigned_drone_id) REFERENCES drones(id) ON DELETE SET NULL
);

-- Flight logs table
CREATE TABLE flight_logs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    drone_id INT NOT NULL,
    task_id INT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    latitude DECIMAL(10, 8) NOT NULL,
    longitude DECIMAL(11, 8) NOT NULL,
    event VARCHAR(255) NOT NULL,
    FOREIGN KEY (drone_id) REFERENCES drones(id) ON DELETE CASCADE,
    FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE SET NULL
);

-- Payments table
CREATE TABLE payments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    task_id INT NOT NULL,
    amount DECIMAL(10, 2) NOT NULL,
    status ENUM('pending', 'completed', 'failed', 'refunded') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE
);

-- Indexes for optimization
CREATE INDEX idx_users_phone ON users(phone);
CREATE INDEX idx_drones_status ON drones(status);
CREATE INDEX idx_tasks_user_id ON tasks(user_id);
CREATE INDEX idx_tasks_status ON tasks(status);
CREATE INDEX idx_tasks_assigned_drone_id ON tasks(assigned_drone_id);
CREATE INDEX idx_flight_logs_drone_id ON flight_logs(drone_id);
CREATE INDEX idx_flight_logs_task_id ON flight_logs(task_id);
CREATE INDEX idx_payments_user_id ON payments(user_id);
CREATE INDEX idx_payments_task_id ON payments(task_id);
CREATE INDEX idx_payments_status ON payments(status);