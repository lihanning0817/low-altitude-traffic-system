# 📘 前端API服务使用指南

**项目**: 低空交通管理系统
**创建日期**: 2025-10-14
**版本**: v1.0

---

## 📋 目录

1. [概述](#概述)
2. [API服务列表](#api服务列表)
3. [快速开始](#快速开始)
4. [详细使用说明](#详细使用说明)
5. [错误处理](#错误处理)
6. [最佳实践](#最佳实践)

---

## 概述

本系统的前端API服务层位于 `src/services/` 目录下,为所有业务模块提供统一的API调用接口。

### 服务架构

```
src/services/
├── index.js              # 统一导出文件
├── authApi.js            # 认证服务 (已存在)
├── flightTaskApi.js      # 飞行任务管理 (已存在)
├── droneApi.js           # 无人机管理 (新创建)
├── airspaceApi.js        # 空域管理 (新创建)
├── permitApi.js          # 飞行许可 (新创建)
├── emergencyApi.js       # 应急响应 (已存在)
├── weatherApi.js         # 天气服务 (已存在)
├── mapApi.js             # 地图服务 (新创建)
└── systemMonitorApi.js   # 系统监控 (已存在)
```

### 核心特性

✅ **统一的API调用接口** - 所有API服务采用相同的调用模式
✅ **自动JWT认证** - 自动从localStorage读取access_token并添加到请求头
✅ **统一错误处理** - 所有API错误都会被拦截并格式化
✅ **中文友好** - 所有错误消息都是中文提示
✅ **TypeScript友好** - 清晰的参数和返回值结构

---

## API服务列表

| 服务名 | 文件名 | 后端基础URL | 状态 | 说明 |
|-------|--------|------------|------|------|
| authApi | authApi.js | /api/v1/auth | ✅ 完整 | 用户认证、登录、Token管理 |
| taskApi | flightTaskApi.js | /api/v1/tasks | ✅ 完整 | 飞行任务的增删改查 |
| droneApi | droneApi.js | /api/v1/drones | ✅ 新增 | 无人机管理、状态查询 |
| airspaceApi | airspaceApi.js | /api/v1/airspaces | ✅ 新增 | 空域管理、冲突检测 |
| permitApi | permitApi.js | /api/v1/flight-permits | ✅ 新增 | 飞行许可申请、审批 |
| emergencyApi | emergencyApi.js | /api/v1/emergency | ✅ 完整 | 应急事件、降落点查询 |
| weatherApi | weatherApi.js | /api/v1/weather | ✅ 完整 | 天气查询、飞行安全检查 |
| mapApi | mapApi.js | /api/v1/map | ✅ 新增 | 地图标记、路线规划 |
| monitorApi | systemMonitorApi.js | /api/v1 | ✅ 完整 | 系统健康、性能监控 |

---

## 快速开始

### 1. 导入API服务

#### 方式一: 按需导入

```javascript
// 在Vue组件中导入需要的API服务
import { taskApi, droneApi, authApi } from '@/services'

export default {
  methods: {
    async loadTasks() {
      const response = await taskApi.getTasks()
      console.log(response.data)
    }
  }
}
```

#### 方式二: 导入全部

```javascript
import api from '@/services'

export default {
  methods: {
    async loadData() {
      const tasks = await api.taskApi.getTasks()
      const drones = await api.droneApi.getDrones()
    }
  }
}
```

### 2. 基本使用示例

```javascript
import { taskApi } from '@/services'

export default {
  data() {
    return {
      tasks: [],
      loading: false,
      error: null
    }
  },
  methods: {
    async fetchTasks() {
      this.loading = true
      this.error = null

      try {
        const response = await taskApi.getTasks({
          status: 'pending',
          page: 1,
          page_size: 10
        })

        if (response.success) {
          this.tasks = response.data.tasks
        }
      } catch (error) {
        this.error = error.message
        console.error('获取任务失败:', error)
      } finally {
        this.loading = false
      }
    }
  }
}
```

---

## 详细使用说明

### 1️⃣ 认证服务 (authApi)

#### 登录

```javascript
import { authApi } from '@/services'

// 用户登录
const response = await authApi.login({
  username: 'admin',
  password: 'admin123'
})

// response.data 包含:
// - access_token: 访问令牌
// - refresh_token: 刷新令牌
// - user: 用户信息
```

#### 获取当前用户

```javascript
const response = await authApi.getCurrentUser()
console.log(response.data.user)
```

#### 登出

```javascript
await authApi.logout()
```

---

### 2️⃣ 飞行任务管理 (taskApi)

#### 获取任务列表

```javascript
import { taskApi } from '@/services'

// 获取所有任务
const response = await taskApi.getTasks()

// 带分页和筛选
const response = await taskApi.getTasks({
  page: 1,
  page_size: 20,
  status: 'pending',  // pending, in_progress, completed, cancelled
  search: '故宫'
})

console.log(response.data.tasks)  // 任务数组
```

#### 创建任务

```javascript
const newTask = {
  name: '沈阳故宫文物巡检',
  description: '使用DJI-001进行故宫文物巡检任务',
  task_type: 'inspection',
  drone_id: 1,
  airspace_id: 1,
  start_time: '2025-10-14 14:00:00',
  end_time: '2025-10-14 16:00:00',
  route: {
    start: { lat: 41.796700, lng: 123.451200 },
    end: { lat: 41.806700, lng: 123.461200 }
  },
  altitude: 120.0
}

const response = await taskApi.createTask(newTask)
console.log('任务ID:', response.data.id)
```

#### 更新任务

```javascript
await taskApi.updateTask(taskId, {
  status: 'in_progress',
  progress: 50
})
```

#### 删除任务

```javascript
await taskApi.deleteTask(taskId)
```

---

### 3️⃣ 无人机管理 (droneApi)

#### 获取无人机列表

```javascript
import { droneApi } from '@/services'

// 获取所有无人机
const response = await droneApi.getDrones()
console.log(response.data.drones)        // 无人机数组
console.log(response.data.total)          // 总数
console.log(response.data.active_count)   // 活跃数量
```

#### 获取活跃无人机

```javascript
const response = await droneApi.getActiveDrones()
```

#### 获取低电量无人机

```javascript
// 获取电量低于20%的无人机
const response = await droneApi.getLowBatteryDrones(20)
console.log(`找到 ${response.data.count} 架低电量无人机`)
```

#### 获取无人机详细状态

```javascript
const response = await droneApi.getDroneStatus(droneId)

// response.data 包含:
console.log(response.data.battery_status)  // excellent/good/low/critical
console.log(response.data.warnings)        // 警告信息数组
console.log(response.data.is_operational)  // 是否可用
```

#### 查找附近的无人机

```javascript
const response = await droneApi.getNearbyDrones(
  41.7967,  // 纬度
  123.4512, // 经度
  5         // 搜索半径(公里)
)

// 返回按距离排序的无人机列表
response.data.drones.forEach(drone => {
  console.log(`${drone.name} - ${drone.distance.toFixed(2)} km`)
})
```

#### 获取无人机统计

```javascript
const response = await droneApi.getDroneStatistics()

console.log(response.data.status)           // 按状态统计
console.log(response.data.models)           // 按型号统计
console.log(response.data.battery.average)  // 平均电量
console.log(response.data.operational_rate) // 运行率
```

---

### 4️⃣ 空域管理 (airspaceApi)

#### 获取空域列表

```javascript
import { airspaceApi } from '@/services'

const response = await airspaceApi.getAirspaces()
console.log(response.data.airspaces)
```

#### 创建空域

```javascript
const newAirspace = {
  name: '沈阳故宫管控空域',
  type: 'restricted',  // restricted, controlled, uncontrolled
  description: '文物保护管控空域',
  status: 'active',
  north_lat: 41.8067,
  south_lat: 41.7867,
  east_lng: 123.4612,
  west_lng: 123.4412,
  min_altitude: 0.0,
  max_altitude: 200.0,
  max_concurrent_flights: 10
}

await airspaceApi.createAirspace(newAirspace)
```

#### 更新空域

```javascript
await airspaceApi.updateAirspace(airspaceId, {
  status: 'inactive',
  max_concurrent_flights: 5
})
```

#### 删除空域

```javascript
await airspaceApi.deleteAirspace(airspaceId)
```

#### 查找包含指定位置的空域

```javascript
const response = await airspaceApi.findAirspacesContainingPoint(
  41.7967,  // 纬度
  123.4512  // 经度
)

console.log(`该位置在 ${response.data.count} 个空域内`)
response.data.airspaces.forEach(airspace => {
  console.log(`- ${airspace.name} (${airspace.type})`)
})
```

#### 获取空域统计

```javascript
const response = await airspaceApi.getAirspaceStatistics()

console.log(response.data.total)                  // 总数
console.log(response.data.by_type)                // 按类型统计
console.log(response.data.avg_max_altitude)       // 平均最大高度
console.log(response.data.total_max_concurrent)   // 总并发容量
```

---

### 5️⃣ 飞行许可 (permitApi)

#### 申请飞行许可

```javascript
import { permitApi } from '@/services'

const permitApplication = {
  task_id: 7,
  airspace_id: 1,
  start_time: '2025-10-14 14:00:00',
  end_time: '2025-10-14 16:00:00',
  remarks: '沈阳故宫文物巡检飞行许可申请'
}

const response = await permitApi.applyPermit(permitApplication)
console.log('许可编号:', response.data.permit_code)
```

#### 获取许可列表

```javascript
const response = await permitApi.getPermits({
  status: 'pending'  // pending, approved, rejected, revoked
})
```

#### 批准许可

```javascript
await permitApi.approvePermit(permitId, {
  approved_by: userId,
  approval_notes: '符合安全要求，批准飞行'
})
```

#### 拒绝许可

```javascript
await permitApi.rejectPermit(permitId, {
  reason: '天气条件不适合飞行，建议改期'
})
```

#### 获取待审批许可

```javascript
const response = await permitApi.getPendingPermits()
console.log(`有 ${response.data.count} 个许可待审批`)
```

#### 获取我的许可

```javascript
const response = await permitApi.getMyPermits(currentUserId)
```

#### 获取许可统计

```javascript
const response = await permitApi.getPermitStatistics()

console.log('总数:', response.data.total)
console.log('待审批:', response.data.by_status.pending)
console.log('已批准:', response.data.by_status.approved)
console.log('今日申请:', response.data.today)
console.log('本周申请:', response.data.this_week)
```

---

### 6️⃣ 应急响应 (emergencyApi)

#### 创建应急事件

```javascript
import { emergencyApi } from '@/services'

const event = {
  event_type: 'low_battery',  // low_battery, mechanical_failure, weather, etc.
  severity: 'high',            // low, medium, high, critical
  drone_id: 3,
  location: { lat: 41.7967, lng: 123.4512 },
  description: '无人机电量不足10%，需要紧急降落'
}

const response = await emergencyApi.createEvent(event)
console.log('事件ID:', response.data.id)
```

#### 获取事件列表

```javascript
const response = await emergencyApi.getEvents({
  status: 'pending',  // pending, responding, resolved, cancelled
  severity: 'high',
  page: 1,
  page_size: 20
})
```

#### 响应事件

```javascript
await emergencyApi.respondToEvent(eventId, {
  responder_id: userId,
  response_plan: '指引无人机前往最近降落点',
  eta: '2分钟'
})
```

#### 解决事件

```javascript
await emergencyApi.resolveEvent(eventId, {
  resolution: '无人机已安全降落，正在回收'
})
```

#### 查找最近的应急降落点

```javascript
const response = await emergencyApi.getNearestLandingPoints(
  41.7967,  // 纬度
  123.4512, // 经度
  3,        // 返回数量
  5000      // 搜索半径(米)
)

response.data.landing_points.forEach(point => {
  console.log(`${point.name} - ${point.distance.toFixed(0)}米`)
  console.log(`类型: ${point.type}, 安全等级: ${point.safety_rating}`)
  console.log(`24小时可达: ${point.accessible_24h ? '是' : '否'}`)
})
```

#### 获取应急统计

```javascript
const response = await emergencyApi.getStatistics()
```

---

### 7️⃣ 天气服务 (weatherApi)

#### 获取当前天气(按城市名)

```javascript
import { weatherApi } from '@/services'

const response = await weatherApi.getCurrentWeather('沈阳')
console.log(response.data.weather)
```

#### 获取当前天气(按坐标)

```javascript
const response = await weatherApi.getCurrentWeatherByCoords(
  41.7967,  // 纬度
  123.4512  // 经度
)
```

#### 获取天气预报

```javascript
const response = await weatherApi.getForecast(
  41.7967,  // 纬度
  123.4512, // 经度
  5         // 预报天数
)

response.data.forecast.forEach(day => {
  console.log(`${day.date}: ${day.dayweather}, ${day.daytemp}°C`)
})
```

#### 检查飞行安全

```javascript
const response = await weatherApi.checkFlightSafety(
  41.7967,  // 纬度
  123.4512  // 经度
)

if (response.data.safety.safe) {
  console.log('天气条件适合飞行')
} else {
  console.log('不适合飞行:', response.data.safety.reasons)
}
```

---

### 8️⃣ 地图服务 (mapApi)

#### 地理编码(地址→坐标)

```javascript
import { mapApi } from '@/services'

const response = await mapApi.geocode('沈阳故宫')

console.log('坐标:', response.data.location)
```

#### 获取地图标记

```javascript
const response = await mapApi.getMarkers()
```

#### 创建标记

```javascript
await mapApi.createMarker({
  name: '应急降落点1',
  type: 'landing_point',
  lat: 41.7967,
  lng: 123.4512,
  description: '沈阳故宫东门应急降落区'
})
```

#### 删除标记

```javascript
await mapApi.deleteMarker(markerId)
```

#### 获取路线

```javascript
const response = await mapApi.getRoute(
  41.7967, 123.4512,  // 起点
  41.8067, 123.4612   // 终点
)
```

---

### 9️⃣ 系统监控 (monitorApi)

#### 系统健康检查

```javascript
import { monitorApi } from '@/services'

const response = await monitorApi.checkHealth()

if (response.data.status === 'ok') {
  console.log('系统运行正常')
}
```

#### 获取系统信息

```javascript
const response = await monitorApi.getSystemInfo()

console.log('系统名称:', response.data.name)
console.log('版本:', response.data.version)
console.log('运行时间:', response.data.uptime)
```

#### 获取系统统计

```javascript
const response = await monitorApi.getStatistics()
```

---

## 错误处理

### 统一的错误格式

所有API错误都会被格式化为以下结构:

```javascript
{
  success: false,
  code: 'UNAUTHORIZED',  // 错误代码
  message: '未授权，请重新登录',  // 中文错误消息
  error: Error对象  // 原始错误对象
}
```

### 错误代码列表

| 错误代码 | HTTP状态码 | 说明 |
|---------|-----------|------|
| BAD_REQUEST | 400 | 请求参数错误 |
| UNAUTHORIZED | 401 | 未授权，需要登录 |
| FORBIDDEN | 403 | 无权限执行此操作 |
| NOT_FOUND | 404 | 资源不存在 |
| CONFLICT | 409 | 资源冲突 |
| INTERNAL_ERROR | 500 | 服务器内部错误 |
| NETWORK_ERROR | - | 网络连接失败 |
| REQUEST_ERROR | - | 请求配置错误 |

### 错误处理最佳实践

#### 1. 使用try-catch

```javascript
async fetchData() {
  this.loading = true
  this.error = null

  try {
    const response = await taskApi.getTasks()
    this.tasks = response.data.tasks
  } catch (error) {
    // 显示友好的错误消息
    this.error = error.message

    // 根据错误代码采取不同行动
    if (error.code === 'UNAUTHORIZED') {
      // 跳转到登录页
      this.$router.push('/login')
    }
  } finally {
    this.loading = false
  }
}
```

#### 2. 全局错误处理

在Vue组件中可以使用全局错误处理器:

```javascript
// main.js
Vue.config.errorHandler = (err, vm, info) => {
  console.error('Global error:', err)

  if (err.code === 'UNAUTHORIZED') {
    router.push('/login')
  } else {
    // 显示全局错误提示
    vm.$message.error(err.message || '操作失败')
  }
}
```

#### 3. 使用Vue组合式API

```javascript
import { ref } from 'vue'
import { taskApi } from '@/services'

export default {
  setup() {
    const tasks = ref([])
    const loading = ref(false)
    const error = ref(null)

    const fetchTasks = async () => {
      loading.value = true
      error.value = null

      try {
        const response = await taskApi.getTasks()
        tasks.value = response.data.tasks
      } catch (err) {
        error.value = err.message
      } finally {
        loading.value = false
      }
    }

    return { tasks, loading, error, fetchTasks }
  }
}
```

---

## 最佳实践

### 1. 使用加载状态

```javascript
data() {
  return {
    loading: false,
    tasks: []
  }
},
methods: {
  async loadTasks() {
    this.loading = true
    try {
      const response = await taskApi.getTasks()
      this.tasks = response.data.tasks
    } finally {
      this.loading = false
    }
  }
}
```

### 2. 防抖和节流

```javascript
import { debounce } from 'lodash'

export default {
  methods: {
    // 搜索防抖
    searchTasks: debounce(async function(keyword) {
      const response = await taskApi.getTasks({ search: keyword })
      this.tasks = response.data.tasks
    }, 300)
  }
}
```

### 3. 请求取消

对于可能重复触发的请求,建议使用axios的取消功能:

```javascript
data() {
  return {
    cancelToken: null
  }
},
methods: {
  async loadTasks() {
    // 取消之前的请求
    if (this.cancelToken) {
      this.cancelToken.cancel('New request started')
    }

    // 创建新的取消令牌
    this.cancelToken = axios.CancelToken.source()

    try {
      const response = await taskApi.getTasks({
        cancelToken: this.cancelToken.token
      })
      this.tasks = response.data.tasks
    } catch (error) {
      if (axios.isCancel(error)) {
        console.log('Request cancelled')
      }
    }
  }
}
```

### 4. 数据缓存

对于不常变化的数据,可以使用Vuex缓存:

```javascript
// store/modules/drones.js
export default {
  state: {
    drones: [],
    lastUpdate: null
  },
  mutations: {
    SET_DRONES(state, drones) {
      state.drones = drones
      state.lastUpdate = Date.now()
    }
  },
  actions: {
    async fetchDrones({ commit, state }) {
      // 如果数据在5分钟内已更新,直接返回
      if (state.lastUpdate && Date.now() - state.lastUpdate < 300000) {
        return state.drones
      }

      const response = await droneApi.getDrones()
      commit('SET_DRONES', response.data.drones)
      return response.data.drones
    }
  }
}
```

### 5. 并发请求

```javascript
async loadAllData() {
  this.loading = true

  try {
    // 并发请求多个API
    const [tasksRes, dronesRes, airspacesRes] = await Promise.all([
      taskApi.getTasks(),
      droneApi.getDrones(),
      airspaceApi.getAirspaces()
    ])

    this.tasks = tasksRes.data.tasks
    this.drones = dronesRes.data.drones
    this.airspaces = airspacesRes.data.airspaces
  } catch (error) {
    console.error('加载数据失败:', error)
  } finally {
    this.loading = false
  }
}
```

---

## 总结

本API服务层提供了:

✅ **9个完整的API服务类** - 覆盖所有业务模块
✅ **统一的调用接口** - 相同的使用模式
✅ **自动认证管理** - 无需手动添加Token
✅ **完善的错误处理** - 友好的中文错误提示
✅ **丰富的工具方法** - 统计、搜索、筛选等

开始使用这些API服务,让您的前端开发更加高效!

---

**文档版本**: v1.0
**最后更新**: 2025-10-14
**维护者**: Claude Code
