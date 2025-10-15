<template>
  <div class="drone-management">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">无人机管理</h1>
      <p class="page-subtitle">管理所有无人机设备及其状态</p>
    </div>

    <!-- 统计卡片 -->
    <div class="stats-cards">
      <AppleCard class="stat-card stat-card--success">
        <div class="stat-icon">✈️</div>
        <div class="stat-content">
          <div class="stat-value">{{ onlineCount }}</div>
          <div class="stat-label">在线</div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card stat-card--info">
        <div class="stat-icon">📍</div>
        <div class="stat-content">
          <div class="stat-value">{{ flyingCount }}</div>
          <div class="stat-label">飞行中</div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card stat-card--warning">
        <div class="stat-icon">🔋</div>
        <div class="stat-content">
          <div class="stat-value">{{ lowBatteryCount }}</div>
          <div class="stat-label">低电量</div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card stat-card--secondary">
        <div class="stat-icon">🔧</div>
        <div class="stat-content">
          <div class="stat-value">{{ maintenanceCount }}</div>
          <div class="stat-label">维护中</div>
        </div>
      </AppleCard>
    </div>

    <!-- 工具栏 -->
    <div class="toolbar">
      <div class="search-section">
        <AppleInput
          v-model="searchQuery"
          type="text"
          placeholder="搜索无人机编号或型号..."
          class="search-input"
          @input="handleSearch"
        />
      </div>

      <div class="action-section">
        <!-- 视图切换 -->
        <div class="view-toggle">
          <button
            :class="['view-button', { active: viewMode === 'grid' }]"
            @click="viewMode = 'grid'"
            title="网格视图"
          >
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor">
              <rect x="3" y="3" width="7" height="7" rx="1" />
              <rect x="14" y="3" width="7" height="7" rx="1" />
              <rect x="3" y="14" width="7" height="7" rx="1" />
              <rect x="14" y="14" width="7" height="7" rx="1" />
            </svg>
          </button>
          <button
            :class="['view-button', { active: viewMode === 'list' }]"
            @click="viewMode = 'list'"
            title="列表视图"
          >
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor">
              <line x1="4" y1="6" x2="20" y2="6" stroke-width="2" />
              <line x1="4" y1="12" x2="20" y2="12" stroke-width="2" />
              <line x1="4" y1="18" x2="20" y2="18" stroke-width="2" />
            </svg>
          </button>
        </div>

        <!-- 筛选 -->
        <select v-model="statusFilter" class="status-filter">
          <option value="">所有状态</option>
          <option value="online">在线</option>
          <option value="offline">离线</option>
          <option value="flying">飞行中</option>
          <option value="maintenance">维护中</option>
        </select>

        <!-- 添加按钮(管理员) -->
        <AppleButton
          v-if="userRole === 'admin'"
          variant="primary"
          @click="showAddModal = true"
        >
          + 添加无人机
        </AppleButton>

        <!-- 刷新按钮 -->
        <AppleButton variant="secondary" @click="handleRefresh">
          🔄
        </AppleButton>
      </div>
    </div>

    <!-- 加载状态 -->
    <div v-if="loading" class="loading-state">
      <div class="spinner" />
      <p>加载中...</p>
    </div>

    <!-- 空状态 -->
    <div v-else-if="filteredDrones.length === 0" class="empty-state">
      <p>暂无无人机数据</p>
    </div>

    <!-- 网格视图 -->
    <div v-else-if="viewMode === 'grid'" class="drones-grid">
      <AppleCard
        v-for="drone in filteredDrones"
        :key="drone.id"
        hoverable
        class="drone-card"
      >
        <!-- 无人机头部 -->
        <div class="drone-header">
          <div class="drone-icon">
            <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="currentColor">
              <path
                stroke-linecap="round"
                stroke-linejoin="round"
                stroke-width="2"
                d="M12 4v16m8-8H4m15.364 6.364L5.636 5.636m12.728 0L5.636 18.364"
              />
            </svg>
          </div>
          <span :class="`status-dot status-dot--${drone.status}`" />
        </div>

        <!-- 无人机信息 -->
        <div class="drone-body">
          <h3 class="drone-name">{{ drone.drone_id || `无人机 #${drone.id}` }}</h3>
          <p class="drone-model">{{ drone.model || 'N/A' }}</p>

          <div class="drone-info">
            <div class="info-item">
              <span class="info-label">状态:</span>
              <span :class="`status-badge status-badge--${drone.status}`">
                {{ getStatusText(drone.status) }}
              </span>
            </div>

            <div class="info-item">
              <span class="info-label">电量:</span>
              <div class="battery-indicator">
                <div
                  :class="[
                    'battery-fill',
                    getBatteryClass(drone.battery_level)
                  ]"
                  :style="{ width: (drone.battery_level || 0) + '%' }"
                />
                <span class="battery-text">{{ drone.battery_level || 0 }}%</span>
              </div>
            </div>

            <div v-if="drone.location" class="info-item">
              <span class="info-label">位置:</span>
              <span class="info-value">
                {{ drone.location.latitude?.toFixed(4) }}, {{ drone.location.longitude?.toFixed(4) }}
              </span>
            </div>

            <div v-if="drone.last_seen" class="info-item">
              <span class="info-label">最后活跃:</span>
              <span class="info-value">{{ formatTime(drone.last_seen) }}</span>
            </div>
          </div>
        </div>

        <!-- 操作按钮 -->
        <div v-if="userRole === 'admin'" class="drone-actions">
          <AppleButton
            variant="secondary"
            size="small"
            @click="handleEdit(drone)"
          >
            编辑
          </AppleButton>
          <AppleButton
            variant="danger"
            size="small"
            @click="handleDelete(drone.id)"
          >
            删除
          </AppleButton>
        </div>
      </AppleCard>
    </div>

    <!-- 列表视图 -->
    <div v-else class="drones-list">
      <div class="list-table">
        <div class="table-header">
          <div class="table-col">编号</div>
          <div class="table-col">型号</div>
          <div class="table-col">状态</div>
          <div class="table-col">电量</div>
          <div class="table-col">位置</div>
          <div class="table-col">最后活跃</div>
          <div v-if="userRole === 'admin'" class="table-col">操作</div>
        </div>

        <div
          v-for="drone in filteredDrones"
          :key="drone.id"
          class="table-row"
        >
          <div class="table-col">
            <span class="drone-id">{{ drone.drone_id || `#${drone.id}` }}</span>
          </div>
          <div class="table-col">{{ drone.model || 'N/A' }}</div>
          <div class="table-col">
            <span :class="`status-badge status-badge--${drone.status}`">
              {{ getStatusText(drone.status) }}
            </span>
          </div>
          <div class="table-col">
            <div class="battery-indicator">
              <div
                :class="['battery-fill', getBatteryClass(drone.battery_level)]"
                :style="{ width: (drone.battery_level || 0) + '%' }"
              />
              <span class="battery-text">{{ drone.battery_level || 0 }}%</span>
            </div>
          </div>
          <div class="table-col">
            <span v-if="drone.location" class="location-text">
              {{ drone.location.latitude?.toFixed(4) }}, {{ drone.location.longitude?.toFixed(4) }}
            </span>
            <span v-else class="text-muted">N/A</span>
          </div>
          <div class="table-col">{{ formatTime(drone.last_seen) }}</div>
          <div v-if="userRole === 'admin'" class="table-col table-actions">
            <button class="action-btn" @click="handleEdit(drone)" title="编辑">
              ✏️
            </button>
            <button class="action-btn" @click="handleDelete(drone.id)" title="删除">
              🗑️
            </button>
          </div>
        </div>
      </div>
    </div>

    <!-- 添加/编辑无人机模态框 -->
    <AppleModal
      v-model="showAddModal"
      :title="editingDrone ? '编辑无人机' : '添加无人机'"
      :show-close="true"
    >
      <form class="drone-form" @submit.prevent="handleSubmit">
        <AppleInput
          v-model="formData.drone_id"
          label="无人机编号"
          placeholder="例: DRONE-001"
          required
          :disabled="!!editingDrone"
          class="form-field"
        />

        <AppleInput
          v-model="formData.model"
          label="型号"
          placeholder="例: DJI Mavic 3"
          required
          class="form-field"
        />

        <div class="form-field">
          <label class="form-label">状态</label>
          <select v-model="formData.status" class="form-select" required>
            <option value="online">在线</option>
            <option value="offline">离线</option>
            <option value="flying">飞行中</option>
            <option value="maintenance">维护中</option>
          </select>
        </div>

        <AppleInput
          v-model.number="formData.battery_level"
          type="number"
          label="电量(%)"
          placeholder="0-100"
          min="0"
          max="100"
          class="form-field"
        />

        <div class="form-section">
          <h3 class="form-section-title">位置信息(可选)</h3>
          <div class="form-row">
            <AppleInput
              v-model.number="formData.location.latitude"
              type="number"
              step="0.000001"
              label="纬度"
              placeholder="例: 41.796700"
              class="form-field"
            />
            <AppleInput
              v-model.number="formData.location.longitude"
              type="number"
              step="0.000001"
              label="经度"
              placeholder="例: 123.451200"
              class="form-field"
            />
          </div>
        </div>

        <div class="modal-actions">
          <AppleButton
            type="button"
            variant="secondary"
            @click="handleCancelEdit"
          >
            取消
          </AppleButton>
          <AppleButton
            type="submit"
            variant="primary"
            :loading="submitting"
          >
            {{ submitting ? '提交中...' : (editingDrone ? '更新' : '创建') }}
          </AppleButton>
        </div>
      </form>
    </AppleModal>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div
        v-if="showToast"
        :class="['toast-notification', toastType]"
      >
        <div class="toast-icon">
          {{ toastIcon }}
        </div>
        <div class="toast-message">
          {{ toastMessage }}
        </div>
      </div>
    </Transition>

    <!-- Confirm 对话框 -->
    <AppleModal
      v-model="showConfirm"
      :title="confirmTitle"
      :show-close="false"
    >
      <div class="confirm-dialog">
        <div class="confirm-icon">
          {{ confirmIcon }}
        </div>
        <div class="confirm-message">
          {{ confirmMessage }}
        </div>
        <div class="confirm-actions">
          <AppleButton
            variant="secondary"
            @click="handleConfirmCancel"
          >
            取消
          </AppleButton>
          <AppleButton
            :variant="confirmType === 'error' ? 'danger' : 'primary'"
            @click="handleConfirmOk"
          >
            确定
          </AppleButton>
        </div>
      </div>
    </AppleModal>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { useStore } from 'vuex'
import droneApi from '@/services/droneApi'
import { AppleButton, AppleCard, AppleInput, AppleModal } from '@/components/apple'

const store = useStore()

// 用户角色
const userRole = computed(() => store.state.user?.role || 'user')

// 数据状态
const loading = ref(false)
const drones = ref([])
const searchQuery = ref('')
const statusFilter = ref('')
const viewMode = ref('grid') // 'grid' 或 'list'

// 添加/编辑模态框
const showAddModal = ref(false)
const editingDrone = ref(null)
const submitting = ref(false)

// Toast 通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

// Confirm 对话框状态
const showConfirm = ref(false)
const confirmTitle = ref('')
const confirmMessage = ref('')
const confirmType = ref('warning')
let confirmResolve = null
let confirmReject = null

// Toast 图标计算属性
const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

// Confirm 图标计算属性
const confirmIcon = computed(() => {
  switch (confirmType.value) {
    case 'warning': return '⚠️'
    case 'error': return '❌'
    case 'info': return 'ℹ️'
    default: return '❓'
  }
})

// 显示 Toast 通知
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

// 显示 Confirm 对话框
const showConfirmDialog = (message, title = '确认', type = 'warning') => {
  return new Promise((resolve, reject) => {
    confirmMessage.value = message
    confirmTitle.value = title
    confirmType.value = type
    showConfirm.value = true
    confirmResolve = resolve
    confirmReject = reject
  })
}

// 处理 Confirm 确认
const handleConfirmOk = () => {
  showConfirm.value = false
  if (confirmResolve) confirmResolve(true)
}

// 处理 Confirm 取消
const handleConfirmCancel = () => {
  showConfirm.value = false
  if (confirmReject) confirmReject(new Error('cancel'))
}

const formData = reactive({
  drone_id: '',
  model: '',
  status: 'offline',
  battery_level: 100,
  location: {
    latitude: null,
    longitude: null
  }
})

// 计算属性: 过滤后的无人机
const filteredDrones = computed(() => {
  let filtered = drones.value

  // 搜索过滤
  if (searchQuery.value) {
    const query = searchQuery.value.toLowerCase()
    filtered = filtered.filter(drone =>
      drone.drone_id?.toLowerCase().includes(query) ||
      drone.model?.toLowerCase().includes(query)
    )
  }

  // 状态过滤
  if (statusFilter.value) {
    filtered = filtered.filter(drone => drone.status === statusFilter.value)
  }

  return filtered
})

// 统计数据
const onlineCount = computed(() =>
  drones.value.filter(d => d.status === 'online').length
)

const flyingCount = computed(() =>
  drones.value.filter(d => d.status === 'flying').length
)

const lowBatteryCount = computed(() =>
  drones.value.filter(d => d.battery_level < 20).length
)

const maintenanceCount = computed(() =>
  drones.value.filter(d => d.status === 'maintenance').length
)

// 获取状态文本
const getStatusText = (status) => {
  const statusMap = {
    online: '在线',
    offline: '离线',
    flying: '飞行中',
    maintenance: '维护中'
  }
  return statusMap[status] || status
}

// 获取电量样式类
const getBatteryClass = (level) => {
  if (level >= 60) return 'battery-fill--high'
  if (level >= 20) return 'battery-fill--medium'
  return 'battery-fill--low'
}

// 格式化时间
const formatTime = (time) => {
  if (!time) return 'N/A'
  return new Date(time).toLocaleString('zh-CN', {
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit'
  })
}

// 加载无人机列表
const loadDrones = async () => {
  try {
    loading.value = true
    const response = await droneApi.getDrones()

    if (response.success) {
      drones.value = response.data || []
      console.log('成功加载无人机列表:', drones.value.length)
    } else {
      showToastNotification(response.message || '加载无人机失败', 'warning')
    }
  } catch (error) {
    console.error('加载无人机失败:', error)
    showToastNotification(error.message || '加载无人机失败', 'error')
  } finally {
    loading.value = false
  }
}

// 搜索处理
const handleSearch = () => {
  // 搜索由计算属性自动处理
}

// 刷新列表
const handleRefresh = () => {
  loadDrones()
  showToastNotification('已刷新', 'success')
}

// 编辑无人机
const handleEdit = (drone) => {
  editingDrone.value = drone
  formData.drone_id = drone.drone_id
  formData.model = drone.model
  formData.status = drone.status
  formData.battery_level = drone.battery_level || 100
  formData.location.latitude = drone.location?.latitude || null
  formData.location.longitude = drone.location?.longitude || null
  showAddModal.value = true
}

// 取消编辑
const handleCancelEdit = () => {
  showAddModal.value = false
  editingDrone.value = null
  resetForm()
}

// 重置表单
const resetForm = () => {
  formData.drone_id = ''
  formData.model = ''
  formData.status = 'offline'
  formData.battery_level = 100
  formData.location.latitude = null
  formData.location.longitude = null
}

// 提交表单
const handleSubmit = async () => {
  try {
    submitting.value = true

    // 准备数据
    const submitData = {
      drone_id: formData.drone_id,
      model: formData.model,
      status: formData.status,
      battery_level: formData.battery_level
    }

    // 添加位置(如果填写了)
    if (formData.location.latitude && formData.location.longitude) {
      submitData.location = {
        latitude: parseFloat(formData.location.latitude),
        longitude: parseFloat(formData.location.longitude)
      }
    }

    let response
    if (editingDrone.value) {
      // 更新
      response = await droneApi.updateDrone(editingDrone.value.id, submitData)
    } else {
      // 创建
      response = await droneApi.createDrone(submitData)
    }

    if (response.success) {
      showToastNotification(editingDrone.value ? '更新成功' : '创建成功', 'success')
      showAddModal.value = false
      editingDrone.value = null
      resetForm()
      await loadDrones()
    } else {
      showToastNotification(response.message || '操作失败', 'error')
    }
  } catch (error) {
    console.error('提交失败:', error)
    showToastNotification(error.message || '提交失败', 'error')
  } finally {
    submitting.value = false
  }
}

// 删除无人机
const handleDelete = async (id) => {
  try {
    await showConfirmDialog(
      '确定要删除此无人机吗?此操作无法撤销。',
      '确认删除',
      'error'
    )

    const response = await droneApi.deleteDrone(id)

    if (response.success) {
      showToastNotification('删除成功', 'success')
      await loadDrones()
    } else {
      showToastNotification(response.message || '删除失败', 'error')
    }
  } catch (error) {
    if (error.message !== 'cancel') {
      console.error('删除失败:', error)
      showToastNotification(error.message || '删除失败', 'error')
    }
  }
}

// 组件挂载时加载数据
onMounted(() => {
  loadDrones()
})
</script>

<style scoped>
.drone-management {
  padding: var(--space-6);
  max-width: 1400px;
  margin: 0 auto;
}

/* 页面头部 */
.page-header {
  margin-bottom: var(--space-6);
}

.page-title {
  font-size: var(--font-size-4xl);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-2) 0;
}

.page-subtitle {
  font-size: var(--font-size-lg);
  color: var(--color-text-secondary);
  margin: 0;
}

/* 统计卡片 */
.stats-cards {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: var(--space-4);
  margin-bottom: var(--space-8);
}

.stat-card {
  display: flex;
  align-items: center;
  gap: var(--space-4);
  padding: var(--space-5);
}

.stat-icon {
  font-size: 32px;
}

.stat-content {
  flex: 1;
}

.stat-value {
  font-size: var(--font-size-3xl);
  font-weight: 700;
  color: var(--color-text-primary);
}

.stat-label {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  margin-top: var(--space-1);
}

/* 工具栏 */
.toolbar {
  display: flex;
  justify-content: space-between;
  gap: var(--space-4);
  margin-bottom: var(--space-6);
  align-items: center;
}

.search-section {
  flex: 1;
  max-width: 400px;
}

.action-section {
  display: flex;
  gap: var(--space-3);
  align-items: center;
}

/* 视图切换按钮 */
.view-toggle {
  display: flex;
  gap: var(--space-1);
  background: var(--color-bg-secondary);
  border-radius: var(--radius-sm);
  padding: var(--space-1);
}

.view-button {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 36px;
  height: 36px;
  background: transparent;
  border: none;
  border-radius: var(--radius-xs);
  color: var(--color-text-secondary);
  cursor: pointer;
  transition: var(--transition-button);
}

.view-button:hover {
  background: var(--color-bg-tertiary);
  color: var(--color-text-primary);
}

.view-button.active {
  background: var(--color-bg-primary);
  color: var(--apple-blue);
  box-shadow: var(--shadow-sm);
}

.status-filter {
  height: 36px;
  padding: 0 var(--space-3);
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  cursor: pointer;
}

/* 加载和空状态 */
.loading-state,
.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: var(--space-12);
  color: var(--color-text-secondary);
}

.spinner {
  width: 40px;
  height: 40px;
  border: 3px solid var(--color-border-default);
  border-top-color: var(--apple-blue);
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

/* 网格视图 */
.drones-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
  gap: var(--space-4);
}

.drone-card {
  display: flex;
  flex-direction: column;
}

.drone-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-4);
}

.drone-icon {
  width: 48px;
  height: 48px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--color-bg-secondary);
  border-radius: var(--radius-md);
  color: var(--apple-blue);
}

.status-dot {
  width: 12px;
  height: 12px;
  border-radius: 50%;
  box-shadow: 0 0 0 2px var(--color-bg-primary);
}

.status-dot--online {
  background: var(--apple-green);
}

.status-dot--offline {
  background: var(--apple-gray-400);
}

.status-dot--flying {
  background: var(--apple-blue);
}

.status-dot--maintenance {
  background: var(--apple-orange);
}

.drone-body {
  flex: 1;
  margin-bottom: var(--space-4);
}

.drone-name {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-1) 0;
}

.drone-model {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  margin: 0 0 var(--space-4) 0;
}

.drone-info {
  display: flex;
  flex-direction: column;
  gap: var(--space-3);
}

.info-item {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  font-size: var(--font-size-sm);
}

.info-label {
  color: var(--color-text-secondary);
  min-width: 50px;
}

.info-value {
  color: var(--color-text-primary);
  flex: 1;
}

/* 状态徽章 */
.status-badge {
  display: inline-flex;
  padding: var(--space-1) var(--space-2);
  border-radius: var(--radius-xs);
  font-size: var(--font-size-xs);
  font-weight: 600;
}

.status-badge--online {
  background: rgba(48, 209, 88, 0.1);
  color: var(--apple-green);
}

.status-badge--offline {
  background: rgba(134, 134, 139, 0.1);
  color: var(--apple-gray-500);
}

.status-badge--flying {
  background: rgba(0, 113, 227, 0.1);
  color: var(--apple-blue);
}

.status-badge--maintenance {
  background: rgba(255, 149, 0, 0.1);
  color: var(--apple-orange);
}

/* 电量指示器 */
.battery-indicator {
  position: relative;
  flex: 1;
  height: 20px;
  background: var(--color-bg-tertiary);
  border-radius: var(--radius-xs);
  overflow: hidden;
  min-width: 80px;
}

.battery-fill {
  position: absolute;
  left: 0;
  top: 0;
  bottom: 0;
  border-radius: var(--radius-xs);
  transition: width 0.3s var(--easing-default);
}

.battery-fill--high {
  background: var(--apple-green);
}

.battery-fill--medium {
  background: var(--apple-orange);
}

.battery-fill--low {
  background: var(--apple-red);
}

.battery-text {
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  font-size: var(--font-size-xs);
  font-weight: 600;
  color: var(--color-text-primary);
  z-index: 1;
}

.drone-actions {
  display: flex;
  gap: var(--space-2);
  padding-top: var(--space-4);
  border-top: 1px solid var(--color-border-default);
}

/* 列表视图 */
.drones-list {
  background: var(--color-bg-primary);
  border-radius: var(--radius-md);
  overflow: hidden;
  box-shadow: var(--shadow-card);
}

.list-table {
  display: flex;
  flex-direction: column;
}

.table-header,
.table-row {
  display: grid;
  grid-template-columns: 120px 150px 100px 120px 180px 120px 100px;
  gap: var(--space-4);
  padding: var(--space-4);
  align-items: center;
}

.table-header {
  background: var(--color-bg-secondary);
  font-size: var(--font-size-sm);
  font-weight: 600;
  color: var(--color-text-secondary);
  border-bottom: 1px solid var(--color-border-default);
}

.table-row {
  border-bottom: 1px solid var(--color-border-default);
  transition: background 0.2s ease;
}

.table-row:hover {
  background: var(--color-bg-secondary);
}

.table-row:last-child {
  border-bottom: none;
}

.table-col {
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
}

.drone-id {
  font-weight: 600;
  color: var(--apple-blue);
}

.table-actions {
  display: flex;
  gap: var(--space-2);
}

.action-btn {
  background: none;
  border: none;
  cursor: pointer;
  font-size: 16px;
  padding: var(--space-1);
  opacity: 0.7;
  transition: opacity 0.2s ease;
}

.action-btn:hover {
  opacity: 1;
}

.text-muted {
  color: var(--color-text-tertiary);
}

/* 表单 */
.drone-form {
  display: flex;
  flex-direction: column;
  gap: var(--space-4);
}

.form-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-4);
}

.form-field {
  margin-bottom: 0;
}

.form-label {
  display: block;
  font-size: var(--font-size-sm);
  font-weight: 500;
  color: var(--color-text-primary);
  margin-bottom: var(--space-2);
}

.form-select {
  width: 100%;
  height: 44px;
  padding: 0 var(--space-4);
  font-size: var(--font-size-base);
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: var(--transition-input);
}

.form-select:hover {
  border-color: var(--color-border-hover);
}

.form-select:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
  outline: none;
}

.form-section {
  padding: var(--space-4);
  background: var(--color-bg-secondary);
  border-radius: var(--radius-md);
}

.form-section-title {
  font-size: var(--font-size-base);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-4) 0;
}

.modal-actions {
  display: flex;
  justify-content: flex-end;
  gap: var(--space-3);
  margin-top: var(--space-6);
}

/* 响应式 */
@media (max-width: 768px) {
  .drone-management {
    padding: var(--space-4);
  }

  .toolbar {
    flex-direction: column;
    align-items: stretch;
  }

  .search-section {
    max-width: none;
  }

  .action-section {
    flex-wrap: wrap;
  }

  .stats-cards {
    grid-template-columns: repeat(2, 1fr);
  }

  .drones-grid {
    grid-template-columns: 1fr;
  }

  .drones-list {
    overflow-x: auto;
  }

  .table-header,
  .table-row {
    min-width: 900px;
  }

  .form-row {
    grid-template-columns: 1fr;
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
  font-size: var(--font-size-base, 15px);
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 500;
  flex: 1;
  line-height: 1.4;
}

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

@keyframes slideInRight {
  from {
    transform: translateX(400px);
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
    transform: translateX(400px);
    opacity: 0;
  }
}

/* Confirm 对话框样式 */
.confirm-dialog {
  padding: var(--space-6);
  text-align: center;
}

.confirm-icon {
  font-size: 48px;
  margin-bottom: var(--space-4);
}

.confirm-message {
  font-size: var(--font-size-lg);
  color: var(--color-text-primary);
  margin-bottom: var(--space-6);
  line-height: var(--line-height-relaxed);
}

.confirm-actions {
  display: flex;
  gap: var(--space-3);
  justify-content: center;
}

/* Toast 暗色模式 */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: #1C1C1E;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
  }

  .toast-message {
    color: #F5F5F7;
  }

  .toast-notification.success {
    border-left-color: #30D158;
  }

  .toast-notification.error {
    border-left-color: #FF453A;
  }

  .toast-notification.warning {
    border-left-color: #FF9F0A;
  }

  .toast-notification.info {
    border-left-color: #0A84FF;
  }

  .confirm-message {
    color: #F5F5F7;
  }
}
</style>
