/**
 * 飞行冲突检测 API 服务
 * Flight Conflict Detection API Service
 */

import axios from 'axios'
import { getAuthToken } from './authApi'

// 创建 axios 实例
const apiClient = axios.create({
  baseURL: 'http://localhost:8081/api/v1/conflict-detection',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

// 请求拦截器 - 添加认证 token
apiClient.interceptors.request.use(
  (config) => {
    const token = getAuthToken()
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// 响应拦截器 - 统一处理响应
apiClient.interceptors.response.use(
  (response) => {
    return {
      success: true,
      data: response.data.data || response.data,
      message: response.data.message || '操作成功'
    }
  },
  (error) => {
    console.error('Conflict Detection API Error:', error)

    const errorResponse = {
      success: false,
      code: error.response?.status || 500,
      message: error.response?.data?.message || error.message || '请求失败'
    }

    // 401 未授权
    if (error.response?.status === 401) {
      errorResponse.code = 'UNAUTHORIZED'
      errorResponse.message = '未授权,请重新登录'
    }

    // 网络错误
    if (error.code === 'ECONNABORTED' || error.message === 'Network Error') {
      errorResponse.code = 'NETWORK_ERROR'
      errorResponse.message = '网络连接失败'
    }

    return Promise.reject(errorResponse)
  }
)

/**
 * 飞行冲突检测 API 服务类
 */
class ConflictDetectionApiService {
  /**
   * 检测飞行任务冲突
   * @param {Object} flightData - 飞行数据
   * @returns {Promise}
   */
  async detectFlightConflicts(flightData) {
    if (!this.validateFlightData(flightData)) {
      throw new Error('飞行数据验证失败')
    }

    try {
      return await apiClient.post('/flights', flightData)
    } catch (error) {
      throw error
    }
  }

  /**
   * 批量检测多个飞行任务冲突
   * @param {Array} flightsData - 多个飞行数据
   * @returns {Promise}
   */
  async detectMultipleFlightConflicts(flightsData) {
    if (!Array.isArray(flightsData) || flightsData.length === 0) {
      throw new Error('飞行数据列表不能为空')
    }

    try {
      return await apiClient.post('/flights/batch', { flights: flightsData })
    } catch (error) {
      throw error
    }
  }

  /**
   * 获取所有冲突记录
   * @param {Object} params - 查询参数
   * @returns {Promise}
   */
  async getConflicts(params = {}) {
    try {
      return await apiClient.get('/conflicts', { params })
    } catch (error) {
      throw error
    }
  }

  /**
   * 根据ID获取单个冲突详情
   * @param {number} id - 冲突ID
   * @returns {Promise}
   */
  async getConflictById(id) {
    if (!id) {
      throw new Error('冲突ID不能为空')
    }

    try {
      return await apiClient.get(`/conflicts/${id}`)
    } catch (error) {
      throw error
    }
  }

  /**
   * 解决冲突
   * @param {number} id - 冲突ID
   * @param {Object} resolutionData - 解决方案数据
   * @returns {Promise}
   */
  async resolveConflict(id, resolutionData = {}) {
    if (!id) {
      throw new Error('冲突ID不能为空')
    }

    try {
      return await apiClient.post(`/conflicts/${id}/resolve`, resolutionData)
    } catch (error) {
      throw error
    }
  }

  /**
   * 获取未解决的冲突
   * @returns {Promise}
   */
  async getUnresolvedConflicts() {
    try {
      return await apiClient.get('/conflicts', {
        params: { status: 'unresolved' }
      })
    } catch (error) {
      throw error
    }
  }

  /**
   * 获取高危冲突(严重程度为high)
   * @returns {Promise}
   */
  async getHighSeverityConflicts() {
    try {
      return await apiClient.get('/conflicts', {
        params: { severity: 'high' }
      })
    } catch (error) {
      throw error
    }
  }

  /**
   * 根据飞行任务ID获取相关冲突
   * @param {number} flightId - 飞行任务ID
   * @returns {Promise}
   */
  async getConflictsByFlightId(flightId) {
    if (!flightId) {
      throw new Error('飞行任务ID不能为空')
    }

    try {
      return await apiClient.get('/conflicts', {
        params: { flight_id: flightId }
      })
    } catch (error) {
      throw error
    }
  }

  /**
   * 获取冲突统计信息
   * @param {Object} params - 查询参数(时间范围等)
   * @returns {Promise}
   */
  async getConflictStatistics(params = {}) {
    try {
      return await apiClient.get('/conflicts/statistics', { params })
    } catch (error) {
      throw error
    }
  }

  /**
   * 验证飞行数据
   * @param {Object} flightData - 飞行数据
   * @returns {boolean}
   */
  validateFlightData(flightData) {
    if (!flightData) {
      throw new Error('飞行数据不能为空')
    }

    const requiredFields = ['task_id', 'start_time', 'end_time']

    for (const field of requiredFields) {
      if (!flightData[field]) {
        throw new Error(`${field} 是必填字段`)
      }
    }

    // 验证时间范围
    const startTime = new Date(flightData.start_time)
    const endTime = new Date(flightData.end_time)

    if (isNaN(startTime.getTime()) || isNaN(endTime.getTime())) {
      throw new Error('时间格式无效')
    }

    if (startTime >= endTime) {
      throw new Error('结束时间必须晚于开始时间')
    }

    // 验证路线数据(如果有)
    if (flightData.route) {
      if (!flightData.route.start || !flightData.route.end) {
        throw new Error('路线必须包含起点和终点')
      }

      // 验证起点
      if (!this.validateCoordinate(flightData.route.start)) {
        throw new Error('起点坐标无效')
      }

      // 验证终点
      if (!this.validateCoordinate(flightData.route.end)) {
        throw new Error('终点坐标无效')
      }
    }

    // 验证高度(如果有)
    if (flightData.altitude !== undefined) {
      if (flightData.altitude < 0 || flightData.altitude > 1000) {
        throw new Error('飞行高度必须在0-1000米之间')
      }
    }

    return true
  }

  /**
   * 验证坐标
   * @param {Object} coordinate - 坐标对象 {lat, lng}
   * @returns {boolean}
   */
  validateCoordinate(coordinate) {
    if (!coordinate || typeof coordinate !== 'object') {
      return false
    }

    const { lat, lng } = coordinate

    if (lat === undefined || lng === undefined) {
      return false
    }

    // 验证纬度范围
    if (lat < -90 || lat > 90) {
      return false
    }

    // 验证经度范围
    if (lng < -180 || lng > 180) {
      return false
    }

    return true
  }

  /**
   * 获取冲突严重程度文本
   * @param {string} severity - 严重程度
   * @returns {string}
   */
  getSeverityText(severity) {
    const severityMap = {
      low: '低',
      medium: '中',
      high: '高',
      critical: '紧急'
    }
    return severityMap[severity] || severity
  }

  /**
   * 获取冲突状态文本
   * @param {string} status - 状态
   * @returns {string}
   */
  getStatusText(status) {
    const statusMap = {
      unresolved: '未解决',
      resolving: '解决中',
      resolved: '已解决',
      ignored: '已忽略'
    }
    return statusMap[status] || status
  }

  /**
   * 获取冲突类型文本
   * @param {string} type - 冲突类型
   * @returns {string}
   */
  getConflictTypeText(type) {
    const typeMap = {
      time: '时间冲突',
      space: '空间冲突',
      airspace: '空域冲突',
      route: '路线冲突'
    }
    return typeMap[type] || type
  }

  /**
   * 计算两个时间段是否重叠
   * @param {Date} start1 - 时间段1开始
   * @param {Date} end1 - 时间段1结束
   * @param {Date} start2 - 时间段2开始
   * @param {Date} end2 - 时间段2结束
   * @returns {boolean}
   */
  hasTimeOverlap(start1, end1, start2, end2) {
    return start1 < end2 && start2 < end1
  }

  /**
   * 格式化冲突数据用于展示
   * @param {Object} conflict - 冲突数据
   * @returns {Object}
   */
  formatConflictForDisplay(conflict) {
    return {
      ...conflict,
      severity_text: this.getSeverityText(conflict.severity),
      status_text: this.getStatusText(conflict.status),
      type_text: this.getConflictTypeText(conflict.conflict_type),
      detected_at_formatted: conflict.detected_at
        ? new Date(conflict.detected_at).toLocaleString('zh-CN')
        : '',
      resolved_at_formatted: conflict.resolved_at
        ? new Date(conflict.resolved_at).toLocaleString('zh-CN')
        : ''
    }
  }
}

// 导出单例
const conflictDetectionApi = new ConflictDetectionApiService()
export default conflictDetectionApi
