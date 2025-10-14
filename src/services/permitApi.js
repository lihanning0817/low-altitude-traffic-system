/**
 * 飞行许可API服务
 * Flight Permit API Service
 */

import axios from 'axios'

class PermitApiService {
  constructor() {
    this.baseURL = '/api/v1/flight-permits'

    this.api = axios.create({
      baseURL: this.baseURL,
      timeout: 10000,
      headers: { 'Content-Type': 'application/json' }
    })

    this.api.interceptors.request.use(
      (config) => {
        const token = localStorage.getItem('access_token')
        if (token) config.headers.Authorization = `Bearer ${token}`
        return config
      },
      (error) => Promise.reject(error)
    )

    this.api.interceptors.response.use(
      (response) => response.data,
      (error) => this.handleError(error)
    )
  }

  // 获取飞行许可列表
  async getPermits(params = {}) {
    try {
      const response = await this.api.get('/', { params })
      return {
        success: true,
        data: response.data,
        message: response.message || '获取飞行许可列表成功'
      }
    } catch (error) {
      console.error('[PermitAPI] 获取许可列表失败:', error)
      throw error
    }
  }

  // 获取许可详情
  async getPermitById(permitId) {
    try {
      if (!permitId) throw new Error('许可ID不能为空')
      const response = await this.api.get(`/${permitId}`)
      return {
        success: true,
        data: response.data,
        message: response.message || '获取许可详情成功'
      }
    } catch (error) {
      console.error(`[PermitAPI] 获取许可详情失败 (ID: ${permitId}):`, error)
      throw error
    }
  }

  // 申请飞行许可
  async applyPermit(permitData) {
    try {
      this.validatePermitData(permitData)
      const response = await this.api.post('/', permitData)
      return {
        success: true,
        data: response.data,
        message: response.message || '飞行许可申请提交成功'
      }
    } catch (error) {
      console.error('[PermitAPI] 申请许可失败:', error)
      throw error
    }
  }

  // 批准飞行许可
  async approvePermit(permitId, approvalData = {}) {
    try {
      if (!permitId) throw new Error('许可ID不能为空')
      const response = await this.api.post(`/${permitId}/approve`, approvalData)
      return {
        success: true,
        data: response.data,
        message: response.message || '飞行许可已批准'
      }
    } catch (error) {
      console.error(`[PermitAPI] 批准许可失败 (ID: ${permitId}):`, error)
      throw error
    }
  }

  // 拒绝飞行许可
  async rejectPermit(permitId, rejectionData) {
    try {
      if (!permitId) throw new Error('许可ID不能为空')
      if (!rejectionData?.reason) throw new Error('拒绝原因不能为空')

      const response = await this.api.post(`/${permitId}/reject`, rejectionData)
      return {
        success: true,
        data: response.data,
        message: response.message || '飞行许可已拒绝'
      }
    } catch (error) {
      console.error(`[PermitAPI] 拒绝许可失败 (ID: ${permitId}):`, error)
      throw error
    }
  }

  // 撤销飞行许可
  async revokePermit(permitId, reason) {
    try {
      if (!permitId) throw new Error('许可ID不能为空')
      const response = await this.api.post(`/${permitId}/revoke`, { reason })
      return {
        success: true,
        data: response.data,
        message: response.message || '飞行许可已撤销'
      }
    } catch (error) {
      console.error(`[PermitAPI] 撤销许可失败 (ID: ${permitId}):`, error)
      throw error
    }
  }

  // 获取待审批的许可列表
  async getPendingPermits() {
    try {
      const response = await this.getPermits({ status: 'pending' })
      const permits = response.data.permits || []
      const pending = permits.filter(p => p.status === 'pending')

      return {
        success: true,
        data: { permits: pending, count: pending.length },
        message: `找到 ${pending.length} 个待审批许可`
      }
    } catch (error) {
      console.error('[PermitAPI] 获取待审批许可失败:', error)
      throw error
    }
  }

  // 获取我的许可列表
  async getMyPermits(userId) {
    try {
      if (!userId) throw new Error('用户ID不能为空')
      const response = await this.getPermits({ applicant_id: userId })
      return {
        success: true,
        data: response.data,
        message: '获取我的许可列表成功'
      }
    } catch (error) {
      console.error('[PermitAPI] 获取我的许可失败:', error)
      throw error
    }
  }

  // 获取许可统计
  async getPermitStatistics() {
    try {
      const response = await this.getPermits()
      const permits = response.data.permits || []

      const stats = {
        total: permits.length,
        by_status: {
          pending: 0,
          approved: 0,
          rejected: 0,
          revoked: 0
        },
        today: 0,
        this_week: 0
      }

      const now = new Date()
      const todayStart = new Date(now.getFullYear(), now.getMonth(), now.getDate())
      const weekStart = new Date(todayStart.getTime() - 7 * 24 * 60 * 60 * 1000)

      permits.forEach(permit => {
        if (stats.by_status.hasOwnProperty(permit.status)) {
          stats.by_status[permit.status]++
        }

        const applicationTime = new Date(permit.application_time)
        if (applicationTime >= todayStart) stats.today++
        if (applicationTime >= weekStart) stats.this_week++
      })

      return {
        success: true,
        data: stats,
        message: '获取许可统计成功'
      }
    } catch (error) {
      console.error('[PermitAPI] 获取许可统计失败:', error)
      throw error
    }
  }

  // 验证许可数据
  validatePermitData(data) {
    const required = ['task_id', 'airspace_id', 'start_time', 'end_time']
    for (const field of required) {
      if (!data[field]) throw new Error(`${field} 是必填字段`)
    }

    const startTime = new Date(data.start_time)
    const endTime = new Date(data.end_time)

    if (startTime >= endTime) {
      throw new Error('结束时间必须晚于开始时间')
    }

    if (startTime < new Date()) {
      throw new Error('开始时间不能早于当前时间')
    }

    const duration = (endTime - startTime) / (1000 * 60 * 60)
    if (duration > 24) {
      throw new Error('单次飞行许可时长不能超过24小时')
    }
  }

  // 错误处理
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
          errorMessage = '无权限操作飞行许可'
          errorCode = 'FORBIDDEN'
          break
        case 404:
          errorMessage = '飞行许可不存在'
          errorCode = 'NOT_FOUND'
          break
        case 409:
          errorMessage = data.message || '飞行时间冲突'
          errorCode = 'CONFLICT'
          break
        case 500:
          errorMessage = '服务器内部错误'
          errorCode = 'INTERNAL_ERROR'
          break
        default:
          errorMessage = data.message || errorMessage
      }
    } else if (error.request) {
      errorMessage = '网络连接失败'
      errorCode = 'NETWORK_ERROR'
    } else {
      errorMessage = error.message
      errorCode = 'REQUEST_ERROR'
    }

    console.error('[PermitAPI] Error:', { code: errorCode, message: errorMessage })
    return Promise.reject({ success: false, code: errorCode, message: errorMessage, error })
  }
}

const permitApi = new PermitApiService()
export default permitApi
export { PermitApiService }
