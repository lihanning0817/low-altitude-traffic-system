/**
 * 地图服务API
 * Map Service API
 */

import axios from 'axios'

class MapApiService {
  constructor() {
    this.baseURL = '/api/v1/map'

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

  // 地理编码 (地址转坐标)
  async geocode(address) {
    try {
      if (!address) throw new Error('地址不能为空')
      const response = await this.api.get('/geocode', {
        params: { address }
      })
      return {
        success: true,
        data: response.data,
        message: response.message || '地理编码成功'
      }
    } catch (error) {
      console.error('[MapAPI] 地理编码失败:', error)
      throw error
    }
  }

  // 获取地图标记列表
  async getMarkers(params = {}) {
    try {
      const response = await this.api.get('/markers', { params })
      return {
        success: true,
        data: response.data,
        message: response.message || '获取地图标记成功'
      }
    } catch (error) {
      console.error('[MapAPI] 获取标记失败:', error)
      throw error
    }
  }

  // 创建地图标记
  async createMarker(markerData) {
    try {
      const response = await this.api.post('/markers', markerData)
      return {
        success: true,
        data: response.data,
        message: response.message || '创建标记成功'
      }
    } catch (error) {
      console.error('[MapAPI] 创建标记失败:', error)
      throw error
    }
  }

  // 删除地图标记
  async deleteMarker(markerId) {
    try {
      if (!markerId) throw new Error('标记ID不能为空')
      const response = await this.api.delete(`/markers/${markerId}`)
      return {
        success: true,
        data: response.data,
        message: response.message || '删除标记成功'
      }
    } catch (error) {
      console.error(`[MapAPI] 删除标记失败 (ID: ${markerId}):`, error)
      throw error
    }
  }

  // 获取路线
  async getRoute(startLat, startLng, endLat, endLng) {
    try {
      if (!startLat || !startLng || !endLat || !endLng) {
        throw new Error('起点和终点坐标不能为空')
      }

      const response = await this.api.get('/route', {
        params: {
          start_lat: startLat,
          start_lng: startLng,
          end_lat: endLat,
          end_lng: endLng
        }
      })

      return {
        success: true,
        data: response.data,
        message: response.message || '获取路线成功'
      }
    } catch (error) {
      console.error('[MapAPI] 获取路线失败:', error)
      throw error
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
        case 404:
          errorMessage = '地图数据不可用'
          errorCode = 'NOT_FOUND'
          break
        case 500:
          errorMessage = '地图服务暂时不可用'
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

    console.error('[MapAPI] Error:', { code: errorCode, message: errorMessage })
    return Promise.reject({ success: false, code: errorCode, message: errorMessage, error })
  }
}

const mapApi = new MapApiService()
export default mapApi
export { MapApiService }
