-- 创建MySQL X Protocol用户脚本
-- 请使用MySQL客户端或MySQL Workbench执行此脚本

-- 1. 创建新用户用于X Protocol (使用caching_sha2_password认证)
CREATE USER IF NOT EXISTS 'xprotocol_user'@'localhost' IDENTIFIED WITH caching_sha2_password BY 'Aabc123456+';

-- 2. 授予该用户对low_altitude_traffic_system数据库的所有权限
GRANT ALL PRIVILEGES ON low_altitude_traffic_system.* TO 'xprotocol_user'@'localhost';

-- 3. 刷新权限
FLUSH PRIVILEGES;

-- 4. 验证用户创建成功
SELECT
    user,
    host,
    plugin as authentication_plugin,
    password_expired
FROM mysql.user
WHERE user IN ('root', 'xprotocol_user')
ORDER BY user;

-- 5. 显示用户权限
SHOW GRANTS FOR 'xprotocol_user'@'localhost';
