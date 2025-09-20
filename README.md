# 智能低空交通管理系统

## 系统简介

智能低空交通管理系统是一个基于Vue.js前端和C++后端的城市低空交通管理解决方案。系统集成了高德地图API，提供飞行任务管理、设备监控、路径规划、空域管理、气象监测、应急响应和数据分析等功能。

## 功能特性

1. **用户认证**: 用户注册、登录和权限管理
2. **飞行任务管理**: 创建、查看、更新飞行任务状态
3. **地图监控**: 实时显示无人机和其他设备位置
4. **路径规划**: 基于高德地图API的智能路径规划
5. **空域管理**: 空域信息查看和飞行许可申请
6. **气象监测**: 实时天气信息和路线天气预测
7. **应急响应**: 紧急事件报告和响应管理
8. **数据分析**: 飞行数据统计和可视化分析

## 技术架构

### 前端技术栈
- Vue.js 3 (Composition API)
- Element Plus UI组件库
- Vuex状态管理
- Vue Router路由管理
- 高德地图API集成
- ECharts数据可视化

### 后端技术栈
- C++ (基于Boost.Beast的WebSocket服务器)
- MySQL数据库
- JWT Token认证
- CMake构建系统

## 项目结构

```
low-altitude-traffic-system/
├── backend/                 # C++后端代码
│   ├── controllers/         # 控制器层
│   ├── models/             # 数据模型层
│   ├── services/           # 业务逻辑层
│   ├── utils/              # 工具类
│   └── CMakeLists.txt      # 构建配置
├── public/                  # 静态资源
├── src/                     # Vue前端代码
│   ├── assets/             # 静态资源
│   ├── components/         # Vue组件
│   ├── router/             # 路由配置
│   ├── services/           # API服务
│   ├── store/              # Vuex状态管理
│   └── views/              # 页面视图
├── tests/                   # 测试文件
└── docker/                  # Docker配置
```

## 运行方式

### 前端运行

1. 安装依赖:
   ```bash
   npm install
   ```

2. 启动开发服务器:
   ```bash
   npm run serve
   ```

3. 构建生产版本:
   ```bash
   npm run build
   ```

### 后端运行

1. 安装依赖:
   - MySQL开发库
   - Boost库
   - websocketpp
   - OpenSSL
   - unofficial-libmysql (Windows)

2. 构建项目:
   ```bash
   cd backend
   mkdir build
   cd build
   cmake ..
   make
   ```

3. 运行服务器:
   ```bash
   ./low_altitude_traffic_system
   ```

### 数据库配置

1. 创建MySQL数据库:
   ```sql
   CREATE DATABASE low_altitude_traffic;
   ```

2. 执行数据库脚本:
   ```bash
   mysql -u username -p low_altitude_traffic < backend/database/schema.sql
   ```

### Docker部署

使用Docker Compose一键部署:

```bash
docker-compose up -d
```

## 配置说明

### 高德地图API密钥配置

系统集成了高德地图API，需要配置有效的API密钥才能正常使用地图功能:

1. 在[高德地图开放平台](https://console.amap.com/)申请API密钥
2. 编辑 `public/index.html` 文件，找到以下行:
   ```html
   <script type="text/javascript" src="https://webapi.amap.com/maps?v=1.4.15&key=YOUR_AMAP_API_KEY"></script>
   ```
3. 将 `YOUR_AMAP_API_KEY` 替换为您申请的API密钥

### 数据库配置

系统使用MySQL数据库存储数据，需要正确配置数据库连接:

1. 创建MySQL数据库:
   ```sql
   CREATE DATABASE low_altitude_traffic;
   CREATE USER 'lat_user'@'localhost' IDENTIFIED BY 'lat_password';
   GRANT ALL PRIVILEGES ON low_altitude_traffic.* TO 'lat_user'@'localhost';
   FLUSH PRIVILEGES;
   ```
2. 执行数据库脚本创建表结构:
   ```bash
   mysql -u lat_user -p low_altitude_traffic < backend/database/schema.sql
   ```
3. 修改后端数据库配置文件 `backend/config/database.cpp`:
   ```cpp
   const std::string DB_HOST = "localhost";
   const std::string DB_USER = "lat_user";
   const std::string DB_PASSWORD = "lat_password";
   const std::string DB_NAME = "low_altitude_traffic";
   const int DB_PORT = 3306;
   ```

## API接口

后端API通过RESTful接口提供服务，主要端点包括:

- 用户认证: `/api/v1/users`
- 飞行任务: `/api/v1/flight-tasks`
- 设备管理: `/api/v1/devices`
- 空域管理: `/api/v1/airspaces`
- 气象数据: `/api/v1/weather`
- 应急响应: `/api/v1/emergency-events`

## 测试

端到端测试计划位于 `tests/e2e/test_plan.md`，包含完整的测试用例和执行计划。

## 许可证

本项目仅供学习和参考使用。
