# 低空交通管理系统 - 项目功能与交互说明文档

## 目录
1. [系统概述与架构](#1-系统概述与架构)
2. [完整路由映射表](#2-完整路由映射表)
3. [后端API接口](#3-后端api接口)
4. [详细页面功能说明](#4-详细页面功能说明)
5. [认证与权限流程](#5-认证与权限流程)
6. [数据流与状态管理](#6-数据流与状态管理)
7. [用户操作流程](#7-用户操作流程)
8. [实时更新与通知机制](#8-实时更新与通知机制)
9. [响应式设计与移动端适配](#9-响应式设计与移动端适配)
10. [错误处理与异常场景](#10-错误处理与异常场景)
11. [开发者快速参考](#11-开发者快速参考)
12. [技术亮点总结](#12-技术亮点总结)

---

## 1. 系统概述与架构

### 1.1 项目简介
**低空交通管理系统**是一个全栈应用，旨在为低空飞行任务提供全方位的管理、监控和应急响应能力。

### 1.2 技术栈
- **前端**: Vue 3 (Composition API) + Vue Router + Vuex + Element Plus + 高德地图API
- **后端**: C++ (Boost.Beast) + MySQL X DevAPI
- **认证**: JWT (access_token + refresh_token)
- **数据库**: MySQL 8.0+
- **HTTP客户端**: Axios with interceptors

### 1.3 系统架构层级
```
┌─────────────────────────────────────────────────────┐
│              用户界面层 (Vue 3)                      │
│  HomePage | LoginPage | Dashboard | TaskCenter      │
│  MapMonitor | EmergencyResponse | WeatherMonitoring │
└─────────────────────────────────────────────────────┘
                        ↓ HTTP/HTTPS
┌─────────────────────────────────────────────────────┐
│           路由层 (Vue Router + Guards)              │
│  认证守卫 | 权限检查 | 重定向逻辑                    │
└─────────────────────────────────────────────────────┘
                        ↓
┌─────────────────────────────────────────────────────┐
│         状态管理层 (Vuex Store)                      │
│  user | tasks | devices | airspaces | notifications │
└─────────────────────────────────────────────────────┘
                        ↓ Axios + Interceptors
┌─────────────────────────────────────────────────────┐
│              后端API层 (C++ Boost.Beast)            │
│  Auth | Tasks | Permits | Conflicts | Emergency     │
└─────────────────────────────────────────────────────┘
                        ↓ MySQL X DevAPI
┌─────────────────────────────────────────────────────┐
│              数据持久化层 (MySQL)                    │
│  users | flight_tasks | permits | conflicts         │
└─────────────────────────────────────────────────────┘
```

---

## 2. 完整路由映射表

| 序号 | 路由路径 | 路由名称 | 组件文件 | 认证要求 | 权限要求 | 功能描述 |
|------|----------|----------|----------|----------|----------|----------|
| 1 | `/` | home | HomePage.vue | 无 | 无 | 官方网站首页/产品展示 |
| 2 | `/login` | login | LoginPage.vue | 无 | 无 | 用户登录页面 |
| 3 | `/register` | register | RegisterPage.vue | 无 | 无 | 用户注册页面 |
| 4 | `/dashboard` | dashboard | HomeDashboard.vue | 是 | 无 | 用户仪表盘 |
| 5 | `/admin/login` | admin-login | AdminLogin.vue | 无 | 无 | 管理员登录页面 |
| 6 | `/admin/dashboard` | admin-dashboard | AdminDashboard.vue | 是 | admin | 管理员仪表盘 |
| 7 | `/tasks` | tasks | FlightTaskList.vue | 是 | 无 | 飞行任务列表与管理 |
| 8 | `/map` | map | MapComponent.vue | 无 | 无 | 实时地图监控 |
| 9 | `/airspace` | airspace | AirspaceManagement.vue | 无 | 无 | 空域管理 |
| 10 | `/weather` | weather | WeatherIntegration.vue | 无 | 无 | 天气集成与查询 |
| 11 | `/emergency` | emergency | EmergencyResponse.vue | 无 | 无 | 应急响应中心 |
| 12 | `/analytics` | analytics | AnalyticsDashboard.vue | 无 | 无 | 数据分析仪表盘 |
| 13 | `/profile` | profile | ProfileSettings.vue | 无 | 无 | 个人资料设置 |
| 14 | `/settings` | settings | SystemSettings.vue | 无 | 无 | 系统设置 |
| 15 | `/system-monitor` | system-monitor | SystemMonitor.vue | 是 | 无 | 系统监控 |
| 16 | `/404` | not-found | NotFoundPage.vue | 无 | 无 | 404错误页面 |
| 17 | `/:pathMatch(.*)` | - | NotFoundPage.vue | 无 | 无 | 捕获所有未匹配路由 |

### 2.1 路由守卫逻辑
**文件位置**: `src/router/index.js` (lines 126-202)

**认证检查流程**:
```javascript
1. 检查本地存储中的 access_token 和 user 信息
2. 解析用户角色 (admin/user/guest)
3. 公共页面直接放行 (home, login, register, admin-login)
4. 检查管理员权限要求 (requiresAdmin)
5. 检查认证要求 (requiresAuth)
6. 重定向逻辑:
   - 未认证用户访问受保护页面 → /login?redirect=...
   - 非管理员访问管理页面 → /admin/login?redirect=...
   - 已认证用户访问登录页面 → /dashboard
```

---

## 3. 后端API接口

### 3.1 认证模块 (Auth API)
**基础路径**: `/api/v1/auth`

| 端点 | 方法 | 功能 | 请求体 | 响应 |
|------|------|------|--------|------|
| `/register` | POST | 用户注册 | `{username, email, password, role?}` | `{user_id, access_token, refresh_token}` |
| `/login` | POST | 用户登录 | `{username/email, password}` | `{access_token, refresh_token, user: {...}}` |
| `/refresh` | POST | 刷新Token | `{refresh_token}` | `{access_token}` |
| `/me` | GET | 获取当前用户信息 | - | `{user: {...}}` |
| `/password` | PUT | 修改密码 | `{old_password, new_password}` | `{message}` |
| `/logout` | POST | 登出 | - | `{message}` |

### 3.2 飞行任务模块 (Flight Tasks API)
**基础路径**: `/api/v1/tasks`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/` | GET | 获取任务列表 | `?status=&user_id=&limit=&offset=` | `{tasks: [...]}` |
| `/` | POST | 创建新任务 | `{task_name, drone_id, scheduled_time, route_data, ...}` | `{task_id, task_code}` |
| `/:id` | GET | 获取任务详情 | - | `{task: {...}}` |
| `/:id` | PUT | 更新任务 | `{task_name?, status?, route_data?, ...}` | `{message}` |
| `/:id` | DELETE | 删除任务 | - | `{message}` |
| `/:id/start` | POST | 启动任务 | - | `{message}` |
| `/:id/complete` | POST | 完成任务 | `{completion_notes?}` | `{message}` |

### 3.3 飞行许可模块 (Flight Permits API)
**基础路径**: `/api/v1/flight-permits`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/` | POST | 申请飞行许可 | `{task_id, airspace_id, start_time, end_time, remarks?}` | `{permit_id, permit_code, status}` |
| `/` | GET | 获取许可列表 | `?status=&applicant_id=` | `{permits: [...]}` |
| `/:id/approve` | PUT | 批准许可 | - | `{message}` |
| `/:id/reject` | PUT | 拒绝许可 | - | `{message}` |

### 3.4 冲突检测模块 (Conflict Detection API)
**基础路径**: `/api/v1`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/flights` | POST | 注册飞行并检测冲突 | `{task_id, start_time, end_time, route: {start, end}, altitude}` | `{flight_id, conflicts: [...]}` |
| `/flight-conflicts` | GET | 获取冲突列表 | `?status=&severity=&task_id=` | `{conflicts: [...]}` |
| `/flight-conflicts/:id/resolve` | PUT | 解决冲突 | `{resolution_notes?}` | `{message}` |

### 3.5 应急降落点模块 (Emergency Landing API)
**基础路径**: `/api/v1/emergency-landing-points`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/` | GET | 获取所有降落点 | - | `{landing_points: [...]}` |
| `/nearest` | GET | 获取最近降落点 | `?lat=&lng=&limit=&radius=` | `{landing_points: [...]}` |
| `/` | POST | 创建降落点 | `{point_code, name, location: {lat, lng}, capacity, ...}` | `{point_id}` |
| `/:id` | PUT | 更新降落点 | `{name?, capacity?, status?, ...}` | `{message}` |
| `/:id` | DELETE | 删除降落点 | - | `{message}` |

### 3.6 应急事件模块 (Emergency Events API)
**基础路径**: `/api/v1/emergency`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/events` | GET | 获取应急事件列表 | `?status=&severity=` | `{events: [...]}` |
| `/events` | POST | 创建应急事件 | `{event_type, severity, task_id, location: {lat, lng}, description}` | `{event_id, event_code}` |
| `/events/:id` | GET | 获取事件详情 | - | `{event: {...}}` |
| `/events/:id/resolve` | PUT | 解决事件 | `{resolution_notes}` | `{message}` |

### 3.7 天气模块 (Weather API)
**基础路径**: `/api/v1/weather`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/current` | GET | 获取当前天气 | `?city=&lat=&lng=` | `{weather: {...}, temperature, wind_speed, ...}` |
| `/forecast` | GET | 获取天气预报 | `?city=&days=` | `{forecast: [...]}` |

### 3.8 地图模块 (Map API)
**基础路径**: `/api/v1/map`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/geocode` | GET | 地址编码 | `?address=` | `{location: {lat, lng}, formatted_address}` |
| `/reverse-geocode` | GET | 逆地址编码 | `?lat=&lng=` | `{address, formatted_address}` |
| `/routes` | POST | 路径规划 | `{start: {lat, lng}, end: {lat, lng}, waypoints?: [...]}` | `{route: {...}, distance, duration}` |

### 3.9 系统监控模块 (System Monitor API)
**基础路径**: `/api/v1`

| 端点 | 方法 | 功能 | 请求体/参数 | 响应 |
|------|------|------|-------------|------|
| `/health` | GET | 健康检查 | - | `{status: "healthy", timestamp}` |
| `/info` | GET | 系统信息 | - | `{version, uptime, environment}` |
| `/drones` | GET | 获取无人机列表 | `?status=` | `{drones: [...]}` |
| `/users` | GET | 获取用户列表 | `?role=&status=` | `{users: [...]}` |

---

## 4. 详细页面功能说明

### 4.1 HomePage (首页 - `/`)
**文件**: `src/views/HomePage.vue`

**功能描述**:
- 展示系统官方网站
- 产品介绍和功能亮点
- 引导用户注册/登录

**交互元素**:
1. **导航栏**:
   - Logo (点击返回首页)
   - "登录" 按钮 → 跳转到 `/login`
   - "注册" 按钮 → 跳转到 `/register`

2. **Hero区域**:
   - 主标题和副标题
   - CTA按钮: "开始使用" → 跳转到 `/register`

3. **功能展示区**:
   - 6个功能卡片展示系统核心能力
   - 每个卡片包含图标、标题、描述

4. **页脚**:
   - 版权信息
   - 快速链接

**页面状态**: 无需认证，所有用户可访问

---

### 4.2 LoginPage (登录页 - `/login`)
**文件**: `src/views/LoginPage.vue`

**功能描述**:
- 用户身份认证
- 支持用户名/邮箱登录
- 记住登录状态

**交互元素**:
1. **登录表单**:
   - **用户名/邮箱输入框**:
     - 验证规则: 必填
     - 实时验证
   - **密码输入框**:
     - 验证规则: 必填, 最小6位
     - 显示/隐藏密码切换按钮
   - **记住我复选框**:
     - 勾选后保存凭证到localStorage
   - **登录按钮**:
     - 触发登录API调用
     - 加载状态显示
     - 成功后跳转到 `/dashboard` 或重定向URL

2. **底部链接**:
   - "还没有账号？立即注册" → 跳转到 `/register`
   - "忘记密码？" → 跳转到 `/reset-password` (如果实现)

**页面交互流程**:
```
用户输入凭证 → 点击"登录" → 调用 POST /api/v1/auth/login
  ↓
成功响应:
  1. 保存 access_token 到 localStorage
  2. 保存 user 信息到 Vuex store
  3. 重定向到 /dashboard 或查询参数中的 redirect URL
  ↓
失败响应:
  - 显示错误消息 (Element Plus Message)
  - 清空密码输入框
```

**状态管理**:
- Vuex Action: `user/login`
- Mutation: `SET_USER`, `SET_TOKEN`

---

### 4.3 HomeDashboard (用户仪表盘 - `/dashboard`)
**文件**: `src/views/HomeDashboard.vue`

**功能描述**:
- 系统核心数据概览
- 快速访问主要功能
- 实时活动动态

**交互元素**:
1. **欢迎横幅**:
   - 显示 "欢迎回来, {username}!"
   - 当前日期和时间

2. **统计卡片区** (4个卡片):
   - **飞行任务**:
     - 总任务数 (totalTasks)
     - 运行中/已完成/待处理任务数
     - 点击卡片 → 跳转到 `/tasks`
   - **设备状态**:
     - 在线设备数 (onlineDevices)
     - 无人机数量 (droneCount)
     - 点击卡片 → 跳转到 `/devices`
   - **空域管理**:
     - 可用空域数 (availableAirspaces)
     - 受限空域数 (restrictedAirspaces)
     - 点击卡片 → 跳转到 `/airspace`
   - **系统健康度**:
     - 飞行冲突数 (flightConflictsCount)
     - 应急事件数 (emergencyEventsCount)
     - 未读通知数 (unreadNotifications)
     - 点击卡片 → 跳转到 `/system-monitor`

3. **快速操作区** (6个操作按钮):
   - **创建新任务**: 打开任务创建对话框
   - **查看地图**: 跳转到 `/map`
   - **天气查询**: 跳转到 `/weather`
   - **应急响应**: 跳转到 `/emergency`
   - **数据分析**: 跳转到 `/analytics`
   - **系统设置**: 跳转到 `/settings`

4. **最近活动列表**:
   - 显示最近10条系统活动
   - 每条活动包含: 图标、标题、时间戳
   - 支持滚动查看更多
   - 点击活动项 → 跳转到相关详情页

**数据源**:
- Vuex Getters:
  - `totalTasks`, `runningTasks`, `completedTasks`, `pendingTasks`
  - `onlineDevices`, `devicesByType`, `droneCount`
  - `availableAirspaces`, `restrictedAirspaces`
  - `flightConflictsCount`, `emergencyEventsCount`, `unreadNotifications`

**生命周期**:
```javascript
onMounted(() => {
  // 加载仪表盘数据
  store.dispatch('tasks/fetchTasks')
  store.dispatch('devices/fetchDevices')
  store.dispatch('airspaces/fetchAirspaces')
  store.dispatch('notifications/fetchNotifications')

  // 启动轮询 (每30秒刷新一次)
  const pollInterval = setInterval(() => {
    store.dispatch('dashboard/refresh')
  }, 30000)

  onBeforeUnmount(() => {
    clearInterval(pollInterval)
  })
})
```

---

### 4.4 TaskCenter (任务中心 - `/tasks`)
**文件**: `src/components/FlightTaskList.vue`

**功能描述**:
- 飞行任务CRUD操作
- 任务状态管理
- 批量操作

**交互元素**:
1. **工具栏**:
   - **搜索框**: 按任务名称/编号搜索
   - **状态过滤器**: 下拉选择 (全部/待处理/进行中/已完成/已取消)
   - **日期范围选择器**: 筛选特定时间段的任务
   - **创建任务按钮**: 打开任务创建对话框

2. **任务列表/表格**:
   - **列**:
     - 任务编号 (task_code)
     - 任务名称 (task_name)
     - 无人机ID (drone_id)
     - 计划时间 (scheduled_time)
     - 状态 (status) - 显示为带颜色的Tag
     - 操作列
   - **操作按钮**:
     - 查看详情: 打开详情抽屉
     - 编辑: 打开编辑对话框
     - 启动任务 (仅 status=pending): 调用 POST `/tasks/:id/start`
     - 完成任务 (仅 status=ongoing): 调用 POST `/tasks/:id/complete`
     - 删除: 确认后调用 DELETE `/tasks/:id`

3. **分页器**:
   - 支持分页显示
   - 每页10/20/50条可选

4. **任务创建/编辑对话框**:
   - **任务名称**: 必填文本输入
   - **无人机选择**: 下拉选择可用无人机
   - **计划时间**: 日期时间选择器
   - **路线数据**: 地图选点或JSON输入
   - **备注**: 多行文本输入
   - **提交按钮**: 调用 POST `/tasks` 或 PUT `/tasks/:id`
   - **取消按钮**: 关闭对话框

**页面交互流程**:
```
页面加载 → 调用 GET /api/v1/tasks → 渲染任务列表
  ↓
用户操作:
1. 搜索/筛选 → 本地过滤或重新调用API
2. 创建任务 → 打开对话框 → 填写表单 → 提交 → 刷新列表
3. 编辑任务 → 打开对话框 → 预填数据 → 提交 → 刷新列表
4. 启动任务 → 确认 → 调用API → 更新状态
5. 删除任务 → 确认 → 调用API → 移除列表项
```

**状态管理**:
- Vuex Module: `tasks`
- Actions: `fetchTasks`, `createTask`, `updateTask`, `deleteTask`, `startTask`, `completeTask`

---

### 4.5 MapMonitor (地图监控 - `/map`)
**文件**: `src/components/MapComponent.vue`

**功能描述**:
- 实时无人机位置监控
- 飞行路线可视化
- 空域边界显示
- 应急降落点标记

**交互元素**:
1. **地图控件**:
   - **缩放控件**: +/- 按钮
   - **图层切换**: 卫星图/街道图/混合图
   - **定位按钮**: 居中到用户当前位置
   - **全屏按钮**: 全屏显示地图

2. **地图标记**:
   - **无人机标记**:
     - 不同颜色表示不同状态 (飞行中=绿色, 待命=蓝色, 故障=红色)
     - 点击标记 → 显示无人机信息弹窗
     - 信息包含: 编号、型号、电量、高度、速度
   - **应急降落点标记**:
     - 黄色三角形图标
     - 点击标记 → 显示降落点详情
   - **空域边界**:
     - 多边形覆盖层
     - 可用空域 (绿色边界)
     - 受限空域 (红色边界)
   - **飞行路线**:
     - 实时绘制的折线
     - 不同任务用不同颜色

3. **侧边栏面板**:
   - **无人机列表**:
     - 显示所有在线无人机
     - 点击列表项 → 地图居中到该无人机
   - **任务列表**:
     - 显示进行中的任务
     - 勾选/取消勾选 → 显示/隐藏飞行路线

**实时更新机制**:
```javascript
// WebSocket连接或轮询
const updateDronePositions = () => {
  store.dispatch('devices/fetchDronePositions')
}

onMounted(() => {
  // 每5秒更新一次无人机位置
  const updateInterval = setInterval(updateDronePositions, 5000)

  onBeforeUnmount(() => {
    clearInterval(updateInterval)
  })
})
```

---

### 4.6 EmergencyResponse (应急响应 - `/emergency`)
**文件**: `src/components/EmergencyResponse.vue`

**功能描述**:
- 应急事件管理
- 快速响应流程
- 最近降落点推荐

**交互元素**:
1. **应急事件列表**:
   - **筛选器**: 按状态/严重程度筛选
   - **事件卡片**:
     - 事件编号 (event_code)
     - 事件类型 (event_type)
     - 严重程度标签 (critical=红色, high=橙色, medium=黄色, low=绿色)
     - 关联任务ID
     - 发生时间
     - 描述
     - 状态 (active/resolved)
   - **操作按钮**:
     - 查看详情: 打开详情抽屉
     - 解决事件: 打开解决对话框

2. **创建应急事件按钮**:
   - 点击打开创建对话框
   - **表单字段**:
     - 事件类型: 下拉选择 (设备故障/天气异常/空域冲突/通信中断/其他)
     - 严重程度: 单选 (critical/high/medium/low)
     - 关联任务: 下拉选择或输入任务ID
     - 位置: 地图选点 (lat, lng)
     - 描述: 多行文本
   - **提交**: 调用 POST `/api/v1/emergency/events`

3. **应急降落点推荐**:
   - 输入当前位置 (自动获取或手动输入)
   - 点击"查找最近降落点"按钮
   - 调用 GET `/api/v1/emergency-landing-points/nearest?lat=...&lng=...`
   - 显示最近的3个降落点
   - 每个降落点显示: 名称、距离、可用容量、联系方式

4. **事件解决对话框**:
   - **解决备注**: 多行文本输入
   - **附件上传**: 上传图片/文档
   - **提交**: 调用 PUT `/api/v1/emergency/events/:id/resolve`

**页面交互流程**:
```
页面加载 → 调用 GET /api/v1/emergency/events → 渲染事件列表
  ↓
用户操作:
1. 创建事件 → 填写表单 → 提交 → 刷新列表
2. 查看详情 → 显示完整信息和处理历史
3. 解决事件 → 填写解决备注 → 提交 → 更新状态
4. 查找降落点 → 调用API → 显示推荐结果 → 可导航到地图
```

---

### 4.7 WeatherMonitoring (天气监控 - `/weather`)
**文件**: `src/components/WeatherIntegration.vue`

**功能描述**:
- 实时天气查询
- 多日天气预报
- 飞行适宜性评估

**交互元素**:
1. **城市搜索栏**:
   - 输入城市名称或选择预设城市
   - 支持自动补全
   - 点击搜索按钮 → 调用 GET `/api/v1/weather/current?city=...`

2. **当前天气卡片**:
   - **显示信息**:
     - 温度 (°C)
     - 天气状况图标 (晴/多云/雨/雪等)
     - 湿度百分比
     - 风速 (m/s)
     - 风向
     - 气压 (hPa)
     - 能见度 (km)
   - **飞行建议**:
     - 根据天气条件自动判断 (适宜/谨慎/不适宜)
     - 显示相应的颜色和图标

3. **天气预报表**:
   - 显示未来7天的天气预报
   - 每天显示: 日期、最高/最低温度、天气图标、降水概率

4. **气象图表**:
   - 温度趋势折线图
   - 降水概率柱状图

**数据刷新**:
```javascript
// 每10分钟自动刷新天气数据
const refreshWeather = () => {
  store.dispatch('weather/fetchCurrentWeather', { city: currentCity.value })
}

onMounted(() => {
  const refreshInterval = setInterval(refreshWeather, 600000)

  onBeforeUnmount(() => {
    clearInterval(refreshInterval)
  })
})
```

---

## 5. 认证与权限流程

### 5.1 JWT认证机制
**Token类型**:
- **access_token**: 短期有效 (15分钟), 用于API认证
- **refresh_token**: 长期有效 (7天), 用于刷新access_token

**Token存储**:
- localStorage: `access_token`, `refresh_token`, `user`

**Token验证流程**:
```
1. 用户登录 → 后端返回 access_token + refresh_token
2. 前端保存到 localStorage
3. 每次API请求 → Axios拦截器自动添加 Authorization: Bearer {access_token}
4. 后端验证Token:
   - 有效 → 处理请求
   - 过期 → 返回 401 Unauthorized
5. 前端收到401 → 自动调用 POST /api/v1/auth/refresh
   - 成功 → 保存新access_token, 重试原请求
   - 失败 → 清空Token, 重定向到登录页
```

**Axios拦截器实现** (推断):
```javascript
// 请求拦截器
axios.interceptors.request.use((config) => {
  const token = localStorage.getItem('access_token')
  if (token) {
    config.headers.Authorization = `Bearer ${token}`
  }
  return config
})

// 响应拦截器
axios.interceptors.response.use(
  (response) => response,
  async (error) => {
    if (error.response?.status === 401) {
      const refresh_token = localStorage.getItem('refresh_token')
      if (refresh_token) {
        try {
          const { data } = await axios.post('/api/v1/auth/refresh', { refresh_token })
          localStorage.setItem('access_token', data.access_token)
          // 重试原请求
          error.config.headers.Authorization = `Bearer ${data.access_token}`
          return axios.request(error.config)
        } catch {
          // 刷新失败，清空Token并跳转到登录页
          localStorage.clear()
          router.push('/login')
        }
      } else {
        localStorage.clear()
        router.push('/login')
      }
    }
    return Promise.reject(error)
  }
)
```

### 5.2 权限控制
**角色定义**:
- **guest**: 未登录用户 (只能访问公共页面)
- **user**: 普通用户 (可访问基本功能)
- **admin**: 管理员 (可访问所有功能 + 管理页面)

**权限检查位置**:
1. **路由守卫** (`router.beforeEach`):
   - 检查 `meta.requiresAuth` 和 `meta.requiresAdmin`
   - 未通过检查 → 重定向到登录页

2. **API层** (后端):
   - 每个受保护的API端点都验证Token
   - 管理员操作 (如批准许可) 额外检查用户角色

3. **组件层** (UI):
   - 使用 `v-if` 根据用户角色显示/隐藏按钮
   - 示例: `<el-button v-if="isAdmin" @click="approvePermit">批准</el-button>`

---

## 6. 数据流与状态管理

### 6.1 Vuex Store结构 (推断)
```
store/
├── index.js              // 根Store
├── modules/
│   ├── user.js           // 用户状态 (当前用户信息, Token)
│   ├── tasks.js          // 飞行任务状态
│   ├── devices.js        // 设备状态 (无人机, 传感器)
│   ├── airspaces.js      // 空域状态
│   ├── permits.js        // 飞行许可状态
│   ├── conflicts.js      // 飞行冲突状态
│   ├── emergency.js      // 应急事件状态
│   ├── weather.js        // 天气数据状态
│   ├── notifications.js  // 通知状态
│   └── dashboard.js      // 仪表盘聚合数据
```

### 6.2 数据流示例: 创建飞行任务
```
Component (FlightTaskList.vue)
  ↓ 用户点击"创建任务"按钮
  ↓ 填写表单
  ↓ 点击"提交"
  ↓
Vuex Action: store.dispatch('tasks/createTask', taskData)
  ↓
Action内部:
  1. commit('SET_LOADING', true)
  2. const response = await axios.post('/api/v1/tasks', taskData)
  3. commit('ADD_TASK', response.data.task)
  4. commit('SET_LOADING', false)
  5. return response.data
  ↓
Component接收结果:
  - 成功: 显示成功消息, 关闭对话框, 刷新列表
  - 失败: 显示错误消息
```

### 6.3 状态订阅与响应式更新
```javascript
// 组件中订阅Vuex状态
const tasks = computed(() => store.getters['tasks/allTasks'])
const loading = computed(() => store.state.tasks.loading)

// 当store中的tasks数组变化时，组件自动重新渲染
```

---

## 7. 用户操作流程

### 7.1 场景1: 新用户注册并创建第一个飞行任务
**步骤**:
```
1. 用户访问官网首页 (/)
   → 点击"开始使用"或导航栏的"注册"按钮
   → 跳转到 /register

2. 填写注册表单
   - 输入用户名 (唯一性验证)
   - 输入邮箱 (格式验证)
   - 输入密码 (强度验证)
   - 选择角色 (可选, 默认user)
   → 点击"注册"按钮
   → 调用 POST /api/v1/auth/register

3. 注册成功
   → 自动登录 (保存Token)
   → 跳转到 /dashboard

4. 仪表盘初始引导
   → 显示"欢迎新用户"提示
   → 点击"创建新任务"快速操作按钮
   → 打开任务创建对话框

5. 填写任务表单
   - 输入任务名称: "沈阳故宫文物巡检"
   - 选择无人机: 从下拉列表选择
   - 选择计划时间: 2025-10-14 14:00
   - 绘制飞行路线: 在地图上选择起点和终点
   - 输入备注: "常规文物巡检任务"
   → 点击"提交"
   → 调用 POST /api/v1/tasks

6. 任务创建成功
   → 显示成功消息: "任务创建成功, 任务编号: TASK-20251014001"
   → 关闭对话框
   → 跳转到 /tasks 查看任务列表
   → 新任务显示在列表顶部, 状态为"待处理"

7. 申请飞行许可
   → 点击任务列表中的"申请许可"按钮
   → 打开许可申请对话框
   - 自动填充 task_id
   - 选择空域: 从可用空域列表选择
   - 确认飞行时间段
   - 输入申请说明
   → 点击"提交"
   → 调用 POST /api/v1/flight-permits

8. 许可申请提交
   → 显示消息: "飞行许可申请已提交, 等待管理员审批"
   → 任务状态更新为"许可待审批"

9. 等待审批 (管理员操作)
   → 用户可在 /dashboard 查看许可审批进度
   → 收到通知: "您的飞行许可已批准"

10. 启动任务
    → 返回 /tasks
    → 点击任务的"启动"按钮
    → 调用 POST /api/v1/tasks/:id/start
    → 任务状态更新为"进行中"
    → 可在 /map 实时查看无人机位置
```

### 7.2 场景2: 管理员审批飞行许可流程
**步骤**:
```
1. 管理员登录
   → 访问 /admin/login
   → 输入管理员凭证
   → 调用 POST /api/v1/auth/login
   → 跳转到 /admin/dashboard

2. 查看待审批许可
   → 管理员仪表盘显示"待审批许可: 3"
   → 点击"待审批许可"卡片
   → 跳转到许可管理页面 (或在当前页面显示列表)
   → 调用 GET /api/v1/flight-permits?status=pending

3. 审查许可详情
   → 点击某个许可的"查看详情"按钮
   → 显示详情抽屉:
     - 申请人信息
     - 任务详情
     - 飞行时间段
     - 空域信息
     - 申请说明
     - 冲突检测结果 (如果有)

4. 执行批准/拒绝操作
   → 如果无冲突且符合规定:
     - 点击"批准"按钮
     - 调用 PUT /api/v1/flight-permits/:id/approve
     - 显示成功消息: "许可已批准"
     - 系统自动发送通知给申请人

   → 如果存在问题:
     - 点击"拒绝"按钮
     - 填写拒绝原因
     - 调用 PUT /api/v1/flight-permits/:id/reject
     - 系统自动发送拒绝通知和原因给申请人

5. 批量审批 (可选)
   → 勾选多个许可
   → 点击"批量批准"按钮
   → 系统依次调用审批API
   → 显示批量操作结果摘要
```

### 7.3 场景3: 应急事件处理流程
**步骤**:
```
1. 实时监控发现异常
   → 用户在 /map 实时监控页面
   → 发现无人机出现异常 (如电量低、通信中断)
   → 地图上无人机标记变为红色并闪烁

2. 创建应急事件
   → 点击"创建应急事件"按钮 (或点击异常无人机标记)
   → 打开应急事件创建对话框:
     - 事件类型: 自动识别为"设备故障"
     - 严重程度: 系统判断为"high"
     - 关联任务: 自动填充当前任务ID
     - 位置: 自动填充无人机当前位置
     - 描述: 用户补充详细情况
   → 点击"提交"
   → 调用 POST /api/v1/emergency/events

3. 系统自动响应
   → 事件创建成功后:
     - 向所有在线管理员发送实时通知
     - 在应急响应页面顶部显示红色警报横幅
     - 自动查找最近的应急降落点
     - 调用 GET /api/v1/emergency-landing-points/nearest

4. 应急决策
   → 响应团队在 /emergency 页面查看事件详情
   → 查看推荐的应急降落点列表
   → 联系无人机操作员
   → 引导无人机降落到最近的安全点

5. 执行应急降落
   → 操作员手动控制或启动自动降落
   → 在 /map 实时观察无人机航迹
   → 无人机成功降落到指定降落点

6. 解决事件
   → 返回 /emergency
   → 点击事件的"解决"按钮
   → 填写解决对话框:
     - 解决备注: "无人机已安全降落, 电池故障, 已更换"
     - 上传现场照片
   → 点击"提交"
   → 调用 PUT /api/v1/emergency/events/:id/resolve

7. 事件归档
   → 事件状态更新为"已解决"
   → 系统生成事件报告
   → 相关人员收到解决通知
   → 事件记录可在历史记录中查询
```

---

## 8. 实时更新与通知机制

### 8.1 数据更新策略
**轮询 (Polling)**:
```javascript
// 示例: 仪表盘每30秒刷新数据
const pollDashboard = () => {
  store.dispatch('dashboard/refresh')
}

onMounted(() => {
  const interval = setInterval(pollDashboard, 30000)
  onBeforeUnmount(() => clearInterval(interval))
})
```

**WebSocket (推荐用于实时性要求高的场景)**:
```javascript
// 示例: 连接WebSocket接收实时无人机位置
const ws = new WebSocket('ws://backend-url/ws/drone-positions')

ws.onmessage = (event) => {
  const data = JSON.parse(event.data)
  store.commit('devices/UPDATE_DRONE_POSITION', data)
}

onBeforeUnmount(() => {
  ws.close()
})
```

### 8.2 通知系统
**通知类型**:
1. **系统通知**: 任务状态变更、许可审批结果
2. **应急通知**: 应急事件创建、冲突检测
3. **消息通知**: 管理员消息、系统公告

**通知显示方式**:
- **顶部通知栏**: 显示未读通知数量
- **Element Plus Notification**: 弹出式通知 (右上角)
- **页面内通知列表**: 在 /dashboard 或专门的通知中心

**通知持久化**:
- 后端存储通知记录
- 用户登录时加载未读通知
- 调用 GET `/api/v1/notifications?unread=true`

---

## 9. 响应式设计与移动端适配

### 9.1 断点定义 (推断)
```scss
// 移动端
@media (max-width: 768px) {
  // 单列布局, 隐藏侧边栏, 底部导航
}

// 平板
@media (min-width: 769px) and (max-width: 1024px) {
  // 两列布局, 可折叠侧边栏
}

// 桌面
@media (min-width: 1025px) {
  // 三列布局, 固定侧边栏
}
```

### 9.2 移动端优化
- **触摸优化**: 按钮和链接增大点击区域
- **滑动手势**: 支持左右滑动切换页面/抽屉
- **简化布局**: 移动端隐藏次要信息, 突出核心功能
- **底部导航栏**: 替代桌面端的侧边栏
- **地图手势**: 支持双指缩放、单指拖动

---

## 10. 错误处理与异常场景

### 10.1 网络错误
**场景**: API请求失败 (超时、断网、服务器错误)

**处理**:
```javascript
try {
  const response = await axios.get('/api/v1/tasks')
  return response.data
} catch (error) {
  if (error.code === 'ECONNABORTED') {
    ElMessage.error('请求超时, 请检查网络连接')
  } else if (error.response?.status >= 500) {
    ElMessage.error('服务器错误, 请稍后重试')
  } else {
    ElMessage.error('请求失败: ' + error.message)
  }
  throw error
}
```

### 10.2 认证错误
**场景**: Token过期或无效

**处理**:
- Axios拦截器自动尝试刷新Token
- 刷新失败 → 清空本地存储 → 重定向到登录页
- 显示消息: "登录已过期, 请重新登录"

### 10.3 权限错误
**场景**: 用户尝试访问无权限的资源

**处理**:
- 后端返回 403 Forbidden
- 前端显示: "权限不足, 请联系管理员"
- 自动返回上一页或跳转到安全页面

### 10.4 数据验证错误
**场景**: 表单提交数据不符合要求

**处理**:
- 前端验证: Element Plus表单验证规则
- 后端验证: 返回 400 Bad Request + 详细错误信息
- 显示字段级错误提示

### 10.5 业务逻辑错误
**场景**: 飞行冲突、空域不可用、设备离线

**处理**:
- 后端返回业务错误代码和描述
- 前端根据错误代码显示友好提示
- 提供解决建议或替代方案

---

## 11. 开发者快速参考

### 11.1 常用API调用示例

**用户登录**:
```javascript
const login = async (credentials) => {
  const { data } = await axios.post('/api/v1/auth/login', {
    username: credentials.username,
    password: credentials.password
  })
  localStorage.setItem('access_token', data.access_token)
  localStorage.setItem('refresh_token', data.refresh_token)
  localStorage.setItem('user', JSON.stringify(data.user))
  return data
}
```

**获取飞行任务列表**:
```javascript
const getTasks = async (params = {}) => {
  const { data } = await axios.get('/api/v1/tasks', { params })
  return data.tasks
}
```

**创建飞行任务**:
```javascript
const createTask = async (taskData) => {
  const { data } = await axios.post('/api/v1/tasks', taskData)
  return data
}
```

**申请飞行许可**:
```javascript
const applyPermit = async (permitData) => {
  const { data } = await axios.post('/api/v1/flight-permits', {
    task_id: permitData.taskId,
    airspace_id: permitData.airspaceId,
    start_time: permitData.startTime,
    end_time: permitData.endTime,
    remarks: permitData.remarks
  })
  return data
}
```

### 11.2 Vuex Store操作示例

**读取状态**:
```javascript
import { useStore } from 'vuex'
import { computed } from 'vue'

const store = useStore()
const currentUser = computed(() => store.state.user.currentUser)
const tasks = computed(() => store.getters['tasks/allTasks'])
```

**触发Action**:
```javascript
// 异步操作
await store.dispatch('tasks/fetchTasks')
await store.dispatch('user/login', credentials)
```

**提交Mutation**:
```javascript
// 同步操作
store.commit('tasks/SET_LOADING', true)
store.commit('user/SET_TOKEN', token)
```

### 11.3 路由跳转示例

**编程式导航**:
```javascript
import { useRouter } from 'vue-router'

const router = useRouter()

// 跳转到指定路径
router.push('/tasks')

// 跳转到命名路由
router.push({ name: 'task-detail', params: { id: 123 } })

// 带查询参数
router.push({ path: '/tasks', query: { status: 'ongoing' } })

// 返回上一页
router.back()
```

**声明式导航**:
```vue
<router-link to="/tasks">任务中心</router-link>
<router-link :to="{ name: 'task-detail', params: { id: task.id } }">
  查看详情
</router-link>
```

---

## 12. 技术亮点总结

### 12.1 架构设计
- **前后端分离**: Vue 3前端 + C++后端, 通过RESTful API通信
- **组件化开发**: 可复用的Vue组件库
- **状态集中管理**: Vuex统一管理应用状态
- **路由权限控制**: 基于角色的路由守卫

### 12.2 安全性
- **JWT双Token机制**: access_token + refresh_token
- **Token自动刷新**: Axios拦截器处理Token过期
- **HTTPS加密传输**: 生产环境强制HTTPS
- **SQL注入防护**: 后端使用参数化查询
- **XSS防护**: Vue默认转义HTML内容

### 12.3 用户体验
- **实时数据更新**: 轮询/WebSocket实时刷新
- **响应式设计**: 支持桌面/平板/移动端
- **友好错误提示**: 详细的错误信息和解决建议
- **流畅的交互**: Loading状态、骨架屏、动画过渡

### 12.4 可扩展性
- **模块化Vuex Store**: 易于添加新功能模块
- **懒加载路由**: 优化首屏加载速度
- **组件懒加载**: 按需加载大型组件
- **API版本控制**: 支持多版本API共存

### 12.5 开发效率
- **Element Plus组件库**: 开箱即用的UI组件
- **高德地图集成**: 强大的地图功能
- **Axios封装**: 统一的HTTP请求处理
- **开发工具支持**: Vue DevTools调试支持

---

## 附录: API响应格式规范

### 成功响应
```json
{
  "code": 200,
  "message": "操作成功",
  "data": {
    // 具体数据
  }
}
```

### 错误响应
```json
{
  "code": 400,
  "message": "请求参数错误",
  "error": {
    "field": "username",
    "reason": "用户名已存在"
  }
}
```

### 分页响应
```json
{
  "code": 200,
  "message": "获取成功",
  "data": {
    "items": [...],
    "pagination": {
      "total": 100,
      "page": 1,
      "per_page": 20,
      "total_pages": 5
    }
  }
}
```

---

**文档版本**: v1.0
**生成日期**: 2025-10-14
**维护者**: 项目技术团队
**联系方式**: [项目仓库链接]
