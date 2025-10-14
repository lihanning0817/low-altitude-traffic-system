/**
 * API服务统一导出
 * Unified API Services Export
 *
 * 这个文件统一导出所有API服务类，方便在Vue组件中导入使用
 *
 * 使用示例:
 * import { taskApi, droneApi, authApi } from '@/services'
 *
 * 或者导入全部:
 * import api from '@/services'
 * api.taskApi.getTasks()
 */

// 认证服务
export { default as authApi } from './authApi.js'

// 飞行任务管理
export { default as taskApi } from './flightTaskApi.js'

// 无人机管理
export { default as droneApi } from './droneApi.js'

// 空域管理
export { default as airspaceApi } from './airspaceApi.js'

// 飞行许可
export { default as permitApi } from './permitApi.js'

// 应急响应
export { default as emergencyApi } from './emergencyApi.js'

// 天气服务
export { default as weatherApi } from './weatherApi.js'

// 地图服务
export { default as mapApi } from './mapApi.js'

// 系统监控
export { default as monitorApi } from './systemMonitorApi.js'

/**
 * 默认导出所有API服务实例
 */
import authApi from './authApi.js'
import taskApi from './flightTaskApi.js'
import droneApi from './droneApi.js'
import airspaceApi from './airspaceApi.js'
import permitApi from './permitApi.js'
import emergencyApi from './emergencyApi.js'
import weatherApi from './weatherApi.js'
import mapApi from './mapApi.js'
import monitorApi from './systemMonitorApi.js'

export default {
  authApi,
  taskApi,
  droneApi,
  airspaceApi,
  permitApi,
  emergencyApi,
  weatherApi,
  mapApi,
  monitorApi
}
