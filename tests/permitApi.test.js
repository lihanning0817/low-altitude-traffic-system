/**
 * permitApi 单元测试
 * permitApi Unit Tests
 */

import { describe, it, expect, beforeEach, vi } from 'vitest'
import { PermitApiService } from '@/services/permitApi.js'
import { mockPermits } from './mockData.js'
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

describe('PermitApiService', () => {
  let permitApi

  beforeEach(() => {
    vi.clearAllMocks()
    localStorage.clear()
    permitApi = new PermitApiService()
  })

  describe('getPermits', () => {
    it('应该成功获取飞行许可列表', async () => {
      const mockResponse = {
        data: {
          permits: mockPermits,
          total: mockPermits.length
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await permitApi.getPermits()

      expect(result.success).toBe(true)
      expect(result.data.permits).toHaveLength(2)
      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/', { params: {} })
    })

    it('应该支持查询参数', async () => {
      const params = { status: 'pending' }
      mockAxiosInstance.get.mockResolvedValue({ data: { permits: [] } })

      await permitApi.getPermits(params)

      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/', { params })
    })
  })

  describe('getPermitById', () => {
    it('应该成功获取单个许可详情', async () => {
      const permitId = 1
      const mockResponse = { data: mockPermits[0] }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await permitApi.getPermitById(permitId)

      expect(result.success).toBe(true)
      expect(result.data).toEqual(mockPermits[0])
      expect(mockAxiosInstance.get).toHaveBeenCalledWith(`/${permitId}`)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(permitApi.getPermitById(null)).rejects.toThrow('许可ID不能为空')
    })
  })

  describe('applyPermit', () => {
    it('应该成功申请飞行许可', async () => {
      const permitData = {
        task_id: 7,
        airspace_id: 1,
        start_time: '2025-10-15 14:00:00',
        end_time: '2025-10-15 16:00:00',
        remarks: '测试飞行'
      }

      const mockResponse = {
        data: {
          id: 3,
          permit_code: 'FP-2025101400003',
          ...permitData
        }
      }

      mockAxiosInstance.post.mockResolvedValue(mockResponse)

      const result = await permitApi.applyPermit(permitData)

      expect(result.success).toBe(true)
      expect(result.data.permit_code).toBeTruthy()
      expect(mockAxiosInstance.post).toHaveBeenCalledWith('/', permitData)
    })

    it('应该验证必填字段', async () => {
      const invalidPermit = {
        task_id: 7,
        start_time: '2025-10-15 14:00:00'
      }

      await expect(permitApi.applyPermit(invalidPermit)).rejects.toThrow('是必填字段')
    })

    it('应该验证时间有效性', async () => {
      const invalidPermit = {
        task_id: 7,
        airspace_id: 1,
        start_time: '2025-10-15 16:00:00',
        end_time: '2025-10-15 14:00:00' // 结束时间早于开始时间
      }

      await expect(permitApi.applyPermit(invalidPermit)).rejects.toThrow('结束时间必须晚于开始时间')
    })

    it('应该验证飞行时长不超过24小时', async () => {
      const invalidPermit = {
        task_id: 7,
        airspace_id: 1,
        start_time: '2025-10-15 08:00:00',
        end_time: '2025-10-16 09:00:00' // 超过24小时
      }

      await expect(permitApi.applyPermit(invalidPermit)).rejects.toThrow('单次飞行许可时长不能超过24小时')
    })
  })

  describe('approvePermit', () => {
    it('应该成功批准飞行许可', async () => {
      const permitId = 1
      const approvalData = {
        approver_id: 4,
        remarks: '批准通过'
      }

      mockAxiosInstance.post.mockResolvedValue({
        data: { ...mockPermits[0], status: 'approved' }
      })

      const result = await permitApi.approvePermit(permitId, approvalData)

      expect(result.success).toBe(true)
      expect(result.message).toContain('批准')
      expect(mockAxiosInstance.post).toHaveBeenCalledWith(`/${permitId}/approve`, approvalData)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(permitApi.approvePermit(null, {})).rejects.toThrow('许可ID不能为空')
    })
  })

  describe('rejectPermit', () => {
    it('应该成功拒绝飞行许可', async () => {
      const permitId = 1
      const rejectionData = {
        reason: '天气不佳',
        rejecter_id: 4
      }

      mockAxiosInstance.post.mockResolvedValue({
        data: { ...mockPermits[0], status: 'rejected' }
      })

      const result = await permitApi.rejectPermit(permitId, rejectionData)

      expect(result.success).toBe(true)
      expect(result.message).toContain('拒绝')
      expect(mockAxiosInstance.post).toHaveBeenCalledWith(`/${permitId}/reject`, rejectionData)
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(permitApi.rejectPermit(null, { reason: '测试' })).rejects.toThrow('许可ID不能为空')
    })

    it('应该在拒绝原因为空时抛出错误', async () => {
      await expect(permitApi.rejectPermit(1, {})).rejects.toThrow('拒绝原因不能为空')
    })
  })

  describe('revokePermit', () => {
    it('应该成功撤销飞行许可', async () => {
      const permitId = 2
      const reason = '任务取消'

      mockAxiosInstance.post.mockResolvedValue({
        data: { ...mockPermits[1], status: 'revoked' }
      })

      const result = await permitApi.revokePermit(permitId, reason)

      expect(result.success).toBe(true)
      expect(result.message).toContain('撤销')
      expect(mockAxiosInstance.post).toHaveBeenCalledWith(`/${permitId}/revoke`, { reason })
    })

    it('应该在ID为空时抛出错误', async () => {
      await expect(permitApi.revokePermit(null, '测试')).rejects.toThrow('许可ID不能为空')
    })
  })

  describe('getPendingPermits', () => {
    it('应该获取待审批的许可列表', async () => {
      const mockResponse = {
        data: {
          permits: mockPermits
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await permitApi.getPendingPermits()

      expect(result.success).toBe(true)
      expect(result.data.permits.every(p => p.status === 'pending')).toBe(true)
    })
  })

  describe('getMyPermits', () => {
    it('应该获取我的许可列表', async () => {
      const userId = 4
      const mockResponse = {
        data: {
          permits: mockPermits
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await permitApi.getMyPermits(userId)

      expect(result.success).toBe(true)
      expect(mockAxiosInstance.get).toHaveBeenCalledWith('/', { params: { applicant_id: userId } })
    })

    it('应该在用户ID为空时抛出错误', async () => {
      await expect(permitApi.getMyPermits(null)).rejects.toThrow('用户ID不能为空')
    })
  })

  describe('getPermitStatistics', () => {
    it('应该获取许可统计信息', async () => {
      const mockResponse = {
        data: {
          permits: mockPermits
        }
      }

      mockAxiosInstance.get.mockResolvedValue(mockResponse)

      const result = await permitApi.getPermitStatistics()

      expect(result.success).toBe(true)
      expect(result.data).toHaveProperty('total')
      expect(result.data).toHaveProperty('by_status')
      expect(result.data.total).toBe(2)
      expect(result.data.by_status).toHaveProperty('pending', 1)
      expect(result.data.by_status).toHaveProperty('approved', 1)
    })
  })

  describe('validatePermitData', () => {
    it('应该验证必填字段', () => {
      expect(() => {
        permitApi.validatePermitData({})
      }).toThrow('task_id 是必填字段')

      expect(() => {
        permitApi.validatePermitData({ task_id: 1 })
      }).toThrow('airspace_id 是必填字段')

      expect(() => {
        permitApi.validatePermitData({ task_id: 1, airspace_id: 1 })
      }).toThrow('start_time 是必填字段')

      expect(() => {
        permitApi.validatePermitData({
          task_id: 1,
          airspace_id: 1,
          start_time: '2025-10-15 14:00:00'
        })
      }).toThrow('end_time 是必填字段')
    })

    it('应该验证开始时间不早于当前时间', () => {
      const pastDate = new Date(Date.now() - 1000 * 60 * 60 * 24) // 昨天
      const futureDate = new Date(Date.now() + 1000 * 60 * 60 * 2) // 2小时后

      expect(() => {
        permitApi.validatePermitData({
          task_id: 1,
          airspace_id: 1,
          start_time: pastDate.toISOString(),
          end_time: futureDate.toISOString()
        })
      }).toThrow('开始时间不能早于当前时间')
    })

    it('应该在数据有效时不抛出错误', () => {
      const startTime = new Date(Date.now() + 1000 * 60 * 60) // 1小时后
      const endTime = new Date(Date.now() + 1000 * 60 * 60 * 2) // 2小时后

      expect(() => {
        permitApi.validatePermitData({
          task_id: 1,
          airspace_id: 1,
          start_time: startTime.toISOString(),
          end_time: endTime.toISOString()
        })
      }).not.toThrow()
    })
  })

  describe('错误处理', () => {
    it('应该处理401未授权错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(401, '未授权')
      )

      await expect(permitApi.getPermits()).rejects.toMatchObject({
        code: 'UNAUTHORIZED',
        success: false,
        message: '未授权，请重新登录'
      })
    })

    it('应该处理403权限错误', async () => {
      mockAxiosInstance.post.mockRejectedValue(
        createAxiosError(403, '无权限操作飞行许可')
      )

      const permitData = {
        task_id: 1,
        airspace_id: 1,
        start_time: '2025-10-15 14:00:00',
        end_time: '2025-10-15 16:00:00'
      }

      await expect(permitApi.applyPermit(permitData)).rejects.toMatchObject({
        code: 'FORBIDDEN',
        success: false,
        message: '无权限操作飞行许可'
      })
    })

    it('应该处理404不存在错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(404, '飞行许可不存在')
      )

      await expect(permitApi.getPermitById(999)).rejects.toMatchObject({
        code: 'NOT_FOUND',
        success: false,
        message: '飞行许可不存在'
      })
    })

    it('应该处理409冲突错误', async () => {
      mockAxiosInstance.post.mockRejectedValue(
        createAxiosError(409, '飞行时间冲突')
      )

      const permitData = {
        task_id: 1,
        airspace_id: 1,
        start_time: '2025-10-15 14:00:00',
        end_time: '2025-10-15 16:00:00'
      }

      await expect(permitApi.applyPermit(permitData)).rejects.toMatchObject({
        code: 'CONFLICT',
        success: false
      })
    })

    it('应该处理网络错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(createNetworkError())

      await expect(permitApi.getPermits()).rejects.toMatchObject({
        code: 'NETWORK_ERROR',
        success: false,
        message: '网络连接失败'
      })
    })

    it('应该处理500服务器错误', async () => {
      mockAxiosInstance.get.mockRejectedValue(
        createAxiosError(500, '服务器内部错误')
      )

      await expect(permitApi.getPermits()).rejects.toMatchObject({
        code: 'INTERNAL_ERROR',
        success: false,
        message: '服务器内部错误'
      })
    })
  })
})
