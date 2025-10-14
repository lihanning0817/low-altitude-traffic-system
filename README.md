# 低空交通管理系统 (Low Altitude Traffic Management System)

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Vue](https://img.shields.io/badge/Vue-3.2.13-brightgreen.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-orange.svg)

**一个面向无人机飞行任务规划、监控与管理的智能化低空交通管理平台**

[功能特性](#功能特性) • [技术架构](#技术架构) • [快速开始](#快速开始) • [系统截图](#系统截图) • [开发指南](#开发指南)

</div>

---

## 📋 目录

- [项目简介](#项目简介)
- [功能特性](#功能特性)
- [技术架构](#技术架构)
- [系统要求](#系统要求)
- [快速开始](#快速开始)
- [项目结构](#项目结构)
- [核心功能模块](#核心功能模块)
- [API文档](#api文档)
- [开发规范](#开发规范)
- [常见问题](#常见问题)
- [贡献指南](#贡献指南)
- [许可证](#许可证)

---

## 🎯 项目简介

低空交通管理系统是一个基于 **Vue.js 3 前端** + **C++ 高性能后端** 的全栈解决方案，专为无人机低空飞行场景设计。系统提供从飞行任务规划、航线优化、实时监控、空域管理到应急响应的全流程数字化管理能力。

### 适用场景

- 🚁 城市无人机物流配送
- 🔍 农业植保与测绘巡查
- 🚨 应急救援与消防监控
- 📡 低空空域交通管理中心
- 🎓 无人机飞行训练与教学

---

## ✨ 功能特性

### 核心功能

| 模块 | 功能描述 |
|------|---------|
| 🛫 **飞行任务管理** | 创建、审批、执行、追踪无人机飞行任务，支持多任务并发管理 |
| 🗺️ **智能航线规划** | 基于高德地图API的三维航线规划，自动避障与路径优化 |
| 🌦️ **实时天气集成** | 接入气象数据，提供飞行前天气评估与实时预警 |
| 📍 **空域管理** | 低空空域划分、禁飞区设置、空域占用冲突检测 |
| 🚨 **应急响应系统** | 飞行异常检测、紧急降落点推荐、应急事件处置流程 |
| 📊 **数据分析看板** | 飞行数据统计、任务成功率分析、设备健康度监控 |
| 👥 **多角色权限** | 管理员、飞行员、调度员、观察员等多角色权限隔离 |
| 🔐 **安全认证** | JWT Token认证、密码加密存储、会话管理 |

### 技术亮点

- ⚡ **高性能后端**: C++17 + Crow框架，支持高并发请求处理
- 🎨 **现代化UI**: Vue 3 Composition API + Element Plus，采用Apple设计风格
- 🌐 **RESTful API**: 标准化API设计，易于集成与扩展
- 📱 **响应式设计**: 支持桌面端与移动端自适应布局
- 🔄 **实时通信**: WebSocket支持实时飞行数据推送
- 🗄️ **数据持久化**: MySQL数据库，支持事务与索引优化
- 🧪 **测试覆盖**: 集成Vitest单元测试框架

---

## 🏗️ 技术架构

### 前端技术栈

```
Vue.js 3.2.13         → 渐进式JavaScript框架
Vue Router 4.5.1      → 前端路由管理
Vuex 4.1.0            → 全局状态管理
Axios 1.12.2          → HTTP客户端
Element Plus 2.11.2   → UI组件库
ECharts 6.0.0         → 数据可视化图表
Animate.css 4.1.1     → CSS动画库
```

### 后端技术栈

```
C++ 17                → 核心编程语言
Crow Framework        → 轻量级HTTP服务器框架
MySQL 8.0             → 关系型数据库
JWT (JSON Web Token)  → 身份认证机制
CMake                 → 项目构建工具
vcpkg                 → C++依赖包管理器
```

### 系统架构图

```
┌─────────────────────────────────────────────────────────────┐
│                         用户层                               │
│  Web浏览器 (Vue.js SPA)  │  移动端 (Responsive Design)      │
└─────────────────────────────────────────────────────────────┘
                            ↓ HTTPS/WSS
┌─────────────────────────────────────────────────────────────┐
│                      应用服务层                              │
│  RESTful API (C++ Crow) │ WebSocket 实时通信                │
│  JWT身份认证 │ 权限控制 │ 路由管理                          │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                      业务逻辑层                              │
│  任务管理 │ 航线规划 │ 空域管理 │ 设备控制                   │
│  应急响应 │ 天气集成 │ 数据分析 │ 用户管理                   │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                      数据访问层                              │
│  MySQL连接池 │ DAO数据访问对象 │ 事务管理                    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                      数据存储层                              │
│  MySQL数据库 (用户/任务/航线/日志/设备/空域...)              │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                      外部服务集成                            │
│  高德地图API │ 气象数据API │ 第三方支付接口                  │
└─────────────────────────────────────────────────────────────┘
```

---

## 💻 系统要求

### 开发环境

| 组件 | 版本要求 |
|------|---------|
| **Node.js** | >= 16.x |
| **npm/yarn** | >= 7.x / >= 1.22 |
| **MySQL** | >= 8.0 |
| **C++ 编译器** | GCC >= 9.0 / MSVC >= 2019 / Clang >= 10.0 |
| **CMake** | >= 3.15 |
| **vcpkg** | 最新版 |

### 操作系统支持

- ✅ Windows 10/11 (推荐 Visual Studio 2019/2022)
- ✅ macOS 11+ (Big Sur及以上)
- ✅ Linux (Ubuntu 20.04+, Debian 11+, CentOS 8+)

---

## 🚀 快速开始

### 1. 克隆项目

```bash
# GitHub
git clone https://github.com/Zguoxu/low-altitude-traffic-system.git

# Gitee镜像 (国内推荐)
git clone https://gitee.com/ACE_001/low_altitude_traffic_system.git

cd low-altitude-traffic-system
```

### 2. 数据库初始化

#### 创建数据库

```bash
# 连接MySQL
mysql -u root -p

# 创建数据库
CREATE DATABASE low_altitude_traffic_system
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

# 退出MySQL
exit
```

#### 导入数据结构

```bash
# 方法1: 命令行导入
mysql -u root -p low_altitude_traffic_system < database/init.sql

# 方法2: 使用MySQL Workbench/Navicat等GUI工具导入
# 打开 database/init.sql 文件并执行
```

#### 验证数据表

```sql
USE low_altitude_traffic_system;
SHOW TABLES;
-- 应显示: users, flight_tasks, flight_logs, system_config 等表
```

### 3. 后端启动

```bash
cd backend

# 使用CMake构建项目
cmake -B build -S .

# 编译 (Release模式)
cmake --build build --config Release

# Windows运行
.\build\Release\low_altitude_traffic_system_backend.exe

# Linux/macOS运行
./build/Release/low_altitude_traffic_system_backend

# 后端服务默认运行在: http://localhost:8081
```

#### 配置后端参数 (可选)

编辑 `backend/config/server.json` 修改数据库连接等配置:

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "username": "root",
    "password": "your_password",
    "database": "low_altitude_traffic_system"
  },
  "server": {
    "port": 8081,
    "host": "0.0.0.0"
  },
  "jwt": {
    "secret": "your_jwt_secret_key",
    "expiration": 86400
  }
}
```

### 4. 前端启动

```bash
# 回到项目根目录
cd ..

# 安装依赖
npm install

# 启动开发服务器
npm run serve

# 前端服务默认运行在: http://localhost:8080
```

### 5. 访问系统

🌐 打开浏览器访问: `http://localhost:8080`

#### 默认账号 (测试用)

| 角色 | 用户名 | 密码 |
|------|-------|------|
| 管理员 | admin | admin123 |
| 飞行员 | pilot | pilot123 |

⚠️ **安全提示**: 生产环境请立即修改默认密码！

---

## 📁 项目结构

```
low-altitude-traffic-system/
├── backend/                    # C++ 后端目录
│   ├── config/                # 配置文件 (server.json)
│   ├── controllers/           # 控制器层 (处理HTTP请求)
│   │   ├── AuthController.cpp         # 用户认证
│   │   ├── TaskController.cpp         # 任务管理
│   │   ├── DroneController.cpp        # 设备控制
│   │   └── WebSocketController.cpp    # 实时通信
│   ├── models/                # 数据模型层 (DAO)
│   │   ├── user.cpp / user_dao.cpp
│   │   ├── flight_task.cpp / flight_task_dao.cpp
│   │   ├── airspace.cpp / airspace_dao.cpp
│   │   └── emergency_event.cpp
│   ├── services/              # 业务逻辑层
│   │   ├── flight_task_service.cpp    # 任务服务
│   │   ├── route_service.cpp          # 航线规划
│   │   ├── weather_service.cpp        # 天气集成
│   │   ├── airspace_service.cpp       # 空域管理
│   │   └── emergency_service.cpp      # 应急响应
│   ├── utils/                 # 工具类
│   │   ├── jwt.cpp            # JWT认证工具
│   │   ├── password.cpp       # 密码加密
│   │   └── db_pool.cpp        # 数据库连接池
│   ├── main.cpp               # 程序入口
│   └── CMakeLists.txt         # CMake构建配置
│
├── src/                       # Vue.js 前端目录
│   ├── components/            # 可复用组件
│   │   ├── FlightTaskList.vue         # 任务列表
│   │   ├── MapComponent.vue           # 地图组件
│   │   ├── AirspaceManagement.vue     # 空域管理
│   │   ├── WeatherIntegration.vue     # 天气面板
│   │   ├── EmergencyResponse.vue      # 应急响应
│   │   ├── AnalyticsDashboard.vue     # 数据分析
│   │   └── ProfileSettings.vue        # 用户设置
│   ├── views/                 # 页面级组件
│   │   ├── HomePage.vue               # 首页
│   │   ├── LoginPage.vue              # 登录页
│   │   ├── AdminDashboard.vue         # 管理面板
│   │   ├── HomeDashboard.vue          # 用户主页
│   │   └── TaskTracking.vue           # 任务追踪
│   ├── router/                # 路由配置
│   │   └── index.js
│   ├── store/                 # Vuex状态管理
│   │   └── index.js
│   ├── services/              # API服务封装
│   │   └── api.js
│   ├── assets/                # 静态资源
│   │   ├── images/
│   │   └── styles/
│   ├── App.vue                # 根组件
│   └── main.js                # 前端入口
│
├── database/                  # 数据库脚本
│   ├── init.sql              # 初始化脚本
│   ├── schema.sql            # 表结构定义
│   └── migration_*.sql       # 数据迁移脚本
│
├── docs/                      # 项目文档
├── tests/                     # 测试文件
├── public/                    # 公共静态文件
├── package.json               # Node依赖配置
├── vue.config.js              # Vue CLI配置
├── .gitignore                 # Git忽略规则
└── README.md                  # 项目说明文档 (本文件)
```

---

## 🔧 核心功能模块

### 1️⃣ 用户认证与权限管理

**功能说明**:
- 用户注册/登录 (JWT Token认证)
- 多角色权限控制 (admin/pilot/controller/viewer)
- 密码加密存储 (bcrypt算法)
- 会话管理与Token刷新

**技术实现**:
- 前端: `src/router/index.js` 路由守卫拦截
- 后端: `backend/controllers/AuthController.cpp` + `backend/utils/jwt.cpp`

**相关API**:
```
POST /api/auth/register  → 用户注册
POST /api/auth/login     → 用户登录
GET  /api/auth/profile   → 获取用户信息
POST /api/auth/logout    → 用户登出
```

---

### 2️⃣ 飞行任务管理

**功能说明**:
- 创建飞行任务 (任务名称、类型、时间、航线)
- 任务审批流程 (待审批 → 已批准 → 执行中 → 已完成)
- 任务状态追踪与实时更新
- 任务历史记录与日志

**核心代码位置**:
- 前端: `src/components/FlightTaskList.vue`
- 后端: `backend/services/flight_task_service.cpp`
- 数据库: `flight_tasks` 表

**相关API**:
```
GET    /api/tasks           → 获取任务列表
POST   /api/tasks           → 创建新任务
GET    /api/tasks/:id       → 获取任务详情
PUT    /api/tasks/:id       → 更新任务
DELETE /api/tasks/:id       → 删除任务
POST   /api/tasks/:id/approve → 审批任务
```

---

### 3️⃣ 智能航线规划

**功能说明**:
- 基于高德地图API的路径规划
- 自动避障与最优路径算法
- 三维航线可视化展示
- 航线导出与导入 (JSON格式)

**核心代码位置**:
- 前端: `src/components/MapComponent.vue`
- 后端: `backend/services/route_service.cpp` + `backend/services/amap_route_service.cpp`

**相关API**:
```
POST /api/routes/plan      → 航线规划请求
GET  /api/routes/:id       → 获取航线详情
POST /api/routes/optimize  → 航线优化
```

---

### 4️⃣ 实时天气集成

**功能说明**:
- 接入气象数据API (如高德天气/和风天气)
- 飞行前天气适航性评估
- 实时天气预警推送
- 历史天气数据查询

**核心代码位置**:
- 前端: `src/components/WeatherIntegration.vue`
- 后端: `backend/services/weather_service.cpp`

**相关API**:
```
GET /api/weather/current?city={城市名}  → 实时天气
GET /api/weather/forecast?city={城市名} → 天气预报
GET /api/weather/aviation-check        → 飞行适航检查
```

---

### 5️⃣ 空域管理

**功能说明**:
- 低空空域划分与管理
- 禁飞区/限制区设置
- 空域占用冲突检测
- 空域申请与审批

**核心代码位置**:
- 前端: `src/components/AirspaceManagement.vue`
- 后端: `backend/services/airspace_service.cpp`
- 数据库: `airspaces` 表

**相关API**:
```
GET  /api/airspaces        → 获取空域列表
POST /api/airspaces        → 创建空域
GET  /api/airspaces/:id    → 空域详情
POST /api/airspaces/check-conflict → 冲突检测
```

---

### 6️⃣ 应急响应系统

**功能说明**:
- 飞行异常实时检测 (失联/低电量/偏航)
- 紧急降落点智能推荐
- 应急事件上报与处置流程
- 应急响应日志记录

**核心代码位置**:
- 前端: `src/components/EmergencyResponse.vue`
- 后端: `backend/services/emergency_service.cpp`
- 数据库: `emergency_events`, `emergency_landing_points` 表

**相关API**:
```
POST /api/emergency/report       → 上报应急事件
GET  /api/emergency/events       → 获取事件列表
POST /api/emergency/landing-point → 获取降落点推荐
PUT  /api/emergency/events/:id   → 更新事件状态
```

---

### 7️⃣ 数据分析看板

**功能说明**:
- 飞行任务统计 (总数/成功率/完成率)
- 设备健康度监控
- 用户活跃度分析
- 数据可视化图表 (ECharts)

**核心代码位置**:
- 前端: `src/components/AnalyticsDashboard.vue`
- 后端: `backend/services/traffic_service.cpp`

**相关API**:
```
GET /api/analytics/tasks      → 任务统计
GET /api/analytics/devices    → 设备统计
GET /api/analytics/users      → 用户统计
GET /api/analytics/overview   → 综合看板
```

---

## 📚 API文档

### 基础URL

```
开发环境: http://localhost:8081/api/v1
生产环境: https://your-domain.com/api/v1
```

### 认证方式

所有需要认证的接口需在请求头中携带JWT Token:

```http
Authorization: Bearer {your_access_token}
```

### 接口响应格式

#### 成功响应
```json
{
  "code": 200,
  "message": "success",
  "data": {
    // 实际返回数据
  }
}
```

#### 错误响应
```json
{
  "code": 400,
  "message": "错误描述信息",
  "error": "ERROR_CODE"
}
```

### 核心接口示例

详细API文档请参考: [API Documentation](docs/API.md) (待完善)

---

## 📝 开发规范

### Git提交规范

采用 **约定式提交 (Conventional Commits)** 格式:

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Type类型**:
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更新
- `style`: 代码格式调整 (不影响功能)
- `refactor`: 代码重构
- `test`: 测试相关
- `chore`: 构建/工具配置

**示例**:
```bash
git commit -m "feat(task): 添加飞行任务批量导入功能"
git commit -m "fix(auth): 修复JWT token过期未刷新问题"
git commit -m "docs: 更新API文档和部署指南"
```

### 分支管理规范

```
master           → 主分支 (生产环境)
develop          → 开发分支 (测试环境)
feature/xxx      → 功能分支
bugfix/xxx       → Bug修复分支
hotfix/xxx       → 紧急修复分支
```

**工作流程**:
```bash
# 1. 从master拉取最新代码
git checkout master
git pull origin master

# 2. 创建功能分支
git checkout -b feature/weather-integration

# 3. 开发并提交
git add .
git commit -m "feat(weather): 集成高德天气API"

# 4. 推送分支
git push origin feature/weather-integration

# 5. 在GitHub/Gitee创建Pull Request
# 6. 代码审查通过后合并到master
```

### 代码风格规范

#### 前端 (Vue.js)
- 使用 ESLint 进行代码检查
- 组件命名: PascalCase (如 `FlightTaskList.vue`)
- 方法命名: camelCase (如 `getUserInfo()`)
- 常量命名: UPPER_SNAKE_CASE (如 `API_BASE_URL`)

#### 后端 (C++)
- 类名: PascalCase (如 `FlightTaskService`)
- 函数名: camelCase (如 `createTask()`)
- 变量名: snake_case (如 `user_id`)
- 常量名: UPPER_SNAKE_CASE (如 `MAX_ALTITUDE`)

---

## 🧪 测试

### 运行前端测试

```bash
# 运行单元测试
npm test

# 运行测试并生成覆盖率报告
npm run test:coverage

# 启动测试UI界面
npm run test:ui
```

### 运行后端测试

```bash
cd backend
cmake --build build --target test
```

---

## 🐛 常见问题

### 1. 后端编译失败: 找不到MySQL头文件

**解决方法**:
```bash
# 使用vcpkg安装MySQL Connector C++
vcpkg install libmysql:x64-windows

# 或手动下载MySQL Connector C++
# https://dev.mysql.com/downloads/connector/cpp/
```

### 2. 前端启动报错: Cannot find module 'xxx'

**解决方法**:
```bash
# 清除缓存并重新安装依赖
rm -rf node_modules package-lock.json
npm install
```

### 3. 数据库连接失败: Access denied for user

**解决方法**:
- 检查 `backend/config/server.json` 中的数据库配置
- 确认MySQL用户权限
```sql
GRANT ALL PRIVILEGES ON low_altitude_traffic_system.* TO 'root'@'localhost';
FLUSH PRIVILEGES;
```

### 4. 跨域请求被拦截 (CORS错误)

**解决方法**:
- 检查后端是否配置了CORS中间件
- 前端开发时可在 `vue.config.js` 配置代理:
```javascript
module.exports = {
  devServer: {
    proxy: {
      '/api': {
        target: 'http://localhost:8081',
        changeOrigin: true
      }
    }
  }
}
```

### 5. JWT Token验证失败

**解决方法**:
- 检查前端是否正确存储Token
- 确认Token未过期 (默认24小时)
- 检查后端JWT密钥配置是否一致

---

## 🚢 部署指南

### Docker部署 (推荐)

```bash
# 构建Docker镜像
docker-compose build

# 启动服务
docker-compose up -d

# 查看日志
docker-compose logs -f
```

### Linux生产环境部署

#### 1. 后端部署

```bash
# 编译生产版本
cd backend
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 使用systemd管理服务
sudo cp backend.service /etc/systemd/system/
sudo systemctl enable backend
sudo systemctl start backend
```

#### 2. 前端部署

```bash
# 构建生产版本
npm run build

# 将dist目录部署到Nginx
sudo cp -r dist/* /var/www/html/

# Nginx配置示例
server {
    listen 80;
    server_name your-domain.com;

    location / {
        root /var/www/html;
        try_files $uri $uri/ /index.html;
    }

    location /api {
        proxy_pass http://localhost:8081;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

---

## 🤝 贡献指南

欢迎贡献代码！请遵循以下步骤:

1. Fork 本项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'feat: Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 贡献者行为准则

- 尊重所有贡献者
- 使用友善和包容的语言
- 接受建设性的批评
- 关注对社区最有利的事情

---

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源协议。

---

## 👥 开发团队

- **项目负责人**: [@Zguoxu](https://github.com/Zguoxu)
- **核心开发**: 见 [Contributors](https://github.com/Zguoxu/low-altitude-traffic-system/graphs/contributors)

---

## 📧 联系方式

- 📮 邮箱: support@example.com
- 🐛 问题反馈: [GitHub Issues](https://github.com/Zguoxu/low-altitude-traffic-system/issues)
- 💬 讨论区: [GitHub Discussions](https://github.com/Zguoxu/low-altitude-traffic-system/discussions)

---

## 🌟 Star History

如果这个项目对你有帮助，请点击右上角 ⭐ Star 支持我们！

---

<div align="center">

**低空交通管理系统** - 让无人机飞行更安全、更智能

Made with ❤️ by [Zguoxu Team](https://github.com/Zguoxu)

</div>
