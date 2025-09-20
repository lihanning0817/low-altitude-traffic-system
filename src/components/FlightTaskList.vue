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
          prop="priority"
          label="优先级"
          width="100"
          align="center"
        >
          <template #default="{ row }">
            <el-tag
              :type="getPriorityType(row.priority)"
              :effect="row.priority === '高' ? 'dark' : 'plain'"
              size="small"
            >
              {{ row.priority }}
            </el-tag>
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
              <span>{{ row.status }}</span>
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
                :percentage="row.progress"
                :color="getProgressColor(row.progress)"
                :stroke-width="8"
                :show-text="false"
              />
              <span class="progress-text">{{ row.progress }}%</span>
            </div>
          </template>
        </el-table-column>

        <el-table-column
          prop="createTime"
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
              <span>{{ row.createTime }}</span>
            </div>
          </template>
        </el-table-column>

        <el-table-column
          label="操作"
          width="200"
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
                v-if="row.status !== '已完成'"
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
        <el-form-item label="优先级">
          <el-select
            v-model="newTask.priority"
            placeholder="请选择优先级"
          >
            <el-option
              label="低"
              value="低"
            />
            <el-option
              label="中"
              value="中"
            />
            <el-option
              label="高"
              value="高"
            />
          </el-select>
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
  </div>
</template>

<script setup>
import { ref, computed, reactive } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  Plus, DataLine, Clock, CircleCheck, Warning, Search, Refresh,
  Files, View, VideoPlay, Delete, Calendar
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'

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

// 防抖搜索处理
const debouncedSearch = debounce((value) => {
  searchKeyword.value = value
}, 300)

const tasks = ref([
  {
    id: 'T001',
    name: '城区巡航任务',
    description: '对城区主要交通干道进行巡航监控',
    status: '进行中',
    priority: '高',
    progress: 65,
    createTime: '2024-01-15 09:30:00'
  },
  {
    id: 'T002',
    name: '应急响应任务',
    description: '响应交通事故紧急情况',
    status: '已完成',
    priority: '高',
    progress: 100,
    createTime: '2024-01-14 14:20:00'
  },
  {
    id: 'T003',
    name: '定点监控任务',
    description: '对重点区域进行定点监控',
    status: '待执行',
    priority: '中',
    progress: 0,
    createTime: '2024-01-16 08:00:00'
  },
  {
    id: 'T004',
    name: '数据收集任务',
    description: '收集交通流量数据',
    status: '进行中',
    priority: '中',
    progress: 30,
    createTime: '2024-01-15 16:45:00'
  },
  {
    id: 'T005',
    name: '设备检测任务',
    description: '检测无人机设备状态',
    status: '已完成',
    priority: '低',
    progress: 100,
    createTime: '2024-01-13 10:15:00'
  }
])

const newTask = reactive({
  name: '',
  description: '',
  priority: '中'
})

// 计算属性
const filteredTasks = computed(() => {
  let result = tasks.value

  if (searchKeyword.value) {
    result = result.filter(task =>
      task.name.includes(searchKeyword.value) ||
      task.id.includes(searchKeyword.value)
    )
  }

  if (statusFilter.value) {
    result = result.filter(task => task.status === statusFilter.value)
  }

  return result
})

const runningTasks = computed(() => tasks.value.filter(t => t.status === '进行中').length)
const completedTasks = computed(() => tasks.value.filter(t => t.status === '已完成').length)
const pendingTasks = computed(() => tasks.value.filter(t => t.status === '待执行').length)

// 方法
const refreshTasks = () => {
  loading.value = true
  setTimeout(() => {
    loading.value = false
    ElMessage.success('任务列表已刷新')
  }, 1000)
}

const viewDetail = (task) => {
  ElMessage.info(`查看任务详情：${task.name}`)
}

const executeTask = async (task) => {
  try {
    await ElMessageBox.confirm(`确定要执行任务 "${task.name}" 吗？`, '确认执行', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning'
    })

    task.status = '进行中'
    task.progress = 10
    ElMessage.success('任务执行成功')
  } catch {
    // 用户取消操作
  }
}

const deleteTask = async (task) => {
  try {
    await ElMessageBox.confirm(`确定要删除任务 "${task.name}" 吗？`, '确认删除', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'error'
    })

    const index = tasks.value.findIndex(t => t.id === task.id)
    if (index > -1) {
      tasks.value.splice(index, 1)
      ElMessage.success('任务删除成功')
    }
  } catch {
    // 用户取消操作
  }
}

const createTask = () => {
  if (!newTask.name) {
    ElMessage.warning('请输入任务名称')
    return
  }

  const taskId = `T${String(tasks.value.length + 1).padStart(3, '0')}`
  tasks.value.push({
    id: taskId,
    name: newTask.name,
    description: newTask.description || '暂无描述',
    status: '待执行',
    priority: newTask.priority,
    progress: 0,
    createTime: new Date().toLocaleString('zh-CN', {
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit'
    }).replace(/\//g, '-')
  })

  showCreateDialog.value = false
  resetCreateForm()
  ElMessage.success('任务创建成功')
}

const resetCreateForm = () => {
  newTask.name = ''
  newTask.description = ''
  newTask.priority = '中'
}

const handleRowClick = (row) => {
  viewDetail(row)
}

const getPriorityType = (priority) => {
  const typeMap = { '高': 'danger', '中': 'warning', '低': 'info' }
  return typeMap[priority] || 'info'
}

const getStatusClass = (status) => {
  const classMap = {
    '进行中': 'status-running',
    '已完成': 'status-completed',
    '待执行': 'status-pending'
  }
  return classMap[status] || ''
}

const getProgressColor = (progress) => {
  if (progress < 30) return '#f56c6c'
  if (progress < 70) return '#e6a23c'
  return '#67c23a'
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
