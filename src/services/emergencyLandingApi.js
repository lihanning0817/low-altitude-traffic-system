/**
 * 应急降落点 API 服务
 * Emergency Landing Points API Service
 */

import axios from 'axios'
import { getAuthToken } from './authApi'

// 创建 axios 实例
// 使用相对路径，通过Vue开发服务器代理转发
const apiClient = axios.create({
  baseURL: '/api/v1/emergency-landing-points',
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
    console.error('Emergency Landing API Error:', error)

    const errorResponse = {
      success: false,
      code: error.response?.status || 500,
      message: error.response?.data?.message || error.message || '请求失败'
    }

    // 401 未授权
    if (error.response?.status === 401) {
      errorResponse.code = 'UNAUTHORIZED'
      errorResponse.message = '未授权，请重新登录'
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
 * 应急降落点 API 服务类
 */
class EmergencyLandingApiService {
  /**
   * 获取所有降落点列表
   * @param {Object} params - 查询参数
   * @returns {Promise}
   */
  async getLandingPoints(params = {}) {
    try {
      return await apiClient.get('/', { params })
    } catch (error) {
      throw error
    }
  }

  /**
   * 根据 ID 获取单个降落点详情
   * @param {number} id - 降落点 ID
   * @returns {Promise}
   */
  async getLandingPointById(id) {
    if (!id) {
      throw new Error('降落点ID不能为空')
    }

    try {
      return await apiClient.get(`/${id}`)
    } catch (error) {
      throw error
    }
  }

  /**
   * 创建新的降落点
   * @param {Object} data - 降落点数据
   * @returns {Promise}
   */
  async createLandingPoint(data) {
    // 验证必填字段
    this.validateLandingPointData(data)

    try {
      return await apiClient.post('/', data)
    } catch (error) {
      throw error
    }
  }

  /**
   * 更新降落点信息
   * @param {number} id - 降落点 ID
   * @param {Object} data - 更新的数据
   * @returns {Promise}
   */
  async updateLandingPoint(id, data) {
    if (!id) {
      throw new Error('降落点ID不能为空')
    }

    try {
      return await apiClient.put(`/${id}`, data)
    } catch (error) {
      throw error
    }
  }

  /**
   * 删除降落点
   * @param {number} id - 降落点 ID
   * @returns {Promise}
   */
  async deleteLandingPoint(id) {
    if (!id) {
      throw new Error('降落点ID不能为空')
    }

    try {
      return await apiClient.delete(`/${id}`)
    } catch (error) {
      throw error
    }
  }

  /**
   * 查找最近的降落点
   * @param {number} lat - 纬度
   * @param {number} lng - 经度
   * @param {number} limit - 返回数量限制
   * @param {number} radius - 搜索半径（米）
   * @returns {Promise}
   */
  async findNearestLandingPoints(lat, lng, limit = 3, radius = 5000) {
    if (!lat || !lng) {
      throw new Error('纬度和经度不能为空')
    }

    try {
      return await apiClient.get('/nearest', {
        params: { lat, lng, limit, radius }
      })
    } catch (error) {
      throw error
    }
  }

  /**
   * 验证降落点数据
   * @param {Object} data - 降落点数据
   */
  validateLandingPointData(data) {
    const requiredFields = ['name', 'latitude', 'longitude']

    for (const field of requiredFields) {
      if (!data[field]) {
        throw new Error(`${field} 是必填字段`)
      }
    }

    // 验证纬度范围
    if (data.latitude < -90 || data.latitude > 90) {
      throw new Error('纬度必须在 -90 到 90 之间')
    }

    // 验证经度范围
    if (data.longitude < -180 || data.longitude > 180) {
      throw new Error('经度必须在 -180 到 180 之间')
    }
  }

  /**
   * 计算两点间的距离（使用 Haversine 公式）
   * @param {number} lat1 - 点1纬度
   * @param {number} lon1 - 点1经度
   * @param {number} lat2 - 点2纬度
   * @param {number} lon2 - 点2经度
   * @returns {number} 距离（公里）
   */
  calculateDistance(lat1, lon1, lat2, lon2) {
    const R = 6371 // 地球半径（公里）
    const dLat = this.toRad(lat2 - lat1)
    const dLon = this.toRad(lon2 - lon1)

    const a =
      Math.sin(dLat / 2) * Math.sin(dLat / 2) +
      Math.cos(this.toRad(lat1)) *
        Math.cos(this.toRad(lat2)) *
        Math.sin(dLon / 2) *
        Math.sin(dLon / 2)

    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a))
    const distance = R * c

    return distance
  }

  /**
   * 角度转弧度
   * @param {number} deg - 角度
   * @returns {number} 弧度
   */
  toRad(deg) {
    return deg * (Math.PI / 180)
  }
}

// 导出单例
const emergencyLandingApi = new EmergencyLandingApiService()
export default emergencyLandingApi
