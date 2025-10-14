# 🧪 模块API测试报告

**项目**: 低空交通管理系统 (Low Altitude Traffic System)
**测试日期**: 2025-10-14
**测试人员**: QA Team
**后端地址**: http://localhost:8081

---

## 📋 测试概述

本文档对除用户认证外的所有核心业务模块进行API级别的功能测试。

### 测试模块列表

1. ✅ 飞行任务管理模块 (Flight Task Management)
2. ✅ 设备管理模块 (Device Management)
3. ✅ 无人机管理模块 (Drone Management)
4. ✅ 空域管理模块 (Airspace Management)
5. ✅ 飞行许可模块 (Flight Permit)
6. ✅ 应急响应模块 (Emergency Response)
7. ✅ 天气服务模块 (Weather Service)
8. ✅ 地图服务模块 (Map Service)
9. ✅ 系统监控模块 (System Monitor)

---

## 🔐 测试准备

### 1. 获取访问Token

所有API测试都需要先获取有效的访问Token。

```bash
# 登录获取Token
curl -s -X POST http://localhost:8081/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "admin123"
  }'
```

**预期响应**:
```json
{
  "success": true,
  "message": "登录成功",
  "data": {
    "access_token": "eyJhbGc...",
    "refresh_token": "eyJhbGc...",
    "user": {
      "id": 4,
      "username": "admin",
      "email": "admin@example.com",
      "role": "admin"
    }
  }
}
```

**设置Token环境变量**:
```bash
TOKEN="<你的access_token>"
```

---

## 1️⃣ 飞行任务管理模块

### API端点
- `GET /api/v1/tasks` - 获取任务列表
- `GET /api/v1/tasks/:id` - 获取任务详情
- `POST /api/v1/tasks` - 创建新任务
- `PUT /api/v1/tasks/:id` - 更新任务
- `DELETE /api/v1/tasks/:id` - 删除任务

### 测试用例 1.1: 获取任务列表

```bash
curl -s -X GET "http://localhost:8081/api/v1/tasks" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "tasks": [
      {
        "id": 1,
        "task_id": "TASK-001",
        "task_name": "任务名称",
        "status": "pending",
        "created_at": "2025-10-14T10:00:00Z"
      }
    ],
    "total": 10
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**实际响应**: _____________________

---

### 测试用例 1.2: 创建新任务

```bash
curl -s -X POST "http://localhost:8081/api/v1/tasks" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "task_name": "测试飞行任务",
    "description": "这是一个测试任务",
    "planned_start_time": "2025-10-15 10:00:00",
    "planned_end_time": "2025-10-15 12:00:00",
    "route": {
      "start": {"lat": 39.9042, "lng": 116.4074},
      "end": {"lat": 39.9142, "lng": 116.4174}
    }
  }'
```

**预期响应**:
```json
{
  "success": true,
  "message": "任务创建成功",
  "data": {
    "task_id": 123,
    "task_code": "TASK-123"
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**创建的任务ID**: _____

---

### 测试用例 1.3: 更新任务状态

```bash
curl -s -X PUT "http://localhost:8081/api/v1/tasks/123" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "status": "in_progress"
  }'
```

**预期响应**:
```json
{
  "success": true,
  "message": "任务更新成功"
}
```

**测试结果**: [ ] PASS  [ ] FAIL

---

## 2️⃣ 设备管理模块

### API端点
- `GET /api/v1/devices` - 获取设备列表
- `GET /api/v1/devices/:id` - 获取设备详情
- `POST /api/v1/devices` - 注册新设备
- `PUT /api/v1/devices/:id` - 更新设备信息

### 测试用例 2.1: 获取设备列表

```bash
curl -s -X GET "http://localhost:8081/api/v1/devices" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "devices": [
      {
        "id": 1,
        "device_id": "DEV-001",
        "device_name": "设备名称",
        "device_type": "sensor",
        "status": "online"
      }
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**设备数量**: _____

---

### 测试用例 2.2: 注册新设备

```bash
curl -s -X POST "http://localhost:8081/api/v1/devices" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "device_name": "测试传感器",
    "device_type": "sensor",
    "location": {
      "lat": 39.9042,
      "lng": 116.4074
    }
  }'
```

**测试结果**: [ ] PASS  [ ] FAIL

---

## 3️⃣ 无人机管理模块

### API端点
- `GET /api/v1/drones` - 获取无人机列表
- `GET /api/v1/drones/:id` - 获取无人机详情
- `POST /api/v1/drones` - 注册新无人机
- `PUT /api/v1/drones/:id/status` - 更新无人机状态

### 测试用例 3.1: 获取无人机列表

```bash
curl -s -X GET "http://localhost:8081/api/v1/drones" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "drones": [
      {
        "id": 1,
        "drone_id": "DRONE-001",
        "model": "DJI Mavic 3",
        "status": "idle",
        "battery_level": 95
      }
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**无人机数量**: _____

---

### 测试用例 3.2: 获取无人机实时状态

```bash
curl -s -X GET "http://localhost:8081/api/v1/drones/1/status" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "drone_id": "DRONE-001",
    "status": "flying",
    "location": {
      "lat": 39.9042,
      "lng": 116.4074,
      "altitude": 100
    },
    "battery_level": 85,
    "speed": 15.5
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL

---

## 4️⃣ 空域管理模块

### API端点
- `GET /api/v1/airspaces` - 获取空域列表
- `GET /api/v1/airspaces/:id` - 获取空域详情
- `POST /api/v1/airspaces` - 创建新空域
- `GET /api/v1/airspaces/check` - 检查空域可用性

### 测试用例 4.1: 获取空域列表

```bash
curl -s -X GET "http://localhost:8081/api/v1/airspaces" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "airspaces": [
      {
        "id": 1,
        "airspace_id": "AS-001",
        "name": "空域名称",
        "type": "restricted",
        "status": "active"
      }
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL

---

### 测试用例 4.2: 检查空域冲突

```bash
curl -s -X POST "http://localhost:8081/api/v1/airspaces/check" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "route": {
      "start": {"lat": 39.9042, "lng": 116.4074},
      "end": {"lat": 39.9142, "lng": 116.4174}
    },
    "start_time": "2025-10-15 10:00:00",
    "end_time": "2025-10-15 12:00:00"
  }'
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "has_conflict": false,
    "conflicting_airspaces": []
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL

---

## 5️⃣ 飞行许可模块

### API端点
- `GET /api/v1/flight-permits` - 获取飞行许可列表
- `POST /api/v1/flight-permits` - 申请飞行许可
- `PUT /api/v1/flight-permits/:id/approve` - 批准飞行许可
- `DELETE /api/v1/flight-permits/:id` - 取消飞行许可

### 测试用例 5.1: 获取飞行许可列表

```bash
curl -s -X GET "http://localhost:8081/api/v1/flight-permits" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "permits": [
      {
        "id": 1,
        "permit_code": "FP-001",
        "task_id": 1,
        "status": "pending",
        "application_time": "2025-10-14T10:00:00Z"
      }
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**许可数量**: _____

---

### 测试用例 5.2: 申请飞行许可

```bash
curl -s -X POST "http://localhost:8081/api/v1/flight-permits" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "task_id": 7,
    "airspace_id": 1,
    "start_time": "2025-10-15 10:00:00",
    "end_time": "2025-10-15 12:00:00",
    "remarks": "测试飞行许可申请"
  }'
```

**预期响应**:
```json
{
  "success": true,
  "message": "飞行许可申请成功",
  "data": {
    "permit_id": 123,
    "permit_code": "FP-123"
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**许可编号**: _____

---

## 6️⃣ 应急响应模块

### API端点
- `GET /api/v1/emergency/events` - 获取应急事件列表
- `POST /api/v1/emergency/events` - 创建应急事件
- `GET /api/v1/emergency-landing-points` - 获取应急降落点
- `GET /api/v1/emergency-landing-points/nearest` - 查找最近降落点

### 测试用例 6.1: 获取应急事件列表

```bash
curl -s -X GET "http://localhost:8081/api/v1/emergency/events" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "events": [
      {
        "id": 1,
        "event_type": "low_battery",
        "severity": "high",
        "status": "active",
        "location": {"lat": 39.9042, "lng": 116.4074}
      }
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL

---

### 测试用例 6.2: 查找最近的应急降落点

```bash
curl -s -X GET "http://localhost:8081/api/v1/emergency-landing-points/nearest?lat=39.9042&lng=116.4074&limit=3&radius=5000" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "landing_points": [
      {
        "id": 1,
        "point_code": "ELP-001",
        "name": "应急降落点1",
        "distance": 1234.56,
        "capacity": "medium"
      }
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**找到的降落点数量**: _____

---

## 7️⃣ 天气服务模块

### API端点
- `GET /api/v1/weather/current` - 获取当前天气
- `GET /api/v1/weather/forecast` - 获取天气预报
- `GET /api/v1/weather/alerts` - 获取天气预警

### 测试用例 7.1: 获取当前天气

```bash
curl -s -X GET "http://localhost:8081/api/v1/weather/current?city=沈阳" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "city": "沈阳",
    "temperature": 15.5,
    "weather": "晴",
    "wind_speed": 3.2,
    "humidity": 65,
    "update_time": "2025-10-14T10:00:00Z"
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**当前温度**: _____°C

---

### 测试用例 7.2: 获取天气预报

```bash
curl -s -X GET "http://localhost:8081/api/v1/weather/forecast?city=沈阳&days=3" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "city": "沈阳",
    "forecasts": [
      {
        "date": "2025-10-15",
        "temp_max": 20,
        "temp_min": 10,
        "weather": "多云",
        "wind_speed": 4.0
      }
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL

---

## 8️⃣ 地图服务模块

### API端点
- `GET /api/v1/map/geocode` - 地理编码(地址→坐标)
- `GET /api/v1/map/reverse-geocode` - 逆地理编码(坐标→地址)
- `POST /api/v1/map/route` - 路径规划

### 测试用例 8.1: 地理编码

```bash
curl -s -X GET "http://localhost:8081/api/v1/map/geocode?address=沈阳故宫" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "address": "沈阳故宫",
    "location": {
      "lat": 41.796700,
      "lng": 123.451200
    }
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**获取的坐标**: (_____, _____)

---

### 测试用例 8.2: 路径规划

```bash
curl -s -X POST "http://localhost:8081/api/v1/map/route" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "start": {"lat": 39.9042, "lng": 116.4074},
    "end": {"lat": 39.9142, "lng": 116.4174},
    "waypoints": []
  }'
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "distance": 1234.56,
    "duration": 180,
    "route": [
      {"lat": 39.9042, "lng": 116.4074},
      {"lat": 39.9092, "lng": 116.4124},
      {"lat": 39.9142, "lng": 116.4174}
    ]
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**路径距离**: _____ 米

---

## 9️⃣ 系统监控模块

### API端点
- `GET /api/v1/system/health` - 系统健康检查
- `GET /api/v1/system/metrics` - 系统指标
- `GET /api/v1/system/logs` - 系统日志

### 测试用例 9.1: 系统健康检查

```bash
curl -s -X GET "http://localhost:8081/api/v1/system/health" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "status": "healthy",
    "uptime": 123456,
    "services": {
      "database": "online",
      "cache": "online",
      "message_queue": "online"
    }
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**系统状态**: _____

---

### 测试用例 9.2: 获取系统指标

```bash
curl -s -X GET "http://localhost:8081/api/v1/system/metrics" \
  -H "Authorization: Bearer $TOKEN"
```

**预期响应**:
```json
{
  "success": true,
  "data": {
    "cpu_usage": 35.5,
    "memory_usage": 62.3,
    "disk_usage": 45.8,
    "active_connections": 25,
    "requests_per_minute": 150
  }
}
```

**测试结果**: [ ] PASS  [ ] FAIL
**CPU使用率**: _____%
**内存使用率**: _____%

---

## 📊 测试结果汇总

### 模块测试统计

| 模块 | 测试用例数 | 通过 | 失败 | 通过率 |
|------|-----------|------|------|--------|
| 飞行任务管理 | 3 | ___ | ___ | ___% |
| 设备管理 | 2 | ___ | ___ | ___% |
| 无人机管理 | 2 | ___ | ___ | ___% |
| 空域管理 | 2 | ___ | ___ | ___% |
| 飞行许可 | 2 | ___ | ___ | ___% |
| 应急响应 | 2 | ___ | ___ | ___% |
| 天气服务 | 2 | ___ | ___ | ___% |
| 地图服务 | 2 | ___ | ___ | ___% |
| 系统监控 | 2 | ___ | ___ | ___% |
| **总计** | **19** | **___** | **___** | **___%** |

---

## 🐛 发现的问题

### 问题列表

| 问题ID | 模块 | 严重程度 | 描述 | 状态 |
|--------|------|---------|------|------|
| ___ | ___ | ___ | ___ | ___ |

---

## ✅ 测试结论

**测试日期**: _____________________
**测试人员**: _____________________

**总体评估**: [ ] ✅ 通过  [ ] ❌ 未通过  [ ] ⏳ 部分通过

**建议**:
- [ ] 所有API响应格式统一
- [ ] 错误处理机制完善
- [ ] 增加数据验证
- [ ] 优化响应速度
- [ ] 添加API文档

---

**文档版本**: v1.0
**创建日期**: 2025-10-14
