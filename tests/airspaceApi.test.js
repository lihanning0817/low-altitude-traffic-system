/**
 * airspaceApi 单元测试
 * airspaceApi Unit Tests
 */

import { describe, it, expect, beforeEach, vi } from 'vitest'
import { AirspaceApiService } from '@/services/airspaceApi.js'
import { mockAirspaces } from './mockData.js'
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

describe('AirspaceApiService', () => {
  let airspaceApi

  beforeEach(() => {
    vi.clearAllMocks()
    localStorage.clear()
    airspaceApi = new AirspaceApiService()
  })

  describe('getAirspaces', () => {
    it('应该成功获取空域列表', async () => {
      const mockResponse = {
        data: {
          airspaces: mockAirspaces,
          total: mockAirspaces.length
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await airspaceApi.getAirspaces()

      expect(result.success).toBe(true)
      expect(result.data.airspaces).toHaveLength(2)
      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/', { params: {} })
    })

    it('应该支持查询参数', async () => {
      const params = { type: 'restricted' }
      mockAxiosInstance.get.mockResolvedValue({ data: { airspaces: [] } })

      await airspaceApi.getAirspaces(params)

      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/', { params })
    })
  })

  describe('getAirspaceById', () => {
    it('应该成功获取单个空域详情', async () => {
      const airspaceId = 1
      const mockResponse = { data: mockAirspaces[0] }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await airspaceApi.getAirspaceById(airspaceId)

      expect(result.success).toBe(true)
      expect(result.data).toEqual(mockAirspaces[0])
      expect(mockAxiosInstance.get).toHaveBeenCalledWith(`/${airspaceId}`)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(airspaceApi.getAirspaceById(null)).rejects.toThrow('空域ID不能为空')
    })
  })

  describe('createAirspace', () => {
    it('应该成功创建空域', async () => {
      const newAirspace = {
        name: '测试空域',
        type: 'controlled',
        north_lat: 42.0,
        south_lat: 41.0,
        east_lng: 124.0,
        west_lng: 123.0
      }

      const mockResponse = { data: { id: 3, ...newAirspace } }
      mockAxiosInstance.post.mockResolvedValue(mockResponse)

      const result = await airspaceApi.createAirspace(newAirspace)

      expect(result.success).toBe(true)
      expect(result.data.name).toBe('测试空域')
      expect(mockAxiosInstance.post).toHaveBeenCalledWith('/', newAirspace)
    })

    it('应该验证必填字段', async () => {
      const invalidAirspace = { type: 'controlled' }

      await expect(airspaceApi.createAirspace(invalidAirspace)).rejects.toThrow('name 是必填字段')
    })

    it('应该验证北纬大于南纬', async () => {
      const invalidAirspace = {
        name: '测试空域',
        type: 'controlled',
        north_lat: 41.0,
        south_lat: 42.0
      }

      await expect(airspaceApi.createAirspace(invalidAirspace)).rejects.toThrow('北纬必须大于南纬')
    })

    it('应该验证东经大于西经', async () => {
      const invalidAirspace = {
        name: '测试空域',
        type: 'controlled',
        north_lat: 42.0,
        south_lat: 41.0,
        east_lng: 123.0,
        west_lng: 124.0
      }

      await expect(airspaceApi.createAirspace(invalidAirspace)).rejects.toThrow('东经必须大于西经')
    })

    it('应该验证最大高度大于最小高度', async () => {
      const invalidAirspace = {
        name: '测试空域',
        type: 'controlled',
        max_altitude: 100,
        min_altitude: 200
      }

      await expect(airspaceApi.createAirspace(invalidAirspace)).rejects.toThrow('最大高度必须大于最小高度')
    })
  })

  describe('updateAirspace', () => {
    it('应该成功更新空域信息', async () => {
      const airspaceId = 1
      const updateData = { status: 'inactive' }

      mockAxiosInstance.put.mockResolvedValue({ data: { ...mockAirspaces[0], ...updateData } })

      const result = await airspaceApi.updateAirspace(airspaceId, updateData)

      expect(result.success).toBe(true)
      expect(mockAxiosInstance.put).toHaveBeenCalledWith(`/${airspaceId}`, updateData)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(airspaceApi.updateAirspace(null, {})).rejects.toThrow('空域ID不能为空')
    })
  })

  describe('deleteAirspace', () => {
    it('应该成功删除空域', async () => {
      const airspaceId = 1

      mockAxiosInstance.delete.mockResolvedValue({ data: { message: '删除成功' } })

      const result = await airspaceApi.deleteAirspace(airspaceId)

      expect(result.success).toBe(true)
      expect(mockAxiosInstance.delete).toHaveBeenCalledWith(`/${airspaceId}`)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(airspaceApi.deleteAirspace(null)).rejects.toThrow('空域ID不能为空')
    })
  })

  describe('checkPointInAirspace', () => {
    it('应该正确判断点是否在空域内', () => {
      const airspace = mockAirspaces[0] // 沈阳故宫文化遗产保护区

      // 在空域内的点
      expect(airspaceApi.checkPointInAirspace(41.795, 123.450, airspace)).toBe(true)

      // 在空域外的点
      expect(airspaceApi.checkPointInAirspace(41.810, 123.450, airspace)).toBe(false)
      expect(airspaceApi.checkPointInAirspace(41.795, 123.470, airspace)).toBe(false)
    })

    it('应该在空域为空时返回false', () => {
      expect(airspaceApi.checkPointInAirspace(41.795, 123.450, null)).toBe(false)
    })
  })

  describe('findAirspacesContainingPoint', () => {
    it('应该找到包含指定位置的空域', async () => {
      const mockResponse = { data: { airspaces: mockAirspaces } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      // 在沈阳故宫保护区内的点
      const result = await airspaceApi.findAirspacesContainingPoint(41.795, 123.450)

      expect(result.success).toBe(true)
      expect(result.data.airspaces.length).toBeGreaterThan(0)
      expect(result.data.location).toEqual({ lat: 41.795, lng: 123.450 })
    })

    it('应该在没有匹配的空域时返回空数组', async () => {
      const mockResponse = { data: { airspaces: mockAirspaces } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      // 不在任何空域内的点
      const result = await airspaceApi.findAirspacesContainingPoint(50.0, 130.0)

      expect(result.success).toBe(true)
      expect(result.data.airspaces).toHaveLength(0)
      expect(result.data.count).toBe(0)
    })
  })

  describe('getAirspaceStatistics', () => {
    it('应该获取空域统计信息', async () => {
      const mockResponse = { data: { airspaces: mockAirspaces } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await airspaceApi.getAirspaceStatistics()

      expect(result.success).toBe(true)
      expect(result.data).toHaveProperty('total')
      expect(result.data).toHaveProperty('by_type')
      expect(result.data).toHaveProperty('by_status')
      expect(result.data).toHaveProperty('avg_max_altitude')
      expect(result.data.total).toBe(2)
    })

    it('应该正确计算平均最大高度', async () => {
      const mockResponse = { data: { airspaces: mockAirspaces } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await airspaceApi.getAirspaceStatistics()

      // (150 + 200) / 2 = 175
      expect(result.data.avg_max_altitude).toBe(175)
    })

    it('应该正确统计空域类型', async () => {
      const mockResponse = { data: { airspaces: mockAirspaces } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await airspaceApi.getAirspaceStatistics()

      expect(result.data.by_type).toHaveProperty('restricted', 1)
      expect(result.data.by_type).toHaveProperty('controlled', 1)
    })
  })

  describe('错误处理', () => {
    it('应该处理401未授权错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(401, '未授权')
      )

      await expect(airspaceApi.getAirspaces()).rejects.toMatchObject({
        code: 'UNAUTHORIZED',
        success: false,
        message: '未授权，请重新登录'
      })
    })

    it('应该处理403权限错误', async () => {
      mockAxiosInstance.post.mockRejectedValue(
        createAxiosError(403, '无权限操作空域')
      )

      const newAirspace = { name: '测试', type: 'controlled' }
      await expect(airspaceApi.createAirspace(newAirspace)).rejects.toMatchObject({
        code: 'FORBIDDEN',
        success: false,
        message: '无权限操作空域'
      })
    })

    it('应该处理404不存在错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(404, '空域不存在')
      )

      await expect(airspaceApi.getAirspaceById(999)).rejects.toMatchObject({
        code: 'NOT_FOUND',
        success: false,
        message: '空域不存在'
      })
    })

    it('应该处理409冲突错误', async () => {
      mockAxiosInstance.post.mockRejectedValue(
        createAxiosError(409, '空域范围重叠')
      )

      const newAirspace = { name: '测试', type: 'controlled' }
      await expect(airspaceApi.createAirspace(newAirspace)).rejects.toMatchObject({
        code: 'CONFLICT',
        success: false
      })
    })

    it('应该处理网络错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(createNetworkError())

      await expect(airspaceApi.getAirspaces()).rejects.toMatchObject({
        code: 'NETWORK_ERROR',
        success: false,
        message: '网络连接失败'
      })
    })

    it('应该处理500服务器错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(500, '服务器内部错误')
      )

      await expect(airspaceApi.getAirspaces()).rejects.toMatchObject({
        code: 'INTERNAL_ERROR',
        success: false,
        message: '服务器内部错误'
      })
    })
  })

  describe('validateAirspaceData', () => {
    it('应该验证必填字段', () => {
      expect(() => {
        airspaceApi.validateAirspaceData({})
      }).toThrow('name 是必填字段')

      expect(() => {
        airspaceApi.validateAirspaceData({ name: '测试空域' })
      }).toThrow('type 是必填字段')
    })

    it('应该在数据有效时不抛出错误', () => {
      expect(() => {
        airspaceApi.validateAirspaceData({
          name: '测试空域',
          type: 'controlled',
          north_lat: 42.0,
          south_lat: 41.0,
          east_lng: 124.0,
          west_lng: 123.0,
          max_altitude: 200,
          min_altitude: 0
        })
      }).not.toThrow()
    })
  })
})
