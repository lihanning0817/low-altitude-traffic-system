/**
 * 空域管理API服务
 * Airspace Management API Service
 *
 * 提供空域的管理功能
 * - 获取空域列表
 * - 获取空域详情
 * - 创建/更新/删除空域
 * - 检查位置是否在空域内
 */

import axios from 'axios'

class AirspaceApiService {
  constructor() {
    this.baseURL = '/api/v1/airspaces'

    this.api = axios.create({
      baseURL: this.baseURL,
      timeout: 10000,
      headers: { 'Content-Type': 'application/json' }
    })

    // 请求拦截器
    this.api.interceptors.request.use(
      (config) => {
        const token = localStorage.getItem('access_token')
        if (token) config.headers.Authorization = `Bearer ${token}`
        return config
      },
      (error) => Promise.reject(error)
    )

    // 响应拦截器
    this.api.interceptors.response.use(
      (response) => response.data,
      (error) => this.handleError(error)
    )
  }

  /**
   * 获取空域列表
   */
  async getAirspaces(params = {}) {
    try {
      const response = await this.api.get('/', { params })
      return {
        success: true,
        data: response.data,
        message: response.message || '获取空域列表成功'
      }
    } catch (error) {
      console.error('[AirspaceAPI] 获取空域列表失败:', error)
      throw error
    }
  }

  /**
   * 获取空域详情
   */
  async getAirspaceById(airspaceId) {
    try {
      if (!airspaceId) throw new Error('空域ID不能为空')
      const response = await this.api.get(`/${airspaceId}`)
      return {
        success: true,
        data: response.data,
        message: response.message || '获取空域详情成功'
      }
    } catch (error) {
      console.error(`[AirspaceAPI] 获取空域详情失败 (ID: ${airspaceId}):`, error)
      throw error
    }
  }

  /**
   * 创建空域
   */
  async createAirspace(airspaceData) {
    try {
      this.validateAirspaceData(airspaceData)
      const response = await this.api.post('/', airspaceData)
      return {
        success: true,
        data: response.data,
        message: response.message || '创建空域成功'
      }
    } catch (error) {
      console.error('[AirspaceAPI] 创建空域失败:', error)
      throw error
    }
  }

  /**
   * 更新空域
   */
  async updateAirspace(airspaceId, airspaceData) {
    try {
      if (!airspaceId) throw new Error('空域ID不能为空')
      const response = await this.api.put(`/${airspaceId}`, airspaceData)
      return {
        success: true,
        data: response.data,
        message: response.message || '更新空域成功'
      }
    } catch (error) {
      console.error(`[AirspaceAPI] 更新空域失败 (ID: ${airspaceId}):`, error)
      throw error
    }
  }

  /**
   * 删除空域
   */
  async deleteAirspace(airspaceId) {
    try {
      if (!airspaceId) throw new Error('空域ID不能为空')
      const response = await this.api.delete(`/${airspaceId}`)
      return {
        success: true,
        data: response.data,
        message: response.message || '删除空域成功'
      }
    } catch (error) {
      console.error(`[AirspaceAPI] 删除空域失败 (ID: ${airspaceId}):`, error)
      throw error
    }
  }

  /**
   * 检查位置是否在空域内
   */
  checkPointInAirspace(lat, lng, airspace) {
    if (!airspace) return false

    const inLatRange = lat >= airspace.south_lat && lat <= airspace.north_lat
    const inLngRange = lng >= airspace.west_lng && lng <= airspace.east_lng

    return inLatRange && inLngRange
  }

  /**
   * 查找包含指定位置的空域
   */
  async findAirspacesContainingPoint(lat, lng) {
    try {
      const response = await this.getAirspaces()
      const airspaces = response.data.airspaces || []

      const matchingAirspaces = airspaces.filter(airspace =>
        this.checkPointInAirspace(lat, lng, airspace)
      )

      return {
        success: true,
        data: {
          airspaces: matchingAirspaces,
          count: matchingAirspaces.length,
          location: { lat, lng }
        },
        message: `找到 ${matchingAirspaces.length} 个包含该位置的空域`
      }
    } catch (error) {
      console.error('[AirspaceAPI] 查找空域失败:', error)
      throw error
    }
  }

  /**
   * 获取空域统计
   */
  async getAirspaceStatistics() {
    try {
      const response = await this.getAirspaces()
      const airspaces = response.data.airspaces || []

      const stats = {
        total: airspaces.length,
        by_type: {},
        by_status: {},
        avg_max_altitude: 0,
        total_max_concurrent: 0
      }

      let totalMaxAlt = 0

      airspaces.forEach(airspace => {
        stats.by_type[airspace.type] = (stats.by_type[airspace.type] || 0) + 1
        stats.by_status[airspace.status] = (stats.by_status[airspace.status] || 0) + 1
        totalMaxAlt += airspace.max_altitude || 0
        stats.total_max_concurrent += airspace.max_concurrent_flights || 0
      })

      stats.avg_max_altitude = airspaces.length > 0 ?
        Math.round(totalMaxAlt / airspaces.length) : 0

      return {
        success: true,
        data: stats,
        message: '获取空域统计成功'
      }
    } catch (error) {
      console.error('[AirspaceAPI] 获取空域统计失败:', error)
      throw error
    }
  }

  /**
   * 验证空域数据
   */
  validateAirspaceData(data) {
    const required = ['name', 'type']
    for (const field of required) {
      if (!data[field]) throw new Error(`${field} 是必填字段`)
    }

    if (data.north_lat && data.south_lat && data.north_lat <= data.south_lat) {
      throw new Error('北纬必须大于南纬')
    }

    if (data.east_lng && data.west_lng && data.east_lng <= data.west_lng) {
      throw new Error('东经必须大于西经')
    }

    if (data.max_altitude && data.min_altitude && data.max_altitude <= data.min_altitude) {
      throw new Error('最大高度必须大于最小高度')
    }
  }

  /**
   * 错误处理
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
          errorMessage = '无权限操作空域'
          errorCode = 'FORBIDDEN'
          break
        case 404:
          errorMessage = '空域不存在'
          errorCode = 'NOT_FOUND'
          break
        case 409:
          errorMessage = data.message || '空域冲突'
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

    console.error('[AirspaceAPI] Error:', { code: errorCode, message: errorMessage })
    return Promise.reject({ success: false, code: errorCode, message: errorMessage, error })
  }
}

const airspaceApi = new AirspaceApiService()
export default airspaceApi
export { AirspaceApiService }
