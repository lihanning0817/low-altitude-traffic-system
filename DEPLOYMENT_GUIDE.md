# 系统监控模块部署指南

## 系统要求

### 后端要求
- Windows 10/11 或 Windows Server 2019+
- Visual Studio 2019/2022 或 CMake 3.20+
- MySQL 8.0+ 或 MariaDB 10.6+
- vcpkg 包管理器
- 已安装的依赖库：
  - Boost 1.82+
  - MySQL X DevAPI
  - nlohmann/json
  - spdlog

### 前端要求
- Node.js 16+
- Vue 3.x
- 现代浏览器支持

## 快速部署步骤

### 1. 数据库准备

#### 创建数据库
```sql
-- 连接到 MySQL
mysql -u root -p

-- 创建数据库
CREATE DATABASE IF NOT EXISTS low_altitude_traffic_system
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 导入数据库结构
USE low_altitude_traffic_system;
SOURCE database/schema.sql;
```

#### 验证数据库
```sql
-- 检查表是否创建成功
SHOW TABLES;

-- 检查默认用户
SELECT username, role, status FROM users;
```

### 2. 编译后端

#### 方法一：使用 Visual Studio
```bash
# 打开解决方案文件
start backend/low_altitude_traffic_system_backend.sln

# 在 Visual Studio 中：
# 1. 选择 Release 配置
# 2. 选择 x64 平台
# 3. 生成解决方案 (Ctrl+Shift+B)
```

#### 方法二：使用 CMake
```bash
cd backend
cmake --build . --config Release
```

### 3. 配置服务器

#### 创建配置文件
```bash
# 复制配置模板
copy config\server.json.template config\server.json

# 编辑配置文件
notepad config\server.json
```

#### 配置文件示例 (config/server.json)
```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8081,
    "threads": 4
  },
  "database": {
    "host": "localhost",
    "port": 33060,
    "database": "low_altitude_traffic_system",
    "username": "root",
    "password": "your_password",
    "connection_timeout": 10,
    "use_ssl": false
  },
  "system": {
    "name": "城市智能低空交通系统",
    "version": "1.0.0",
    "environment": "production"
  },
  "logging": {
    "level": "info",
    "file": "logs/server.log"
  },
  "jwt": {
    "secret_key": "your-very-secure-secret-key-here",
    "issuer": "low-altitude-traffic-system",
    "access_token_expiry": 900,
    "refresh_token_expiry": 604800
  },
  "external_apis": {
    "amap": {
      "key": "your-amap-api-key"
    }
  }
}
```

### 4. 启动服务器

#### 创建启动脚本 (start_server.bat)
```batch
@echo off
title 低空交通系统后端服务
echo 正在启动低空交通系统后端服务...

cd /d %~dp0backend
if not exist logs mkdir logs

echo 检查配置文件...
if not exist config\server.json (
    echo 错误: 配置文件 config\server.json 不存在
    echo 请先复制并编辑 config\server.json.template
    pause
    exit /b 1
)

echo 启动服务器...
Release\low_altitude_traffic_system_backend.exe config\server.json

if errorlevel 1 (
    echo 服务器启动失败，错误代码: %errorlevel%
    pause
)
```

#### 运行服务器
```bash
# 方法一：使用启动脚本
start_server.bat

# 方法二：直接运行
cd backend
Release\low_altitude_traffic_system_backend.exe config\server.json
```

### 5. 验证部署

#### 使用测试脚本
```bash
# PowerShell 测试
powershell -ExecutionPolicy Bypass -File test_system_monitor.ps1

# 或使用批处理
test_system_monitor.bat
```

#### 手动验证
```bash
# 健康检查
curl http://localhost:8081/api/v1/health

# 系统信息
curl http://localhost:8081/api/v1/info
```

### 6. 前端集成

#### 安装前端依赖
```bash
cd frontend  # 您的 Vue 项目目录
npm install
```

#### 配置 API 基地址
```javascript
// .env.development
VITE_API_BASE_URL=http://localhost:8081/api/v1

// .env.production
VITE_API_BASE_URL=https://your-domain.com/api/v1
```

#### 集成系统监控组件
```javascript
// main.js
import { systemMonitorApi } from './services/systemMonitorApi'

// 全局注册
app.config.globalProperties.$systemMonitor = systemMonitorApi
```

## 生产环境部署

### 1. 安全配置

#### 配置 HTTPS
```json
{
  "server": {
    "use_ssl": true,
    "ssl_cert_file": "ssl/server.crt",
    "ssl_key_file": "ssl/server.key"
  }
}
```

#### 配置防火墙
```bash
# Windows 防火墙设置
netsh advfirewall firewall add rule name="Low Altitude Traffic System" dir=in action=allow protocol=TCP localport=8081
```

### 2. 性能优化

#### 数据库连接池
```json
{
  "database": {
    "max_connections": 20,
    "connection_pool_size": 10
  }
}
```

#### 日志轮转
```json
{
  "logging": {
    "max_file_size": "10MB",
    "max_files": 5
  }
}
```

### 3. 监控和维护

#### 服务监控脚本 (monitor_service.bat)
```batch
@echo off
:check
ping localhost -n 1 >nul
curl -s http://localhost:8081/api/v1/health >nul
if errorlevel 1 (
    echo %date% %time% - 服务异常，正在重启...
    taskkill /f /im low_altitude_traffic_system_backend.exe >nul 2>&1
    timeout /t 5 /nobreak >nul
    start /b call start_server.bat
)
timeout /t 30 /nobreak >nul
goto check
```

#### 日志清理脚本 (cleanup_logs.bat)
```batch
@echo off
forfiles /p backend\logs /s /m *.log /d -7 /c "cmd /c del @path"
echo 已清理7天前的日志文件
```

## 故障排除

### 常见问题

#### 1. 数据库连接失败
```
错误: Failed to initialize database connection
解决方案:
- 检查 MySQL 服务是否启动
- 验证 X Protocol 端口 (33060) 是否开启
- 检查数据库配置信息
- 确认用户权限
```

#### 2. 编译错误
```
错误: SystemMonitorController.cpp 编译失败
解决方案:
- 确保所有依赖库已正确安装
- 检查 vcpkg 集成状态
- 验证 C++ 编译器版本
```

#### 3. 端口占用
```
错误: bind: Address already in use
解决方案:
- 检查端口是否被其他程序占用
- 使用 netstat -an | findstr :8081 查看
- 修改配置文件中的端口号
```

#### 4. JWT 验证失败
```
错误: Token validation failed
解决方案:
- 检查 JWT 密钥配置
- 确认 Token 未过期
- 验证请求头格式
```

### 调试模式

#### 启用调试日志
```json
{
  "logging": {
    "level": "debug"
  }
}
```

#### 查看实时日志
```bash
# PowerShell
Get-Content backend\logs\server.log -Wait -Tail 50

# CMD
tail -f backend\logs\server.log
```

## 性能基准

### 典型性能指标
- 健康检查响应时间: < 10ms
- 无人机列表查询: < 50ms
- 用户列表查询: < 100ms
- 并发连接数: 100+
- 内存使用: < 100MB
- CPU 使用: < 5% (空闲时)

### 性能测试命令
```bash
# 使用 Apache Bench 测试
ab -n 1000 -c 10 http://localhost:8081/api/v1/health

# 使用 curl 测试延迟
curl -w "@curl-format.txt" -o /dev/null -s http://localhost:8081/api/v1/health
```

## 备份与恢复

### 数据库备份
```bash
# 备份数据库
mysqldump -u root -p --single-transaction low_altitude_traffic_system > backup_$(date +%Y%m%d).sql

# 恢复数据库
mysql -u root -p low_altitude_traffic_system < backup_20240101.sql
```

### 配置文件备份
```bash
# 备份配置
copy config\server.json config\server.json.backup.%date:~0,4%%date:~5,2%%date:~8,2%
```

这个部署指南为系统监控模块提供了完整的部署流程，从开发环境到生产环境的完整配置说明。