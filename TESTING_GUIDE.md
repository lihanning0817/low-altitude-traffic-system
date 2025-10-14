# API服务层单元测试指南

## 概述

本项目为所有新创建的API服务类编写了完整的单元测试，使用Vitest作为测试框架。测试覆盖了以下4个API服务：

- `droneApi` - 无人机管理API
- `airspaceApi` - 空域管理API
- `permitApi` - 飞行许可API
- `mapApi` - 地图服务API

## 测试框架配置

### 已安装的依赖

```json
{
  "devDependencies": {
    "@vitejs/plugin-vue": "^5.0.0",
    "@vitest/ui": "^1.3.0",
    "jsdom": "^24.0.0",
    "happy-dom": "^13.0.0",
    "vite": "^5.0.0",
    "vitest": "^1.3.0"
  }
}
```

### 配置文件

#### vitest.config.js
- 配置Vue插件支持
- 设置jsdom测试环境
- 配置路径别名 `@` 指向 `./src`
- 配置代码覆盖率报告

#### tests/setup.js
- Mock localStorage
- Mock console方法减少测试输出噪音

## 测试文件结构

```
tests/
├── setup.js              # 测试环境设置
├── mockData.js           # Mock测试数据
├── testUtils.js          # 测试工具函数
├── droneApi.test.js      # 无人机API测试
├── airspaceApi.test.js   # 空域API测试
├── permitApi.test.js     # 飞行许可API测试
└── mapApi.test.js        # 地图服务API测试
```

## 运行测试

### 安装依赖

首先需要安装测试相关的依赖：

```bash
npm install
```

### 测试命令

```bash
# 运行所有测试
npm test

# 运行测试并生成覆盖率报告
npm run test:coverage

# 运行测试UI界面
npm run test:ui
```

## 测试覆盖内容

### 1. droneApi测试 (tests/droneApi.test.js)

测试覆盖:
- ✅ 获取无人机列表 (getDrones)
- ✅ 获取单个无人机详情 (getDroneById)
- ✅ 创建无人机 (createDrone)
- ✅ 更新无人机信息 (updateDrone)
- ✅ 删除无人机 (deleteDrone)
- ✅ 获取低电量无人机 (getLowBatteryDrones)
- ✅ 获取附近的无人机 (getNearbyDrones)
- ✅ 计算两点距离 (calculateDistance)
- ✅ 获取无人机统计信息 (getDroneStatistics)
- ✅ 数据验证 (validateDroneData)
- ✅ 错误处理 (401, 404, 500, 网络错误)

### 2. airspaceApi测试 (tests/airspaceApi.test.js)

测试覆盖:
- ✅ 获取空域列表 (getAirspaces)
- ✅ 获取单个空域详情 (getAirspaceById)
- ✅ 创建空域 (createAirspace)
- ✅ 更新空域信息 (updateAirspace)
- ✅ 删除空域 (deleteAirspace)
- ✅ 检查点是否在空域内 (checkPointInAirspace)
- ✅ 查找包含指定位置的空域 (findAirspacesContainingPoint)
- ✅ 获取空域统计信息 (getAirspaceStatistics)
- ✅ 数据验证 (validateAirspaceData)
  - 北纬必须大于南纬
  - 东经必须大于西经
  - 最大高度必须大于最小高度
- ✅ 错误处理 (401, 403, 404, 409, 500, 网络错误)

### 3. permitApi测试 (tests/permitApi.test.js)

测试覆盖:
- ✅ 获取飞行许可列表 (getPermits)
- ✅ 获取单个许可详情 (getPermitById)
- ✅ 申请飞行许可 (applyPermit)
- ✅ 批准飞行许可 (approvePermit)
- ✅ 拒绝飞行许可 (rejectPermit)
- ✅ 撤销飞行许可 (revokePermit)
- ✅ 获取待审批的许可 (getPendingPermits)
- ✅ 获取我的许可列表 (getMyPermits)
- ✅ 获取许可统计信息 (getPermitStatistics)
- ✅ 数据验证 (validatePermitData)
  - 必填字段验证
  - 结束时间必须晚于开始时间
  - 开始时间不能早于当前时间
  - 单次飞行时长不超过24小时
- ✅ 错误处理 (401, 403, 404, 409, 500, 网络错误)

### 4. mapApi测试 (tests/mapApi.test.js)

测试覆盖:
- ✅ 地理编码 (geocode)
- ✅ 获取地图标记列表 (getMarkers)
- ✅ 创建地图标记 (createMarker)
- ✅ 删除地图标记 (deleteMarker)
- ✅ 获取路线 (getRoute)
- ✅ 参数验证
- ✅ 错误处理 (401, 404, 500, 网络错误)
- ✅ 综合场景测试 (地理编码→创建标记)

## Mock数据说明

### mockData.js

提供了完整的测试数据：

- `mockDrones` - 3个无人机测试数据
- `mockAirspaces` - 2个空域测试数据
- `mockPermits` - 2个飞行许可测试数据
- `mockTasks` - 2个飞行任务测试数据
- `mockEmergencyEvents` - 应急事件数据
- `mockLandingPoints` - 应急降落点数据
- `mockWeather` - 天气数据
- `mockMarkers` - 地图标记数据
- `mockRoute` - 路线数据

### testUtils.js

提供了测试工具函数：

- `createMockAxios()` - 创建Mock Axios实例
- `createSuccessResponse()` - 创建成功响应
- `createErrorResponse()` - 创建错误响应
- `createAxiosError()` - 创建Axios错误对象
- `createNetworkError()` - 创建网络错误
- `wait()` - 等待函数
- `setupLocalStorageMock()` - Mock localStorage
- `validateApiResponse()` - 验证API响应格式

## 测试模式说明

每个测试文件都遵循相同的结构：

```javascript
describe('ServiceName', () => {
  let serviceApi

  beforeEach(() => {
    // 清理Mock和重置状态
    vi.clearAllMocks()
    localStorage.clear()
    serviceApi = new ServiceApiClass()
  })

  describe('methodName', () => {
    it('应该成功执行...', async () => {
      // 1. 准备Mock数据
      // 2. Mock axios响应
      // 3. 调用API方法
      // 4. 断言结果
    })

    it('应该处理错误...', async () => {
      // 测试错误场景
    })
  })
})
```

## 常见测试场景

### 1. 成功场景测试

```javascript
it('应该成功获取数据', async () => {
  const mockResponse = { data: { items: [] } }
  mockAxiosInstance.get.mockResolvedValue(mockResponse)

  const result = await api.getData()

  expect(result.success).toBe(true)
  expect(result.data).toBeDefined()
})
```

### 2. 参数验证测试

```javascript
it('应该验证必填字段', async () => {
  const invalidData = {}

  await expect(api.createItem(invalidData))
    .rejects.toThrow('字段不能为空')
})
```

### 3. 错误处理测试

```javascript
it('应该处理401错误', async () => {
  mockAxiosInstance.get.mockRejectedValue(
    createAxiosError(401, '未授权')
  )

  await expect(api.getData()).rejects.toMatchObject({
    code: 'UNAUTHORIZED',
    success: false
  })
})
```

### 4. 业务逻辑测试

```javascript
it('应该正确过滤低电量无人机', async () => {
  const mockResponse = { data: { drones: mockDrones } }
  mockAxiosInstance.get.mockResolvedValue(mockResponse)

  const result = await droneApi.getLowBatteryDrones(20)

  result.data.drones.forEach(drone => {
    expect(drone.battery).toBeLessThanOrEqual(20)
  })
})
```

## 代码覆盖率目标

当前测试覆盖率目标：
- **语句覆盖率 (Statements)**: > 80%
- **分支覆盖率 (Branches)**: > 75%
- **函数覆盖率 (Functions)**: > 90%
- **行覆盖率 (Lines)**: > 80%

查看覆盖率报告：
```bash
npm run test:coverage
```

覆盖率报告会生成在 `coverage/` 目录下，可以在浏览器中打开 `coverage/index.html` 查看详细报告。

## 持续集成

建议在CI/CD流程中添加测试步骤：

```yaml
# .github/workflows/test.yml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - uses: actions/setup-node@v2
        with:
          node-version: '18'
      - run: npm install
      - run: npm test
      - run: npm run test:coverage
```

## 最佳实践

1. **保持测试独立**: 每个测试应该独立运行，不依赖其他测试
2. **使用describe分组**: 按功能模块组织测试用例
3. **清晰的测试名称**: 使用"应该..."格式描述测试期望
4. **Mock外部依赖**: 所有外部依赖(axios, localStorage)都应该Mock
5. **测试边界条件**: 包括空值、null、undefined等边界情况
6. **测试错误路径**: 不仅测试成功场景，也要测试失败场景
7. **保持测试简单**: 一个测试只验证一个行为

## 常见问题

### Q: 如何Mock axios?
A: 在测试文件顶部使用 `vi.mock('axios')` 并提供Mock实现

### Q: 如何测试异步方法?
A: 使用 `async/await` 或返回Promise，确保测试函数是异步的

### Q: 如何处理localStorage?
A: setup.js中已经Mock了localStorage，直接使用即可

### Q: 测试运行很慢怎么办?
A: 使用 `--run` 参数禁用watch模式：`vitest --run`

## 下一步

1. 为其他已有的API服务(authApi, flightTaskApi等)编写单元测试
2. 增加集成测试
3. 添加E2E测试
4. 提高代码覆盖率到90%以上

## 参考资料

- [Vitest官方文档](https://vitest.dev/)
- [Vue Test Utils](https://test-utils.vuejs.org/)
- [Testing Library](https://testing-library.com/)
