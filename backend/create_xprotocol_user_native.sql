-- 创建MySQL X Protocol用户脚本 (使用mysql_native_password)
-- 请使用MySQL客户端或MySQL Workbench执行此脚本

-- 1. 删除旧用户(如果存在)
DROP USER IF EXISTS 'xprotocol_user'@'localhost';

-- 2. 创建新用户用于X Protocol (使用mysql_native_password认证)
CREATE USER 'xprotocol_user'@'localhost' IDENTIFIED WITH mysql_native_password BY 'Aabc123456+';

-- 3. 授予该用户对low_altitude_traffic_system数据库的所有权限
GRANT ALL PRIVILEGES ON low_altitude_traffic_system.* TO 'xprotocol_user'@'localhost';

-- 4. 刷新权限
FLUSH PRIVILEGES;

-- 5. 验证用户创建成功
SELECT
    user,
    host,
    plugin as authentication_plugin,
    password_expired
FROM mysql.user
WHERE user IN ('root', 'xprotocol_user')
ORDER BY user;

-- 6. 显示用户权限
SHOW GRANTS FOR 'xprotocol_user'@'localhost';
