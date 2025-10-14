# 🔧 低空交通管理系统 - 问题修复实施计划

**创建日期**: 2025-10-14
**状态**: 进行中
**优先级**: P0 Critical

---

## 📋 待修复问题清单

根据综合测试报告,以下问题需要立即解决:

### 1. ❌ 空域管理API未实现 (P0 - Critical)
**当前状态**: `/api/v1/airspaces` 返回404
**数据库状态**: ✅ airspaces表存在,有3条数据
**影响**: 核心功能不可用,无法进行空域管理

### 2. ❌ 设备管理API未实现 (P1 - High)
**当前状态**: `/api/v1/devices` 返回404
**数据库状态**: ❓ 未找到devices表
**分析**: 设备功能可能已合并到drones模块中

### 3. ⚠️ 天气服务返回null (P2 - Medium)
**当前状态**: API正常但返回`{"weather": null}`
**原因**: 外部API未正确配置或密钥失效

### 4. ⚠️ 地图服务地理编码失败 (P2 - Medium)
**当前状态**: API正常但返回`GEOCODING_FAILED`
**原因**: 高德地图API配置问题

### 5. ⚠️ 登录API参数要求不一致 (P1 - High)
**问题**: 前端role可选,后端要求必填
**影响**: 用户体验差

---

## 🎯 实施方案

### 方案1: 实现空域管理API (P0)

#### 1.1 实施结论
经过分析,我发现空域管理功能**已经通过数据库直接查询实现了基础功能**,但**缺少专门的Controller和Repository**。

#### 1.2 快速修复方案
由于时间紧迫且数据库已有数据,**最快的解决方案是直接在main.cpp中添加空域管理路由**,使用现有的DatabaseManager进行查询。

#### 1.3 需要实现的API端点
```cpp
GET    /api/v1/airspaces              - 获取空域列表
GET    /api/v1/airspaces/:id          - 获取单个空域详情
POST   /api/v1/airspaces              - 创建新空域
PUT    /api/v1/airspaces/:id          - 更新空域信息
DELETE /api/v1/airspaces/:id          - 删除空域
POST   /api/v1/airspaces/check-conflict - 检查空域冲突
```

#### 1.4 数据库表结构
```sql
airspaces表字段:
- id (PK)
- airspace_id (唯一编号)
- name (空域名称)
- type (restricted/controlled/uncontrolled/temporary)
- description (描述)
- north_lat, south_lat, east_lng, west_lng (边界坐标)
- min_altitude, max_altitude (高度范围)
- active_start_time, active_end_time (活跃时间)
- status (active/inactive/temporary)
- authority (管理机构)
- contact_info (联系信息)
- max_concurrent_flights (最大并发飞行数)
- 等...
```

---

### 方案2: 确认设备管理需求 (P1)

#### 2.1 分析结论
经过代码分析,发现:
1. ❌ **没有devices表**
2. ✅ **有drones表且功能完整**
3. ✅ **SystemMonitorController中已实现了`/api/v1/drones`**

#### 2.2 建议方案
**将设备管理功能合并到无人机管理中**:
- 前端将`/api/v1/devices`的调用改为`/api/v1/drones`
- 或者创建一个API别名,`/api/v1/devices`重定向到`/api/v1/drones`

#### 2.3 实施步骤
**选项A: 修改前端** (推荐)
```javascript
// 前端src/api/deviceApi.js
export const getDevices = () => {
  // 改为调用drones API
  return axios.get('/api/v1/drones')
}
```

**选项B: 添加API别名** (后端)
```cpp
// main.cpp中添加
server.get("/api/v1/devices", [systemMonitorController](const auto& req, auto& res) {
    // 直接调用getDronesList
    auto response = systemMonitorController->getDronesList(req);
    res = std::move(response);
});
```

---

### 方案3: 配置天气服务 (P2)

#### 3.1 问题分析
查看main.cpp第107行:
```cpp
std::string weatherApiKey = config.getString("weather.api_key", "");
```

**问题**: `weather.api_key`为空字符串,导致外部API调用失败。

#### 3.2 解决方案
需要在`config/server.json`中添加和风天气API密钥:

```json
{
  "weather": {
    "api_key": "your_qweather_api_key_here",
    "provider": "qweather",
    "base_url": "https://devapi.qweather.com/v7",
    "cache_ttl": 1800
  }
}
```

#### 3.3 获取和风天气API密钥
1. 访问: https://dev.qweather.com/
2. 注册账号
3. 创建应用获取API密钥
4. 免费版每天1000次调用

---

### 方案4: 配置地图服务 (P2)

#### 4.1 问题分析
查看main.cpp第571行:
```cpp
std::string amap_key = config.getString("amap.api_key", "1872806f332dab32a1a3dc895b0ad542");
```

**发现**: 已有默认API密钥,但可能失效或限流。

#### 4.2 解决方案
更新`config/server.json`中的高德地图API密钥:

```json
{
  "amap": {
    "api_key": "your_new_amap_api_key_here",
    "base_url": "https://restapi.amap.com/v3"
  },
  "external_apis": {
    "amap": {
      "key": "your_new_amap_api_key_here"
    }
  }
}
```

#### 4.3 获取高德地图API密钥
1. 访问: https://lbs.amap.com/
2. 注册账号
3. 创建应用,选择"Web服务"
4. 获取API密钥

---

### 方案5: 修复登录API参数要求 (P1)

#### 5.1 问题描述
- **前端期望**: `{"username": "admin", "password": "admin123"}` (role可选)
- **后端要求**: `{"username": "admin", "email": "admin@example.com", "password": "admin123", "role": "admin"}` (全部必填)

#### 5.2 解决方案
修改`AuthController.cpp`的login方法,使role参数可选:

```cpp
// AuthController.cpp login方法
void AuthController::login(const http::request<http::string_body>& req) {
    auto json = parseJson(req.body());

    std::string username = json["username"];
    std::string password = json["password"];
    std::string role = json.value("role", ""); // 改为可选

    // 查询用户
    auto user = userRepo->findByUsername(username);
    if (!user) {
        return error("用户名或密码错误");
    }

    // 验证密码
    if (!verifyPassword(password, user.password_hash)) {
        return error("用户名或密码错误");
    }

    // 如果前端传入了role,验证是否匹配
    if (!role.empty() && user.role != role) {
        return error("角色不匹配");
    }

    // 使用数据库中的实际角色生成token
    std::string actualRole = user.role;
    auto token = jwtService->generateToken(user.id, username, actualRole);

    return success({
        {"access_token", token},
        {"user", user.toJson()}
    });
}
```

---

## 📊 实施优先级

| 优先级 | 任务 | 预计时间 | 状态 |
|-------|------|---------|------|
| **P0** | 实现空域管理API | 2-3小时 | 🔄 进行中 |
| **P1** | 确认设备管理需求 | 30分钟 | ⏳ 待定 |
| **P1** | 修复登录API参数 | 30分钟 | ⏳ 待开始 |
| **P2** | 配置天气服务API | 15分钟 | ⏳ 待开始 |
| **P2** | 配置地图服务API | 15分钟 | ⏳ 待开始 |

---

## 🚀 快速实施指南

### 阶段1: 立即修复 (30分钟内完成)

#### 步骤1: 添加空域管理API路由 (10分钟)
直接在`main.cpp`的`setupRoutes`函数中添加空域管理的路由处理。

#### 步骤2: 添加设备API别名 (5分钟)
```cpp
// main.cpp中添加
server.get("/api/v1/devices", [systemMonitorController](const auto& req, auto& res) {
    auto response = systemMonitorController->getDronesList(req);
    res = std::move(response);
});
```

#### 步骤3: 更新配置文件 (5分钟)
在`config/server.json`中添加天气和地图API密钥。

#### 步骤4: 修复登录API (10分钟)
修改`AuthController.cpp`,使role参数可选。

---

### 阶段2: 编译和测试 (30分钟)

#### 步骤1: 编译后端
```bash
cd backend
cmake --build build --config Release
```

#### 步骤2: 重启后端服务
```bash
./build/Release/low_altitude_traffic_system_backend.exe
```

#### 步骤3: API测试
```bash
# 测试空域管理
curl -X GET "http://localhost:8081/api/v1/airspaces" -H "Authorization: Bearer $TOKEN"

# 测试设备管理
curl -X GET "http://localhost:8081/api/v1/devices" -H "Authorization: Bearer $TOKEN"

# 测试登录(不传role)
curl -X POST "http://localhost:8081/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

---

## ✅ 验收标准

### 1. 空域管理API
- ✅ GET /api/v1/airspaces 返回空域列表
- ✅ 返回数据包含3个空域(沈阳故宫、浑南新区、北站交通枢纽)
- ✅ 响应格式符合API规范

### 2. 设备管理API
- ✅ GET /api/v1/devices 返回设备列表
- ✅ 返回数据与drones数据一致

### 3. 天气服务
- ✅ GET /api/v1/weather/current?city=沈阳 返回天气数据
- ✅ weather字段不为null

### 4. 地图服务
- ✅ GET /api/v1/map/geocode?address=沈阳故宫 返回坐标
- ✅ 地理编码成功,返回经纬度

### 5. 登录API
- ✅ 支持只传username和password登录
- ✅ role参数可选
- ✅ 后端自动从数据库查询角色

---

## 📝 实施日志

### 2025-10-14 开始实施

- [x] 创建实施计划文档
- [ ] 开始实现空域管理API
- [ ] 添加设备API别名
- [ ] 更新配置文件
- [ ] 修复登录API
- [ ] 编译和测试
- [ ] 更新测试报告

---

## 🎯 最终目标

完成后,系统测试结果应该变为:

| 指标 | 当前结果 | 目标结果 |
|------|---------|---------|
| 前端Bug修复验证 | 10/10 已修复 ✅ | 10/10 已修复 ✅ |
| API模块可用性 | 7/11 正常工作 (63.6%) ⚠️ | **11/11 正常工作 (100%)** ✅ |
| 关键安全问题 | 3个 (已修复) ✅ | 3个 (已修复) ✅ |
| 未实现功能 | 2个API模块 ❌ | **0个API模块** ✅ |
| 外部服务问题 | 2个 (天气/地图) ⚠️ | **0个** ✅ |
| 系统稳定性 | 优秀 ✅ | 优秀 ✅ |

**总体评分**: 从 ⭐⭐⭐⭐ (4/5) 提升到 ⭐⭐⭐⭐⭐ (5/5)

---

**文档结束**
