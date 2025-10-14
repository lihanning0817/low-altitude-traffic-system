/**
 * 无人机管理API服务
 * Drone Management API Service
 *
 * 提供无人机的管理功能
 * - 获取无人机列表
 * - 获取无人机详情
 * - 获取无人机实时状态
 * - 获取无人机统计信息
 */

import axios from 'axios'

class DroneApiService {
  constructor() {
    this.baseURL = '/api/v1/drones'

    // 创建axios实例
    this.api = axios.create({
      baseURL: this.baseURL,
      timeout: 10000,
      headers: {
        'Content-Type': 'application/json'
      }
    })

    // 请求拦截器 - 自动添加认证Token
    this.api.interceptors.request.use(
      (config) => {
        const token = localStorage.getItem('access_token')
        if (token) {
          config.headers.Authorization = `Bearer ${token}`
        }
        return config
      },
      (error) => {
        return Promise.reject(error)
      }
    )

    // 响应拦截器 - 统一处理响应和错误
    this.api.interceptors.response.use(
      (response) => {
        return response.data
      },
      (error) => {
        return this.handleError(error)
      }
    )
  }

  /**
   * 获取无人机列表
   * @param {Object} params - 查询参数
   * @param {string} params.status - 无人机状态 (active, inactive, maintenance)
   * @param {number} params.battery_min - 最小电量
   * @param {string} params.model - 无人机型号
   * @returns {Promise<Object>} 无人机列表数据
   */
  async getDrones(params = {}) {
    try {
      const response = await this.api.get('/', { params })

      // 处理响应数据
      const drones = response.data?.drones || []
      const total = response.data?.total || drones.length
      const activeCount = response.data?.active_count ||
        drones.filter(d => d.status === 'active').length

      return {
        success: true,
        data: {
          drones: drones,
          total: total,
          active_count: activeCount,
          inactive_count: total - activeCount
        },
        message: response.message || '获取无人机列表成功'
      }
    } catch (error) {
      console.error('[DroneAPI] 获取无人机列表失败:', error)
      throw error
    }
  }

  /**
   * 获取无人机详情
   * @param {number|string} droneId - 无人机ID
   * @returns {Promise<Object>} 无人机详情数据
   */
  async getDroneById(droneId) {
    try {
      if (!droneId) {
        throw new Error('无人机ID不能为空')
      }

      const response = await this.api.get(`/${droneId}`)
      return {
        success: true,
        data: response.data,
        message: response.message || '获取无人机详情成功'
      }
    } catch (error) {
      console.error(`[DroneAPI] 获取无人机详情失败 (ID: ${droneId}):`, error)
      throw error
    }
  }

  /**
   * 获取活跃的无人机列表
   * @returns {Promise<Object>} 活跃无人机数据
   */
  async getActiveDrones() {
    try {
      const response = await this.getDrones({ status: 'active' })
      return {
        success: true,
        data: response.data.drones.filter(drone => drone.status === 'active'),
        message: '获取活跃无人机列表成功'
      }
    } catch (error) {
      console.error('[DroneAPI] 获取活跃无人机失败:', error)
      throw error
    }
  }

  /**
   * 获取低电量无人机列表
   * @param {number} threshold - 电量阈值 (默认20%)
   * @returns {Promise<Object>} 低电量无人机数据
   */
  async getLowBatteryDrones(threshold = 20) {
    try {
      const response = await this.getDrones()
      const lowBatteryDrones = response.data.drones.filter(
        drone => drone.battery <= threshold
      )

      return {
        success: true,
        data: {
          drones: lowBatteryDrones,
          count: lowBatteryDrones.length,
          threshold: threshold
        },
        message: `找到 ${lowBatteryDrones.length} 架低电量无人机`
      }
    } catch (error) {
      console.error('[DroneAPI] 获取低电量无人机失败:', error)
      throw error
    }
  }

  /**
   * 获取无人机实时状态
   * @param {number|string} droneId - 无人机ID
   * @returns {Promise<Object>} 实时状态数据
   */
  async getDroneStatus(droneId) {
    try {
      if (!droneId) {
        throw new Error('无人机ID不能为空')
      }

      const response = await this.getDroneById(droneId)
      const drone = response.data

      // 分析无人机状态
      const statusAnalysis = {
        drone_id: drone.drone_id,
        name: drone.name,
        status: drone.status,
        battery: drone.battery,
        battery_status: this.analyzeBatteryStatus(drone.battery),
        location: drone.location,
        last_update: drone.last_update,
        is_operational: drone.status === 'active' && drone.battery > 20,
        warnings: this.generateWarnings(drone)
      }

      return {
        success: true,
        data: statusAnalysis,
        message: '获取无人机状态成功'
      }
    } catch (error) {
      console.error(`[DroneAPI] 获取无人机状态失败 (ID: ${droneId}):`, error)
      throw error
    }
  }

  /**
   * 分析电池状态
   * @param {number} battery - 电池电量百分比
   * @returns {string} 电池状态描述
   */
  analyzeBatteryStatus(battery) {
    if (battery >= 80) return 'excellent'
    if (battery >= 50) return 'good'
    if (battery >= 20) return 'low'
    if (battery >= 10) return 'critical'
    return 'emergency'
  }

  /**
   * 生成无人机警告信息
   * @param {Object} drone - 无人机数据
   * @returns {Array<string>} 警告信息数组
   */
  generateWarnings(drone) {
    const warnings = []

    if (drone.battery <= 10) {
      warnings.push('电量极低，请立即充电')
    } else if (drone.battery <= 20) {
      warnings.push('电量不足，建议尽快充电')
    }

    if (drone.status === 'maintenance') {
      warnings.push('无人机正在维护中')
    }

    if (drone.status === 'inactive') {
      warnings.push('无人机处于非活跃状态')
    }

    // 检查最后更新时间 (超过1小时未更新)
    if (drone.last_update) {
      const lastUpdate = new Date(drone.last_update * 1000)
      const now = new Date()
      const hoursSinceUpdate = (now - lastUpdate) / (1000 * 60 * 60)

      if (hoursSinceUpdate > 1) {
        warnings.push(`数据已 ${Math.floor(hoursSinceUpdate)} 小时未更新`)
      }
    }

    return warnings
  }

  /**
   * 获取无人机统计信息
   * @returns {Promise<Object>} 统计数据
   */
  async getDroneStatistics() {
    try {
      const response = await this.getDrones()
      const drones = response.data.drones

      // 按状态统计
      const statusStats = {
        active: 0,
        inactive: 0,
        maintenance: 0,
        total: drones.length
      }

      // 按型号统计
      const modelStats = {}

      // 电量统计
      let totalBattery = 0
      let lowBatteryCount = 0

      drones.forEach(drone => {
        // 状态统计
        if (statusStats.hasOwnProperty(drone.status)) {
          statusStats[drone.status]++
        }

        // 型号统计
        if (drone.model) {
          modelStats[drone.model] = (modelStats[drone.model] || 0) + 1
        }

        // 电量统计
        totalBattery += drone.battery || 0
        if (drone.battery <= 20) {
          lowBatteryCount++
        }
      })

      const avgBattery = drones.length > 0 ? totalBattery / drones.length : 0

      return {
        success: true,
        data: {
          status: statusStats,
          models: modelStats,
          battery: {
            average: Math.round(avgBattery * 10) / 10,
            low_battery_count: lowBatteryCount
          },
          operational_rate: drones.length > 0 ?
            Math.round((statusStats.active / drones.length) * 100) : 0
        },
        message: '获取无人机统计成功'
      }
    } catch (error) {
      console.error('[DroneAPI] 获取无人机统计失败:', error)
      throw error
    }
  }

  /**
   * 查找附近的无人机
   * @param {number} lat - 纬度
   * @param {number} lng - 经度
   * @param {number} radius - 搜索半径 (公里，默认5公里)
   * @returns {Promise<Object>} 附近的无人机列表
   */
  async getNearbyDrones(lat, lng, radius = 5) {
    try {
      if (!lat || !lng) {
        throw new Error('经纬度不能为空')
      }

      const response = await this.getDrones()
      const drones = response.data.drones

      // 计算每个无人机到指定位置的距离
      const dronesWithDistance = drones
        .filter(drone => drone.location && drone.location.lat && drone.location.lng)
        .map(drone => {
          const distance = this.calculateDistance(
            lat, lng,
            drone.location.lat, drone.location.lng
          )
          return {
            ...drone,
            distance: distance
          }
        })
        .filter(drone => drone.distance <= radius)
        .sort((a, b) => a.distance - b.distance)

      return {
        success: true,
        data: {
          drones: dronesWithDistance,
          count: dronesWithDistance.length,
          search_radius: radius
        },
        message: `在半径 ${radius}km 内找到 ${dronesWithDistance.length} 架无人机`
      }
    } catch (error) {
      console.error('[DroneAPI] 查找附近无人机失败:', error)
      throw error
    }
  }

  /**
   * 计算两点之间的距离 (Haversine公式)
   * @param {number} lat1 - 纬度1
   * @param {number} lng1 - 经度1
   * @param {number} lat2 - 纬度2
   * @param {number} lng2 - 经度2
   * @returns {number} 距离 (公里)
   */
  calculateDistance(lat1, lng1, lat2, lng2) {
    const R = 6371 // 地球半径 (公里)
    const dLat = this.toRadians(lat2 - lat1)
    const dLng = this.toRadians(lng2 - lng1)

    const a =
      Math.sin(dLat / 2) * Math.sin(dLat / 2) +
      Math.cos(this.toRadians(lat1)) *
        Math.cos(this.toRadians(lat2)) *
        Math.sin(dLng / 2) *
        Math.sin(dLng / 2)

    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a))
    return R * c
  }

  /**
   * 角度转弧度
   * @param {number} degrees - 角度
   * @returns {number} 弧度
   */
  toRadians(degrees) {
    return degrees * (Math.PI / 180)
  }

  /**
   * 统一错误处理
   * @param {Error} error - 错误对象
   * @returns {Promise<never>} 拒绝的Promise
   */
  handleError(error) {
    let errorMessage = '请求失败，请稍后重试'
    let errorCode = 'UNKNOWN_ERROR'

    if (error.response) {
      const { status, data } = error.response

      switch (status) {
        case 400:
          errorMessage = data.message || '请求参数错误'
          errorCode = 'BAD_REQUEST'
          break
        case 401:
          errorMessage = '未授权，请重新登录'
          errorCode = 'UNAUTHORIZED'
          break
        case 403:
          errorMessage = '无权限访问无人机信息'
          errorCode = 'FORBIDDEN'
          break
        case 404:
          errorMessage = '无人机不存在'
          errorCode = 'NOT_FOUND'
          break
        case 500:
          errorMessage = '服务器内部错误'
          errorCode = 'INTERNAL_ERROR'
          break
        default:
          errorMessage = data.message || errorMessage
      }
    } else if (error.request) {
      errorMessage = '网络连接失败，请检查网络'
      errorCode = 'NETWORK_ERROR'
    } else if (error.message) {
      errorMessage = error.message
      errorCode = 'REQUEST_ERROR'
    }

    console.error('[DroneAPI] Error:', {
      code: errorCode,
      message: errorMessage,
      originalError: error
    })

    return Promise.reject({
      success: false,
      code: errorCode,
      message: errorMessage,
      error: error
    })
  }
}

// 创建单例实例
const droneApi = new DroneApiService()

// 导出实例和类
export default droneApi
export { DroneApiService }
