# 数据库迁移执行指南

本文档说明如何应用数据库迁移脚本以修复Critical Bug #1。

## 前提条件

1. MySQL 8.0 已安装（位置：`D:\mysql-8.0.43-winx64`）
2. 数据库 `low_altitude_traffic_system` 已创建
3. 拥有数据库管理员权限

## 步骤1：启动MySQL服务器

### 方法A：使用mysqld直接启动（推荐用于开发环境）

打开命令提示符（管理员权限），执行：

```bash
cd D:\mysql-8.0.43-winx64\bin
mysqld --console
```

保持此窗口打开，MySQL将在前台运行。

### 方法B：安装并启动MySQL服务（推荐用于生产环境）

```bash
# 以管理员身份运行
cd D:\mysql-8.0.43-winx64\bin

# 安装MySQL服务
mysqld --install MySQL80

# 启动服务
net start MySQL80
```

## 步骤2：验证数据库连接

打开新的命令提示符窗口：

```bash
D:\mysql-8.0.43-winx64\bin\mysql.exe -u root -p
```

输入密码：`Aabc123456+`

成功连接后，检查数据库：

```sql
SHOW DATABASES;
USE low_altitude_traffic_system;
SHOW TABLES;
```

## 步骤3：备份现有emergency_events表（如果存在）

⚠️ **重要：在应用迁移前先备份！**

```sql
USE low_altitude_traffic_system;

-- 检查表是否存在
SHOW TABLES LIKE 'emergency_events';

-- 如果存在，创建备份
CREATE TABLE emergency_events_backup AS SELECT * FROM emergency_events;

-- 验证备份
SELECT COUNT(*) FROM emergency_events_backup;
```

## 步骤4：应用迁移脚本

### 方法A：通过命令行执行

```bash
cd D:\low-altitude-traffic-system\database

D:\mysql-8.0.43-winx64\bin\mysql.exe -u root -pAabc123456+ low_altitude_traffic_system < migration_emergency_events_fix.sql
```

### 方法B：在MySQL客户端执行

```sql
USE low_altitude_traffic_system;
SOURCE D:/low-altitude-traffic-system/database/migration_emergency_events_fix.sql;
```

## 步骤5：验证迁移结果

```sql
USE low_altitude_traffic_system;

-- 查看新表结构
DESCRIBE emergency_events;

-- 验证字段是否正确
SELECT
    COLUMN_NAME,
    COLUMN_TYPE,
    IS_NULLABLE,
    COLUMN_KEY
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'low_altitude_traffic_system'
  AND TABLE_NAME = 'emergency_events'
ORDER BY ORDINAL_POSITION;

-- 检查关键字段
SELECT
    COLUMN_NAME
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = 'low_altitude_traffic_system'
  AND TABLE_NAME = 'emergency_events'
  AND COLUMN_NAME IN ('event_code', 'task_id', 'location');
```

**预期结果：**
- `event_code` 字段存在（VARCHAR(50), UNIQUE）
- `task_id` 字段存在（BIGINT）
- `location` 字段存在（JSON类型）
- 不应存在 `event_id`, `user_id`, `location_lat`, `location_lng` 字段

## 步骤6：恢复历史数据（可选）

如果备份表中有重要数据，需要转换后插入新表：

```sql
USE low_altitude_traffic_system;

-- 查看备份数据
SELECT * FROM emergency_events_backup LIMIT 5;

-- 转换并插入数据（根据实际字段调整）
INSERT INTO emergency_events
    (event_code, task_id, drone_id, type, severity, status, title, description, location, created_at)
SELECT
    CONCAT('EMG-', DATE_FORMAT(created_at, '%Y%m%d'), '-', LPAD(id, 4, '0')) as event_code,
    task_id,
    drone_id,
    type,
    severity,
    status,
    title,
    description,
    JSON_OBJECT('lat', location_lat, 'lon', location_lng) as location,
    created_at
FROM emergency_events_backup
WHERE location_lat IS NOT NULL AND location_lng IS NOT NULL;

-- 验证数据
SELECT COUNT(*) FROM emergency_events;
SELECT * FROM emergency_events LIMIT 3;
```

## 步骤7：清理备份表（可选）

确认数据迁移成功后，可以删除备份表：

```sql
-- 再次确认数据正确
SELECT COUNT(*) FROM emergency_events;

-- 删除备份
DROP TABLE emergency_events_backup;
```

## 故障排查

### 问题1：无法连接MySQL

**错误信息：**
```
ERROR 2003 (HY000): Can't connect to MySQL server on 'localhost:3306' (10061)
```

**解决方案：**
1. 确认MySQL服务已启动：`mysqld --console`
2. 检查防火墙设置
3. 验证端口3306未被占用：`netstat -an | findstr 3306`

### 问题2：外键约束失败

**错误信息：**
```
ERROR 1215 (HY000): Cannot add foreign key constraint
```

**解决方案：**
1. 确保父表存在：
   ```sql
   SHOW TABLES LIKE 'flight_tasks';
   SHOW TABLES LIKE 'drones';
   SHOW TABLES LIKE 'users';
   ```
2. 如果父表不存在，先创建父表或临时注释掉外键约束
3. 确认父表主键类型与外键类型匹配（都应为BIGINT）

### 问题3：字符集问题

**错误信息：**
```
ERROR 1273 (HY000): Unknown collation: 'utf8mb4_unicode_ci'
```

**解决方案：**
```sql
-- 检查可用的字符集
SHOW COLLATION WHERE Charset = 'utf8mb4';

-- 使用通用字符集
ALTER DATABASE low_altitude_traffic_system CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
```

## 验收标准

迁移成功的标志：

✅ emergency_events表结构符合backend/src/repositories/EmergencyEventRepository.cpp要求
✅ 包含event_code字段（而非event_id）
✅ 包含task_id字段（而非user_id）
✅ location字段为JSON类型（而非location_lat/location_lng分离字段）
✅ 所有ENUM类型正确定义
✅ 外键约束已建立（如果父表存在）
✅ 索引已创建

## 下一步

迁移完成后，请：

1. 重启后端服务器
2. 测试应急事件相关API：
   - POST /api/v1/emergency/events（创建事件）
   - GET /api/v1/emergency/events（获取事件列表）
   - GET /api/v1/emergency/events/:id（获取单个事件）
3. 检查后端日志确认无数据库错误

## 联系支持

如遇到问题，请检查：
- 后端日志：`backend/logs/server.log`
- MySQL错误日志：`D:\mysql-8.0.43-winx64\data\*.err`
