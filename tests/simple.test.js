/**
 * 简单测试示例
 */

import { describe, it, expect } from 'vitest'

describe('Simple Test', () => {
  it('应该通过基本测试', () => {
    expect(1 + 1).toBe(2)
  })

  it('应该测试字符串', () => {
    expect('hello').toBe('hello')
  })
})
