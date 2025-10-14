/**
 * mapApi 单元测试
 * mapApi Unit Tests
 */

import { describe, it, expect, beforeEach, vi } from 'vitest'
import { MapApiService } from '@/services/mapApi.js'
import { mockMarkers, mockRoute } from './mockData.js'
import {
  createAxiosError,
  createNetworkError,
  mockAxiosInstance
} from './testUtils.js'

// Mock axios
vi.mock('axios', () => ({
  default: {
    create: vi.fn(() => mockAxiosInstance)
  }
}))

describe('MapApiService', () => {
  let mapApi

  beforeEach(() => {
    vi.clearAllMocks()
    localStorage.clear()
    mapApi = new MapApiService()
  })

  describe('geocode', () => {
    it('应该成功进行地理编码', async () => {
      const address = '沈阳故宫'
      const mockResponse = {
        data: {
          address,
          location: {
            lat: 41.796700,
            lng: 123.451200
          }
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await mapApi.geocode(address)

      expect(result.success).toBe(true)
      expect(result.data.location).toHaveProperty('lat')
      expect(result.data.location).toHaveProperty('lng')
      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/geocode', {
        params: { address }
      })
    })

    it('应该在地址为空时抛出错误', async () => {
      await expect(mapApi.geocode('')).rejects.toThrow('地址不能为空')
      await expect(mapApi.geocode(null)).rejects.toThrow('地址不能为空')
    })

    it('应该处理地理编码失败', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(404, '地址未找到')
      )

      await expect(mapApi.geocode('不存在的地址')).rejects.toMatchObject({
        code: 'NOT_FOUND',
        success: false
      })
    })
  })

  describe('getMarkers', () => {
    it('应该成功获取地图标记列表', async () => {
      const mockResponse = {
        data: {
          markers: mockMarkers,
          total: mockMarkers.length
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await mapApi.getMarkers()

      expect(result.success).toBe(true)
      expect(result.data.markers).toHaveLength(2)
      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/markers', { params: {} })
    })

    it('应该支持查询参数', async () => {
      const params = { type: 'drone' }
      mockAxiosInstance.get.mockResolvedValue({ data: { markers: [] } })

      await mapApi.getMarkers(params)

      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/markers', { params })
    })
  })

  describe('createMarker', () => {
    it('应该成功创建地图标记', async () => {
      const markerData = {
        type: 'landing_point',
        name: '测试停机坪',
        location: {
          lat: 41.800000,
          lng: 123.450000
        }
      }

      const mockResponse = {
        data: {
          id: 3,
          ...markerData
        }
      }

      mockAxiosInstance.post.mockResolvedValue(mockResponse)

      const result = await mapApi.createMarker(markerData)

      expect(result.success).toBe(true)
      expect(result.data.id).toBe(3)
      expect(mockAxiosInstance.post).toHaveBeenCalledWith('/markers', markerData)
    })

    it('应该处理创建失败', async () => {
      mockAxiosInstance.post.mockRejectedValue(
        createAxiosError(400, '标记数据无效')
      )

      await expect(mapApi.createMarker({})).rejects.toMatchObject({
        code: 'BAD_REQUEST',
        success: false
      })
    })
  })

  describe('deleteMarker', () => {
    it('应该成功删除地图标记', async () => {
      const markerId = 1

      mockAxiosInstance.delete.mockResolvedValue({
        data: { message: '删除成功' }
      })

      const result = await mapApi.deleteMarker(markerId)

      expect(result.success).toBe(true)
      expect(mockAxiosInstance.delete).toHaveBeenCalledWith(`/markers/${markerId}`)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(mapApi.deleteMarker(null)).rejects.toThrow('标记ID不能为空')
    })

    it('应该处理删除不存在的标记', async () => {
      mockAxiosInstance.delete.mockRejectedValue(
        createAxiosError(404, '标记不存在')
      )

      await expect(mapApi.deleteMarker(999)).rejects.toMatchObject({
        code: 'NOT_FOUND',
        success: false
      })
    })
  })

  describe('getRoute', () => {
    it('应该成功获取路线', async () => {
      const startLat = 41.796700
      const startLng = 123.451200
      const endLat = 41.806700
      const endLng = 123.461200

      const mockResponse = {
        data: mockRoute
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await mapApi.getRoute(startLat, startLng, endLat, endLng)

      expect(result.success).toBe(true)
      expect(result.data).toHaveProperty('start')
      expect(result.data).toHaveProperty('end')
      expect(result.data).toHaveProperty('distance')
      expect(result.data).toHaveProperty('waypoints')
      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/route', {
        params: {
          start_lat: startLat,
          start_lng: startLng,
          end_lat: endLat,
          end_lng: endLng
        }
      })
    })

    it('应该在坐标参数缺失时抛出错误', async () => {
      await expect(mapApi.getRoute(null, 123.45, 41.80, 123.46))
        .rejects.toThrow('起点和终点坐标不能为空')

      await expect(mapApi.getRoute(41.79, null, 41.80, 123.46))
        .rejects.toThrow('起点和终点坐标不能为空')

      await expect(mapApi.getRoute(41.79, 123.45, null, 123.46))
        .rejects.toThrow('起点和终点坐标不能为空')

      await expect(mapApi.getRoute(41.79, 123.45, 41.80, null))
        .rejects.toThrow('起点和终点坐标不能为空')
    })

    it('应该处理路线规划失败', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(400, '无法规划路线')
      )

      await expect(mapApi.getRoute(41.79, 123.45, 41.80, 123.46))
        .rejects.toMatchObject({
          code: 'BAD_REQUEST',
          success: false
        })
    })
  })

  describe('错误处理', () => {
    it('应该处理401未授权错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(401, '未授权')
      )

      await expect(mapApi.getMarkers()).rejects.toMatchObject({
        code: 'UNAUTHORIZED',
        success: false,
        message: '未授权，请重新登录'
      })
    })

    it('应该处理404地图数据不可用', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(404, '地图数据不可用')
      )

      await expect(mapApi.geocode('测试地址')).rejects.toMatchObject({
        code: 'NOT_FOUND',
        success: false,
        message: '地图数据不可用'
      })
    })

    it('应该处理500服务器错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(500, '地图服务暂时不可用')
      )

      await expect(mapApi.getMarkers()).rejects.toMatchObject({
        code: 'INTERNAL_ERROR',
        success: false,
        message: '地图服务暂时不可用'
      })
    })

    it('应该处理网络错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(createNetworkError())

      await expect(mapApi.getMarkers()).rejects.toMatchObject({
        code: 'NETWORK_ERROR',
        success: false,
        message: '网络连接失败'
      })
    })

    it('应该处理其他错误', async () => {
      const customError = new Error('自定义错误')
      mockAxiosInstance.get.mockRejectedValue(customError)

      await expect(mapApi.getMarkers()).rejects.toMatchObject({
        code: 'REQUEST_ERROR',
        success: false,
        message: '自定义错误'
      })
    })
  })

  describe('综合场景测试', () => {
    it('应该支持地理编码到创建标记的完整流程', async () => {
      // 1. 地理编码
      const address = '沈阳故宫'
      const geocodeResponse = {
        data: {
          address,
          location: {
            lat: 41.796700,
            lng: 123.451200
          }
        }
      }

      mockAxiosInstance.get.mockResolvedValueOnce(geocodeResponse)

      const geocodeResult = await mapApi.geocode(address)

      expect(geocodeResult.success).toBe(true)

      // 2. 使用编码结果创建标记
      const markerData = {
        type: 'landmark',
        name: address,
        location: geocodeResult.data.location
      }

      const createMarkerResponse = {
        data: {
          id: 1,
          ...markerData
        }
      }

      mockAxiosInstance.post.mockResolvedValueOnce(createMarkerResponse)

      const markerResult = await mapApi.createMarker(markerData)

      expect(markerResult.success).toBe(true)
      expect(markerResult.data.location).toEqual(geocodeResult.data.location)
    })
  })
})
