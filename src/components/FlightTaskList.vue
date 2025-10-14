<template>
  <div class="task-management">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h1 class="page-title">任务管理中心</h1>
          <p class="page-subtitle">管理和监控所有低空飞行任务</p>
        </div>
        <div class="action-section">
          <AppleButton
            variant="primary"
            size="large"
            @click="showCreateDialog = true"
          >
            创建任务
          </AppleButton>
        </div>
      </div>
    </div>

    <!-- 统计卡片 -->
    <div class="stats-section">
      <div class="stats-grid">
        <AppleCard class="stat-card stat-card--primary">
          <div class="stat-icon">📊</div>
          <div class="stat-content">
            <h3>{{ tasks.length }}</h3>
            <p>总任务数</p>
          </div>
        </AppleCard>

        <AppleCard class="stat-card stat-card--warning">
          <div class="stat-icon">⏱️</div>
          <div class="stat-content">
            <h3>{{ runningTasks }}</h3>
            <p>进行中</p>
          </div>
        </AppleCard>

        <AppleCard class="stat-card stat-card--success">
          <div class="stat-icon">✅</div>
          <div class="stat-content">
            <h3>{{ completedTasks }}</h3>
            <p>已完成</p>
          </div>
        </AppleCard>

        <AppleCard class="stat-card stat-card--info">
          <div class="stat-icon">⏳</div>
          <div class="stat-content">
            <h3>{{ pendingTasks }}</h3>
            <p>待执行</p>
          </div>
        </AppleCard>
      </div>
    </div>

    <!-- 筛选和搜索区域 -->
    <AppleCard class="filter-card">
      <div class="filter-content">
        <div class="search-area">
          <AppleInput
            v-model="searchKeyword"
            placeholder="搜索任务ID或名称..."
            class="search-input"
          />
        </div>

        <div class="filter-controls">
          <select
            v-model="statusFilter"
            class="filter-select"
          >
            <option value="">
              全部状态
            </option>
            <option
              v-for="option in TASK_STATUS_OPTIONS"
              :key="option.value"
              :value="option.value"
            >
              {{ option.label }}
            </option>
          </select>

          <AppleButton
            variant="secondary"
            @click="refreshTasks"
          >
            刷新
          </AppleButton>
        </div>
      </div>
    </AppleCard>

    <!-- 任务列表 -->
    <AppleCard class="task-table-card">
      <template #header>
        <div class="card-header">
          <span class="card-title">任务列表</span>
          <span class="task-count-badge">共 {{ filteredTasks.length }} 个任务</span>
        </div>
      </template>

      <div
        v-loading="loading"
        class="task-table-wrapper"
      >
        <el-table
          :data="filteredTasks"
          class="task-table"
          @row-click="handleRowClick"
        >
          <el-table-column
            prop="id"
            label="任务ID"
            width="120"
            sortable
          >
            <template #default="{ row }">
              <div class="task-id">
                <span class="task-id-icon">📄</span>
                <span>{{ row.id }}</span>
              </div>
            </template>
          </el-table-column>

          <el-table-column
            prop="name"
            label="任务名称"
            min-width="200"
          >
            <template #default="{ row }">
              <div class="task-name">
                <span class="name">{{ row.name }}</span>
                <span
                  v-if="row.description"
                  class="description"
                >{{ row.description }}</span>
              </div>
            </template>
          </el-table-column>

          <el-table-column
            prop="status"
            label="状态"
            width="120"
            align="center"
          >
            <template #default="{ row }">
              <div class="status-indicator">
                <div
                  class="status-dot"
                  :class="getStatusClass(row.status)"
                />
                <span>{{ getStatusText(row.status) }}</span>
              </div>
            </template>
          </el-table-column>

          <el-table-column
            prop="progress"
            label="进度"
            width="150"
            align="center"
          >
            <template #default="{ row }">
              <div class="progress-wrapper">
                <div class="progress-bar">
                  <div
                    class="progress-fill"
                    :class="getProgressClass(getTaskProgress(row.status))"
                    :style="{ width: getTaskProgress(row.status) + '%' }"
                  />
                </div>
                <span class="progress-text">{{ getTaskProgress(row.status) }}%</span>
              </div>
            </template>
          </el-table-column>

          <el-table-column
            prop="createdAt"
            label="创建时间"
            width="180"
            sortable
          >
            <template #default="{ row }">
              <div class="time-info">
                <span class="time-icon">📅</span>
                <span>{{ formatTime(row.createdAt) }}</span>
              </div>
            </template>
          </el-table-column>

          <el-table-column
            label="操作"
            width="280"
            align="center"
            fixed="right"
          >
            <template #default="{ row }">
              <div class="action-buttons">
                <AppleButton
                  variant="secondary"
                  size="small"
                  @click.stop="viewDetail(row)"
                >
                  详情
                </AppleButton>
                <AppleButton
                  variant="secondary"
                  size="small"
                  @click.stop="editTask(row)"
                >
                  编辑
                </AppleButton>
                <AppleButton
                  v-if="row.status !== 'completed'"
                  variant="primary"
                  size="small"
                  @click.stop="executeTask(row)"
                >
                  执行
                </AppleButton>
                <AppleButton
                  variant="danger"
                  size="small"
                  @click.stop="deleteTask(row)"
                >
                  删除
                </AppleButton>
              </div>
            </template>
          </el-table-column>
        </el-table>
      </div>
    </AppleCard>

    <!-- 创建任务对话框 -->
    <AppleModal
      v-model="showCreateDialog"
      title="创建新任务"
      :show-close="true"
      @close="resetCreateForm"
    >
      <form
        class="task-form"
        @submit.prevent="createTask"
      >
        <div class="form-group">
          <label class="form-label">任务名称</label>
          <AppleInput
            v-model="newTask.name"
            placeholder="请输入任务名称"
            required
          />
        </div>

        <div class="form-group">
          <label class="form-label">任务描述</label>
          <textarea
            v-model="newTask.description"
            class="form-textarea"
            placeholder="请输入任务描述"
            rows="3"
          />
        </div>

        <div class="form-group">
          <label class="form-label">计划时间</label>
          <input
            v-model="newTask.scheduled_time"
            type="datetime-local"
            class="form-datetime"
          >
        </div>

        <div class="modal-actions">
          <AppleButton
            variant="secondary"
            @click="showCreateDialog = false"
          >
            取消
          </AppleButton>
          <AppleButton
            type="submit"
            variant="primary"
          >
            确定
          </AppleButton>
        </div>
      </form>
    </AppleModal>

    <!-- 编辑任务对话框 -->
    <AppleModal
      v-model="showEditDialog"
      title="编辑任务"
      :show-close="true"
      @close="resetEditForm"
    >
      <form
        v-if="editingTask"
        class="task-form"
        @submit.prevent="updateTask"
      >
        <div class="form-group">
          <label class="form-label">任务名称</label>
          <AppleInput
            v-model="editingTask.name"
            placeholder="请输入任务名称"
            required
          />
        </div>

        <div class="form-group">
          <label class="form-label">任务描述</label>
          <textarea
            v-model="editingTask.description"
            class="form-textarea"
            placeholder="请输入任务描述"
            rows="3"
          />
        </div>

        <div class="form-group">
          <label class="form-label">计划时间</label>
          <input
            v-model="editingTask.scheduledTime"
            type="datetime-local"
            class="form-datetime"
          >
        </div>

        <div class="modal-actions">
          <AppleButton
            variant="secondary"
            @click="showEditDialog = false"
          >
            取消
          </AppleButton>
          <AppleButton
            type="submit"
            variant="primary"
          >
            确定
          </AppleButton>
        </div>
      </form>
    </AppleModal>
  </div>
</template>

<script setup>
import { ref, computed, reactive, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { AppleButton, AppleCard, AppleInput, AppleModal } from '@/components/apple'
import { useStore } from 'vuex'
// 🔒 BUG #5修复: 导入任务状态常量
import { TASK_STATUS, getTaskStatusLabel, TASK_STATUS_OPTIONS } from '@/constants/taskConstants'

// Vuex store
const store = useStore()

const loading = ref(false)
const searchKeyword = ref('')
const statusFilter = ref('')
const showCreateDialog = ref(false)
const showEditDialog = ref(false)
const editingTask = ref(null)

// 从 Vuex store 获取任务数据
const tasks = computed(() => store.state.flightTasks || [])

const newTask = reactive({
  name: '',
  description: '',
  scheduled_time: ''
})

// 🔒 BUG #5修复: 使用常量而不是硬编码中文
const filteredTasks = computed(() => {
  let result = tasks.value

  if (searchKeyword.value) {
    result = result.filter(task =>
      task.name.includes(searchKeyword.value) ||
      task.id.toString().includes(searchKeyword.value)
    )
  }

  if (statusFilter.value) {
    // 直接使用英文状态值进行过滤
    result = result.filter(task => task.status === statusFilter.value)
  }

  return result
})

const runningTasks = computed(() => tasks.value.filter(t => t.status === 'ongoing').length)
const completedTasks = computed(() => tasks.value.filter(t => t.status === 'completed').length)
const pendingTasks = computed(() => tasks.value.filter(t => t.status === 'pending').length)

// 方法
const refreshTasks = async () => {
  loading.value = true
  try {
    await store.dispatch('fetchFlightTasks')
    ElMessage.success('任务列表已刷新')
  } catch (error) {
    ElMessage.error('刷新失败：' + error.message)
  } finally {
    loading.value = false
  }
}

// 加载任务数据
const loadTasks = async () => {
  loading.value = true
  try {
    await store.dispatch('fetchFlightTasks')
  } catch (error) {
    ElMessage.error('加载任务失败：' + error.message)
  } finally {
    loading.value = false
  }
}

const viewDetail = (task) => {
  ElMessage.info(`查看任务详情:${task.name}`)
}

const editTask = (task) => {
  editingTask.value = { ...task }
  showEditDialog.value = true
}

const executeTask = async (task) => {
  try {
    await ElMessageBox.confirm(`确定要执行任务 "${task.name}" 吗?`, '确认执行', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning'
    })

    await store.dispatch('updateTask', {
      id: task.id,
      updates: { status: 'ongoing' }
    })
    ElMessage.success('任务执行成功')
  } catch (error) {
    if (error.message) {
      ElMessage.error('执行失败：' + error.message)
    }
  }
}

const deleteTask = async (task) => {
  try {
    await ElMessageBox.confirm(`确定要删除任务 "${task.name}" 吗?`, '确认删除', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'error'
    })

    await store.dispatch('deleteTask', task.id)
    ElMessage.success('任务删除成功')
  } catch (error) {
    if (error.message) {
      ElMessage.error('删除失败：' + error.message)
    }
  }
}

const createTask = async () => {
  if (!newTask.name) {
    ElMessage.warning('请输入任务名称')
    return
  }

  try {
    await store.dispatch('createTask', {
      name: newTask.name,
      description: newTask.description || '',
      scheduled_time: newTask.scheduled_time || null
    })

    showCreateDialog.value = false
    resetCreateForm()
    ElMessage.success('任务创建成功')
  } catch (error) {
    ElMessage.error('创建失败：' + error.message)
  }
}

const updateTask = async () => {
  if (!editingTask.value.name) {
    ElMessage.warning('请输入任务名称')
    return
  }

  try {
    await store.dispatch('updateTask', {
      id: editingTask.value.id,
      updates: {
        name: editingTask.value.name,
        description: editingTask.value.description,
        scheduled_time: editingTask.value.scheduledTime || null
      }
    })

    showEditDialog.value = false
    editingTask.value = null
    ElMessage.success('任务更新成功')
  } catch (error) {
    ElMessage.error('更新失败：' + error.message)
  }
}

const resetCreateForm = () => {
  newTask.name = ''
  newTask.description = ''
  newTask.scheduled_time = ''
}

const resetEditForm = () => {
  editingTask.value = null
}

// 🔒 BUG #5修复: 使用常量工具函数获取状态文本
const getStatusText = (status) => {
  return getTaskStatusLabel(status)
}

// 格式化时间
const formatTime = (timeStr) => {
  if (!timeStr) return ''
  const date = new Date(timeStr)
  return date.toLocaleString('zh-CN')
}

// 组件挂载时加载数据
onMounted(() => {
  loadTasks()
})

const handleRowClick = (row) => {
  viewDetail(row)
}

const getStatusClass = (status) => {
  const classMap = {
    'ongoing': 'status-running',
    'completed': 'status-completed',
    'pending': 'status-pending'
  }
  return classMap[status] || ''
}

const getProgressClass = (progress) => {
  if (progress < 30) return 'progress-fill--low'
  if (progress < 70) return 'progress-fill--medium'
  return 'progress-fill--high'
}

// 获取进度（暂时返回基于状态的进度）
const getTaskProgress = (status) => {
  const progressMap = {
    'pending': 0,
    'ongoing': 50,
    'completed': 100
  }
  return progressMap[status] || 0
}
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.task-management {
  min-height: 100%;
  padding: var(--space-6);
  background: var(--color-bg-primary);
}

/* 页面头部 */
.page-header {
  margin-bottom: var(--space-6);
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: var(--space-6) 0;
}

.title-section {
  flex: 1;
}

.page-title {
  margin: 0 0 var(--space-2) 0;
  font-size: var(--font-size-3xl);
  font-weight: 600;
  color: var(--color-text-primary);
  letter-spacing: var(--letter-spacing-tight);
  line-height: var(--line-height-compact);
}

.page-subtitle {
  margin: 0;
  color: var(--color-text-secondary);
  font-size: var(--font-size-base);
  line-height: var(--line-height-relaxed);
}

/* 统计卡片 */
.stats-section {
  margin-bottom: var(--space-6);
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: var(--space-4);
}

.stat-card {
  padding: var(--space-6);
  display: flex;
  align-items: center;
  gap: var(--space-4);
  transition: var(--transition-smooth);
  cursor: default;
}

.stat-card:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-lg);
}

.stat-icon {
  width: 56px;
  height: 56px;
  border-radius: var(--radius-lg);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 28px;
  flex-shrink: 0;
  background: var(--color-bg-secondary);
}

.stat-card--primary .stat-icon {
  background: rgba(0, 113, 227, 0.08);
}

.stat-card--warning .stat-icon {
  background: rgba(255, 149, 0, 0.08);
}

.stat-card--success .stat-icon {
  background: rgba(52, 199, 89, 0.08);
}

.stat-card--info .stat-icon {
  background: rgba(90, 200, 250, 0.08);
}

.stat-content {
  flex: 1;
}

.stat-content h3 {
  margin: 0 0 var(--space-1) 0;
  font-size: var(--font-size-3xl);
  font-weight: 700;
  color: var(--color-text-primary);
  line-height: var(--line-height-compact);
}

.stat-content p {
  margin: 0;
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  font-weight: 500;
  line-height: var(--line-height-relaxed);
}

/* 筛选区域 */
.filter-card {
  margin-bottom: var(--space-6);
  padding: var(--space-5);
}

.filter-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: var(--space-4);
}

.search-area {
  flex: 1;
  max-width: 400px;
}

.filter-controls {
  display: flex;
  gap: var(--space-3);
  align-items: center;
}

.filter-select {
  width: 180px;
  height: 44px;
  padding: 0 var(--space-4);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-base);
  font-weight: 400;
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  outline: none;
  transition: var(--transition-input);
  -webkit-appearance: none;
  box-sizing: border-box;
  background-image: url("data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' fill='none' viewBox='0 0 20 20'%3e%3cpath stroke='%2386868b' stroke-linecap='round' stroke-linejoin='round' stroke-width='1.5' d='m6 8 4 4 4-4'/%3e%3c/svg%3e");
  background-position: right 12px center;
  background-repeat: no-repeat;
  background-size: 16px;
  padding-right: 40px;
  cursor: pointer;
}

.filter-select:hover {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.filter-select:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
  background-color: var(--color-bg-primary);
}

/* 任务列表 */
.task-table-card {
  margin-bottom: var(--space-6);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: var(--space-5);
  border-bottom: 1px solid var(--color-border-default);
}

.card-title {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
}

.task-count-badge {
  padding: var(--space-1) var(--space-3);
  background: var(--color-bg-secondary);
  border-radius: var(--radius-full);
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  font-weight: 500;
}

.task-table-wrapper {
  min-height: 400px;
}

.task-table {
  width: 100%;
}

.task-id {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  font-weight: 600;
  color: var(--apple-blue);
}

.task-id-icon {
  font-size: 16px;
}

.task-name .name {
  display: block;
  font-weight: 600;
  color: var(--color-text-primary);
  margin-bottom: var(--space-1);
  line-height: var(--line-height-compact);
}

.task-name .description {
  display: block;
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  line-height: var(--line-height-relaxed);
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  justify-content: center;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

.status-running {
  background: var(--apple-orange);
}

.status-completed {
  background: var(--apple-green);
}

.status-pending {
  background: var(--apple-gray);
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.5; }
  100% { opacity: 1; }
}

.progress-wrapper {
  display: flex;
  align-items: center;
  gap: var(--space-3);
}

.progress-bar {
  flex: 1;
  height: 8px;
  background: var(--color-bg-secondary);
  border-radius: var(--radius-full);
  overflow: hidden;
}

.progress-fill {
  height: 100%;
  border-radius: var(--radius-full);
  transition: width 0.3s ease, background-color 0.3s ease;
}

.progress-fill--low {
  background: var(--apple-red);
}

.progress-fill--medium {
  background: var(--apple-orange);
}

.progress-fill--high {
  background: var(--apple-green);
}

.progress-text {
  font-size: var(--font-size-sm);
  font-weight: 600;
  color: var(--color-text-secondary);
  min-width: 40px;
  text-align: right;
}

.time-info {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  justify-content: center;
}

.time-icon {
  font-size: 14px;
}

.action-buttons {
  display: flex;
  gap: var(--space-2);
  justify-content: center;
  flex-wrap: wrap;
}

/* 表单样式 */
.task-form {
  padding: var(--space-5);
}

.form-group {
  margin-bottom: var(--space-5);
}

.form-label {
  display: block;
  margin-bottom: var(--space-2);
  font-size: var(--font-size-sm);
  font-weight: 500;
  color: var(--color-text-primary);
}

.form-textarea,
.form-datetime {
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
  box-sizing: border-box;
  resize: vertical;
}

.form-textarea:hover,
.form-datetime:hover {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.form-textarea:focus,
.form-datetime:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
  background-color: var(--color-bg-primary);
}

.modal-actions {
  display: flex;
  gap: var(--space-3);
  justify-content: flex-end;
  margin-top: var(--space-6);
  padding-top: var(--space-5);
  border-top: 1px solid var(--color-border-default);
}

/* Element Plus 表格样式覆盖 */
:deep(.el-table) {
  background: transparent;
  color: var(--color-text-primary);
  font-family: var(--font-family-primary);
}

:deep(.el-table__header) {
  background-color: var(--color-bg-secondary);
}

:deep(.el-table th) {
  background-color: var(--color-bg-secondary);
  color: var(--color-text-secondary);
  font-weight: 600;
  border-bottom: 1px solid var(--color-border-default);
}

:deep(.el-table td) {
  border-bottom: 1px solid var(--color-border-default);
}

:deep(.el-table__row:hover) {
  background-color: var(--color-bg-secondary);
  cursor: pointer;
}

:deep(.el-table__body tr.current-row > td) {
  background-color: var(--color-bg-tertiary);
}

/* 响应式设计 */
@media (max-width: 1024px) {
  .task-management {
    padding: var(--space-4);
  }

  .stats-grid {
    grid-template-columns: repeat(2, 1fr);
  }

  .action-buttons {
    flex-direction: column;
  }
}

@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: var(--space-4);
    align-items: flex-start;
  }

  .filter-content {
    flex-direction: column;
    gap: var(--space-4);
  }

  .search-area,
  .filter-controls {
    width: 100%;
    max-width: none;
  }

  .filter-controls {
    justify-content: space-between;
  }

  .stats-grid {
    grid-template-columns: 1fr;
  }

  .page-title {
    font-size: var(--font-size-2xl);
  }
}

@media (max-width: 480px) {
  .task-management {
    padding: var(--space-3);
  }

  .action-buttons {
    width: 100%;
  }

  .action-buttons button {
    flex: 1;
    min-width: 0;
  }
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .task-management {
    background: #000000;
  }

  .page-title {
    color: #f2f2f7;
  }

  .page-subtitle {
    color: #86868b;
  }

  .stat-content h3 {
    color: #f2f2f7;
  }

  .stat-content p {
    color: #86868b;
  }

  .card-title {
    color: #f2f2f7;
  }

  .task-count-badge {
    background: #2c2c2e;
    color: #86868b;
  }

  .task-name .name {
    color: #f2f2f7;
  }

  .task-name .description {
    color: #86868b;
  }

  .form-label {
    color: #f2f2f7;
  }

  .form-textarea,
  .form-datetime,
  .filter-select {
    background: #2c2c2e;
    border: 1px solid #48484a;
    color: #f2f2f7;
  }

  .form-textarea:focus,
  .form-datetime:focus,
  .filter-select:focus {
    border-color: #0a84ff;
    box-shadow: 0 0 0 3px rgba(10, 132, 255, 0.1);
  }
}
</style>
