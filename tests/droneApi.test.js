/**
 * droneApi 单元测试
 * droneApi Unit Tests
 */

import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mockDrones } from './mockData.js'
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

import { DroneApiService } from '@/services/droneApi.js'

describe('DroneApiService', () => {
  let droneApi

  beforeEach(() => {
    vi.clearAllMocks()
    localStorage.clear()
    droneApi = new DroneApiService()
  })

  describe('getDrones', () => {
    it('应该成功获取无人机列表', async () => {
      const mockResponse = {
        data: {
          drones: mockDrones,
          total: mockDrones.length
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await droneApi.getDrones()

      expect(result.success).toBe(true)
      expect(result.data.drones).toHaveLength(3)
      expect(result.data.total).toBe(3)
      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/', { params: {} })
    })

    it('应该支持查询参数', async () => {
      const params = { status: 'active' }
      mockAxiosInstance.get.mockResolvedValue({ data: { drones: [] } })

      await droneApi.getDrones(params)

      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/', { params })
    })
  })

  describe('getDroneById', () => {
    it('应该成功获取单个无人机详情', async () => {
      const droneId = 1
      const mockResponse = { data: mockDrones[0] }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await droneApi.getDroneById(droneId)

      expect(result.success).toBe(true)
      expect(result.data).toEqual(mockDrones[0])
      expect(mockAxiosInstance.get).toHaveBeenCalledWith(`/${droneId}`)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(droneApi.getDroneById(null)).rejects.toThrow('无人机ID不能为空')
    })

    it('应该在无人机不存在时返回404错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(404, '无人机不存在')
      )

      await expect(droneApi.getDroneById(999)).rejects.toMatchObject({
        code: 'NOT_FOUND',
        success: false
      })
    })
  })

  describe('createDrone', () => {
    it('应该成功创建无人机', async () => {
      const newDrone = {
        drone_id: 'UAV-004',
        model: 'DJI Mini 3',
        owner_id: 4
      }

      const mockResponse = { data: { id: 4, ...newDrone } }
      mockAxiosInstance.post.mockResolvedValue(mockResponse)

      const result = await droneApi.createDrone(newDrone)

      expect(result.success).toBe(true)
      expect(result.data.drone_id).toBe('UAV-004')
      expect(mockAxiosInstance.post).toHaveBeenCalledWith('/', newDrone)
    })

    it('应该验证必填字段', async () => {
      const invalidDrone = { model: 'DJI Mini 3' }

      await expect(droneApi.createDrone(invalidDrone)).rejects.toThrow('drone_id 是必填字段')
    })
  })

  describe('updateDrone', () => {
    it('应该成功更新无人机信息', async () => {
      const droneId = 1
      const updateData = { status: 'maintenance' }

      mockAxiosInstance.put.mockResolvedValue({ data: { ...mockDrones[0], ...updateData } })

      const result = await droneApi.updateDrone(droneId, updateData)

      expect(result.success).toBe(true)
      expect(mockAxiosInstance.put).toHaveBeenCalledWith(`/${droneId}`, updateData)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(droneApi.updateDrone(null, {})).rejects.toThrow('无人机ID不能为空')
    })
  })

  describe('deleteDrone', () => {
    it('应该成功删除无人机', async () => {
      const droneId = 1

      mockAxiosInstance.delete.mockResolvedValue({ data: { message: '删除成功' } })

      const result = await droneApi.deleteDrone(droneId)

      expect(result.success).toBe(true)
      expect(mockAxiosInstance.delete).toHaveBeenCalledWith(`/${droneId}`)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(droneApi.deleteDrone(null)).rejects.toThrow('无人机ID不能为空')
    })
  })

  describe('getLowBatteryDrones', () => {
    it('应该获取低电量无人机列表', async () => {
      const mockResponse = { data: { drones: mockDrones } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await droneApi.getLowBatteryDrones(20)

      expect(result.success).toBe(true)
      expect(result.data.drones).toHaveLength(1)
      expect(result.data.drones[0].battery).toBeLessThanOrEqual(20)
    })

    it('应该使用默认阈值20%', async () => {
      const mockResponse = { data: { drones: mockDrones } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await droneApi.getLowBatteryDrones()

      expect(result.success).toBe(true)
      result.data.drones.forEach(drone => {
        expect(drone.battery).toBeLessThanOrEqual(20)
      })
    })
  })

  describe('getNearbyDrones', () => {
    it('应该获取指定位置附近的无人机', async () => {
      const mockResponse = { data: { drones: mockDrones } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await droneApi.getNearbyDrones(41.796700, 123.451200, 5)

      expect(result.success).toBe(true)
      expect(result.data.drones).toBeInstanceOf(Array)
      result.data.drones.forEach(drone => {
        expect(drone).toHaveProperty('distance')
        expect(drone.distance).toBeLessThanOrEqual(5)
      })
    })
  })

  describe('calculateDistance', () => {
    it('应该正确计算两点间的距离', () => {
      // 沈阳故宫到北陵公园的大致距离
      const distance = droneApi.calculateDistance(
        41.796700, 123.451200, // 沈阳故宫
        41.845000, 123.420000  // 北陵公园
      )

      expect(distance).toBeGreaterThan(0)
      expect(distance).toBeLessThan(10) // 应该在10公里以内
    })

    it('应该在相同位置时返回0', () => {
      const distance = droneApi.calculateDistance(
        41.796700, 123.451200,
        41.796700, 123.451200
      )

      expect(distance).toBe(0)
    })
  })

  describe('getDroneStatistics', () => {
    it('应该获取无人机统计信息', async () => {
      const mockResponse = { data: { drones: mockDrones } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await droneApi.getDroneStatistics()

      expect(result.success).toBe(true)
      expect(result.data).toHaveProperty('total')
      expect(result.data).toHaveProperty('by_status')
      expect(result.data).toHaveProperty('avg_battery')
      expect(result.data.total).toBe(3)
    })

    it('应该正确计算平均电量', async () => {
      const mockResponse = { data: { drones: mockDrones } }
      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await droneApi.getDroneStatistics()

      // (85 + 15 + 100) / 3 = 66.67
      expect(result.data.avg_battery).toBeCloseTo(66.67, 0)
    })
  })

  describe('错误处理', () => {
    it('应该处理401未授权错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(401, '未授权')
      )

      await expect(droneApi.getDrones()).rejects.toMatchObject({
        code: 'UNAUTHORIZED',
        success: false,
        message: '未授权，请重新登录'
      })
    })

    it('应该处理网络错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(createNetworkError())

      await expect(droneApi.getDrones()).rejects.toMatchObject({
        code: 'NETWORK_ERROR',
        success: false,
        message: '网络连接失败'
      })
    })

    it('应该处理500服务器错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(500, '服务器内部错误')
      )

      await expect(droneApi.getDrones()).rejects.toMatchObject({
        code: 'INTERNAL_ERROR',
        success: false
      })
    })
  })

  describe('validateDroneData', () => {
    it('应该验证必填字段', () => {
      expect(() => {
        droneApi.validateDroneData({})
      }).toThrow('drone_id 是必填字段')

      expect(() => {
        droneApi.validateDroneData({ drone_id: 'UAV-001' })
      }).toThrow('model 是必填字段')

      expect(() => {
        droneApi.validateDroneData({ drone_id: 'UAV-001', model: 'DJI' })
      }).toThrow('owner_id 是必填字段')
    })

    it('应该在数据有效时不抛出错误', () => {
      expect(() => {
        droneApi.validateDroneData({
          drone_id: 'UAV-001',
          model: 'DJI Phantom 4',
          owner_id: 1
        })
      }).not.toThrow()
    })
  })
})
