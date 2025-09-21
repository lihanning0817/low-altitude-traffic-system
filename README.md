# 城市智能低空交通系统

## 项目简介

城市智能低空交通系统是一个面向未来城市空中交通管理的综合解决方案，支持无人机、飞行汽车等低空飞行器的路线规划、实时监控和智能调度。系统采用现代化的前后端分离架构，提供高性能、高安全性的服务。

## 项目架构

### 前端技术栈
- **Vue 3** - 渐进式JavaScript框架
- **Element Plus** - 企业级UI组件库
- **Vuex 4** - 状态管理
- **Vue Router 4** - 路由管理
- **Axios** - HTTP客户端
- **ECharts** - 数据可视化
- **高德地图API** - 地图服务
- **OpenWeather API** - 天气数据

### 后端技术栈
- **C++17** - 高性能系统语言
- **MySQL X DevAPI** - 数据库连接
- **JWT认证** - 自研安全令牌系统
- **Boost Beast** - HTTP服务器
- **BCrypt** - 密码哈希
- **JSON** - 数据交换格式

### 数据库
- **MySQL 8.0+** - 主数据库
- 支持用户管理、权限控制、航线数据存储

## 核心功能

### 用户认证系统 ✅
- 用户注册与登录
- JWT Token认证（Access Token + Refresh Token）
- 密码强度验证和安全哈希存储
- 登录失败锁定机制
- 角色权限管理

### 路线规划模块 🚧
- 低空航线智能规划
- 避障算法集成
- 实时路况分析
- 多目标优化

### 天气数据集成 🚧
- 实时天气信息获取
- 气象条件评估
- 飞行安全预警

### 地图可视化 🚧
- 高德地图集成
- 3D航线显示
- 实时位置追踪

## 快速开始

### 环境要求

**前端开发环境：**
- Node.js 16.0+
- npm 或 yarn
- 现代浏览器

**后端开发环境：**
- Visual Studio 2019/2022 或 GCC 9.0+
- MySQL 8.0+
- vcpkg 包管理器
- CMake 3.16+

### 依赖安装

#### 前端依赖
```bash
# 进入项目目录
cd low-altitude-traffic-system

# 安装前端依赖
npm install
```

#### 后端依赖
```bash
# 进入后端目录
cd backend

# 使用vcpkg安装C++依赖
vcpkg install boost-system:x64-windows
vcpkg install boost-thread:x64-windows
vcpkg install boost-json:x64-windows
vcpkg install mysql-connector-cpp:x64-windows
```

### 配置文件设置

#### 1. 数据库配置
编辑 `backend/config/database.h`：
```cpp
// 数据库连接配置
#define DB_HOST "localhost"
#define DB_PORT 33060
#define DB_USER "your_username"
#define DB_PASSWORD "your_password"
#define DB_SCHEMA "low_altitude_traffic"
```

#### 2. 服务器配置
编辑 `backend/config/server.json`：
```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080
  },
  "security": {
    "jwt_secret": "your-super-secure-jwt-secret-key-256-bits-change-in-production-2024",
    "jwt_access_token_expiry": 900,
    "jwt_refresh_token_expiry": 604800
  }
}
```

**⚠️ 重要安全提醒：** 生产环境部署前必须修改默认JWT密钥！

### 数据库初始化

```bash
# 连接MySQL并创建数据库
mysql -u root -p

# 创建数据库
CREATE DATABASE low_altitude_traffic;

# 导入表结构
mysql -u root -p low_altitude_traffic < database/schema.sql
```

### 启动系统

#### 1. 启动后端服务
```bash
cd backend

# 编译项目（Windows）
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# 运行服务器
./Release/low_altitude_traffic_system_backend.exe
```

#### 2. 启动前端开发服务器
```bash
# 开发模式
npm run serve

# 生产构建
npm run build
```

### 验证安装

访问前端应用：`http://localhost:8080`
测试后端API：`http://localhost:8080/api/v1/auth/ping`

## API文档

详细的API接口文档请参考：[backend/AUTH_API_TESTING.md](backend/AUTH_API_TESTING.md)

### 核心API接口

| 接口 | 方法 | 描述 |
|------|------|------|
| `/api/v1/auth/register` | POST | 用户注册 |
| `/api/v1/auth/login` | POST | 用户登录 |
| `/api/v1/auth/me` | GET | 获取用户信息 |
| `/api/v1/auth/refresh` | POST | 刷新Token |
| `/api/v1/auth/logout` | POST | 用户登出 |

### 使用示例

#### 用户注册
```bash
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "email": "testuser@example.com",
    "password": "Test123456",
    "full_name": "测试用户"
  }'
```

#### 用户登录
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "Test123456"
  }'
```

#### 获取用户信息
```bash
curl -X GET http://localhost:8080/api/v1/auth/me \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN_HERE"
```

## 前端操作说明

1. **用户注册**：在注册页面填写用户名、邮箱、密码等信息
2. **用户登录**：使用注册的账号密码进行登录
3. **查看天气**：登录后可查看实时天气信息
4. **路线规划**：选择起点终点进行航线规划
5. **用户设置**：在个人中心修改密码和个人信息

## 开发者须知

### 安全配置

1. **JWT密钥安全**
   - 生产环境必须使用强随机密钥
   - 定期轮换密钥（建议每6个月）
   - 密钥长度至少256位

2. **数据库安全**
   - 使用强密码
   - 启用SSL连接
   - 限制数据库访问IP

3. **生产部署注意事项**
   - 禁用调试模式
   - 启用HTTPS
   - 配置防火墙规则
   - 定期更新依赖库

### 代码质量

- 遵循C++17标准和现代C++最佳实践
- 使用RAII进行资源管理
- 完整的错误处理和日志记录
- 单元测试覆盖核心功能

### 测试指南

#### 后端测试
```bash
cd backend/build

# JWT服务测试
./test_jwt_service.exe

# 认证控制器测试
./test_auth_controller.exe

# 数据库连接测试
./test_database_connection.exe
```

#### 前端测试
```bash
# ESLint代码检查
npm run lint

# 构建测试
npm run build
```

## 项目状态

- ✅ **用户认证系统** - 已完成
- 🚧 **路线规划模块** - 开发中
- 🚧 **天气数据集成** - 开发中
- 🚧 **地图可视化** - 开发中
- 📋 **权限中间件** - 计划中

## 贡献指南

1. Fork 项目仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 联系方式

如有问题或建议，请通过以下方式联系：

- 提交 Issue
- 发送邮件至：[项目邮箱]
- 项目文档：[docs/](docs/)

---

**注意：** 本系统仍在积极开发中，部分功能可能不稳定。生产环境使用前请进行充分测试。
