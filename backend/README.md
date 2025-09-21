# 城市智能低空交通系统 - C++后端服务

## 概述

这是城市智能低空交通系统的C++后端服务，基于现代C++17标准开发，提供RESTful API接口，支持无人机飞行任务管理、路径规划、设备监控等核心功能。

## 系统要求

### 操作系统
- Windows 10/11 (推荐)
- Linux Ubuntu 20.04+
- macOS 12.0+

### 开发环境
- **C++编译器**: MSVC 2019+, GCC 9+, 或 Clang 10+
- **CMake**: 3.16 或更高版本
- **vcpkg**: 包管理器（推荐）
- **MySQL**: 8.0 或更高版本

### 依赖库
- Boost.Beast (HTTP服务器)
- Boost.JSON (JSON处理)
- nlohmann/json (JSON库)
- MySQL Connector/C++ (数据库连接)
- OpenSSL (加密和HTTPS)
- spdlog (日志系统)
- libcurl (HTTP客户端)

## 快速开始

### 1. 安装vcpkg（如果尚未安装）

**Windows:**
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\\bootstrap-vcpkg.bat
.\\vcpkg integrate install
```

**Linux/macOS:**
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```

### 2. 安装依赖库

```bash
# 进入vcpkg目录
cd vcpkg

# 安装必要的库
./vcpkg install boost-beast boost-json boost-thread boost-system
./vcpkg install nlohmann-json
./vcpkg install mysql-connector-cpp
./vcpkg install openssl
./vcpkg install spdlog
./vcpkg install curl
./vcpkg install gtest  # 可选：用于单元测试
```

### 3. 设置环境变量

**Windows:**
```cmd
set VCPKG_ROOT=C:\\path\\to\\vcpkg
```

**Linux/macOS:**
```bash
export VCPKG_ROOT=/path/to/vcpkg
```

### 4. 配置MySQL数据库

#### 安装MySQL
1. 下载并安装MySQL 8.0：https://dev.mysql.com/downloads/mysql/
2. 启动MySQL服务
3. 创建数据库：

```sql
-- 连接到MySQL
mysql -u root -p

-- 创建数据库
CREATE DATABASE low_altitude_traffic_system CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 导入数据库结构
USE low_altitude_traffic_system;
SOURCE database/schema.sql;
```

### 5. 编译项目

```bash
# 进入后端项目目录
cd backend

# 创建构建目录
mkdir build
cd build

# 配置CMake（Windows使用vcpkg工具链）
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake

# 或者在Linux/macOS上
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# 编译
cmake --build . --config Release
```

### 6. 配置和运行

#### 修改配置文件
编辑 `config/server.json`：

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "database": "low_altitude_traffic_system",
    "username": "your_mysql_username",
    "password": "your_mysql_password"
  },
  "server": {
    "host": "0.0.0.0",
    "port": 8081,
    "threads": 4
  }
}
```

#### 运行服务器

**Windows:**
```cmd
# 确保logs目录存在
mkdir logs

# 运行服务器
.\\Release\\low_altitude_traffic_system_backend.exe config\\server.json
```

**Linux/macOS:**
```bash
# 确保logs目录存在
mkdir -p logs

# 运行服务器
./Release/low_altitude_traffic_system_backend config/server.json
```

### 7. 验证服务

打开浏览器或使用curl测试：

```bash
# 健康检查
curl http://localhost:8081/api/v1/health

# 获取系统信息
curl http://localhost:8081/api/v1/info

# 获取用户列表
curl http://localhost:8081/api/v1/users
```

## 项目结构

```
backend/
├── src/                          # 源代码目录
│   ├── config/                   # 配置管理
│   │   ├── Config.h
│   │   └── Config.cpp
│   ├── server/                   # HTTP服务器
│   │   ├── HttpServer.h
│   │   └── HttpServer.cpp
│   ├── database/                 # 数据库层
│   │   ├── DatabaseManager.h
│   │   └── DatabaseManager.cpp
│   ├── models/                   # 数据模型
│   │   ├── User.h
│   │   └── User.cpp
│   ├── controllers/              # 控制器层（待实现）
│   ├── services/                 # 业务逻辑层（待实现）
│   ├── utils/                    # 工具类（待实现）
│   └── main.cpp                  # 程序入口
├── config/                       # 配置文件
│   └── server.json
├── database/                     # 数据库脚本
│   └── schema.sql
├── tests/                        # 测试代码（待实现）
├── CMakeLists.txt               # CMake配置
├── vcpkg.json                   # vcpkg依赖配置
└── README.md                    # 本文件
```

## API文档

### 基础接口

#### 健康检查
- **GET** `/api/v1/health`
- **响应**:
```json
{
  "success": true,
  "message": "Service is healthy",
  "data": {
    "status": "ok",
    "service": "low-altitude-traffic-system",
    "version": "1.0.0"
  }
}
```

#### 系统信息
- **GET** `/api/v1/info`
- **响应**: 系统基本信息

#### 用户管理
- **GET** `/api/v1/users` - 获取用户列表
- **POST** `/api/v1/users` - 创建用户（待实现）
- **GET** `/api/v1/users/{id}` - 获取用户详情（待实现）
- **PUT** `/api/v1/users/{id}` - 更新用户（待实现）
- **DELETE** `/api/v1/users/{id}` - 删除用户（待实现）

#### 设备管理
- **GET** `/api/v1/drones` - 获取无人机列表
- **POST** `/api/v1/drones` - 注册无人机（待实现）

#### 任务管理
- **GET** `/api/v1/tasks` - 获取任务列表
- **POST** `/api/v1/tasks` - 创建任务（待实现）

## 开发说明

### 添加新的API端点

1. **在main.cpp中的setupRoutes函数中添加路由**：
```cpp
server.post("/api/v1/new-endpoint", [](const auto& req, auto& res) {
    // 处理逻辑
    nlohmann::json response = {
        {"success", true},
        {"message", "处理成功"},
        {"data", {}}
    };

    res.result(boost::beast::http::status::ok);
    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
    res.body() = response.dump();
    res.prepare_payload();
});
```

2. **使用数据库管理器**：
```cpp
auto& db = database::DatabaseManager::getInstance();
auto result = db.executeQuery("SELECT * FROM users");
```

3. **添加新的数据模型**：
- 在`src/models/`目录下创建对应的`.h`和`.cpp`文件
- 参考`User.h`和`User.cpp`的实现方式

### 日志系统

使用spdlog进行日志记录：
```cpp
#include <spdlog/spdlog.h>

spdlog::info("信息日志");
spdlog::warn("警告日志");
spdlog::error("错误日志: {}", error_message);
spdlog::debug("调试日志");
```

### 配置管理

使用配置管理器获取配置：
```cpp
auto& config = config::Config::getInstance();
std::string db_host = config.getString("database.host", "localhost");
int db_port = config.getInt("database.port", 3306);
```

## 测试

### 运行单元测试（可选）

```bash
# 启用测试构建
cmake .. -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake

# 编译测试
cmake --build . --config Release

# 运行测试
ctest -C Release
```

## 部署

### 生产环境部署

1. **编译优化版本**：
```bash
cmake --build . --config Release
```

2. **配置生产环境配置文件**：
```json
{
  "database": {
    "host": "production-db-host",
    "username": "prod_user",
    "password": "secure_password"
  },
  "security": {
    "jwt_secret": "production-secret-key"
  },
  "logging": {
    "level": "info"
  }
}
```

3. **使用系统服务管理器**（如systemd）：
```ini
[Unit]
Description=Low Altitude Traffic System Backend
After=network.target mysql.service

[Service]
Type=simple
User=app
WorkingDirectory=/opt/low-altitude-traffic-system
ExecStart=/opt/low-altitude-traffic-system/low_altitude_traffic_system_backend /opt/low-altitude-traffic-system/config/production.json
Restart=always

[Install]
WantedBy=multi-user.target
```

## 故障排除

### 常见问题

#### 1. 编译错误 - 找不到依赖库
**解决方案**: 确保vcpkg正确安装所有依赖，并设置了正确的工具链文件路径。

#### 2. 数据库连接失败
**解决方案**:
- 检查MySQL服务是否运行
- 验证数据库配置文件中的连接信息
- 确保数据库用户有相应权限

#### 3. 端口冲突
**解决方案**: 修改`config/server.json`中的端口号。

#### 4. 权限问题（Linux/macOS）
**解决方案**:
```bash
chmod +x low_altitude_traffic_system_backend
sudo chown -R $USER:$USER logs/
```

### 调试模式

启用调试日志：
```json
{
  "logging": {
    "level": "debug"
  }
}
```

## 性能优化

### 数据库优化
- 使用连接池（已实现）
- 添加适当的数据库索引
- 使用预编译语句

### 服务器优化
- 调整工作线程数量
- 启用HTTP Keep-Alive
- 使用异步I/O

## 安全注意事项

1. **密码安全**: 生产环境中使用强密码和安全的JWT密钥
2. **数据库安全**: 限制数据库用户权限，使用SSL连接
3. **网络安全**: 在生产环境中使用HTTPS
4. **输入验证**: 对所有用户输入进行验证
5. **日志安全**: 避免在日志中记录敏感信息

## 许可证

本项目采用MIT许可证。详细信息请查看LICENSE文件。

## 支持

如果遇到问题或需要帮助，请：
1. 查看本文档的故障排除部分
2. 检查日志文件（`logs/server.log`）
3. 提交Issue到项目仓库

## 版本历史

- **v1.0.0** (2024-01-15): 初始版本
  - 基础HTTP服务器框架
  - 数据库连接池
  - 基本API端点
  - 配置管理系统
  - 日志系统