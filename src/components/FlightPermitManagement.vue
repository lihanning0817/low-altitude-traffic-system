<template>
  <div class="flight-permit-management">
    <!-- 页面标题 -->
    <div class="page-header">
      <h1 class="page-title">飞行许可管理</h1>
      <p class="page-subtitle">申请、审批和管理飞行许可证</p>
    </div>

    <!-- Tab 切换 -->
    <div class="tabs">
      <button
        v-for="tab in tabs"
        :key="tab.value"
        :class="['tab-button', { 'tab-button--active': activeTab === tab.value }]"
        @click="activeTab = tab.value"
      >
        {{ tab.label }}
        <span v-if="tab.count !== undefined" class="tab-badge">{{ tab.count }}</span>
      </button>
    </div>

    <!-- 申请许可 Tab -->
    <div v-if="activeTab === 'apply'" class="tab-content">
      <AppleCard class="apply-card">
        <template #header>
          <h2>申请飞行许可</h2>
        </template>

        <form @submit.prevent="handleSubmitApplication">
          <div class="form-row">
            <AppleInput
              v-model="application.taskName"
              label="飞行任务名称"
              placeholder="请输入任务名称"
              required
            />
          </div>

          <div class="form-row">
            <div class="form-col">
              <label class="form-label">飞行任务</label>
              <select v-model="application.taskId" class="apple-select" required>
                <option value="">选择飞行任务</option>
                <option v-for="task in tasks" :key="task.id" :value="task.id">
                  {{ task.task_name || task.name }}
                </option>
              </select>
            </div>

            <div class="form-col">
              <label class="form-label">空域</label>
              <select v-model="application.airspaceId" class="apple-select" required>
                <option value="">选择空域</option>
                <option v-for="airspace in airspaces" :key="airspace.id" :value="airspace.id">
                  {{ airspace.name }}
                </option>
              </select>
            </div>
          </div>

          <div class="form-row">
            <div class="form-col">
              <AppleInput
                v-model="application.startTime"
                type="datetime-local"
                label="起飞时间"
                required
              />
            </div>

            <div class="form-col">
              <AppleInput
                v-model="application.endTime"
                type="datetime-local"
                label="降落时间"
                required
              />
            </div>
          </div>

          <div class="form-row">
            <label class="form-label">备注说明</label>
            <textarea
              v-model="application.remarks"
              class="apple-textarea"
              rows="4"
              placeholder="请输入飞行任务的详细说明..."
            />
          </div>

          <div class="form-actions">
            <AppleButton type="submit" variant="primary" :loading="submitting">
              提交申请
            </AppleButton>
            <AppleButton type="button" variant="secondary" @click="resetApplication">
              重置
            </AppleButton>
          </div>
        </form>
      </AppleCard>
    </div>

    <!-- 我的申请 Tab -->
    <div v-if="activeTab === 'my'" class="tab-content">
      <div v-if="loading" class="loading-state">
        <div class="loading-spinner">加载中...</div>
      </div>

      <div v-else-if="myPermits.length === 0" class="empty-state">
        <p>暂无申请记录</p>
      </div>

      <div v-else class="permits-grid">
        <AppleCard
          v-for="permit in myPermits"
          :key="permit.id"
          hoverable
          class="permit-card"
        >
          <div class="permit-header">
            <h3 class="permit-title">{{ permit.task_name || '飞行任务' }}</h3>
            <span :class="['permit-status', `permit-status--${permit.status}`]">
              {{ getStatusText(permit.status) }}
            </span>
          </div>

          <div class="permit-info">
            <div class="info-item">
              <span class="info-label">许可编号:</span>
              <span class="info-value">{{ permit.permit_code }}</span>
            </div>
            <div class="info-item">
              <span class="info-label">空域:</span>
              <span class="info-value">{{ permit.airspace_name || '未知' }}</span>
            </div>
            <div class="info-item">
              <span class="info-label">飞行时间:</span>
              <span class="info-value">{{ formatDateTime(permit.start_time) }} ~ {{ formatDateTime(permit.end_time) }}</span>
            </div>
            <div v-if="permit.remarks" class="info-item">
              <span class="info-label">备注:</span>
              <span class="info-value">{{ permit.remarks }}</span>
            </div>
          </div>
        </AppleCard>
      </div>
    </div>

    <!-- 待审批 Tab (仅管理员) -->
    <div v-if="activeTab === 'pending'" class="tab-content">
      <div v-if="loading" class="loading-state">
        <div class="loading-spinner">加载中...</div>
      </div>

      <div v-else-if="pendingPermits.length === 0" class="empty-state">
        <p>暂无待审批许可</p>
      </div>

      <div v-else class="permits-grid">
        <AppleCard
          v-for="permit in pendingPermits"
          :key="permit.id"
          hoverable
          class="permit-card"
        >
          <div class="permit-header">
            <h3 class="permit-title">{{ permit.task_name || '飞行任务' }}</h3>
            <span class="permit-status permit-status--pending">待审批</span>
          </div>

          <div class="permit-info">
            <div class="info-item">
              <span class="info-label">许可编号:</span>
              <span class="info-value">{{ permit.permit_code }}</span>
            </div>
            <div class="info-item">
              <span class="info-label">申请人:</span>
              <span class="info-value">{{ permit.applicant_name || '未知' }}</span>
            </div>
            <div class="info-item">
              <span class="info-label">飞行时间:</span>
              <span class="info-value">{{ formatDateTime(permit.start_time) }} ~ {{ formatDateTime(permit.end_time) }}</span>
            </div>
          </div>

          <div class="permit-actions">
            <AppleButton size="small" variant="primary" @click="handleApprove(permit.id)">
              批准
            </AppleButton>
            <AppleButton size="small" variant="danger" @click="handleReject(permit.id)">
              拒绝
            </AppleButton>
          </div>
        </AppleCard>
      </div>
    </div>

    <!-- 所有许可 Tab -->
    <div v-if="activeTab === 'all'" class="tab-content">
      <div v-if="loading" class="loading-state">
        <div class="loading-spinner">加载中...</div>
      </div>

      <div v-else-if="allPermits.length === 0" class="empty-state">
        <p>暂无许可记录</p>
      </div>

      <div v-else class="permits-grid">
        <AppleCard
          v-for="permit in allPermits"
          :key="permit.id"
          hoverable
          class="permit-card"
        >
          <div class="permit-header">
            <h3 class="permit-title">{{ permit.task_name || '飞行任务' }}</h3>
            <span :class="['permit-status', `permit-status--${permit.status}`]">
              {{ getStatusText(permit.status) }}
            </span>
          </div>

          <div class="permit-info">
            <div class="info-item">
              <span class="info-label">许可编号:</span>
              <span class="info-value">{{ permit.permit_code }}</span>
            </div>
            <div class="info-item">
              <span class="info-label">申请时间:</span>
              <span class="info-value">{{ formatDateTime(permit.application_time) }}</span>
            </div>
            <div class="info-item">
              <span class="info-label">飞行时间:</span>
              <span class="info-value">{{ formatDateTime(permit.start_time) }} ~ {{ formatDateTime(permit.end_time) }}</span>
            </div>
          </div>
        </AppleCard>
      </div>
    </div>

    <!-- Toast 通知 -->
    <Transition name="toast-slide">
      <div
        v-if="toastState.show"
        :class="['toast-notification', `toast-notification--${toastState.type}`]"
        @click="closeToast"
      >
        <div class="toast-icon">
          <svg v-if="toastState.type === 'success'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M20 6L9 17l-5-5"/>
          </svg>
          <svg v-else-if="toastState.type === 'error'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="12" cy="12" r="10"/>
            <line x1="15" y1="9" x2="9" y2="15"/>
            <line x1="9" y1="9" x2="15" y2="15"/>
          </svg>
          <svg v-else-if="toastState.type === 'warning'" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/>
            <line x1="12" y1="9" x2="12" y2="13"/>
            <line x1="12" y1="17" x2="12.01" y2="17"/>
          </svg>
          <svg v-else viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="12" cy="12" r="10"/>
            <line x1="12" y1="16" x2="12" y2="12"/>
            <line x1="12" y1="8" x2="12.01" y2="8"/>
          </svg>
        </div>
        <div class="toast-message">{{ toastState.message }}</div>
        <button class="toast-close" @click.stop="closeToast">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <line x1="18" y1="6" x2="6" y2="18"/>
            <line x1="6" y1="6" x2="18" y2="18"/>
          </svg>
        </button>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted, computed } from 'vue'
import { AppleButton, AppleCard, AppleInput } from '@/components/apple'
import permitApi from '@/services/permitApi'
import taskApi from '@/services/taskApi'
import airspaceApi from '@/services/airspaceApi'

// 状态
const activeTab = ref('apply')
const loading = ref(false)
const submitting = ref(false)

// Toast 通知状态
const toastState = reactive({
  show: false,
  message: '',
  type: 'info', // success, error, warning, info
  duration: 3000
})

let toastTimer = null

// 显示 Toast 通知
const showToastNotification = (message, type = 'info', duration = 3000) => {
  if (toastTimer) {
    clearTimeout(toastTimer)
  }

  toastState.show = false

  setTimeout(() => {
    toastState.message = message
    toastState.type = type
    toastState.duration = duration
    toastState.show = true

    toastTimer = setTimeout(() => {
      toastState.show = false
    }, duration)
  }, 100)
}

// 关闭 Toast
const closeToast = () => {
  toastState.show = false
  if (toastTimer) {
    clearTimeout(toastTimer)
  }
}

// 数据
const tasks = ref([])
const airspaces = ref([])
const myPermits = ref([])
const pendingPermits = ref([])
const allPermits = ref([])

// 申请表单
const application = reactive({
  taskId: '',
  taskName: '',
  airspaceId: '',
  startTime: '',
  endTime: '',
  remarks: ''
})

// Tab 配置
const tabs = computed(() => [
  { label: '申请许可', value: 'apply' },
  { label: '我的申请', value: 'my', count: myPermits.value.length },
  { label: '待审批', value: 'pending', count: pendingPermits.value.length },
  { label: '所有许可', value: 'all', count: allPermits.value.length }
])

// 加载数据
onMounted(async () => {
  await Promise.all([
    loadTasks(),
    loadAirspaces(),
    loadPermits()
  ])
})

// 加载飞行任务
const loadTasks = async () => {
  try {
    const response = await taskApi.getTasks()
    if (response && response.success) {
      tasks.value = response.data || []
    }
  } catch (error) {
    console.error('加载飞行任务失败:', error)
  }
}

// 加载空域
const loadAirspaces = async () => {
  try {
    const response = await airspaceApi.getAirspaces()
    if (response && response.success) {
      airspaces.value = response.data || []
    }
  } catch (error) {
    console.error('加载空域失败:', error)
  }
}

// 加载许可列表
const loadPermits = async () => {
  loading.value = true
  try {
    const response = await permitApi.getPermits()
    if (response && response.success) {
      const permits = response.data || []

      // 分类
      myPermits.value = permits.filter(p => p.is_mine)
      pendingPermits.value = permits.filter(p => p.status === 'pending')
      allPermits.value = permits
    }
  } catch (error) {
    console.error('加载许可列表失败:', error)
    showToastNotification('加载许可列表失败', 'error')
  } finally {
    loading.value = false
  }
}

// 提交申请
const handleSubmitApplication = async () => {
  submitting.value = true
  try {
    const response = await permitApi.createPermit({
      task_id: application.taskId,
      airspace_id: application.airspaceId,
      start_time: application.startTime,
      end_time: application.endTime,
      remarks: application.remarks
    })

    if (response && response.success) {
      showToastNotification('申请提交成功！', 'success')
      resetApplication()
      await loadPermits()
      activeTab.value = 'my'
    } else {
      showToastNotification(response?.message || '申请提交失败', 'error')
    }
  } catch (error) {
    console.error('提交申请失败:', error)
    showToastNotification('提交申请失败', 'error')
  } finally {
    submitting.value = false
  }
}

// 重置表单
const resetApplication = () => {
  application.taskId = ''
  application.taskName = ''
  application.airspaceId = ''
  application.startTime = ''
  application.endTime = ''
  application.remarks = ''
}

// 批准许可
const handleApprove = async (permitId) => {
  try {
    const response = await permitApi.approvePermit(permitId)
    if (response && response.success) {
      showToastNotification('已批准许可', 'success')
      await loadPermits()
    } else {
      showToastNotification('批准失败', 'error')
    }
  } catch (error) {
    console.error('批准许可失败:', error)
    showToastNotification('批准许可失败', 'error')
  }
}

// 拒绝许可
const handleReject = async (permitId) => {
  try {
    const response = await permitApi.rejectPermit(permitId)
    if (response && response.success) {
      showToastNotification('已拒绝许可', 'success')
      await loadPermits()
    } else {
      showToastNotification('拒绝失败', 'error')
    }
  } catch (error) {
    console.error('拒绝许可失败:', error)
    showToastNotification('拒绝许可失败', 'error')
  }
}

// 格式化时间
const formatDateTime = (dateTime) => {
  if (!dateTime) return '-'
  const date = new Date(dateTime)
  return date.toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit'
  })
}

// 状态文本
const getStatusText = (status) => {
  const statusMap = {
    pending: '待审批',
    approved: '已批准',
    rejected: '已拒绝',
    expired: '已过期'
  }
  return statusMap[status] || status
}
</script>

<style scoped>
.flight-permit-management {
  padding: var(--space-6);
  max-width: 1400px;
  margin: 0 auto;
}

/* 页面标题 */
.page-header {
  margin-bottom: var(--space-8);
}

.page-title {
  font-size: var(--font-size-4xl);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-2) 0;
  letter-spacing: var(--letter-spacing-tight);
}

.page-subtitle {
  font-size: var(--font-size-lg);
  color: var(--color-text-secondary);
  margin: 0;
}

/* Tab 切换 */
.tabs {
  display: flex;
  gap: var(--space-2);
  margin-bottom: var(--space-6);
  border-bottom: 1px solid var(--color-border-subtle);
}

.tab-button {
  position: relative;
  padding: var(--space-3) var(--space-4);
  font-size: var(--font-size-base);
  font-weight: 500;
  color: var(--color-text-secondary);
  background: none;
  border: none;
  border-bottom: 2px solid transparent;
  cursor: pointer;
  transition: var(--transition-fast);
  display: flex;
  align-items: center;
  gap: var(--space-2);
}

.tab-button:hover {
  color: var(--color-text-primary);
}

.tab-button--active {
  color: var(--apple-blue);
  border-bottom-color: var(--apple-blue);
}

.tab-badge {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  min-width: 20px;
  height: 20px;
  padding: 0 var(--space-2);
  font-size: var(--font-size-xs);
  font-weight: 600;
  color: white;
  background: var(--apple-blue);
  border-radius: 10px;
}

/* Tab 内容 */
.tab-content {
  animation: fadeIn var(--duration-normal) var(--easing-default);
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(8px); }
  to { opacity: 1; transform: translateY(0); }
}

/* 表单 */
.apply-card {
  max-width: 800px;
}

.form-row {
  margin-bottom: var(--space-4);
}

.form-row:last-child {
  margin-bottom: 0;
}

.form-col {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.form-row:has(.form-col) {
  display: flex;
  gap: var(--space-4);
}

.form-label {
  font-size: var(--font-size-sm);
  font-weight: 500;
  color: var(--color-text-primary);
}

.apple-select,
.apple-textarea {
  width: 100%;
  padding: var(--space-3) var(--space-4);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-base);
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  outline: none;
  transition: var(--transition-input);
}

.apple-select {
  height: 44px;
}

.apple-select:hover,
.apple-textarea:hover {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.apple-select:focus,
.apple-textarea:focus {
  background-color: var(--color-bg-primary);
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
}

.apple-textarea {
  resize: vertical;
  min-height: 100px;
}

.form-actions {
  display: flex;
  gap: var(--space-3);
  margin-top: var(--space-6);
}

/* 许可卡片网格 */
.permits-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(350px, 1fr));
  gap: var(--space-4);
}

.permit-card {
  transition: var(--transition-card);
}

.permit-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: var(--space-4);
}

.permit-title {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0;
}

.permit-status {
  padding: var(--space-1) var(--space-3);
  font-size: var(--font-size-xs);
  font-weight: 600;
  border-radius: var(--radius-sm);
  white-space: nowrap;
}

.permit-status--pending {
  background: rgba(255, 159, 10, 0.1);
  color: var(--apple-orange);
}

.permit-status--approved {
  background: rgba(48, 209, 88, 0.1);
  color: var(--apple-green);
}

.permit-status--rejected {
  background: rgba(255, 59, 48, 0.1);
  color: var(--apple-red);
}

.permit-status--expired {
  background: var(--color-bg-tertiary);
  color: var(--color-text-tertiary);
}

.permit-info {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.info-item {
  display: flex;
  align-items: flex-start;
  gap: var(--space-2);
  font-size: var(--font-size-sm);
}

.info-label {
  color: var(--color-text-secondary);
  min-width: 80px;
}

.info-value {
  color: var(--color-text-primary);
  flex: 1;
}

.permit-actions {
  display: flex;
  gap: var(--space-2);
  margin-top: var(--space-4);
  padding-top: var(--space-4);
  border-top: 1px solid var(--color-border-subtle);
}

/* 加载状态 */
.loading-state,
.empty-state {
  display: flex;
  align-items: center;
  justify-content: center;
  min-height: 300px;
  color: var(--color-text-secondary);
}

.loading-spinner {
  font-size: var(--font-size-lg);
}

/* 响应式 */
@media (max-width: 768px) {
  .flight-permit-management {
    padding: var(--space-4);
  }

  .tabs {
    overflow-x: auto;
    -webkit-overflow-scrolling: touch;
  }

  .form-row:has(.form-col) {
    flex-direction: column;
  }

  .permits-grid {
    grid-template-columns: 1fr;
  }

  .form-actions {
    flex-direction: column;
  }

  .form-actions button {
    width: 100%;
  }
}

/* Toast 通知样式 */
.toast-notification {
  position: fixed;
  top: 24px;
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  align-items: center;
  gap: var(--space-3);
  min-width: 320px;
  max-width: 500px;
  padding: var(--space-4) var(--space-5);
  background: var(--color-bg-primary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-md);
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.15), 0 4px 12px rgba(0, 0, 0, 0.1);
  backdrop-filter: blur(20px);
  z-index: 10000;
  cursor: pointer;
  transition: var(--transition-card);
}

.toast-notification:hover {
  box-shadow: 0 12px 48px rgba(0, 0, 0, 0.2), 0 6px 16px rgba(0, 0, 0, 0.15);
}

.toast-icon {
  flex-shrink: 0;
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.toast-icon svg {
  width: 100%;
  height: 100%;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-base);
  font-weight: 500;
  color: var(--color-text-primary);
  line-height: 1.5;
}

.toast-close {
  flex-shrink: 0;
  width: 20px;
  height: 20px;
  padding: 0;
  background: none;
  border: none;
  color: var(--color-text-tertiary);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: var(--radius-xs);
  transition: var(--transition-fast);
}

.toast-close:hover {
  background: var(--color-bg-tertiary);
  color: var(--color-text-secondary);
}

.toast-close svg {
  width: 100%;
  height: 100%;
}

/* Toast 类型样式 */
.toast-notification--success {
  border-color: rgba(48, 209, 88, 0.3);
  background: linear-gradient(to right, rgba(48, 209, 88, 0.05), var(--color-bg-primary));
}

.toast-notification--success .toast-icon {
  color: var(--apple-green);
}

.toast-notification--error {
  border-color: rgba(255, 59, 48, 0.3);
  background: linear-gradient(to right, rgba(255, 59, 48, 0.05), var(--color-bg-primary));
}

.toast-notification--error .toast-icon {
  color: var(--apple-red);
}

.toast-notification--warning {
  border-color: rgba(255, 159, 10, 0.3);
  background: linear-gradient(to right, rgba(255, 159, 10, 0.05), var(--color-bg-primary));
}

.toast-notification--warning .toast-icon {
  color: var(--apple-orange);
}

.toast-notification--info {
  border-color: rgba(0, 122, 255, 0.3);
  background: linear-gradient(to right, rgba(0, 122, 255, 0.05), var(--color-bg-primary));
}

.toast-notification--info .toast-icon {
  color: var(--apple-blue);
}

/* Toast 动画 */
.toast-slide-enter-active {
  animation: toastSlideIn 0.4s cubic-bezier(0.16, 1, 0.3, 1);
}

.toast-slide-leave-active {
  animation: toastSlideOut 0.3s cubic-bezier(0.4, 0, 1, 1);
}

@keyframes toastSlideIn {
  from {
    opacity: 0;
    transform: translateX(-50%) translateY(-20px) scale(0.95);
  }
  to {
    opacity: 1;
    transform: translateX(-50%) translateY(0) scale(1);
  }
}

@keyframes toastSlideOut {
  from {
    opacity: 1;
    transform: translateX(-50%) translateY(0) scale(1);
  }
  to {
    opacity: 0;
    transform: translateX(-50%) translateY(-20px) scale(0.95);
  }
}

/* 暗色模式支持 */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: rgba(30, 30, 30, 0.95);
    box-shadow: 0 10px 40px rgba(0, 0, 0, 0.4), 0 4px 12px rgba(0, 0, 0, 0.3);
  }

  .toast-notification:hover {
    box-shadow: 0 12px 48px rgba(0, 0, 0, 0.5), 0 6px 16px rgba(0, 0, 0, 0.4);
  }

  .toast-notification--success {
    background: linear-gradient(to right, rgba(48, 209, 88, 0.1), rgba(30, 30, 30, 0.95));
  }

  .toast-notification--error {
    background: linear-gradient(to right, rgba(255, 59, 48, 0.1), rgba(30, 30, 30, 0.95));
  }

  .toast-notification--warning {
    background: linear-gradient(to right, rgba(255, 159, 10, 0.1), rgba(30, 30, 30, 0.95));
  }

  .toast-notification--info {
    background: linear-gradient(to right, rgba(0, 122, 255, 0.1), rgba(30, 30, 30, 0.95));
  }
}

/* 响应式 Toast */
@media (max-width: 768px) {
  .toast-notification {
    left: var(--space-4);
    right: var(--space-4);
    min-width: auto;
    transform: translateX(0);
  }

  .toast-slide-enter-active,
  .toast-slide-leave-active {
    animation: none;
  }

  @keyframes toastSlideIn {
    from {
      opacity: 0;
      transform: translateY(-20px) scale(0.95);
    }
    to {
      opacity: 1;
      transform: translateY(0) scale(1);
    }
  }

  @keyframes toastSlideOut {
    from {
      opacity: 1;
      transform: translateY(0) scale(1);
    }
    to {
      opacity: 0;
      transform: translateY(-20px) scale(0.95);
    }
  }
}
</style>
