/**
 * 应急响应API服务
 * 与后端EmergencyController交互
 */

import axios from 'axios'
import { ElMessage } from 'element-plus'
import authApi from './authApi'

class EmergencyApiService {
  constructor() {
    this.baseURL = 'http://localhost:8081/api/v1/emergency'

    // 创建axios实例
    this.api = axios.create({
      baseURL: this.baseURL,
      timeout: 10000,
      headers: {
        'Content-Type': 'application/json; charset=utf-8'
      }
    })

    // 请求拦截器 - 自动添加JWT token
    this.api.interceptors.request.use(
      (config) => {
        const token = authApi.getAccessToken()
        if (token) {
          config.headers.Authorization = `Bearer ${token}`
        }
        return config
      },
      (error) => {
        return Promise.reject(error)
      }
    )

    // 响应拦截器 - 统一处理错误
    this.api.interceptors.response.use(
      (response) => {
        return response
      },
      async (error) => {
        if (error.response?.status === 401) {
          authApi.handleAuthError()
        } else {
          this.handleError(error)
        }
        return Promise.reject(error)
      }
    )
  }

  // ========== 紧急事件接口 ==========

  /**
   * 创建紧急事件
   * @param {Object} eventData - 事件数据
   * @returns {Promise}
   */
  async createEvent(eventData) {
    try {
      const response = await this.api.post('/events', eventData)
      return response.data
    } catch (error) {
      console.error('创建紧急事件失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 获取所有紧急事件
   * @param {Object} params - 查询参数 {page, page_size, status, severity}
   * @returns {Promise}
   */
  async getAllEvents(params = {}) {
    try {
      const response = await this.api.get('/events', { params })
      return response.data
    } catch (error) {
      console.error('获取紧急事件列表失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 根据ID获取紧急事件
   * @param {number} eventId - 事件ID
   * @returns {Promise}
   */
  async getEventById(eventId) {
    try {
      const response = await this.api.get(`/events/${eventId}`)
      return response.data
    } catch (error) {
      console.error('获取紧急事件失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 响应紧急事件
   * @param {number} eventId - 事件ID
   * @param {string} responseAction - 响应措施
   * @param {string} responseNotes - 响应备注
   * @returns {Promise}
   */
  async respondToEvent(eventId, responseAction, responseNotes = '') {
    try {
      const response = await this.api.post(`/events/${eventId}/respond`, {
        response_action: responseAction,
        response_notes: responseNotes
      })
      return response.data
    } catch (error) {
      console.error('响应紧急事件失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 解决紧急事件
   * @param {number} eventId - 事件ID
   * @returns {Promise}
   */
  async resolveEvent(eventId) {
    try {
      const response = await this.api.post(`/events/${eventId}/resolve`)
      return response.data
    } catch (error) {
      console.error('解决紧急事件失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 取消紧急事件
   * @param {number} eventId - 事件ID
   * @returns {Promise}
   */
  async cancelEvent(eventId) {
    try {
      const response = await this.api.post(`/events/${eventId}/cancel`)
      return response.data
    } catch (error) {
      console.error('取消紧急事件失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 获取紧急事件统计
   * @returns {Promise}
   */
  async getStatistics() {
    try {
      const response = await this.api.get('/statistics')
      return response.data
    } catch (error) {
      console.error('获取统计信息失败:', error)
      throw this.processError(error)
    }
  }

  // ========== 工具方法 ==========

  /**
   * 处理API错误
   * @param {Error} error - 错误对象
   * @returns {Object}
   */
  processError(error) {
    if (error.response) {
      const { status, data } = error.response
      return {
        success: false,
        message: data.message || `请求失败 (${status})`,
        error_code: data.error_code || 'REQUEST_ERROR',
        status
      }
    } else if (error.request) {
      return {
        success: false,
        message: '网络连接失败，请检查网络或后端服务是否运行',
        error_code: 'NETWORK_ERROR'
      }
    } else {
      return {
        success: false,
        message: error.message || '未知错误',
        error_code: 'UNKNOWN_ERROR'
      }
    }
  }

  /**
   * 统一错误提示处理
   * @param {Error} error - 错误对象
   */
  handleError(error) {
    if (error.response?.status === 401) {
      return
    }

    let message = '请求失败'

    if (error.response) {
      const { status, data } = error.response
      switch (status) {
        case 400:
          message = data.message || '请求参数错误'
          break
        case 403:
          message = '权限不足，无法访问'
          break
        case 404:
          message = '资源不存在'
          break
        case 500:
          message = '服务器内部错误，请稍后重试'
          break
        default:
          message = data.message || `请求失败 (${status})`
      }
    } else if (error.request) {
      message = '网络连接失败，请检查网络连接'
    } else {
      message = error.message || '未知错误'
    }

    ElMessage.error(message)
  }

  /**
   * 获取事件类型文本
   * @param {string} type - 事件类型
   * @returns {string}
   */
  getEventTypeText(type) {
    const types = {
      equipment_failure: '设备故障',
      weather_emergency: '恶劣天气',
      collision_risk: '碰撞风险',
      signal_loss: '信号丢失',
      battery_low: '电量低',
      geofence_violation: '电子围栏违规',
      manual_emergency: '手动紧急',
      other: '其他'
    }
    return types[type] || type
  }

  /**
   * 获取严重程度文本
   * @param {string} severity - 严重程度
   * @returns {string}
   */
  getSeverityText(severity) {
    const severities = {
      low: '低',
      medium: '中',
      high: '高',
      critical: '严重'
    }
    return severities[severity] || severity
  }

  /**
   * 获取状态文本
   * @param {string} status - 状态
   * @returns {string}
   */
  getStatusText(status) {
    const statuses = {
      active: '激活中',
      responding: '响应中',
      resolved: '已解决',
      cancelled: '已取消'
    }
    return statuses[status] || status
  }

  /**
   * 获取事件类型颜色
   * @param {string} type - 事件类型
   * @returns {string}
   */
  getEventTypeColor(type) {
    const colors = {
      equipment_failure: '#F56C6C',
      weather_emergency: '#E6A23C',
      collision_risk: '#F56C6C',
      signal_loss: '#909399',
      battery_low: '#E6A23C',
      geofence_violation: '#F56C6C',
      manual_emergency: '#409EFF',
      other: '#909399'
    }
    return colors[type] || '#909399'
  }

  /**
   * 获取严重程度颜色
   * @param {string} severity - 严重程度
   * @returns {string}
   */
  getSeverityColor(severity) {
    const colors = {
      low: '#67C23A',
      medium: '#E6A23C',
      high: '#F56C6C',
      critical: '#F56C6C'
    }
    return colors[severity] || '#909399'
  }

  /**
   * 获取状态颜色
   * @param {string} status - 状态
   * @returns {string}
   */
  getStatusColor(status) {
    const colors = {
      active: '#F56C6C',
      responding: '#E6A23C',
      resolved: '#67C23A',
      cancelled: '#909399'
    }
    return colors[status] || '#909399'
  }

  /**
   * 获取状态标签类型
   * @param {string} status - 状态
   * @returns {string}
   */
  getStatusTagType(status) {
    const types = {
      active: 'danger',
      responding: 'warning',
      resolved: 'success',
      cancelled: 'info'
    }
    return types[status] || 'info'
  }

  /**
   * 获取严重程度标签类型
   * @param {string} severity - 严重程度
   * @returns {string}
   */
  getSeverityTagType(severity) {
    const types = {
      low: 'success',
      medium: 'warning',
      high: 'danger',
      critical: 'danger'
    }
    return types[severity] || 'info'
  }
}

// 创建单例实例
const emergencyApi = new EmergencyApiService()

export default emergencyApi
