<template>
  <div class="task-management">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h2>任务管理中心</h2>
          <p>管理和监控所有低空飞行任务</p>
        </div>
        <div class="action-section">
          <el-button
            type="primary"
            size="large"
            @click="showCreateDialog = true"
          >
            <el-icon><Plus /></el-icon>
            创建任务
          </el-button>
        </div>
      </div>
    </div>

    <!-- 统计卡片 -->
    <div class="stats-section">
      <div class="stats-grid">
        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <DataLine />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ tasks.length }}</h3>
            <p>总任务数</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Clock />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ runningTasks }}</h3>
            <p>进行中</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <CircleCheck />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ completedTasks }}</h3>
            <p>已完成</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Warning />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ pendingTasks }}</h3>
            <p>待执行</p>
          </div>
        </SmartCard>
      </div>
    </div>

    <!-- 筛选和搜索区域 -->
    <SmartCard
      hover-effect
      class="filter-card"
    >
      <div class="filter-content">
        <div class="search-area">
          <el-input
            :value="searchKeyword"
            placeholder="搜索任务ID或名称..."
            size="large"
            clearable
            class="search-input"
            @input="debouncedSearch"
          >
            <template #prefix>
              <el-icon><Search /></el-icon>
            </template>
          </el-input>
        </div>

        <div class="filter-controls">
          <el-select
            v-model="statusFilter"
            placeholder="按状态筛选"
            size="large"
            clearable
            class="filter-select"
          >
            <el-option
              label="全部状态"
              value=""
            />
            <el-option
              label="进行中"
              value="进行中"
            />
            <el-option
              label="已完成"
              value="已完成"
            />
            <el-option
              label="待执行"
              value="待执行"
            />
          </el-select>

          <el-button
            type="primary"
            size="large"
            @click="refreshTasks"
          >
            <el-icon><Refresh /></el-icon>
            刷新
          </el-button>
        </div>
      </div>
    </SmartCard>

    <!-- 任务列表 -->
    <SmartCard
      hover-effect
      class="task-table-card"
    >
      <template #header>
        <div class="card-header">
          <span class="card-title">任务列表</span>
          <el-tag type="info">
            共 {{ filteredTasks.length }} 个任务
          </el-tag>
        </div>
      </template>

      <el-table
        v-loading="loading"
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
              <el-icon
                size="16"
                color="#409eff"
              >
                <Files />
              </el-icon>
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
              <span class="description">{{ row.description }}</span>
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
              <el-progress
                :percentage="getTaskProgress(row.status)"
                :color="getProgressColor(getTaskProgress(row.status))"
                :stroke-width="8"
                :show-text="false"
              />
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
              <el-icon
                size="14"
                color="#909399"
              >
                <Calendar />
              </el-icon>
              <span>{{ formatTime(row.createdAt) }}</span>
            </div>
          </template>
        </el-table-column>

        <el-table-column
          label="操作"
          width="250"
          align="center"
          fixed="right"
        >
          <template #default="{ row }">
            <div class="action-buttons">
              <el-button
                type="primary"
                size="small"
                @click.stop="viewDetail(row)"
              >
                <el-icon><View /></el-icon>
                详情
              </el-button>
              <el-button
                type="warning"
                size="small"
                @click.stop="editTask(row)"
              >
                <el-icon><Edit /></el-icon>
                编辑
              </el-button>
              <el-button
                v-if="row.status !== 'completed'"
                type="success"
                size="small"
                @click.stop="executeTask(row)"
              >
                <el-icon><VideoPlay /></el-icon>
                执行
              </el-button>
              <el-button
                type="danger"
                size="small"
                @click.stop="deleteTask(row)"
              >
                <el-icon><Delete /></el-icon>
              </el-button>
            </div>
          </template>
        </el-table-column>
      </el-table>
    </SmartCard>

    <!-- 创建任务对话框 -->
    <el-dialog
      v-model="showCreateDialog"
      title="创建新任务"
      width="600px"
      @close="resetCreateForm"
    >
      <el-form
        :model="newTask"
        label-width="100px"
      >
        <el-form-item label="任务名称">
          <el-input
            v-model="newTask.name"
            placeholder="请输入任务名称"
          />
        </el-form-item>
        <el-form-item label="任务描述">
          <el-input
            v-model="newTask.description"
            type="textarea"
            :rows="3"
            placeholder="请输入任务描述"
          />
        </el-form-item>
        <el-form-item label="计划时间">
          <el-date-picker
            v-model="newTask.scheduled_time"
            type="datetime"
            placeholder="请选择计划执行时间"
            style="width: 100%"
          />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="showCreateDialog = false">
          取消
        </el-button>
        <el-button
          type="primary"
          @click="createTask"
        >
          确定
        </el-button>
      </template>
    </el-dialog>

    <!-- 编辑任务对话框 -->
    <el-dialog
      v-model="showEditDialog"
      title="编辑任务"
      width="600px"
      @close="resetEditForm"
    >
      <el-form
        v-if="editingTask"
        :model="editingTask"
        label-width="100px"
      >
        <el-form-item label="任务名称">
          <el-input
            v-model="editingTask.name"
            placeholder="请输入任务名称"
          />
        </el-form-item>
        <el-form-item label="任务描述">
          <el-input
            v-model="editingTask.description"
            type="textarea"
            :rows="3"
            placeholder="请输入任务描述"
          />
        </el-form-item>
        <el-form-item label="计划时间">
          <el-date-picker
            v-model="editingTask.scheduledTime"
            type="datetime"
            placeholder="请选择计划执行时间"
            style="width: 100%"
          />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="showEditDialog = false">
          取消
        </el-button>
        <el-button
          type="primary"
          @click="updateTask"
        >
          确定
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, computed, reactive, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  Plus, DataLine, Clock, CircleCheck, Warning, Search, Refresh,
  Files, View, VideoPlay, Delete, Calendar, Edit
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'
import flightTaskApi from '@/services/flightTaskApi'
import { useStore } from 'vuex'

// Vuex store
const store = useStore()

// 防抖函数
const debounce = (func, wait) => {
  let timeout
  return function executedFunction(...args) {
    const later = () => {
      clearTimeout(timeout)
      func(...args)
    }
    clearTimeout(timeout)
    timeout = setTimeout(later, wait)
  }
}

const loading = ref(false)
const searchKeyword = ref('')
const statusFilter = ref('')
const showCreateDialog = ref(false)
const showEditDialog = ref(false)
const editingTask = ref(null)

// 防抖搜索处理
const debouncedSearch = debounce((value) => {
  searchKeyword.value = value
}, 300)

// 从 Vuex store 获取任务数据
const tasks = computed(() => store.state.flightTasks || [])

const newTask = reactive({
  name: '',
  description: '',
  scheduled_time: ''
})

// 计算属性
const filteredTasks = computed(() => {
  let result = tasks.value

  if (searchKeyword.value) {
    result = result.filter(task =>
      task.name.includes(searchKeyword.value) ||
      task.id.toString().includes(searchKeyword.value)
    )
  }

  if (statusFilter.value) {
    // 将中文状态映射到英文状态
    const statusMap = {
      '进行中': 'ongoing',
      '已完成': 'completed',
      '待执行': 'pending'
    }
    const englishStatus = statusMap[statusFilter.value]
    if (englishStatus) {
      result = result.filter(task => task.status === englishStatus)
    }
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
  ElMessage.info(`查看任务详情：${task.name}`)
}

const editTask = (task) => {
  editingTask.value = { ...task }
  showEditDialog.value = true
}

const executeTask = async (task) => {
  try {
    await ElMessageBox.confirm(`确定要执行任务 "${task.name}" 吗？`, '确认执行', {
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
    await ElMessageBox.confirm(`确定要删除任务 "${task.name}" 吗？`, '确认删除', {
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

// 获取状态显示文本
const getStatusText = (status) => {
  const statusMap = {
    'pending': '待执行',
    'ongoing': '进行中',
    'completed': '已完成'
  }
  return statusMap[status] || status
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

const getProgressColor = (progress) => {
  if (progress < 30) return '#f56c6c'
  if (progress < 70) return '#e6a23c'
  return '#67c23a'
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
.task-management {
  min-height: 100%;
}

/* 页面头部 */
.page-header {
  margin-bottom: 24px;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 24px 0;
}

.title-section h2 {
  margin: 0 0 8px 0;
  font-size: 28px;
  font-weight: 600;
  color: #2c3e50;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.title-section p {
  margin: 0;
  color: #7f8c8d;
  font-size: 16px;
}

/* 统计卡片 */
.stats-section {
  margin-bottom: 24px;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 20px;
}

.stat-card {
  padding: 24px;
  display: flex;
  align-items: center;
  gap: 16px;
  transition: all 0.3s ease;
}

.stat-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 8px 30px rgba(0, 0, 0, 0.12);
}

.stat-icon {
  width: 60px;
  height: 60px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.stat-content h3 {
  margin: 0 0 4px 0;
  font-size: 28px;
  font-weight: 700;
  color: #2c3e50;
}

.stat-content p {
  margin: 0;
  font-size: 14px;
  color: #7f8c8d;
  font-weight: 500;
}

/* 筛选区域 */
.filter-card {
  margin-bottom: 24px;
}

.filter-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 20px;
}

.search-area {
  flex: 1;
  max-width: 400px;
}

.search-input {
  border-radius: 12px;
}

.filter-controls {
  display: flex;
  gap: 12px;
}

.filter-select {
  width: 150px;
}

/* 任务列表 */
.task-table-card {
  margin-bottom: 24px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-title {
  font-size: 18px;
  font-weight: 600;
  color: #2c3e50;
}

.task-table {
  border-radius: 12px;
}

.task-id {
  display: flex;
  align-items: center;
  gap: 8px;
  font-weight: 600;
  color: #409eff;
}

.task-name .name {
  display: block;
  font-weight: 600;
  color: #2c3e50;
  margin-bottom: 4px;
}

.task-name .description {
  display: block;
  font-size: 12px;
  color: #7f8c8d;
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: 8px;
  justify-content: center;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

.status-running {
  background: #409eff;
}

.status-completed {
  background: #67c23a;
}

.status-pending {
  background: #e6a23c;
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.5; }
  100% { opacity: 1; }
}

.progress-wrapper {
  display: flex;
  align-items: center;
  gap: 12px;
}

.progress-text {
  font-size: 12px;
  font-weight: 600;
  color: #606266;
  min-width: 35px;
}

.time-info {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 13px;
  color: #909399;
}

.action-buttons {
  display: flex;
  gap: 8px;
  justify-content: center;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
  }

  .filter-content {
    flex-direction: column;
    gap: 16px;
  }

  .search-area,
  .filter-controls {
    width: 100%;
  }

  .filter-controls {
    justify-content: space-between;
  }

  .stats-grid {
    grid-template-columns: 1fr;
  }

  .action-buttons {
    flex-direction: column;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-card__body) {
  padding: 24px;
}

:deep(.el-table__row:hover) {
  background-color: #f8f9fa;
  cursor: pointer;
}

:deep(.el-table__header) {
  background-color: #fafbfc;
}

:deep(.el-progress-bar__outer) {
  border-radius: 4px;
}

:deep(.el-input__wrapper) {
  border-radius: 12px;
  transition: all 0.3s ease;
}

:deep(.el-input__wrapper:hover) {
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.1);
}

:deep(.el-button) {
  border-radius: 8px;
  font-weight: 500;
}
</style>