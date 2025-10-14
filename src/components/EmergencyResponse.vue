<template>
  <div class="emergency-response">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h2>应急响应</h2>
          <p>紧急事件监控与处理</p>
        </div>
        <div class="action-section">
          <el-button
            type="danger"
            size="large"
            @click="showCreateDialog = true"
          >
            <el-icon><CirclePlus /></el-icon>
            创建紧急事件
          </el-button>
          <el-button
            type="primary"
            size="large"
            @click="refreshEvents"
          >
            <el-icon><Refresh /></el-icon>
            刷新
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
            style="background: linear-gradient(135deg, #f56c6c 0%, #ff8a80 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Warning />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ statistics.active_events || 0 }}</h3>
            <p>激活事件</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #e6a23c 0%, #f5a623 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Bell />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ statistics.total_events || 0 }}</h3>
            <p>总事件数</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #67c23a 0%, #85ce61 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Check />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ statistics.by_status?.resolved || 0 }}</h3>
            <p>已解决</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #909399 0%, #b1b3b8 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Close />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ statistics.by_status?.cancelled || 0 }}</h3>
            <p>已取消</p>
          </div>
        </SmartCard>
      </div>
    </div>

    <!-- 过滤器 -->
    <SmartCard
      class="filter-card"
      hover-effect
    >
      <div class="filter-section">
        <el-select
          v-model="filter.status"
          placeholder="状态筛选"
          clearable
          @change="refreshEvents"
        >
          <el-option
            v-for="option in EmergencyStatusOptions"
            :key="option.value"
            :label="option.label"
            :value="option.value"
          />
        </el-select>

        <el-select
          v-model="filter.severity"
          placeholder="严重程度筛选"
          clearable
          @change="refreshEvents"
        >
          <el-option
            v-for="option in EmergencySeverityOptions"
            :key="option.value"
            :label="option.label"
            :value="option.value"
          />
        </el-select>
      </div>
    </SmartCard>

    <!-- 事件列表 -->
    <SmartCard
      class="events-card"
      hover-effect
    >
      <template #header>
        <div class="card-header">
          <span class="card-title">紧急事件列表</span>
        </div>
      </template>

      <el-table
        v-loading="loading"
        :data="events"
        style="width: 100%"
        @row-click="handleRowClick"
      >
        <el-table-column
          prop="event_code"
          label="事件编号"
          width="150"
        />

        <el-table-column
          prop="title"
          label="标题"
          min-width="200"
        />

        <el-table-column
          label="类型"
          width="120"
        >
          <template #default="{ row }">
            <el-tag
              :color="emergencyApi.getEventTypeColor(row.type)"
              style="color: white;"
            >
              {{ emergencyApi.getEventTypeText(row.type) }}
            </el-tag>
          </template>
        </el-table-column>

        <el-table-column
          label="严重程度"
          width="120"
        >
          <template #default="{ row }">
            <el-tag :type="emergencyApi.getSeverityTagType(row.severity)">
              {{ emergencyApi.getSeverityText(row.severity) }}
            </el-tag>
          </template>
        </el-table-column>

        <el-table-column
          label="状态"
          width="120"
        >
          <template #default="{ row }">
            <el-tag :type="emergencyApi.getStatusTagType(row.status)">
              {{ emergencyApi.getStatusText(row.status) }}
            </el-tag>
          </template>
        </el-table-column>

        <el-table-column
          prop="created_at"
          label="创建时间"
          width="180"
        />

        <el-table-column
          label="操作"
          width="280"
          fixed="right"
        >
          <template #default="{ row }">
            <el-button
              v-if="row.status === EmergencyStatus.ACTIVE"
              type="warning"
              size="small"
              @click.stop="handleRespond(row)"
            >
              响应
            </el-button>
            <el-button
              v-if="row.status === EmergencyStatus.RESPONDING"
              type="success"
              size="small"
              @click.stop="handleResolve(row)"
            >
              解决
            </el-button>
            <el-button
              v-if="row.status === EmergencyStatus.ACTIVE || row.status === EmergencyStatus.RESPONDING"
              type="info"
              size="small"
              @click.stop="handleCancel(row)"
            >
              取消
            </el-button>
            <el-button
              type="primary"
              size="small"
              @click.stop="handleViewDetail(row)"
            >
              详情
            </el-button>
          </template>
        </el-table-column>
      </el-table>

      <div class="pagination">
        <el-pagination
          v-model:current-page="pagination.page"
          v-model:page-size="pagination.page_size"
          :total="pagination.total"
          :page-sizes="[10, 20, 50, 100]"
          layout="total, sizes, prev, pager, next, jumper"
          @current-change="refreshEvents"
          @size-change="refreshEvents"
        />
      </div>
    </SmartCard>

    <!-- 创建事件对话框 -->
    <el-dialog
      v-model="showCreateDialog"
      title="创建紧急事件"
      width="600px"
      :close-on-click-modal="false"
    >
      <el-form
        ref="createFormRef"
        :model="createForm"
        :rules="createRules"
        label-width="100px"
      >
        <el-form-item
          label="任务ID"
          prop="task_id"
        >
          <el-input-number
            v-model="createForm.task_id"
            :min="1"
            style="width: 100%"
          />
        </el-form-item>

        <el-form-item
          label="事件类型"
          prop="type"
        >
          <el-select
            v-model="createForm.type"
            style="width: 100%"
          >
            <el-option
              v-for="option in EmergencyTypeOptions"
              :key="option.value"
              :label="option.label"
              :value="option.value"
            />
          </el-select>
        </el-form-item>

        <el-form-item
          label="严重程度"
          prop="severity"
        >
          <el-select
            v-model="createForm.severity"
            style="width: 100%"
          >
            <el-option
              v-for="option in EmergencySeverityOptions"
              :key="option.value"
              :label="option.label"
              :value="option.value"
            />
          </el-select>
        </el-form-item>

        <el-form-item
          label="标题"
          prop="title"
        >
          <el-input v-model="createForm.title" />
        </el-form-item>

        <el-form-item
          label="描述"
          prop="description"
        >
          <el-input
            v-model="createForm.description"
            type="textarea"
            :rows="4"
          />
        </el-form-item>

        <el-form-item
          label="纬度"
          prop="lat"
        >
          <el-input-number
            v-model="createForm.lat"
            :precision="6"
            style="width: 100%"
          />
        </el-form-item>

        <el-form-item
          label="经度"
          prop="lon"
        >
          <el-input-number
            v-model="createForm.lon"
            :precision="6"
            style="width: 100%"
          />
        </el-form-item>
      </el-form>

      <template #footer>
        <el-button @click="showCreateDialog = false">
          取消
        </el-button>
        <el-button
          type="danger"
          @click="handleCreate"
        >
          创建
        </el-button>
      </template>
    </el-dialog>

    <!-- 响应事件对话框 -->
    <el-dialog
      v-model="showRespondDialog"
      title="响应紧急事件"
      width="600px"
      :close-on-click-modal="false"
    >
      <el-form
        ref="respondFormRef"
        :model="respondForm"
        :rules="respondRules"
        label-width="100px"
      >
        <el-form-item
          label="响应措施"
          prop="response_action"
        >
          <el-input
            v-model="respondForm.response_action"
            type="textarea"
            :rows="3"
            placeholder="请输入响应措施"
          />
        </el-form-item>

        <el-form-item
          label="响应备注"
          prop="response_notes"
        >
          <el-input
            v-model="respondForm.response_notes"
            type="textarea"
            :rows="3"
            placeholder="请输入响应备注（可选）"
          />
        </el-form-item>
      </el-form>

      <template #footer>
        <el-button @click="showRespondDialog = false">
          取消
        </el-button>
        <el-button
          type="warning"
          @click="handleSubmitRespond"
        >
          提交响应
        </el-button>
      </template>
    </el-dialog>

    <!-- 事件详情对话框 -->
    <el-dialog
      v-model="showDetailDialog"
      title="事件详情"
      width="700px"
    >
      <div
        v-if="selectedEvent"
        class="event-detail"
      >
        <div class="detail-section">
          <h3>基本信息</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="label">事件编号:</span>
              <span class="value">{{ selectedEvent.event_code }}</span>
            </div>
            <div class="detail-item">
              <span class="label">任务ID:</span>
              <span class="value">{{ selectedEvent.task_id }}</span>
            </div>
            <div class="detail-item">
              <span class="label">类型:</span>
              <el-tag
                :color="emergencyApi.getEventTypeColor(selectedEvent.type)"
                style="color: white;"
              >
                {{ emergencyApi.getEventTypeText(selectedEvent.type) }}
              </el-tag>
            </div>
            <div class="detail-item">
              <span class="label">严重程度:</span>
              <el-tag :type="emergencyApi.getSeverityTagType(selectedEvent.severity)">
                {{ emergencyApi.getSeverityText(selectedEvent.severity) }}
              </el-tag>
            </div>
            <div class="detail-item">
              <span class="label">状态:</span>
              <el-tag :type="emergencyApi.getStatusTagType(selectedEvent.status)">
                {{ emergencyApi.getStatusText(selectedEvent.status) }}
              </el-tag>
            </div>
          </div>
        </div>

        <div class="detail-section">
          <h3>事件内容</h3>
          <div class="detail-item">
            <span class="label">标题:</span>
            <span class="value">{{ selectedEvent.title }}</span>
          </div>
          <div class="detail-item">
            <span class="label">描述:</span>
            <p class="description">
              {{ selectedEvent.description }}
            </p>
          </div>
        </div>

        <div class="detail-section">
          <h3>位置信息</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="label">纬度:</span>
              <span class="value">{{ selectedEvent.location?.lat }}</span>
            </div>
            <div class="detail-item">
              <span class="label">经度:</span>
              <span class="value">{{ selectedEvent.location?.lon }}</span>
            </div>
          </div>
        </div>

        <div
          v-if="selectedEvent.response_action"
          class="detail-section"
        >
          <h3>响应信息</h3>
          <div class="detail-item">
            <span class="label">响应措施:</span>
            <p class="description">
              {{ selectedEvent.response_action }}
            </p>
          </div>
          <div
            v-if="selectedEvent.response_notes"
            class="detail-item"
          >
            <span class="label">响应备注:</span>
            <p class="description">
              {{ selectedEvent.response_notes }}
            </p>
          </div>
          <div class="detail-item">
            <span class="label">响应时间:</span>
            <span class="value">{{ selectedEvent.responded_at || '--' }}</span>
          </div>
        </div>

        <div class="detail-section">
          <h3>时间信息</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="label">创建时间:</span>
              <span class="value">{{ selectedEvent.created_at }}</span>
            </div>
            <div class="detail-item">
              <span class="label">更新时间:</span>
              <span class="value">{{ selectedEvent.updated_at }}</span>
            </div>
            <div
              v-if="selectedEvent.resolved_at"
              class="detail-item"
            >
              <span class="label">解决时间:</span>
              <span class="value">{{ selectedEvent.resolved_at }}</span>
            </div>
          </div>
        </div>
      </div>

      <template #footer>
        <el-button @click="showDetailDialog = false">
          关闭
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  CirclePlus, Refresh, Warning, Bell, Check, Close
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'
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

const createRules = {
  task_id: [{ required: true, message: '请输入任务ID', trigger: 'blur' }],
  type: [{ required: true, message: '请选择事件类型', trigger: 'change' }],
  severity: [{ required: true, message: '请选择严重程度', trigger: 'change' }],
  title: [{ required: true, message: '请输入标题', trigger: 'blur' }],
  description: [{ required: true, message: '请输入描述', trigger: 'blur' }],
  lat: [{ required: true, message: '请输入纬度', trigger: 'blur' }],
  lon: [{ required: true, message: '请输入经度', trigger: 'blur' }]
}

const respondForm = reactive({
  response_action: '',
  response_notes: ''
})

const respondRules = {
  response_action: [{ required: true, message: '请输入响应措施', trigger: 'blur' }]
}

const createFormRef = ref(null)
const respondFormRef = ref(null)

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
  if (!createFormRef.value) return

  await createFormRef.value.validate(async (valid) => {
    if (valid) {
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
          ElMessage.success('紧急事件创建成功')
          showCreateDialog.value = false
          createFormRef.value.resetFields()
          refreshEvents()
          refreshStatistics()
        }
      } catch (error) {
        console.error('创建事件失败:', error)
      }
    }
  })
}

const handleRespond = (event) => {
  currentRespondEvent.value = event
  respondForm.response_action = ''
  respondForm.response_notes = ''
  showRespondDialog.value = true
}

const handleSubmitRespond = async () => {
  if (!respondFormRef.value) return

  await respondFormRef.value.validate(async (valid) => {
    if (valid) {
      try {
        const response = await emergencyApi.respondToEvent(
          currentRespondEvent.value.id,
          respondForm.response_action,
          respondForm.response_notes
        )

        if (response.success) {
          ElMessage.success('响应提交成功')
          showRespondDialog.value = false
          refreshEvents()
          refreshStatistics()
        }
      } catch (error) {
        console.error('响应事件失败:', error)
      }
    }
  })
}

const handleResolve = async (event) => {
  try {
    await ElMessageBox.confirm('确认解决该紧急事件？', '确认', {
      confirmButtonText: '确认',
      cancelButtonText: '取消',
      type: 'success'
    })

    const response = await emergencyApi.resolveEvent(event.id)

    if (response.success) {
      ElMessage.success('事件已解决')
      refreshEvents()
      refreshStatistics()
    }
  } catch (error) {
    if (error !== 'cancel') {
      console.error('解决事件失败:', error)
    }
  }
}

const handleCancel = async (event) => {
  try {
    await ElMessageBox.confirm('确认取消该紧急事件？', '确认', {
      confirmButtonText: '确认',
      cancelButtonText: '取消',
      type: 'warning'
    })

    const response = await emergencyApi.cancelEvent(event.id)

    if (response.success) {
      ElMessage.success('事件已取消')
      refreshEvents()
      refreshStatistics()
    }
  } catch (error) {
    if (error !== 'cancel') {
      console.error('取消事件失败:', error)
    }
  }
}

const handleViewDetail = (event) => {
  selectedEvent.value = event
  showDetailDialog.value = true
}

const handleRowClick = (row) => {
  handleViewDetail(row)
}

// 初始化
onMounted(() => {
  refreshEvents()
  refreshStatistics()
})
</script>

<style scoped>
.emergency-response {
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
  background: linear-gradient(135deg, #f56c6c 0%, #e6a23c 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.title-section p {
  margin: 0;
  color: #7f8c8d;
  font-size: 16px;
}

.action-section {
  display: flex;
  gap: 12px;
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

/* 过滤器 */
.filter-card {
  margin-bottom: 24px;
}

.filter-section {
  display: flex;
  gap: 16px;
  padding: 16px 0;
}

/* 事件列表 */
.events-card {
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

.pagination {
  margin-top: 20px;
  display: flex;
  justify-content: flex-end;
}

/* 事件详情 */
.event-detail {
  padding: 20px 0;
}

.detail-section {
  margin-bottom: 24px;
}

.detail-section h3 {
  margin: 0 0 16px 0;
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
  border-bottom: 1px solid #eee;
  padding-bottom: 10px;
}

.detail-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 16px;
}

.detail-item {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.detail-item .label {
  font-size: 13px;
  color: #7f8c8d;
  font-weight: 500;
}

.detail-item .value {
  font-size: 15px;
  color: #2c3e50;
  font-weight: 600;
}

.detail-item .description {
  margin: 0;
  font-size: 14px;
  color: #606266;
  line-height: 1.6;
  white-space: pre-wrap;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
  }

  .action-section {
    width: 100%;
    flex-direction: column;
  }

  .action-section .el-button {
    width: 100%;
  }

  .filter-section {
    flex-direction: column;
  }

  .detail-grid {
    grid-template-columns: 1fr;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-table) {
  cursor: pointer;
}

:deep(.el-table__row:hover) {
  background-color: #f8f9fa;
}

:deep(.el-button) {
  border-radius: 8px;
  font-weight: 500;
}
</style>
