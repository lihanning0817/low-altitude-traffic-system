<template>
  <div class="api-test">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">🔌 API连接测试</h1>
      <p class="page-subtitle">前后端接口连接性测试工具</p>
    </div>

    <!-- 综合测试按钮 -->
    <AppleCard class="quick-test-card">
      <div class="quick-test-content">
        <div class="quick-test-info">
          <h2 class="quick-test-title">快速测试</h2>
          <p class="quick-test-desc">一键运行所有API连接测试</p>
        </div>
        <AppleButton
          :disabled="allTesting"
          size="large"
          @click="runAllTests"
        >
          {{ allTesting ? '⏳ 测试中...' : '▶️ 运行所有测试' }}
        </AppleButton>
      </div>

      <!-- 测试摘要 -->
      <div v-if="testSummary" class="test-summary">
        <div :class="['summary-alert', testSummary.success ? 'success' : 'error']">
          <div class="alert-icon">{{ testSummary.success ? '✅' : '❌' }}</div>
          <div class="alert-content">
            <div class="alert-title">{{ testSummary.success ? '测试通过' : '测试失败' }}</div>
            <div class="alert-message">{{ testSummary.message }}</div>
          </div>
        </div>
      </div>
    </AppleCard>

    <!-- 测试项列表 -->
    <div class="test-items">
      <!-- 健康检查 -->
      <AppleCard class="test-card">
        <div class="test-header">
          <div class="test-info">
            <h3 class="test-title">💚 健康检查</h3>
            <p class="test-description">检查后端服务是否正常运行</p>
          </div>
          <AppleButton
            variant="secondary"
            :disabled="loading.health"
            @click="testHealth"
          >
            {{ loading.health ? '⏳ 测试中' : '🧪 测试' }}
          </AppleButton>
        </div>
        <div v-if="results.health" class="test-result">
          <div class="result-label">测试结果</div>
          <pre class="result-content">{{ JSON.stringify(results.health, null, 2) }}</pre>
        </div>
      </AppleCard>

      <!-- 系统信息 -->
      <AppleCard class="test-card">
        <div class="test-header">
          <div class="test-info">
            <h3 class="test-title">ℹ️ 系统信息</h3>
            <p class="test-description">获取后端系统信息和配置</p>
          </div>
          <AppleButton
            variant="secondary"
            :disabled="loading.info"
            @click="testSystemInfo"
          >
            {{ loading.info ? '⏳ 测试中' : '🧪 测试' }}
          </AppleButton>
        </div>
        <div v-if="results.info" class="test-result">
          <div class="result-label">测试结果</div>
          <pre class="result-content">{{ JSON.stringify(results.info, null, 2) }}</pre>
        </div>
      </AppleCard>

      <!-- 无人机数据 -->
      <AppleCard class="test-card">
        <div class="test-header">
          <div class="test-info">
            <h3 class="test-title">🚁 无人机数据</h3>
            <p class="test-description">获取无人机列表和状态信息</p>
          </div>
          <AppleButton
            variant="secondary"
            :disabled="loading.drones"
            @click="testDrones"
          >
            {{ loading.drones ? '⏳ 测试中' : '🧪 测试' }}
          </AppleButton>
        </div>
        <div v-if="results.drones" class="test-result">
          <div class="result-label">测试结果</div>
          <pre class="result-content">{{ JSON.stringify(results.drones, null, 2) }}</pre>
        </div>
      </AppleCard>

      <!-- 任务数据 -->
      <AppleCard class="test-card">
        <div class="test-header">
          <div class="test-info">
            <h3 class="test-title">📋 任务数据</h3>
            <p class="test-description">获取飞行任务列表和详情</p>
          </div>
          <AppleButton
            variant="secondary"
            :disabled="loading.tasks"
            @click="testTasks"
          >
            {{ loading.tasks ? '⏳ 测试中' : '🧪 测试' }}
          </AppleButton>
        </div>
        <div v-if="results.tasks" class="test-result">
          <div class="result-label">测试结果</div>
          <pre class="result-content">{{ JSON.stringify(results.tasks, null, 2) }}</pre>
        </div>
      </AppleCard>

      <!-- 用户数据 -->
      <AppleCard class="test-card">
        <div class="test-header">
          <div class="test-info">
            <h3 class="test-title">👤 用户数据</h3>
            <p class="test-description">获取用户列表和权限信息</p>
          </div>
          <AppleButton
            variant="secondary"
            :disabled="loading.users"
            @click="testUsers"
          >
            {{ loading.users ? '⏳ 测试中' : '🧪 测试' }}
          </AppleButton>
        </div>
        <div v-if="results.users" class="test-result">
          <div class="result-label">测试结果</div>
          <pre class="result-content">{{ JSON.stringify(results.users, null, 2) }}</pre>
        </div>
      </AppleCard>
    </div>

    <!-- 错误信息 -->
    <AppleCard v-if="errors.length > 0" class="errors-card">
      <h3 class="errors-title">❌ 错误信息</h3>
      <div class="errors-list">
        <div
          v-for="(error, index) in errors"
          :key="index"
          class="error-item"
        >
          <div class="error-icon">⚠️</div>
          <div class="error-message">{{ error }}</div>
        </div>
      </div>
    </AppleCard>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div v-if="showToast" :class="['toast-notification', toastType]">
        <div class="toast-icon">{{ toastIcon }}</div>
        <div class="toast-message">{{ toastMessage }}</div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import AppleCard from '@/components/apple/AppleCard.vue'
import AppleButton from '@/components/apple/AppleButton.vue'
import apiService from '@/services/api'

// 响应式数据
const loading = ref({
  health: false,
  info: false,
  drones: false,
  tasks: false,
  users: false
})

const results = ref({
  health: null,
  info: null,
  drones: null,
  tasks: null,
  users: null
})

const errors = ref([])
const allTesting = ref(false)
const testSummary = ref(null)

// Toast 通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

// Toast 通知方法
const showToastNotification = (message, type = 'success') => {
  toastMessage.value = message
  toastType.value = type
  showToast.value = true

  if (toastTimer) {
    clearTimeout(toastTimer)
  }

  toastTimer = setTimeout(() => {
    showToast.value = false
  }, 3000)
}

// API测试方法
const testHealth = async () => {
  loading.value.health = true
  clearErrors()
  try {
    const result = await apiService.getHealth()
    results.value.health = result
    showToastNotification('健康检查成功', 'success')
  } catch (error) {
    handleError('健康检查', error)
  } finally {
    loading.value.health = false
  }
}

const testSystemInfo = async () => {
  loading.value.info = true
  clearErrors()
  try {
    const result = await apiService.getSystemInfo()
    results.value.info = result
    showToastNotification('获取系统信息成功', 'success')
  } catch (error) {
    handleError('系统信息', error)
  } finally {
    loading.value.info = false
  }
}

const testDrones = async () => {
  loading.value.drones = true
  clearErrors()
  try {
    const result = await apiService.getDrones()
    results.value.drones = result
    showToastNotification('获取无人机数据成功', 'success')
  } catch (error) {
    handleError('无人机数据', error)
  } finally {
    loading.value.drones = false
  }
}

const testTasks = async () => {
  loading.value.tasks = true
  clearErrors()
  try {
    const result = await apiService.getTasks()
    results.value.tasks = result
    showToastNotification('获取任务数据成功', 'success')
  } catch (error) {
    handleError('任务数据', error)
  } finally {
    loading.value.tasks = false
  }
}

const testUsers = async () => {
  loading.value.users = true
  clearErrors()
  try {
    const result = await apiService.getUsers()
    results.value.users = result
    showToastNotification('获取用户数据成功', 'success')
  } catch (error) {
    handleError('用户数据', error)
  } finally {
    loading.value.users = false
  }
}

const runAllTests = async () => {
  allTesting.value = true
  clearAll()

  let successCount = 0
  let totalCount = 0

  const tests = [
    { name: '健康检查', method: testHealth },
    { name: '系统信息', method: testSystemInfo },
    { name: '无人机数据', method: testDrones },
    { name: '任务数据', method: testTasks },
    { name: '用户数据', method: testUsers }
  ]

  for (const test of tests) {
    totalCount++
    try {
      await test.method()
      successCount++
    } catch (error) {
      console.error(`${test.name}测试失败:`, error)
    }
  }

  testSummary.value = {
    success: successCount === totalCount,
    message: `测试完成: ${successCount}/${totalCount} 个API测试通过`
  }

  if (successCount === totalCount) {
    showToastNotification('所有API测试通过！前后端连接正常', 'success')
  } else {
    showToastNotification(`部分测试失败: ${successCount}/${totalCount} 通过`, 'warning')
  }

  allTesting.value = false
}

const handleError = (testName, error) => {
  const errorMsg = `${testName}失败: ${error.message}`
  errors.value.push(errorMsg)
  showToastNotification(errorMsg, 'error')
}

const clearErrors = () => {
  errors.value = []
}

const clearAll = () => {
  results.value = {
    health: null,
    info: null,
    drones: null,
    tasks: null,
    users: null
  }
  errors.value = []
  testSummary.value = null
}
</script>

<style scoped>
.api-test {
  min-height: 100vh;
  background: var(--color-bg-secondary, #F5F5F7);
  padding: var(--space-6, 24px);
}

/* 页面头部 */
.page-header {
  max-width: 1200px;
  margin: 0 auto var(--space-8, 32px);
  text-align: center;
}

.page-title {
  margin: 0 0 var(--space-2, 8px) 0;
  font-size: var(--font-size-3xl, 32px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  letter-spacing: var(--letter-spacing-tight, -0.02em);
}

.page-subtitle {
  margin: 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
}

/* 快速测试卡片 */
.quick-test-card {
  max-width: 1200px;
  margin: 0 auto var(--space-6, 24px);
  padding: var(--space-6, 24px);
}

.quick-test-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: var(--space-4, 16px);
  margin-bottom: var(--space-5, 20px);
}

.quick-test-info {
  flex: 1;
}

.quick-test-title {
  margin: 0 0 var(--space-1, 4px) 0;
  font-size: var(--font-size-xl, 24px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.quick-test-desc {
  margin: 0;
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 测试摘要 */
.test-summary {
  padding-top: var(--space-5, 20px);
  border-top: 1px solid var(--color-border, #E5E5E7);
}

.summary-alert {
  display: flex;
  align-items: flex-start;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px);
  border-radius: var(--radius-lg, 12px);
}

.summary-alert.success {
  background: rgba(52, 199, 89, 0.1);
  border: 1px solid rgba(52, 199, 89, 0.3);
}

.summary-alert.error {
  background: rgba(255, 59, 48, 0.1);
  border: 1px solid rgba(255, 59, 48, 0.3);
}

.alert-icon {
  font-size: 24px;
  line-height: 1;
}

.alert-content {
  flex: 1;
}

.alert-title {
  font-size: var(--font-size-base, 16px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin-bottom: var(--space-1, 4px);
}

.alert-message {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 测试项列表 */
.test-items {
  max-width: 1200px;
  margin: 0 auto;
  display: flex;
  flex-direction: column;
  gap: var(--space-4, 16px);
}

/* 测试卡片 */
.test-card {
  padding: var(--space-5, 20px);
}

.test-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: var(--space-4, 16px);
}

.test-info {
  flex: 1;
}

.test-title {
  margin: 0 0 var(--space-1, 4px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.test-description {
  margin: 0;
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 测试结果 */
.test-result {
  margin-top: var(--space-5, 20px);
  padding-top: var(--space-5, 20px);
  border-top: 1px solid var(--color-border, #E5E5E7);
}

.result-label {
  font-size: var(--font-size-sm, 14px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin-bottom: var(--space-2, 8px);
}

.result-content {
  margin: 0;
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-md, 8px);
  font-family: 'SF Mono', 'Monaco', 'Inconsolata', 'Fira Code', 'Droid Sans Mono', 'Source Code Pro', monospace;
  font-size: var(--font-size-xs, 12px);
  line-height: 1.6;
  color: var(--color-text-primary, #1D1D1F);
  max-height: 300px;
  overflow-y: auto;
  overflow-x: auto;
  white-space: pre;
  word-break: break-all;
}

/* 错误信息卡片 */
.errors-card {
  max-width: 1200px;
  margin: var(--space-6, 24px) auto 0;
  padding: var(--space-5, 20px);
  background: rgba(255, 59, 48, 0.05);
  border: 1px solid rgba(255, 59, 48, 0.2);
}

.errors-title {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: #FF3B30;
}

.errors-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-2, 8px);
}

.error-item {
  display: flex;
  align-items: flex-start;
  gap: var(--space-2, 8px);
  padding: var(--space-3, 12px);
  background: var(--color-bg-primary, #FFFFFF);
  border-left: 4px solid #FF3B30;
  border-radius: var(--radius-sm, 6px);
}

.error-icon {
  font-size: 18px;
  line-height: 1;
  flex-shrink: 0;
}

.error-message {
  flex: 1;
  font-size: var(--font-size-sm, 14px);
  color: #FF3B30;
  line-height: 1.5;
}

/* Toast 通知 */
.toast-notification {
  position: fixed;
  top: var(--space-8, 32px);
  right: var(--space-6, 24px);
  z-index: 10000;
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px) var(--space-5, 20px);
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-lg, 12px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  min-width: 280px;
  max-width: 400px;
}

.toast-icon {
  font-size: 24px;
  line-height: 1;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.toast-notification.success {
  border-left: 4px solid #34C759;
}

.toast-notification.error {
  border-left: 4px solid #FF3B30;
}

.toast-notification.warning {
  border-left: 4px solid #FF9500;
}

.toast-notification.info {
  border-left: 4px solid #007AFF;
}

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

@keyframes slideInRight {
  from {
    transform: translateX(100%);
    opacity: 0;
  }
  to {
    transform: translateX(0);
    opacity: 1;
  }
}

@keyframes slideOutRight {
  from {
    transform: translateX(0);
    opacity: 1;
  }
  to {
    transform: translateX(100%);
    opacity: 0;
  }
}

/* 响应式设计 */
@media (max-width: 768px) {
  .api-test {
    padding: var(--space-4, 16px);
  }

  .page-title {
    font-size: var(--font-size-2xl, 28px);
  }

  .quick-test-content {
    flex-direction: column;
    align-items: stretch;
  }

  .test-header {
    flex-direction: column;
    align-items: stretch;
  }

  .toast-notification {
    right: var(--space-4, 16px);
    left: var(--space-4, 16px);
    min-width: auto;
  }
}

/* 深色模式 */
@media (prefers-color-scheme: dark) {
  .api-test {
    background: #000000;
  }

  .page-title,
  .quick-test-title,
  .test-title,
  .result-label,
  .alert-title {
    color: #F5F5F7;
  }

  .page-subtitle,
  .quick-test-desc,
  .test-description,
  .alert-message {
    color: #98989D;
  }

  .result-content {
    background: #1C1C1E;
    border-color: #38383A;
    color: #F5F5F7;
  }

  .summary-alert.success {
    background: rgba(52, 199, 89, 0.15);
    border-color: rgba(52, 199, 89, 0.4);
  }

  .summary-alert.error {
    background: rgba(255, 59, 48, 0.15);
    border-color: rgba(255, 59, 48, 0.4);
  }

  .errors-card {
    background: rgba(255, 59, 48, 0.1);
    border-color: rgba(255, 59, 48, 0.3);
  }

  .error-item {
    background: #1C1C1E;
  }

  .toast-notification {
    background: rgba(28, 28, 30, 0.95);
  }

  .toast-message {
    color: #F5F5F7;
  }
}
</style>
