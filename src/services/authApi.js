// 认证API服务 - 对接后端AuthController
import axios from 'axios'
import { ElMessage } from 'element-plus'

class AuthApiService {
  constructor() {
    this.baseURL = 'http://localhost:8081/api/v1/auth'
    // 初始化时从localStorage加载token
    this.accessToken = localStorage.getItem('access_token')
    this.refreshToken = localStorage.getItem('refresh_token')

    // 创建axios实例
    this.api = axios.create({
      baseURL: this.baseURL,
      timeout: 10000,
      headers: {
        'Content-Type': 'application/json'
      }
    })

    // 请求拦截器 - 自动添加token
    this.api.interceptors.request.use(
      (config) => {
        if (this.accessToken) {
          config.headers.Authorization = `Bearer ${this.accessToken}`
        }
        return config
      },
      (error) => {
        return Promise.reject(error)
      }
    )

    // 响应拦截器 - 统一处理错误和token过期
    this.api.interceptors.response.use(
      (response) => {
        return response
      },
      async (error) => {
        const originalRequest = error.config

        // 401错误处理 - 自动登出并跳转到登录页
        if (error.response?.status === 401) {
          if (!originalRequest._retry) {
            originalRequest._retry = true

            // 尝试刷新token
            if (this.refreshToken) {
              try {
                const response = await this.refreshAccessToken()
                if (response.success) {
                  this.setAccessToken(response.data.access_token)
                  originalRequest.headers.Authorization = `Bearer ${this.accessToken}`
                  return this.api(originalRequest)
                }
              } catch (refreshError) {
                console.error('Token刷新失败:', refreshError)
                this.handleAuthError()
                return Promise.reject(refreshError)
              }
            } else {
              // 没有refresh token，直接登出
              this.handleAuthError()
            }
          } else {
            // 重试失败，登出
            this.handleAuthError()
          }
        }

        // 统一错误提示处理
        this.handleGlobalError(error)

        return Promise.reject(error)
      }
    )
  }

  // Token管理
  setAccessToken(token) {
    this.accessToken = token
    localStorage.setItem('access_token', token)
  }

  setRefreshToken(token) {
    this.refreshToken = token
    localStorage.setItem('refresh_token', token)
  }

  getAccessToken() {
    if (!this.accessToken) {
      this.accessToken = localStorage.getItem('access_token')
    }
    return this.accessToken
  }

  getRefreshToken() {
    if (!this.refreshToken) {
      this.refreshToken = localStorage.getItem('refresh_token')
    }
    return this.refreshToken
  }

  clearTokens() {
    this.accessToken = null
    this.refreshToken = null
    localStorage.removeItem('access_token')
    localStorage.removeItem('refresh_token')
  }

  // 初始化 - 从localStorage加载token
  init() {
    this.accessToken = localStorage.getItem('access_token')
    this.refreshToken = localStorage.getItem('refresh_token')
  }

  // ========== 认证接口 ==========

  /**
   * 用户注册
   * @param {Object} userData - 用户数据
   * @param {string} userData.username - 用户名
   * @param {string} userData.email - 邮箱
   * @param {string} userData.password - 密码
   * @param {string} userData.full_name - 真实姓名
   */
  async register(userData) {
    try {
      const response = await this.api.post('/register', userData)
      return response.data
    } catch (error) {
      console.error('注册失败:', error)
      throw this.handleError(error)
    }
  }

  /**
   * 用户登录
   * @param {Object} loginData - 登录数据
   * @param {string} loginData.username - 用户名或邮箱
   * @param {string} loginData.password - 密码
   */
  async login(loginData) {
    try {
      const response = await this.api.post('/login', loginData)

      if (response.data.success) {
        // 保存token
        this.setAccessToken(response.data.data.access_token)
        this.setRefreshToken(response.data.data.refresh_token)
      }

      return response.data
    } catch (error) {
      console.error('登录失败:', error)
      throw this.handleError(error)
    }
  }

  /**
   * 获取当前用户信息
   */
  async getCurrentUser() {
    try {
      const response = await this.api.get('/me')
      return response.data
    } catch (error) {
      console.error('获取用户信息失败:', error)
      throw this.handleError(error)
    }
  }

  /**
   * 修改密码
   * @param {Object} passwordData - 密码数据
   * @param {string} passwordData.old_password - 旧密码
   * @param {string} passwordData.new_password - 新密码
   */
  async changePassword(passwordData) {
    try {
      const response = await this.api.put('/password', passwordData)
      return response.data
    } catch (error) {
      console.error('修改密码失败:', error)
      throw this.handleError(error)
    }
  }

  /**
   * 刷新访问令牌
   */
  async refreshAccessToken() {
    try {
      const response = await axios.post(`${this.baseURL}/refresh`, {
        refresh_token: this.getRefreshToken()
      })

      if (response.data.success) {
        this.setAccessToken(response.data.data.access_token)
      }

      return response.data
    } catch (error) {
      console.error('Token刷新失败:', error)
      throw this.handleError(error)
    }
  }

  /**
   * 用户登出
   */
  async logout() {
    try {
      await this.api.post('/logout')
      this.clearTokens()
      return { success: true, message: '登出成功' }
    } catch (error) {
      console.error('登出失败:', error)
      // 即使请求失败也清除本地token
      this.clearTokens()
      throw this.handleError(error)
    }
  }

  // ========== 工具方法 ==========

  /**
   * 处理认证错误 - 401错误时自动登出并跳转
   */
  handleAuthError() {
    this.clearTokens()
    ElMessage.error('登录已过期，请重新登录')

    // 延迟跳转，确保消息显示 - 使用动态导入避免循环依赖
    setTimeout(async () => {
      // 动态导入router避免循环依赖
      const { default: router } = await import('@/router')
      const currentPath = router.currentRoute.value.path
      if (currentPath !== '/login') {
        router.push('/login')
      }
    }, 1000)
  }

  /**
   * 统一错误提示处理
   * @param {Error} error - 错误对象
   */
  handleGlobalError(error) {
    // 401错误已在handleAuthError中处理，这里跳过
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
          message = '请求的资源不存在'
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
          message = '服务暂时不可用，请稍后重试'
          break
        default:
          message = data.message || `请求失败 (${status})`
      }
    } else if (error.request) {
      message = '网络连接失败，请检查网络连接'
    } else {
      message = error.message || '未知错误'
    }

    // 显示友好的错误提示
    ElMessage.error(message)
  }

  /**
   * 处理API错误（保留原方法，供手动调用）
   * @param {Error} error - 错误对象
   */
  handleError(error) {
    if (error.response) {
      // 服务器返回错误状态码
      const { status, data } = error.response
      return {
        success: false,
        message: data.message || `请求失败 (${status})`,
        error_code: data.error_code || 'REQUEST_ERROR',
        status
      }
    } else if (error.request) {
      // 请求发出但没有收到响应
      return {
        success: false,
        message: '网络连接失败，请检查网络或后端服务是否运行',
        error_code: 'NETWORK_ERROR'
      }
    } else {
      // 其他错误
      return {
        success: false,
        message: error.message || '未知错误',
        error_code: 'UNKNOWN_ERROR'
      }
    }
  }

  /**
   * 检查是否已登录
   */
  isAuthenticated() {
    return !!this.getAccessToken()
  }

  /**
   * 检查token是否即将过期（需要根据实际token格式实现）
   */
  isTokenExpiringSoon() {
    const token = this.getAccessToken()
    if (!token) return true

    try {
      // 简单的JWT解析（实际项目中可能需要更完善的实现）
      const payload = JSON.parse(atob(token.split('.')[1]))
      const exp = payload.exp * 1000 // 转换为毫秒
      const now = Date.now()
      const fiveMinutes = 5 * 60 * 1000

      return (exp - now) < fiveMinutes
    } catch (error) {
      console.warn('Token解析失败:', error)
      return true
    }
  }
}

// 创建单例实例
const authApi = new AuthApiService()

// 初始化
authApi.init()

export default authApi