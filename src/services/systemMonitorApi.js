// 系统监控API服务 - 对接后端SystemMonitorController
import axios from 'axios'
import { ElMessage } from 'element-plus'

class SystemMonitorApiService {
  constructor() {
    this.baseURL = 'http://localhost:8081/api/v1'

    // 创建axios实例
    this.api = axios.create({
      baseURL: this.baseURL,
      timeout: 30000, // 增加超时时间到30秒
      headers: {
        'Content-Type': 'application/json; charset=utf-8'
      },
      // 添加重试配置
      validateStatus: function (status) {
        return status >= 200 && status < 300; // 默认配置
      }
    })

    // 请求拦截器 - 自动添加JWT token
    this.api.interceptors.request.use(
      (config) => {
        // 动态获取token，避免循环依赖
        const token = localStorage.getItem('access_token')
        if (token) {
          config.headers.Authorization = `Bearer ${token}`
        }

        // 添加调试日志
        console.log(`[SystemMonitor API] ${config.method?.toUpperCase()} ${config.url}`, {
          headers: config.headers,
          hasToken: !!token
        })

        return config
      },
      (error) => {
        console.error('[SystemMonitor API] Request interceptor error:', error)
        return Promise.reject(error)
      }
    )

    // 响应拦截器 - 统一处理错误
    this.api.interceptors.response.use(
      (response) => {
        console.log(`[SystemMonitor API] Response success:`, {
          url: response.config.url,
          status: response.status,
          data: response.data
        })
        return response
      },
      async (error) => {
        console.error(`[SystemMonitor API] Response error:`, {
          url: error.config?.url,
          status: error.response?.status,
          message: error.response?.data?.message || error.message
        })

        // 401错误处理 - 避免循环依赖，直接处理
        if (error.response?.status === 401) {
          this.handle401Error()
        } else {
          // 其他错误统一处理
          this.handleError(error)
        }
        return Promise.reject(error)
      }
    )
  }

  // 401错误处理 - 避免循环依赖
  handle401Error() {
    console.warn('[SystemMonitor API] 401 Unauthorized - clearing tokens')
    localStorage.removeItem('access_token')
    localStorage.removeItem('refresh_token')

    ElMessage.error('登录已过期，请重新登录')

    // 延迟跳转避免在请求过程中立即跳转
    setTimeout(async () => {
      try {
        const { default: router } = await import('@/router')
        const currentPath = router.currentRoute.value.path
        if (currentPath !== '/login') {
          router.push('/login')
        }
      } catch (routerError) {
        console.error('Router import failed:', routerError)
        // 备用跳转方式
        window.location.href = '/login'
      }
    }, 1000)
  }

  // ========== 系统监控接口 ==========

  /**
   * 获取系统健康状态
   * GET /api/v1/health - 无需认证
   */
  async getHealthStatus() {
    try {
      const response = await this.api.get('/health')
      return response.data
    } catch (error) {
      console.error('获取健康状态失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 获取系统基本信息
   * GET /api/v1/info - 无需认证
   */
  async getSystemInfo() {
    try {
      const response = await this.api.get('/info')
      return response.data
    } catch (error) {
      console.error('获取系统信息失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 获取无人机列表
   * GET /api/v1/drones - 需要用户认证（所有登录用户都可以访问）
   */
  async getDronesList() {
    try {
      // 检查认证状态
      const token = localStorage.getItem('access_token')
      const userStr = localStorage.getItem('user')

      if (!token) {
        // 返回友好的错误信息而不是抛出异常
        return {
          success: false,
          message: '请先登录后再访问无人机数据。点击右上角头像进行登录。',
          error_code: 'NOT_AUTHENTICATED',
          requiresAuth: true
        }
      }

      let user = null
      if (userStr) {
        try {
          user = JSON.parse(userStr)
        } catch (e) {
          console.warn('解析用户信息失败:', e)
        }
      }

      // 对于模拟用户（非真实后端用户），返回模拟数据
      if (user && ['testuser'].includes(user.username)) {
        const mockDroneData = [
          {
            id: 1,
            drone_id: "SIM-001",
            name: "模拟无人机-01",
            model: "Phantom 4 Pro",
            status: "active",
            battery: 85.5,
            location: { lat: 39.9042, lng: 116.407396 },
            last_update: Math.floor(Date.now() / 1000)
          },
          {
            id: 2,
            drone_id: "SIM-002",
            name: "模拟无人机-02",
            model: "Mavic 3",
            status: "inactive",
            battery: 45.8,
            location: { lat: 39.9142, lng: 116.417396 },
            last_update: Math.floor(Date.now() / 1000) - 300
          }
        ]

        return {
          success: true,
          data: {
            drones: mockDroneData,
            total: mockDroneData.length,
            active_count: mockDroneData.filter(d => d.status === 'active').length
          },
          message: '获取无人机列表成功（模拟数据）'
        }
      }

      // 真实用户调用后端API
      const response = await this.api.get('/drones')

      // 确保返回正确的数据结构
      if (response.data && typeof response.data === 'object') {
        return {
          success: true,
          data: response.data,
          message: '获取无人机列表成功'
        }
      } else {
        throw new Error('服务器返回数据格式错误')
      }
    } catch (error) {
      console.error('获取无人机列表失败:', error)

      // 如果是认证错误，返回特殊提示
      if (error.response?.status === 401) {
        return {
          success: false,
          message: '登录已过期，请重新登录后访问无人机数据',
          error_code: 'AUTHENTICATION_EXPIRED',
          requiresAuth: true
        }
      }

      throw this.processError(error)
    }
  }

  /**
   * 获取用户列表
   * GET /api/v1/users - 需要管理员权限
   * @param {Object} params - 查询参数
   * @param {number} params.offset - 分页偏移
   * @param {number} params.limit - 分页大小
   * @param {string} params.role - 角色筛选
   */
  async getUsersList(params = {}) {
    try {
      // 检查认证状态
      const token = localStorage.getItem('access_token')
      if (!token) {
        return {
          success: false,
          message: '请先登录后再访问用户数据。点击右上角头像进行登录。',
          error_code: 'NOT_AUTHENTICATED',
          requiresAuth: true
        }
      }

      // 检查管理员权限 (从localStorage中的用户信息判断)
      const userStr = localStorage.getItem('user')
      if (userStr) {
        try {
          const user = JSON.parse(userStr)
          if (user.role !== 'admin') {
            const roleText = user.role === 'user' ? '普通用户' : user.role || '未知'
            return {
              success: false,
              message: `权限不足：只有管理员可以查看用户列表。\n当前用户：${user.full_name || user.username}（${roleText}）\n如需管理员权限，请联系系统管理员。`,
              error_code: 'INSUFFICIENT_PRIVILEGES',
              requiresAdmin: true,
              currentRole: user.role,
              currentUser: user.username
            }
          }
        } catch (parseError) {
          console.warn('解析用户信息失败:', parseError)
          return {
            success: false,
            message: '用户信息异常，请重新登录',
            error_code: 'INVALID_USER_DATA',
            requiresAuth: true
          }
        }
      } else {
        return {
          success: false,
          message: '用户信息缺失，请重新登录',
          error_code: 'MISSING_USER_DATA',
          requiresAuth: true
        }
      }

      // 设置默认分页参数
      const queryParams = {
        offset: params.offset || 0,
        limit: params.limit || 20,
        ...params
      }

      const response = await this.api.get('/users', { params: queryParams })

      // 确保返回正确的数据结构
      if (response.data && typeof response.data === 'object') {
        return {
          success: true,
          data: response.data,
          message: '获取用户列表成功'
        }
      } else {
        throw new Error('服务器返回数据格式错误')
      }
    } catch (error) {
      console.error('获取用户列表失败:', error)

      // 如果是认证错误，返回特殊提示
      if (error.response?.status === 401) {
        return {
          success: false,
          message: '登录已过期，请重新登录后访问用户数据',
          error_code: 'AUTHENTICATION_EXPIRED',
          requiresAuth: true
        }
      }

      // 如果是权限错误，返回特殊提示
      if (error.response?.status === 403) {
        return {
          success: false,
          message: '权限不足：只有管理员可以查看用户列表',
          error_code: 'INSUFFICIENT_PRIVILEGES',
          requiresAdmin: true
        }
      }

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
    // 401错误已经在拦截器中处理，这里跳过
    if (error.response?.status === 401) {
      return
    }

    let message = '请求失败'
    let showMessage = true

    if (error.response) {
      const { status, data } = error.response
      switch (status) {
        case 400:
          message = data.message || '请求参数错误'
          break
        case 403:
          message = data.message || '权限不足，无法访问该资源'
          break
        case 404:
          message = '请求的资源不存在，请检查后端服务是否正常运行'
          break
        case 422:
          message = data.message || '数据验证失败'
          break
        case 500:
          message = '服务器内部错误，请稍后重试'
          break
        case 502:
        case 503:
        case 504:
          message = '服务暂时不可用，请检查后端服务状态'
          break
        default:
          message = data.message || `请求失败 (状态码: ${status})`
      }
    } else if (error.request) {
      // 网络错误
      if (error.code === 'ECONNABORTED') {
        message = '请求超时，请重试或检查后端服务状态。如果问题持续，请重启后端服务'
      } else if (error.code === 'ERR_NETWORK') {
        message = '网络连接失败，请检查：\n1. 网络连接是否正常\n2. 后端服务是否启动(http://localhost:8081)\n3. 防火墙是否阻止连接'
      } else if (error.code === 'ECONNREFUSED') {
        message = '连接被拒绝，后端服务可能未启动，请检查 http://localhost:8081 是否可访问'
      } else {
        message = '网络连接失败，请检查网络连接或重启后端服务'
      }
    } else {
      // 其他错误（如自定义抛出的错误）
      message = error.message || '未知错误'
      // 如果是自定义错误消息，可能已经很明确了，不需要额外处理
      if (error.message && !error.response && !error.request) {
        showMessage = true
      }
    }

    // 显示错误提示（避免重复显示）
    if (showMessage) {
      ElMessage.error(message)
    }

    console.error('[SystemMonitor API] Error details:', {
      message,
      status: error.response?.status,
      data: error.response?.data,
      url: error.config?.url,
      error
    })
  }

  /**
   * 格式化系统健康状态
   * @param {Object} healthData - 健康状态数据
   */
  formatHealthStatus(healthData) {
    if (!healthData || !healthData.data) return null

    const data = healthData.data

    return {
      overall: data.status || 'unknown',
      uptime: data.uptime_seconds || 0,
      services: {
        database: data.services?.database || 'unknown',
        externalApis: data.services?.external_apis || 'unknown'
      },
      timestamp: data.timestamp || Date.now()
    }
  }

  /**
   * 格式化系统信息
   * @param {Object} systemInfo - 系统信息数据
   */
  formatSystemInfo(systemInfo) {
    if (!systemInfo || !systemInfo.data) return null

    const data = systemInfo.data

    return {
      name: data.name || '未知系统',
      version: data.version || '未知版本',
      environment: data.environment || 'unknown',
      startTime: data.start_time || '',
      uptime: data.uptime_seconds || 0,
      serverTime: data.server_time || Date.now()
    }
  }

  /**
   * 格式化无人机数据
   * @param {Object} dronesData - 无人机数据
   */
  formatDronesData(dronesData) {
    if (!dronesData || !dronesData.data) return []

    const data = dronesData.data
    const drones = data.drones || []

    return drones.map(drone => ({
      id: drone.id,
      droneId: drone.drone_id,
      name: drone.name,
      model: drone.model,
      status: drone.status,
      battery: drone.battery,
      location: drone.location,
      lastUpdate: drone.last_update,
      // 添加状态显示文本和颜色
      statusText: this.getDroneStatusText(drone.status),
      statusColor: this.getDroneStatusColor(drone.status),
      batteryColor: this.getBatteryColor(drone.battery)
    }))
  }

  /**
   * 格式化用户数据
   * @param {Object} usersData - 用户数据
   */
  formatUsersData(usersData) {
    if (!usersData || !usersData.data) return { users: [], pagination: {} }

    const data = usersData.data

    return {
      users: (data.users || []).map(user => ({
        id: user.id,
        username: user.username,
        email: user.email,
        fullName: user.full_name,
        role: user.role,
        status: user.status,
        createdAt: user.created_at,
        updatedAt: user.updated_at,
        lastLogin: user.last_login,
        // 添加角色和状态显示文本
        roleText: this.getUserRoleText(user.role),
        statusText: this.getUserStatusText(user.status),
        roleColor: this.getUserRoleColor(user.role),
        statusColor: this.getUserStatusColor(user.status)
      })),
      pagination: data.pagination || {}
    }
  }

  // ========== 状态映射工具方法 ==========

  getDroneStatusText(status) {
    const statusMap = {
      'active': '在线',
      'inactive': '离线',
      'maintenance': '维护中'
    }
    return statusMap[status] || status
  }

  getDroneStatusColor(status) {
    const colorMap = {
      'active': '#67C23A',
      'inactive': '#909399',
      'maintenance': '#E6A23C'
    }
    return colorMap[status] || '#909399'
  }

  getBatteryColor(battery) {
    if (battery >= 50) return '#67C23A'
    if (battery >= 20) return '#E6A23C'
    return '#F56C6C'
  }

  getUserRoleText(role) {
    const roleMap = {
      'admin': '管理员',
      'user': '普通用户'
    }
    return roleMap[role] || role
  }

  getUserStatusText(status) {
    const statusMap = {
      'active': '正常',
      'inactive': '禁用',
      'pending': '待审核'
    }
    return statusMap[status] || status
  }

  getUserRoleColor(role) {
    const colorMap = {
      'admin': '#F56C6C',
      'user': '#67C23A'
    }
    return colorMap[role] || '#909399'
  }

  getUserStatusColor(status) {
    const colorMap = {
      'active': '#67C23A',
      'inactive': '#909399',
      'pending': '#E6A23C'
    }
    return colorMap[status] || '#909399'
  }

  /**
   * 格式化运行时间
   * @param {number} seconds - 秒数
   */
  formatUptime(seconds) {
    if (!seconds || seconds < 0) return '未知'

    const days = Math.floor(seconds / 86400)
    const hours = Math.floor((seconds % 86400) / 3600)
    const minutes = Math.floor((seconds % 3600) / 60)

    if (days > 0) {
      return `${days}天 ${hours}小时 ${minutes}分钟`
    } else if (hours > 0) {
      return `${hours}小时 ${minutes}分钟`
    } else {
      return `${minutes}分钟`
    }
  }

  /**
   * 格式化时间戳
   * @param {number} timestamp - 时间戳
   */
  formatTimestamp(timestamp) {
    if (!timestamp) return '未知'

    const date = new Date(timestamp * 1000)
    return date.toLocaleString('zh-CN', {
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit'
    })
  }
}

// 创建单例实例
const systemMonitorApi = new SystemMonitorApiService()

export default systemMonitorApi