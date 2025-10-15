// 飞行任务API服务 - 对接后端FlightTaskController
import axios from 'axios'
import { ElMessage } from 'element-plus'
import authApi from './authApi'

class FlightTaskApiService {
  constructor() {
    // 使用相对路径，通过Vue开发服务器代理转发
    this.baseURL = '/api/v1/tasks'

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
        // 401错误让authApi处理
        if (error.response?.status === 401) {
          authApi.handleAuthError()
        } else {
          // 其他错误统一处理
          this.handleError(error)
        }
        return Promise.reject(error)
      }
    )
  }

  // ========== 飞行任务接口 ==========

  /**
   * 获取飞行任务列表
   * @param {Object} params - 查询参数
   * @param {string} params.status - 任务状态筛选 (pending/ongoing/completed)
   * @param {number} params.limit - 分页限制
   * @param {number} params.offset - 分页偏移
   * @param {AbortSignal} signal - AbortController信号，用于取消请求
   */
  async getFlightTasks(params = {}, signal = null) {
    try {
      const config = { params }
      if (signal) {
        config.signal = signal
      }
      const response = await this.api.get('', config)
      return response.data
    } catch (error) {
      // 如果是用户主动取消，不显示错误
      if (error.name === 'CanceledError' || error.code === 'ERR_CANCELED') {
        console.log('请求已取消')
        throw { success: false, message: '请求已取消', canceled: true }
      }
      console.error('获取任务列表失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 获取飞行任务列表（别名方法，用于向后兼容）
   * @param {Object} params - 查询参数
   */
  async getTasks(params = {}) {
    return this.getFlightTasks(params)
  }

  /**
   * 获取单个飞行任务详情
   * @param {string|number} id - 任务ID
   */
  async getFlightTask(id) {
    try {
      const response = await this.api.get(`/${id}`)
      return response.data
    } catch (error) {
      console.error('获取任务详情失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 创建新的飞行任务
   * @param {Object} taskData - 任务数据
   * @param {string} taskData.name - 任务名称
   * @param {string} taskData.description - 任务描述
   * @param {Object} taskData.route - 路径信息 (可选)
   * @param {string} taskData.scheduled_time - 计划执行时间 (ISO格式, 可选)
   * @param {string} taskData.status - 任务状态 (可选, 默认pending)
   */
  async createFlightTask(taskData) {
    try {
      // 确保数据格式正确
      const payload = {
        name: taskData.name,
        description: taskData.description || '',
        ...(taskData.route && { route: taskData.route }),
        ...(taskData.scheduled_time && { scheduled_time: taskData.scheduled_time }),
        ...(taskData.status && { status: taskData.status })
      }

      const response = await this.api.post('', payload)
      return response.data
    } catch (error) {
      console.error('创建任务失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 更新飞行任务
   * @param {string|number} id - 任务ID
   * @param {Object} updates - 更新的数据
   */
  async updateFlightTask(id, updates) {
    try {
      const response = await this.api.put(`/${id}`, updates)
      return response.data
    } catch (error) {
      console.error('更新任务失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 删除飞行任务
   * @param {string|number} id - 任务ID
   */
  async deleteFlightTask(id) {
    try {
      const response = await this.api.delete(`/${id}`)
      return response.data
    } catch (error) {
      console.error('删除任务失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 批量更新任务状态
   * @param {Array} taskIds - 任务ID数组
   * @param {string} status - 新状态
   */
  async batchUpdateStatus(taskIds, status) {
    try {
      const promises = taskIds.map(id =>
        this.updateFlightTask(id, { status })
      )
      const results = await Promise.allSettled(promises)

      const successful = results.filter(r => r.status === 'fulfilled').length
      const failed = results.filter(r => r.status === 'rejected').length

      return {
        success: true,
        message: `批量更新完成: ${successful}个成功, ${failed}个失败`,
        successful,
        failed
      }
    } catch (error) {
      console.error('批量更新失败:', error)
      throw this.processError(error)
    }
  }

  // ========== 工具方法 ==========

  /**
   * 处理API错误
   * @param {Error} error - 错误对象
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
    // 401错误已由authApi处理，这里跳过
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
          message = '任务不存在或已被删除'
          break
        case 422:
          message = data.message || '数据验证失败'
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

    // 显示错误提示
    ElMessage.error(message)
  }

  /**
   * 格式化任务数据 - 将后端数据转换为前端格式
   * @param {Object} task - 后端任务数据
   */
  formatTask(task) {
    if (!task) return null

    return {
      id: task.id,
      name: task.name,
      description: task.description || '',
      status: task.status || 'pending',
      route: task.route ? JSON.parse(task.route) : null,
      scheduledTime: task.scheduled_time,
      userId: task.user_id,
      createdAt: task.created_at,
      updatedAt: task.updated_at,
      // 添加状态显示文本
      statusText: this.getStatusText(task.status),
      // 添加状态颜色
      statusColor: this.getStatusColor(task.status)
    }
  }

  /**
   * 获取状态显示文本
   * @param {string} status - 状态值
   */
  getStatusText(status) {
    const statusMap = {
      'pending': '待审批',
      'approved': '已批准',
      'executing': '执行中',
      'completed': '已完成',
      'cancelled': '已取消',
      'failed': '已失败'
    }
    return statusMap[status] || status
  }

  /**
   * 获取状态颜色
   * @param {string} status - 状态值
   */
  getStatusColor(status) {
    const colorMap = {
      'pending': '#909399',    // 灰色 - 待审批
      'approved': '#409EFF',   // 蓝色 - 已批准
      'executing': '#E6A23C',  // 橙色 - 执行中
      'completed': '#67C23A',  // 绿色 - 已完成
      'cancelled': '#C0C4CC',  // 浅灰色 - 已取消
      'failed': '#F56C6C'      // 红色 - 已失败
    }
    return colorMap[status] || '#909399'
  }

  /**
   * 验证任务数据
   * @param {Object} taskData - 任务数据
   * @param {boolean} isUpdate - 是否为更新操作（默认false，即创建操作）
   */
  validateTaskData(taskData, isUpdate = false) {
    const errors = []

    // 创建时name是必需的，更新时只有提供了name才验证
    if (!isUpdate) {
      if (!taskData.name || taskData.name.trim() === '') {
        errors.push('任务名称不能为空')
      }
    } else {
      if (taskData.name !== undefined && (!taskData.name || taskData.name.trim() === '')) {
        errors.push('任务名称不能为空')
      }
    }

    if (taskData.name && taskData.name.length > 255) {
      errors.push('任务名称长度不能超过255个字符')
    }

    if (taskData.description && taskData.description.length > 1000) {
      errors.push('任务描述长度不能超过1000个字符')
    }

    if (taskData.scheduled_time) {
      const scheduledDate = new Date(taskData.scheduled_time)
      if (isNaN(scheduledDate.getTime())) {
        errors.push('计划执行时间格式不正确')
      } else if (scheduledDate < new Date()) {
        errors.push('计划执行时间不能早于当前时间')
      }
    }

    if (taskData.status && !['pending', 'approved', 'executing', 'completed', 'cancelled', 'failed'].includes(taskData.status)) {
      errors.push('任务状态值不正确')
    }

    return {
      isValid: errors.length === 0,
      errors
    }
  }

  /**
   * 获取所有可用状态
   */
  getAllStatuses() {
    return [
      { value: 'pending', label: '待审批', color: '#909399' },
      { value: 'approved', label: '已批准', color: '#409EFF' },
      { value: 'executing', label: '执行中', color: '#E6A23C' },
      { value: 'completed', label: '已完成', color: '#67C23A' },
      { value: 'cancelled', label: '已取消', color: '#C0C4CC' },
      { value: 'failed', label: '已失败', color: '#F56C6C' }
    ]
  }

  /**
   * 获取状态转换规则
   */
  getStatusTransitions() {
    return {
      'pending': ['approved', 'cancelled'],
      'approved': ['executing', 'cancelled'],
      'executing': ['completed', 'failed', 'cancelled'],
      'completed': [], // 完成状态不能转换
      'cancelled': [], // 取消状态不能转换
      'failed': ['pending'] // 失败状态可以重新提交
    }
  }

  /**
   * 检查状态转换是否有效
   * @param {string} fromStatus - 当前状态
   * @param {string} toStatus - 目标状态
   */
  isValidStatusTransition(fromStatus, toStatus) {
    const transitions = this.getStatusTransitions()
    return transitions[fromStatus]?.includes(toStatus) || false
  }

  /**
   * 获取可转换的下一状态
   * @param {string} currentStatus - 当前状态
   */
  getNextValidStatuses(currentStatus) {
    const transitions = this.getStatusTransitions()
    const validStatuses = transitions[currentStatus] || []
    const allStatuses = this.getAllStatuses()

    return allStatuses.filter(status => validStatuses.includes(status.value))
  }
}

// 创建单例实例
const flightTaskApi = new FlightTaskApiService()

export default flightTaskApi