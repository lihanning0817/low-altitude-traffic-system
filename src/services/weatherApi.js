/**
 * 天气API服务
 * 与后端WeatherController交互
 */

import axios from 'axios'
import { ElMessage } from 'element-plus'
import authApi from './authApi'

class WeatherApiService {
  constructor() {
    // 使用相对路径，通过Vue代理转发到后端
    this.baseURL = '/api/v1/weather'

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

  // ========== 天气接口 ==========

  /**
   * 根据城市获取当前天气
   * @param {string} city - 城市名称
   * @param {string} country - 国家代码（可选）
   * @returns {Promise}
   */
  async getCurrentWeatherByCity(city, country = '') {
    try {
      const params = { city }
      if (country) {
        params.country = country
      }

      const response = await this.api.get('/current', { params })
      return response.data
    } catch (error) {
      console.error('获取城市天气失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 根据坐标获取当前天气
   * @param {number} lat - 纬度
   * @param {number} lon - 经度
   * @returns {Promise}
   */
  async getCurrentWeatherByCoords(lat, lon) {
    try {
      const response = await this.api.get('/current/coords', {
        params: { lat, lon }
      })
      return response.data
    } catch (error) {
      console.error('获取坐标天气失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 根据坐标获取天气预报（5天）
   * @param {number} lat - 纬度
   * @param {number} lon - 经度
   * @returns {Promise}
   */
  async getForecast(lat, lon) {
    try {
      const response = await this.api.get('/forecast', {
        params: { lat, lon }
      })
      return response.data
    } catch (error) {
      console.error('获取天气预报失败:', error)
      throw this.processError(error)
    }
  }

  /**
   * 检查指定位置的飞行安全性
   * @param {number} lat - 纬度
   * @param {number} lon - 经度
   * @returns {Promise}
   */
  async checkFlightSafety(lat, lon) {
    try {
      const response = await this.api.get('/flight-safety', {
        params: { lat, lon }
      })
      return response.data
    } catch (error) {
      console.error('检查飞行安全失败:', error)
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
   * 获取天气图标URL
   * @param {string} iconCode - 天气图标代码
   * @returns {string}
   */
  getWeatherIconUrl(iconCode) {
    return `https://openweathermap.org/img/wn/${iconCode}@2x.png`
  }

  /**
   * 根据天气状况获取中文描述
   * @param {string} condition - 天气状况
   * @returns {string}
   */
  getWeatherDescription(condition) {
    const descriptions = {
      'clear sky': '晴空',
      'few clouds': '少云',
      'scattered clouds': '多云',
      'broken clouds': '阴天',
      'overcast clouds': '阴天',
      'shower rain': '阵雨',
      'rain': '雨',
      'thunderstorm': '雷暴',
      'snow': '雪',
      'mist': '薄雾',
      'fog': '雾'
    }
    return descriptions[condition.toLowerCase()] || condition
  }

  /**
   * 获取风向文字描述
   * @param {number} deg - 风向角度（0-360）
   * @returns {string}
   */
  getWindDirection(deg) {
    const directions = ['北', '东北', '东', '东南', '南', '西南', '西', '西北']
    const index = Math.round(deg / 45) % 8
    return directions[index]
  }

  /**
   * 获取飞行安全等级颜色
   * @param {number} score - 安全评分 (0-100)
   * @returns {string}
   */
  getSafetyColor(score) {
    if (score >= 80) return '#67C23A' // 绿色 - 安全
    if (score >= 60) return '#E6A23C' // 橙色 - 警告
    if (score >= 40) return '#F56C6C' // 红色 - 危险
    return '#909399' // 灰色 - 禁止
  }

  /**
   * 获取飞行安全等级文字
   * @param {number} score - 安全评分 (0-100)
   * @returns {string}
   */
  getSafetyLevel(score) {
    if (score >= 80) return '安全'
    if (score >= 60) return '警告'
    if (score >= 40) return '危险'
    return '禁止'
  }

  /**
   * 格式化温度显示
   * @param {number} temp - 温度（摄氏度）
   * @returns {string}
   */
  formatTemperature(temp) {
    return `${temp.toFixed(1)}°C`
  }

  /**
   * 格式化湿度显示
   * @param {number} humidity - 湿度百分比
   * @returns {string}
   */
  formatHumidity(humidity) {
    return `${humidity}%`
  }

  /**
   * 格式化风速显示
   * @param {number} speed - 风速（m/s）
   * @returns {string}
   */
  formatWindSpeed(speed) {
    return `${speed.toFixed(1)} m/s`
  }

  /**
   * 格式化能见度显示
   * @param {number} visibility - 能见度（米）
   * @returns {string}
   */
  formatVisibility(visibility) {
    if (visibility >= 1000) {
      return `${(visibility / 1000).toFixed(1)} km`
    }
    return `${visibility} m`
  }
}

// 创建单例实例
const weatherApi = new WeatherApiService()

export default weatherApi
