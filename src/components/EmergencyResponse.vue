<template>
  <div class="emergency-response">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">🚨 应急响应</h1>
      <p class="page-subtitle">紧急事件监控与处理</p>
      <div class="header-actions">
        <AppleButton variant="danger" @click="showCreateDialog = true">
          ➕ 创建紧急事件
        </AppleButton>
        <AppleButton @click="refreshEvents">
          🔄 刷新
        </AppleButton>
      </div>
    </div>

    <!-- 统计卡片 -->
    <div class="stats-grid">
      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">⚠️</div>
          <div class="stat-info">
            <div class="stat-value">{{ statistics.active_events || 0 }}</div>
            <div class="stat-label">激活事件</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">🔔</div>
          <div class="stat-info">
            <div class="stat-value">{{ statistics.total_events || 0 }}</div>
            <div class="stat-label">总事件数</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">✅</div>
          <div class="stat-info">
            <div class="stat-value">{{ statistics.by_status?.resolved || 0 }}</div>
            <div class="stat-label">已解决</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">❌</div>
          <div class="stat-info">
            <div class="stat-value">{{ statistics.by_status?.cancelled || 0 }}</div>
            <div class="stat-label">已取消</div>
          </div>
        </div>
      </AppleCard>
    </div>

    <!-- 过滤器 -->
    <AppleCard class="filter-card">
      <div class="filter-section">
        <select v-model="filter.status" class="apple-select" @change="refreshEvents">
          <option value="">所有状态</option>
          <option v-for="option in EmergencyStatusOptions" :key="option.value" :value="option.value">
            {{ option.label }}
          </option>
        </select>

        <select v-model="filter.severity" class="apple-select" @change="refreshEvents">
          <option value="">所有严重程度</option>
          <option v-for="option in EmergencySeverityOptions" :key="option.value" :value="option.value">
            {{ option.label }}
          </option>
        </select>
      </div>
    </AppleCard>

    <!-- 事件列表 -->
    <AppleCard class="events-card">
      <div class="card-header">
        <h2 class="section-title">紧急事件列表</h2>
        <span class="count-badge">共 {{ events.length }} 个事件</span>
      </div>

      <div v-if="loading" class="loading-state">
        <div class="loading-spinner"></div>
        <p>加载中...</p>
      </div>

      <div v-else class="events-list">
        <div
          v-for="event in events"
          :key="event.id"
          class="event-item"
          @click="handleViewDetail(event)"
        >
          <div class="event-main">
            <div class="event-header">
              <div class="event-title-row">
                <h3 class="event-code">{{ event.event_code }}</h3>
                <span :class="['severity-badge', `severity-${event.severity}`]">
                  {{ getSeverityText(event.severity) }}
                </span>
              </div>
              <h4 class="event-title">{{ event.title }}</h4>
            </div>

            <div class="event-meta">
              <div class="meta-item">
                <span class="meta-label">类型:</span>
                <span :class="['type-badge', `type-${event.type}`]">
                  {{ getEventTypeText(event.type) }}
                </span>
              </div>
              <div class="meta-item">
                <span class="meta-label">状态:</span>
                <span :class="['status-indicator', `status-${event.status}`]">
                  <span class="status-dot"></span>
                  {{ getStatusText(event.status) }}
                </span>
              </div>
              <div class="meta-item">
                <span class="meta-label">任务ID:</span>
                <span class="meta-value">{{ event.task_id }}</span>
              </div>
              <div class="meta-item">
                <span class="meta-label">创建时间:</span>
                <span class="meta-value">{{ event.created_at }}</span>
              </div>
            </div>
          </div>

          <div class="event-actions" @click.stop>
            <AppleButton
              v-if="event.status === EmergencyStatus.ACTIVE"
              variant="warning"
              size="small"
              @click="handleRespond(event)"
            >
              响应
            </AppleButton>
            <AppleButton
              v-if="event.status === EmergencyStatus.RESPONDING"
              variant="success"
              size="small"
              @click="handleResolve(event)"
            >
              解决
            </AppleButton>
            <AppleButton
              v-if="event.status === EmergencyStatus.ACTIVE || event.status === EmergencyStatus.RESPONDING"
              variant="secondary"
              size="small"
              @click="handleCancel(event)"
            >
              取消
            </AppleButton>
            <AppleButton
              variant="secondary"
              size="small"
              @click="handleViewDetail(event)"
            >
              详情
            </AppleButton>
          </div>
        </div>

        <div v-if="events.length === 0" class="empty-state">
          <div class="empty-icon">🚨</div>
          <p class="empty-text">暂无紧急事件</p>
          <AppleButton @click="showCreateDialog = true">创建第一个事件</AppleButton>
        </div>
      </div>

      <div v-if="events.length > 0" class="pagination">
        <button
          class="page-button"
          :disabled="pagination.page === 1"
          @click="changePage(pagination.page - 1)"
        >
          ← 上一页
        </button>
        <span class="page-info">第 {{ pagination.page }} 页，共 {{ totalPages }} 页</span>
        <button
          class="page-button"
          :disabled="pagination.page >= totalPages"
          @click="changePage(pagination.page + 1)"
        >
          下一页 →
        </button>
      </div>
    </AppleCard>

    <!-- 创建事件模态框 -->
    <AppleModal
      v-if="showCreateDialog"
      title="创建紧急事件"
      @close="showCreateDialog = false"
    >
      <form class="create-form" @submit.prevent="handleCreate">
        <div class="form-group">
          <label class="form-label">任务ID</label>
          <input
            v-model.number="createForm.task_id"
            type="number"
            class="apple-input"
            placeholder="请输入任务ID"
            min="1"
            required
          />
        </div>

        <div class="form-group">
          <label class="form-label">事件类型</label>
          <select v-model="createForm.type" class="apple-select" required>
            <option v-for="option in EmergencyTypeOptions" :key="option.value" :value="option.value">
              {{ option.label }}
            </option>
          </select>
        </div>

        <div class="form-group">
          <label class="form-label">严重程度</label>
          <select v-model="createForm.severity" class="apple-select" required>
            <option v-for="option in EmergencySeverityOptions" :key="option.value" :value="option.value">
              {{ option.label }}
            </option>
          </select>
        </div>

        <div class="form-group">
          <label class="form-label">标题</label>
          <input
            v-model="createForm.title"
            type="text"
            class="apple-input"
            placeholder="请输入事件标题"
            required
          />
        </div>

        <div class="form-group">
          <label class="form-label">描述</label>
          <textarea
            v-model="createForm.description"
            class="apple-textarea"
            rows="4"
            placeholder="请输入事件描述"
            required
          ></textarea>
        </div>

        <div class="form-row">
          <div class="form-group">
            <label class="form-label">纬度</label>
            <input
              v-model.number="createForm.lat"
              type="number"
              class="apple-input"
              step="0.000001"
              required
            />
          </div>

          <div class="form-group">
            <label class="form-label">经度</label>
            <input
              v-model.number="createForm.lon"
              type="number"
              class="apple-input"
              step="0.000001"
              required
            />
          </div>
        </div>
      </form>

      <template #footer>
        <AppleButton variant="secondary" @click="showCreateDialog = false">
          取消
        </AppleButton>
        <AppleButton variant="danger" @click="handleCreate">
          创建
        </AppleButton>
      </template>
    </AppleModal>

    <!-- 响应事件模态框 -->
    <AppleModal
      v-if="showRespondDialog"
      title="响应紧急事件"
      @close="showRespondDialog = false"
    >
      <form class="create-form" @submit.prevent="handleSubmitRespond">
        <div class="form-group">
          <label class="form-label">响应措施</label>
          <textarea
            v-model="respondForm.response_action"
            class="apple-textarea"
            rows="3"
            placeholder="请输入响应措施"
            required
          ></textarea>
        </div>

        <div class="form-group">
          <label class="form-label">响应备注（可选）</label>
          <textarea
            v-model="respondForm.response_notes"
            class="apple-textarea"
            rows="3"
            placeholder="请输入响应备注"
          ></textarea>
        </div>
      </form>

      <template #footer>
        <AppleButton variant="secondary" @click="showRespondDialog = false">
          取消
        </AppleButton>
        <AppleButton variant="warning" @click="handleSubmitRespond">
          提交响应
        </AppleButton>
      </template>
    </AppleModal>

    <!-- 事件详情模态框 -->
    <AppleModal
      v-if="showDetailDialog"
      title="事件详情"
      @close="showDetailDialog = false"
    >
      <div v-if="selectedEvent" class="detail-content">
        <div class="detail-section">
          <h3 class="detail-section-title">基本信息</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="detail-label">事件编号</span>
              <span class="detail-value">{{ selectedEvent.event_code }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">任务ID</span>
              <span class="detail-value">{{ selectedEvent.task_id }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">类型</span>
              <span :class="['type-badge', `type-${selectedEvent.type}`]">
                {{ getEventTypeText(selectedEvent.type) }}
              </span>
            </div>
            <div class="detail-item">
              <span class="detail-label">严重程度</span>
              <span :class="['severity-badge', `severity-${selectedEvent.severity}`]">
                {{ getSeverityText(selectedEvent.severity) }}
              </span>
            </div>
            <div class="detail-item">
              <span class="detail-label">状态</span>
              <span :class="['status-indicator', `status-${selectedEvent.status}`]">
                <span class="status-dot"></span>
                {{ getStatusText(selectedEvent.status) }}
              </span>
            </div>
          </div>
        </div>

        <div class="detail-section">
          <h3 class="detail-section-title">事件内容</h3>
          <div class="detail-item">
            <span class="detail-label">标题</span>
            <span class="detail-value">{{ selectedEvent.title }}</span>
          </div>
          <div class="detail-item">
            <span class="detail-label">描述</span>
            <p class="description-text">{{ selectedEvent.description }}</p>
          </div>
        </div>

        <div class="detail-section">
          <h3 class="detail-section-title">位置信息</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="detail-label">纬度</span>
              <span class="detail-value">{{ selectedEvent.location?.lat }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">经度</span>
              <span class="detail-value">{{ selectedEvent.location?.lon }}</span>
            </div>
          </div>
        </div>

        <div v-if="selectedEvent.response_action" class="detail-section">
          <h3 class="detail-section-title">响应信息</h3>
          <div class="detail-item">
            <span class="detail-label">响应措施</span>
            <p class="description-text">{{ selectedEvent.response_action }}</p>
          </div>
          <div v-if="selectedEvent.response_notes" class="detail-item">
            <span class="detail-label">响应备注</span>
            <p class="description-text">{{ selectedEvent.response_notes }}</p>
          </div>
          <div class="detail-item">
            <span class="detail-label">响应时间</span>
            <span class="detail-value">{{ selectedEvent.responded_at || '--' }}</span>
          </div>
        </div>

        <div class="detail-section">
          <h3 class="detail-section-title">时间信息</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="detail-label">创建时间</span>
              <span class="detail-value">{{ selectedEvent.created_at }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">更新时间</span>
              <span class="detail-value">{{ selectedEvent.updated_at }}</span>
            </div>
            <div v-if="selectedEvent.resolved_at" class="detail-item">
              <span class="detail-label">解决时间</span>
              <span class="detail-value">{{ selectedEvent.resolved_at }}</span>
            </div>
          </div>
        </div>
      </div>

      <template #footer>
        <AppleButton variant="secondary" @click="showDetailDialog = false">
          关闭
        </AppleButton>
      </template>
    </AppleModal>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div v-if="showToast" :class="['toast-notification', toastType]">
        <div class="toast-icon">{{ toastIcon }}</div>
        <div class="toast-message">{{ toastMessage }}</div>
      </div>
    </Transition>

    <!-- 确认对话框 -->
    <Transition name="modal-fade">
      <div v-if="showConfirmDialog" class="modal-overlay" @click="handleConfirmNo">
        <div class="confirm-dialog" @click.stop>
          <div class="confirm-header">
            <h3 class="confirm-title">{{ confirmDialogTitle }}</h3>
          </div>
          <div class="confirm-body">
            <p class="confirm-message">{{ confirmDialogMessage }}</p>
          </div>
          <div class="confirm-footer">
            <AppleButton variant="secondary" @click="handleConfirmNo">
              取消
            </AppleButton>
            <AppleButton @click="handleConfirmYes">
              确认
            </AppleButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import AppleCard from '@/components/apple/AppleCard.vue'
import AppleButton from '@/components/apple/AppleButton.vue'
import AppleModal from '@/components/apple/AppleModal.vue'
import emergencyApi from '@/services/emergencyApi'
import {
  EmergencyType,
  EmergencyTypeOptions,
  EmergencySeverity,
  EmergencySeverityOptions,
  EmergencyStatus,
  EmergencyStatusOptions
} from '@/constants/emergencyConstants'

// 响应式数据
const loading = ref(false)
const showCreateDialog = ref(false)
const showRespondDialog = ref(false)
const showDetailDialog = ref(false)
const events = ref([])
const statistics = ref({})
const selectedEvent = ref(null)
const currentRespondEvent = ref(null)

// Toast 通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

// 确认对话框状态
const showConfirmDialog = ref(false)
const confirmDialogTitle = ref('')
const confirmDialogMessage = ref('')
let confirmDialogCallback = null

const filter = reactive({
  status: '',
  severity: ''
})

const pagination = reactive({
  page: 1,
  page_size: 20,
  total: 0
})

const createForm = reactive({
  task_id: null,
  type: EmergencyType.OTHER,
  severity: EmergencySeverity.MEDIUM,
  title: '',
  description: '',
  lat: 39.9042,
  lon: 116.4074
})

const respondForm = reactive({
  response_action: '',
  response_notes: ''
})

const totalPages = computed(() => {
  return Math.ceil(pagination.total / pagination.page_size) || 1
})

const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

// 方法
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

const showConfirm = (title, message) => {
  return new Promise((resolve) => {
    confirmDialogTitle.value = title
    confirmDialogMessage.value = message
    showConfirmDialog.value = true
    confirmDialogCallback = resolve
  })
}

const handleConfirmYes = () => {
  showConfirmDialog.value = false
  if (confirmDialogCallback) {
    confirmDialogCallback(true)
    confirmDialogCallback = null
  }
}

const handleConfirmNo = () => {
  showConfirmDialog.value = false
  if (confirmDialogCallback) {
    confirmDialogCallback(false)
    confirmDialogCallback = null
  }
}

// 方法
const refreshEvents = async () => {
  loading.value = true

  try {
    const params = {
      page: pagination.page,
      page_size: pagination.page_size
    }

    if (filter.status) {
      params.status = filter.status
    }

    if (filter.severity) {
      params.severity = filter.severity
    }

    const response = await emergencyApi.getAllEvents(params)

    if (response.success) {
      events.value = response.data.events || []
      pagination.total = response.data.total || 0
    }
  } catch (error) {
    console.error('获取事件列表失败:', error)
  } finally {
    loading.value = false
  }
}

const refreshStatistics = async () => {
  try {
    const response = await emergencyApi.getStatistics()

    if (response.success) {
      statistics.value = response.data.statistics || {}
    }
  } catch (error) {
    console.error('获取统计信息失败:', error)
  }
}

const handleCreate = async () => {
  if (!createForm.task_id || !createForm.title || !createForm.description) {
    showToastNotification('请填写所有必填项', 'warning')
    return
  }

  try {
    const eventData = {
      task_id: createForm.task_id,
      type: createForm.type,
      severity: createForm.severity,
      title: createForm.title,
      description: createForm.description,
      location: {
        lat: createForm.lat,
        lon: createForm.lon
      }
    }

    const response = await emergencyApi.createEvent(eventData)

    if (response.success) {
      showToastNotification('紧急事件创建成功', 'success')
      showCreateDialog.value = false
      resetCreateForm()
      refreshEvents()
      refreshStatistics()
    }
  } catch (error) {
    console.error('创建事件失败:', error)
    showToastNotification('创建事件失败', 'error')
  }
}

const resetCreateForm = () => {
  createForm.task_id = null
  createForm.type = EmergencyType.OTHER
  createForm.severity = EmergencySeverity.MEDIUM
  createForm.title = ''
  createForm.description = ''
  createForm.lat = 39.9042
  createForm.lon = 116.4074
}

const handleRespond = (event) => {
  currentRespondEvent.value = event
  respondForm.response_action = ''
  respondForm.response_notes = ''
  showRespondDialog.value = true
}

const handleSubmitRespond = async () => {
  if (!respondForm.response_action) {
    showToastNotification('请输入响应措施', 'warning')
    return
  }

  try {
    const response = await emergencyApi.respondToEvent(
      currentRespondEvent.value.id,
      respondForm.response_action,
      respondForm.response_notes
    )

    if (response.success) {
      showToastNotification('响应提交成功', 'success')
      showRespondDialog.value = false
      refreshEvents()
      refreshStatistics()
    }
  } catch (error) {
    console.error('响应事件失败:', error)
    showToastNotification('响应事件失败', 'error')
  }
}

const handleResolve = async (event) => {
  try {
    const confirmed = await showConfirm('确认解决', '确认解决该紧急事件？')

    if (!confirmed) {
      return
    }

    const response = await emergencyApi.resolveEvent(event.id)

    if (response.success) {
      showToastNotification('事件已解决', 'success')
      refreshEvents()
      refreshStatistics()
    }
  } catch (error) {
    console.error('解决事件失败:', error)
    showToastNotification('解决事件失败', 'error')
  }
}

const handleCancel = async (event) => {
  try {
    const confirmed = await showConfirm('确认取消', '确认取消该紧急事件？')

    if (!confirmed) {
      return
    }

    const response = await emergencyApi.cancelEvent(event.id)

    if (response.success) {
      showToastNotification('事件已取消', 'success')
      refreshEvents()
      refreshStatistics()
    }
  } catch (error) {
    console.error('取消事件失败:', error)
    showToastNotification('取消事件失败', 'error')
  }
}

const handleViewDetail = (event) => {
  selectedEvent.value = event
  showDetailDialog.value = true
}

const changePage = (page) => {
  pagination.page = page
  refreshEvents()
}

const getSeverityText = (severity) => {
  return emergencyApi.getSeverityText(severity)
}

const getEventTypeText = (type) => {
  return emergencyApi.getEventTypeText(type)
}

const getStatusText = (status) => {
  return emergencyApi.getStatusText(status)
}

// 初始化
onMounted(() => {
  refreshEvents()
  refreshStatistics()
})
</script>

<style scoped>
.emergency-response {
  padding: var(--space-6, 24px);
  max-width: 1400px;
  margin: 0 auto;
}

/* 页面头部 */
.page-header {
  margin-bottom: var(--space-8, 32px);
}

.page-title {
  margin: 0 0 var(--space-2, 8px) 0;
  font-size: var(--font-size-3xl, 32px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  letter-spacing: -0.02em;
}

.page-subtitle {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
}

.header-actions {
  display: flex;
  gap: var(--space-3, 12px);
}

/* 统计卡片 */
.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: var(--space-4, 16px);
  margin-bottom: var(--space-6, 24px);
}

.stat-card {
  padding: var(--space-5, 20px);
  transition: all var(--transition-smooth, 0.3s ease);
}

.stat-card:hover {
  transform: translateY(-2px);
}

.stat-content {
  display: flex;
  align-items: center;
  gap: var(--space-4, 16px);
}

.stat-icon {
  font-size: 40px;
  line-height: 1;
}

.stat-info {
  flex: 1;
}

.stat-value {
  font-size: var(--font-size-2xl, 28px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.2;
  margin-bottom: var(--space-1, 4px);
}

.stat-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 过滤器 */
.filter-card {
  padding: var(--space-5, 20px);
  margin-bottom: var(--space-6, 24px);
}

.filter-section {
  display: flex;
  gap: var(--space-3, 12px);
}

.apple-select {
  flex: 1;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  background: var(--color-bg-primary, #FFFFFF);
  cursor: pointer;
  transition: all var(--transition-input, 0.2s ease);
  appearance: none;
  background-image: url("data:image/svg+xml,%3Csvg width='12' height='8' viewBox='0 0 12 8' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M1 1.5L6 6.5L11 1.5' stroke='%2386868B' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/%3E%3C/svg%3E");
  background-repeat: no-repeat;
  background-position: right 12px center;
  padding-right: 36px;
}

.apple-select:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

/* 事件列表 */
.events-card {
  padding: var(--space-6, 24px);
  margin-bottom: var(--space-6, 24px);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-5, 20px);
}

.section-title {
  margin: 0;
  font-size: var(--font-size-xl, 20px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.count-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

.loading-state {
  text-align: center;
  padding: var(--space-10, 40px);
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 3px solid var(--color-bg-secondary, #F5F5F7);
  border-top-color: var(--color-primary, #0071E3);
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin: 0 auto var(--space-4, 16px);
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.events-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3, 12px);
}

.event-item {
  padding: var(--space-5, 20px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: var(--space-4, 16px);
  cursor: pointer;
  transition: all var(--transition-smooth, 0.3s ease);
  background: var(--color-bg-primary, #FFFFFF);
}

.event-item:hover {
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08);
  transform: translateY(-1px);
}

.event-main {
  flex: 1;
  min-width: 0;
}

.event-header {
  margin-bottom: var(--space-3, 12px);
}

.event-title-row {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  margin-bottom: var(--space-2, 8px);
  flex-wrap: wrap;
}

.event-code {
  margin: 0;
  font-size: var(--font-size-sm, 14px);
  font-weight: 600;
  color: var(--color-text-secondary, #86868B);
}

.event-title {
  margin: 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.severity-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-xs, 12px);
  font-weight: 500;
}

.severity-low {
  background: #E8F5E9;
  color: #2E7D32;
}

.severity-medium {
  background: #FFF3E0;
  color: #F57C00;
}

.severity-high {
  background: #FFEBEE;
  color: #C62828;
}

.severity-critical {
  background: #3C0008;
  color: #FFFFFF;
}

.type-badge {
  padding: var(--space-1, 4px) var(--space-2, 8px);
  border-radius: var(--radius-md, 8px);
  font-size: var(--font-size-xs, 12px);
  font-weight: 500;
  background: var(--color-bg-secondary, #F5F5F7);
  color: var(--color-text-primary, #1D1D1F);
}

.event-meta {
  display: flex;
  gap: var(--space-6, 24px);
  flex-wrap: wrap;
}

.meta-item {
  display: flex;
  gap: var(--space-2, 8px);
  font-size: var(--font-size-sm, 14px);
  align-items: center;
}

.meta-label {
  color: var(--color-text-secondary, #86868B);
}

.meta-value {
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 500;
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
  font-weight: 500;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

.status-active .status-dot {
  background: #FF3B30;
}

.status-responding .status-dot {
  background: #FF9500;
}

.status-resolved .status-dot {
  background: #34C759;
}

.status-cancelled .status-dot {
  background: #8E8E93;
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}

.event-actions {
  display: flex;
  gap: var(--space-2, 8px);
  flex-shrink: 0;
}

.empty-state {
  text-align: center;
  padding: var(--space-12, 48px) var(--space-6, 24px);
}

.empty-icon {
  font-size: 64px;
  margin-bottom: var(--space-4, 16px);
  opacity: 0.5;
}

.empty-text {
  margin: 0 0 var(--space-6, 24px) 0;
  font-size: var(--font-size-lg, 18px);
  color: var(--color-text-secondary, #86868B);
}

/* 分页 */
.pagination {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: var(--space-4, 16px);
  margin-top: var(--space-6, 24px);
  padding-top: var(--space-6, 24px);
  border-top: 1px solid var(--color-border, #D2D2D7);
}

.page-button {
  padding: var(--space-2, 8px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  background: var(--color-bg-primary, #FFFFFF);
  color: var(--color-text-primary, #1D1D1F);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
  cursor: pointer;
  transition: all var(--transition-smooth, 0.3s ease);
}

.page-button:hover:not(:disabled) {
  background: var(--color-bg-secondary, #F5F5F7);
  border-color: var(--color-primary, #0071E3);
}

.page-button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.page-info {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 表单样式 */
.create-form {
  padding: var(--space-6, 24px) 0;
}

.form-group {
  margin-bottom: var(--space-5, 20px);
}

.form-label {
  display: block;
  margin-bottom: var(--space-2, 8px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.apple-input,
.apple-textarea {
  width: 100%;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  transition: all var(--transition-input, 0.2s ease);
  background: var(--color-bg-primary, #FFFFFF);
  font-family: inherit;
}

.apple-input:focus,
.apple-textarea:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.apple-textarea {
  resize: vertical;
  min-height: 80px;
}

.form-row {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: var(--space-4, 16px);
}

/* 详情内容 */
.detail-content {
  padding: var(--space-6, 24px) 0;
}

.detail-section {
  margin-bottom: var(--space-6, 24px);
}

.detail-section:last-child {
  margin-bottom: 0;
}

.detail-section-title {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  padding-bottom: var(--space-2, 8px);
  border-bottom: 1px solid var(--color-border, #D2D2D7);
}

.detail-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: var(--space-4, 16px);
}

.detail-item {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
}

.detail-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

.detail-value {
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 500;
}

.description-text {
  margin: 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
  line-height: 1.6;
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-md, 8px);
  border-left: 3px solid var(--color-primary, #0071E3);
  white-space: pre-wrap;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .emergency-response {
    padding: var(--space-4, 16px);
  }

  .page-title {
    font-size: var(--font-size-2xl, 28px);
  }

  .header-actions {
    flex-direction: column;
  }

  .stats-grid {
    grid-template-columns: repeat(2, 1fr);
  }

  .filter-section {
    flex-direction: column;
  }

  .event-item {
    flex-direction: column;
    align-items: stretch;
  }

  .event-actions {
    justify-content: flex-end;
  }

  .detail-grid {
    grid-template-columns: 1fr;
  }

  .form-row {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 480px) {
  .stats-grid {
    grid-template-columns: 1fr;
  }

  .event-meta {
    flex-direction: column;
    gap: var(--space-2, 8px);
  }
}

/* 深色模式 */
@media (prefers-color-scheme: dark) {
  .page-title,
  .section-title,
  .event-title,
  .detail-value {
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .page-subtitle,
  .stat-label,
  .meta-label,
  .detail-label {
    color: var(--color-text-secondary-dark, #A1A1A6);
  }

  .event-item {
    background: var(--color-bg-secondary-dark, #1C1C1E);
    border-color: var(--color-border-dark, #38383A);
  }

  .apple-input,
  .apple-select,
  .apple-textarea {
    background: var(--color-bg-secondary-dark, #1C1C1E);
    border-color: var(--color-border-dark, #38383A);
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .page-button {
    background: var(--color-bg-secondary-dark, #1C1C1E);
    border-color: var(--color-border-dark, #38383A);
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .description-text {
    background: var(--color-bg-tertiary-dark, #2C2C2E);
    color: var(--color-text-secondary-dark, #A1A1A6);
  }
}

/* Toast 通知样式 */
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

.toast-icon {
  font-size: 24px;
  line-height: 1;
  flex-shrink: 0;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.5;
}

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0, 1, 1);
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

/* 确认对话框样式 */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  backdrop-filter: blur(8px);
  -webkit-backdrop-filter: blur(8px);
  z-index: 9999;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: var(--space-6, 24px);
}

.confirm-dialog {
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-2xl, 20px);
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
  max-width: 480px;
  width: 100%;
  overflow: hidden;
}

.confirm-header {
  padding: var(--space-6, 24px) var(--space-6, 24px) var(--space-4, 16px);
  border-bottom: 1px solid var(--color-border, #E5E5E7);
}

.confirm-title {
  margin: 0;
  font-size: var(--font-size-xl, 20px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.confirm-body {
  padding: var(--space-6, 24px);
}

.confirm-message {
  margin: 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
  line-height: 1.6;
}

.confirm-footer {
  padding: var(--space-4, 16px) var(--space-6, 24px);
  border-top: 1px solid var(--color-border, #E5E5E7);
  display: flex;
  gap: var(--space-3, 12px);
  justify-content: flex-end;
}

/* Modal 动画 */
.modal-fade-enter-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.modal-fade-leave-active {
  transition: all 0.2s cubic-bezier(0.4, 0, 1, 1);
}

.modal-fade-enter-from,
.modal-fade-leave-to {
  opacity: 0;
}

.modal-fade-enter-from .confirm-dialog {
  transform: scale(0.95) translateY(-20px);
  opacity: 0;
}

.modal-fade-leave-to .confirm-dialog {
  transform: scale(0.98) translateY(-10px);
  opacity: 0;
}

/* Toast 深色模式 */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: #1C1C1E;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
  }

  .toast-message {
    color: #F5F5F7;
  }

  .confirm-dialog {
    background: #1C1C1E;
  }

  .confirm-title {
    color: #F5F5F7;
  }

  .confirm-message {
    color: #98989D;
  }

  .confirm-header,
  .confirm-footer {
    border-color: #38383A;
  }
}
</style>
