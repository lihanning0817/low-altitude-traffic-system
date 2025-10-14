/**
 * 测试工具函数
 * Test Utility Functions
 */

import { vi } from 'vitest'

/**
 * 创建Mock Axios实例
 */
export function createMockAxios() {
  return {
    create: vi.fn(() => mockAxiosInstance),
    get: vi.fn(),
    post: vi.fn(),
    put: vi.fn(),
    delete: vi.fn(),
    interceptors: {
      request: {
        use: vi.fn((success, error) => {
          return { success, error }
        })
      },
      response: {
        use: vi.fn((success, error) => {
          return { success, error }
        })
      }
    }
  }
}

/**
 * 创建Mock Axios实例
 */
export const mockAxiosInstance = {
  get: vi.fn(),
  post: vi.fn(),
  put: vi.fn(),
  delete: vi.fn(),
  interceptors: {
    request: {
      use: vi.fn()
    },
    response: {
      use: vi.fn()
    }
  }
}

/**
 * 创建成功的API响应
 */
export function createSuccessResponse(data, message = '操作成功') {
  return {
    success: true,
    code: 200,
    message,
    data
  }
}

/**
 * 创建失败的API响应
 */
export function createErrorResponse(code = 'UNKNOWN_ERROR', message = '操作失败') {
  return {
    success: false,
    code,
    message,
    error: new Error(message)
  }
}

/**
 * 创建Mock的axios错误对象
 */
export function createAxiosError(status, message = 'Request failed', data = {}) {
  const error = new Error(message)
  error.response = {
    status,
    data: {
      message,
      ...data
    }
  }
  return error
}

/**
 * 创建网络错误
 */
export function createNetworkError() {
  const error = new Error('Network Error')
  error.request = {}
  return error
}

/**
 * 等待一段时间(用于测试异步操作)
 */
export function wait(ms = 0) {
  return new Promise(resolve => setTimeout(resolve, ms))
}

/**
 * Mock localStorage
 */
export function setupLocalStorageMock() {
  const store = {}

  return {
    getItem: vi.fn((key) => store[key] || null),
    setItem: vi.fn((key, value) => {
      store[key] = value.toString()
    }),
    removeItem: vi.fn((key) => {
      delete store[key]
    }),
    clear: vi.fn(() => {
      Object.keys(store).forEach(key => delete store[key])
    })
  }
}

/**
 * 验证API响应格式
 */
export function validateApiResponse(response) {
  return (
    typeof response === 'object' &&
    response !== null &&
    'success' in response &&
    'data' in response &&
    'message' in response
  )
}
