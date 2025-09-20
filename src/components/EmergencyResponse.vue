<template>
  <div class="emergency-response">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h2>应急响应管理</h2>
          <p>实时监控和处理紧急事件</p>
        </div>
        <div class="status-indicators">
          <div class="status-item">
            <el-icon><Warning /></el-icon>
            <span>{{ activeEmergenciesCount }} 个活跃事件</span>
          </div>
          <div class="status-item">
            <el-icon><Timer /></el-icon>
            <span>平均响应时间 {{ avgResponseTime }}</span>
          </div>
        </div>
      </div>
    </div>

    <!-- 控制面板 -->
    <SmartCard
      hover-effect
      class="control-panel"
    >
      <el-card
        shadow="never"
        class="control-card"
      >
        <div class="control-content">
          <div class="left-controls">
            <el-button
              type="primary"
              @click="showReportDialog = true"
            >
              <el-icon><Plus /></el-icon>
              报告事件
            </el-button>
            <el-button @click="refreshData">
              <el-icon><Refresh /></el-icon>
              刷新
            </el-button>
          </div>

          <div class="right-controls">
            <el-select
              v-model="filterStatus"
              placeholder="状态筛选"
              style="width: 150px"
            >
              <el-option
                label="全部"
                value=""
              />
              <el-option
                label="已报告"
                value="reported"
              />
              <el-option
                label="响应中"
                value="responding"
              />
              <el-option
                label="已解决"
                value="resolved"
              />
            </el-select>
            <el-select
              v-model="filterSeverity"
              placeholder="严重程度"
              style="width: 150px"
            >
              <el-option
                label="全部"
                value=""
              />
              <el-option
                label="低"
                value="low"
              />
              <el-option
                label="中"
                value="medium"
              />
              <el-option
                label="高"
                value="high"
              />
            </el-select>
          </div>
        </div>
      </el-card>
    </SmartCard>

    <!-- 主要内容区域 -->
    <div class="main-content">
      <!-- 事件列表 -->
      <SmartCard
        hover-effect
        class="events-panel"
      >
        <el-card
          shadow="never"
          class="events-card"
        >
          <template #header>
            <div class="panel-header">
              <span class="panel-title">紧急事件</span>
              <el-tag type="danger">
                {{ emergencyEvents.length }} 个事件
              </el-tag>
            </div>
          </template>

          <el-table
            :data="filteredEvents"
            style="width: 100%"
            :row-class-name="getRowClassName"
            @row-click="selectEvent"
          >
            <el-table-column
              type="index"
              width="60"
            />
            <el-table-column
              prop="title"
              label="事件标题"
              min-width="180"
            />
            <el-table-column
              prop="severity"
              label="严重程度"
              width="120"
            >
              <template #default="{ row }">
                <el-tag :type="getSeverityType(row.severity)">
                  {{ row.severity === 'high' ? '高' : row.severity === 'medium' ? '中' : '低' }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column
              prop="status"
              label="状态"
              width="100"
            >
              <template #default="{ row }">
                <el-tag :type="getStatusType(row.status)">
                  {{ getStatusText(row.status) }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column
              prop="timestamp"
              label="时间"
              width="160"
            >
              <template #default="{ row }">
                {{ formatTime(row.timestamp) }}
              </template>
            </el-table-column>
            <el-table-column
              label="操作"
              width="120"
            >
              <template #default="{ row }">
                <el-button
                  v-if="row.status !== 'resolved'"
                  type="primary"
                  size="small"
                  @click.stop="handleResponse(row)"
                >
                  响应
                </el-button>
                <el-button
                  v-else
                  type="info"
                  size="small"
                  @click.stop="handleReopen(row)"
                >
                  重新打开
                </el-button>
              </template>
            </el-table-column>
          </el-table>
        </el-card>
      </SmartCard>

      <!-- 事件详情和地图 -->
      <div class="detail-and-map">
        <!-- 事件详情 -->
        <SmartCard
          hover-effect
          class="detail-panel"
        >
          <el-card
            shadow="never"
            class="detail-card"
          >
            <template #header>
              <div class="panel-header">
                <span class="panel-title">事件详情</span>
                <el-button
                  v-if="selectedEvent"
                  type="primary"
                  size="small"
                  @click="showOnMap"
                >
                  <el-icon><View /></el-icon>
                  显示在地图
                </el-button>
              </div>
            </template>

            <div
              v-if="selectedEvent"
              class="event-detail"
            >
              <div class="detail-item">
                <span class="label">标题:</span>
                <span class="value">{{ selectedEvent.title }}</span>
              </div>
              <div class="detail-item">
                <span class="label">描述:</span>
                <span class="value">{{ selectedEvent.description }}</span>
              </div>
              <div class="detail-item">
                <span class="label">严重程度:</span>
                <el-tag :type="getSeverityType(selectedEvent.severity)">
                  {{ selectedEvent.severity === 'high' ? '高' : selectedEvent.severity === 'medium' ? '中' : '低' }}
                </el-tag>
              </div>
              <div class="detail-item">
                <span class="label">状态:</span>
                <el-tag :type="getStatusType(selectedEvent.status)">
                  {{ getStatusText(selectedEvent.status) }}
                </el-tag>
              </div>
              <div class="detail-item">
                <span class="label">报告时间:</span>
                <span class="value">{{ formatTime(selectedEvent.timestamp) }}</span>
              </div>
              <div class="detail-item">
                <span class="label">报告来源:</span>
                <span class="value">{{ selectedEvent.reportedBy }}</span>
              </div>
              <div class="detail-item">
                <span class="label">响应团队:</span>
                <span class="value">{{ selectedEvent.responseTeam || '未分配' }}</span>
              </div>
              <div class="detail-item">
                <span class="label">预计响应时间:</span>
                <span class="value">{{ selectedEvent.estimatedResponseTime || 'N/A' }}</span>
              </div>
              <div class="detail-item">
                <span class="label">位置:</span>
                <span class="value">{{ selectedEvent.location.lat.toFixed(6) }}, {{ selectedEvent.location.lng.toFixed(6) }}</span>
              </div>
            </div>

            <div
              v-else
              class="no-selection"
            >
              <el-icon><InfoFilled /></el-icon>
              <p>请选择一个事件查看详情</p>
            </div>
          </el-card>
        </SmartCard>

        <!-- 地图 -->
        <SmartCard
          hover-effect
          class="map-panel"
        >
          <el-card
            shadow="never"
            class="map-card"
          >
            <template #header>
              <div class="panel-header">
                <span class="panel-title">事件地图</span>
                <el-button
                  type="primary"
                  size="small"
                  @click="centerMap"
                >
                  <el-icon><Aim /></el-icon>
                  居中
                </el-button>
              </div>
            </template>

            <div
              ref="mapContainer"
              class="map-view"
            />
          </el-card>
        </SmartCard>
      </div>
    </div>

    <!-- 报告事件对话框 -->
    <el-dialog
      v-model="showReportDialog"
      title="报告紧急事件"
      width="500px"
    >
      <el-form
        :model="reportForm"
        label-width="100px"
      >
        <el-form-item label="事件类型">
          <el-select
            v-model="reportForm.type"
            style="width: 100%"
          >
            <el-option
              label="交通事故"
              value="accident"
            />
            <el-option
              label="无人机异常"
              value="drone_failure"
            />
            <el-option
              label="气象预警"
              value="weather_alert"
            />
            <el-option
              label="非法飞行"
              value="illegal_flight"
            />
            <el-option
              label="其他"
              value="other"
            />
          </el-select>
        </el-form-item>
        <el-form-item label="标题">
          <el-input v-model="reportForm.title" />
        </el-form-item>
        <el-form-item label="描述">
          <el-input
            v-model="reportForm.description"
            type="textarea"
            rows="4"
          />
        </el-form-item>
        <el-form-item label="严重程度">
          <el-rate
            v-model="reportForm.severity"
            :max="3"
            :texts="['低', '中', '高']"
            text-color="#ff9900"
          />
        </el-form-item>
        <el-form-item label="位置">
          <el-input
            v-model="reportForm.location"
            placeholder="点击地图选择位置"
            readonly
          />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="showReportDialog = false">
          取消
        </el-button>
        <el-button
          type="primary"
          @click="submitReport"
        >
          提交
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted, watch } from 'vue'
import { useStore } from 'vuex'
import { ElMessage, ElMessageBox } from 'element-plus'
import mapContainer from 'vue'
import { Plus, Refresh, View, Aim, Warning, Timer, InfoFilled } from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'

// 状态管理
const store = useStore()

// 响应式数据
const showReportDialog = ref(false)
const filterStatus = ref('')
const filterSeverity = ref('')
const selectedEvent = ref(null)
const map = ref(null)

const reportForm = ref({
  type: '',
  title: '',
  description: '',
  severity: 2,
  location: ''
})

// 计算属性
const emergencyEvents = computed(() => store.state.emergencyEvents || [])
const activeEmergenciesCount = computed(() => {
  return emergencyEvents.value.filter(e => e.status === 'reported' || e.status === 'responding').length
})
const avgResponseTime = computed(() => {
  // 模拟计算平均响应时间
  const resolvedEvents = emergencyEvents.value.filter(e => e.status === 'resolved')
  if (resolvedEvents.length === 0) return 'N/A'
  return '12分钟'
})

const filteredEvents = computed(() => {
  return emergencyEvents.value.filter(event => {
    const statusMatch = !filterStatus.value || event.status === filterStatus.value
    const severityMatch = !filterSeverity.value || 
      (filterSeverity.value === 'low' && event.severity === 'low') ||
      (filterSeverity.value === 'medium' && event.severity === 'medium') ||
      (filterSeverity.value === 'high' && event.severity === 'high')
    return statusMatch && severityMatch
  })
})

// 方法
const getSeverityType = (severity) => {
  if (severity === 'high') return 'danger'
  if (severity === 'medium') return 'warning'
  return 'success'
}

const getStatusType = (status) => {
  if (status === 'reported') return 'info'
  if (status === 'responding') return 'warning'
  if (status === 'resolved') return 'success'
  return 'primary'
}

const getStatusText = (status) => {
  if (status === 'reported') return '已报告'
  if (status === 'responding') return '响应中'
  if (status === 'resolved') return '已解决'
  return status
}

const formatTime = (timestamp) => {
  if (!timestamp) return 'N/A'
  const date = new Date(timestamp)
  return date.toLocaleString('zh-CN')
}

const getRowClassName = ({ row }) => {
  if (row.severity === 'high') return 'high-severity-row'
  if (row.severity === 'medium') return 'medium-severity-row'
  return ''
}

const selectEvent = (event) => {
  selectedEvent.value = event
}

const handleResponse = (event) => {
  ElMessageBox.confirm(
    `确定要响应事件 "${event.title}" 吗？`,
    '确认响应',
    {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning'
    }
  ).then(() => {
    // 这里应该调用API更新事件状态
    store.dispatch('updateEmergencyEvent', {
      id: event.id,
      updates: { status: 'responding', responseTeam: '应急救援队' }
    })
    ElMessage.success('已开始响应事件')
  }).catch(() => {
    // 用户取消
  })
}

const handleReopen = (event) => {
  ElMessageBox.confirm(
    `确定要重新打开事件 "${event.title}" 吗？`,
    '确认重新打开',
    {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning'
    }
  ).then(() => {
    // 这里应该调用API更新事件状态
    store.dispatch('updateEmergencyEvent', {
      id: event.id,
      updates: { status: 'reported' }
    })
    ElMessage.success('事件已重新打开')
  }).catch(() => {
    // 用户取消
  })
}

const showOnMap = () => {
  if (!selectedEvent.value || !map.value) return

  // 清除现有标记
  map.value.clearMap()

  // 添加事件标记
  const marker = new AMap.Marker({
    position: [selectedEvent.value.location.lng, selectedEvent.value.location.lat],
    title: selectedEvent.value.title,
    content: `
      <div style="background: #fff; border: 2px solid #ff4d4f; border-radius: 8px; padding: 8px; font-size: 12px; font-weight: bold;">
        ${selectedEvent.value.title}
      </div>
    `,
    offset: new AMap.Pixel(-50, -30)
  })

  map.value.add(marker)
  map.value.setZoomAndCenter(15, [selectedEvent.value.location.lng, selectedEvent.value.location.lat])
}

const centerMap = () => {
  if (map.value) {
    map.value.setZoomAndCenter(11, [116.397428, 39.90923])
  }
}

const refreshData = () => {
  // 这里应该重新获取数据
  store.dispatch('fetchEmergencyEvents')
  ElMessage.success('数据已刷新')
}

const submitReport = () => {
  if (!reportForm.value.title || !reportForm.value.description) {
    ElMessage.warning('请填写标题和描述')
    return
  }

  // 这里应该调用API报告事件
  const newEvent = {
    id: 'E' + Date.now(),
    title: reportForm.value.title,
    description: reportForm.value.description,
    severity: reportForm.value.severity === 3 ? 'high' : reportForm.value.severity === 2 ? 'medium' : 'low',
    status: 'reported',
    reportedBy: '用户',
    timestamp: new Date().toISOString(),
    location: { lat: 39.90923, lng: 116.397428 } // 默认位置
  }

  store.dispatch('addEmergencyEvent', newEvent)
  ElMessage.success('事件报告成功')
  showReportDialog.value = false
  reportForm.value = {
    type: '',
    title: '',
    description: '',
    severity: 2,
    location: ''
  }
}

// 地图初始化
const initMap = () => {
  if (window.AMap && mapContainer.value) {
    map.value = new AMap.Map(mapContainer.value, {
      zoom: 11,
      center: [116.397428, 39.90923],
      viewMode: '3D',
      features: ['bg', 'road', 'building', 'point'],
      resizeEnable: true
    })

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

    // 添加点击事件以选择位置
    map.value.on('click', (e) => {
      if (showReportDialog.value) {
        reportForm.value.location = `${e.lnglat.lat.toFixed(6)}, ${e.lnglat.lng.toFixed(6)}`
      }
    })

    // 监听地图缩放和移动事件
    const debouncedZoomChange = debounce(() => {
      // 可以在这里处理缩放变化
    }, 100)

    const debouncedMoveEnd = debounce(() => {
      // 可以在这里处理地图移动
    }, 100)

    // 移除之前的事件监听器，避免重复绑定
    map.value.off('zoomchange', debouncedZoomChange)
    map.value.off('moveend', debouncedMoveEnd)
    
    map.value.on('zoomchange', debouncedZoomChange)
    map.value.on('moveend', debouncedMoveEnd)

// 监听紧急事件变化，实时更新地图标记
    watch(emergencyEvents, (newEvents) => {
      if (!map.value) return

      // 清除所有现有标记
      map.value.clearMap()

      // 重新添加所有事件标记
      newEvents.forEach(event => {
        const marker = new AMap.Marker({
          position: [event.location.lng, event.location.lat],
          title: event.title,
          icon: getMarkerIcon(event.severity),
          offset: new AMap.Pixel(-12, -12),
          zIndex: 100
        })

        // 添加点击事件
        marker.on('click', () => {
          selectEvent(event)
          showOnMap()
        })

        // 添加鼠标悬停事件
        marker.on('mouseover', () => {
          marker.setLabel({
            offset: new AMap.Pixel(10, -20),
            content: `<div style="background: #fff; border: 2px solid ${getSeverityType(event.severity) === 'danger' ? '#ff4d4f' : getSeverityType(event.severity) === 'warning' ? '#faad14' : '#52c41a'}; border-radius: 4px; padding: 4px 8px; font-size: 12px; color: #333;">${event.title}</div>`,
            direction: 'top'
          })
        })

        // 添加鼠标离开事件
        marker.on('mouseout', () => {
          marker.setLabel(null)
        })

        map.value.add(marker)
      })
    }, { deep: true })

    // 定期检查紧急事件状态
    const statusCheckInterval = setInterval(() => {
      if (map.value) {
        // 这里可以添加检查逻辑，例如检查事件是否超时
        const now = new Date()
        emergencyEvents.value.forEach(event => {
          if (event.status === 'reported' && 
              now - new Date(event.timestamp) > 300000) { // 5分钟未响应
            // 可以在地图上高亮显示
            console.log(`事件 ${event.id} 已超时，需要紧急处理`)
          }
        })
      }
    }, 60000) // 每分钟检查一次

    // 组件卸载时清理定时器
    onUnmounted(() => {
      if (statusCheckInterval) {
        clearInterval(statusCheckInterval)
      }
      if (map.value) {
        map.value.destroy()
      }
    })
  }
}

const getMarkerIcon = (severity) => {
  return new AMap.Icon({
    size: new AMap.Size(24, 24),
    image: `https://webapi.amap.com/theme/v1.3/markers/n/mark_b${severity === 'high' ? 'r' : severity === 'medium' ? 'y' : 'g'}.png`,
    imageSize: new AMap.Size(24, 24)
  })
}

// 生命周期钩子
onMounted(() => {
  // 初始化数据
  store.dispatch('initializeEmergencyLandingPoints')
  store.dispatch('fetchEmergencyEvents')
  
  // 初始化地图
  initMap()
})

onUnmounted(() => {
  if (map.value) {
    map.value.destroy()
  }
})
</script>

<style scoped>
.emergency-response {
  display: flex;
  flex-direction: column;
  height: 100%;
  gap: 20px;
}

/* 页面头部 */
.page-header {
  margin-bottom: 0;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 0;
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

.status-indicators {
  display: flex;
  gap: 24px;
  align-items: center;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  color: #606266;
}

/* 控制面板 */
.control-panel {
  margin-bottom: 0;
}

.control-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
}

.control-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 20px;
}

.left-controls,
.right-controls {
  display: flex;
  gap: 12px;
}

/* 主要内容区域 */
.main-content {
  display: flex;
  gap: 20px;
  flex: 1;
}

/* 事件列表 */
.events-panel {
  flex: 1;
}

.events-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
  height: 100%;
}

.panel-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.panel-title {
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
}

/* 事件详情和地图 */
.detail-and-map {
  display: flex;
  flex-direction: column;
  gap: 20px;
  width: 600px;
}

.detail-panel {
  flex: 1;
}

.detail-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
  height: 100%;
}

.event-detail {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.detail-item {
  display: flex;
  gap: 12px;
  align-items: baseline;
}

.detail-item .label {
  font-weight: 600;
  color: #2c3e50;
  width: 120px;
  flex-shrink: 0;
}

.detail-item .value {
  color: #7f8c8d;
  flex: 1;
}

.no-selection {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 300px;
  color: #909399;
}

.no-selection .el-icon {
  font-size: 48px;
  margin-bottom: 16px;
}

/* 地图 */
.map-panel {
  flex: 1;
}

.map-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
  height: 100%;
}

.map-view {
  width: 100%;
  height: 400px;
  border-radius: 12px;
}

/* 表格样式 */
:deep(.el-table .high-severity-row) {
  background: #fef6f6;
}

:deep(.el-table .medium-severity-row) {
  background: #fff7eb;
}

:deep(.el-table__body tr:hover > td) {
  background-color: #f5f7fa !important;
}

:deep(.el-table__inner-wrapper) {
  border-radius: 12px;
  overflow: hidden;
}

:deep(.el-table__header) {
  background: #f8f9fa;
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .main-content {
    flex-direction: column;
  }

  .detail-and-map {
    width: 100%;
  }
}

@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
  }

  .control-content {
    flex-direction: column;
    gap: 16px;
  }

  .left-controls,
  .right-controls {
    width: 100%;
    justify-content: center;
  }

  .status-indicators {
    flex-direction: column;
    gap: 12px;
    align-items: flex-start;
  }

  .map-view {
    height: 300px;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-card__body) {
  padding: 20px;
}

:deep(.el-rate__item) {
  font-size: 24px;
}

:deep(.el-rate__text) {
  margin-left: 8px;
  font-size: 14px;
}
</style>
