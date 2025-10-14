<template>
  <div class="flight-conflict-monitor">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">飞行冲突监控</h1>
      <p class="page-subtitle">实时监控和管理飞行冲突事件</p>
    </div>

    <!-- 统计卡片 -->
    <div class="stats-cards">
      <AppleCard class="stat-card stat-card--critical">
        <div class="stat-icon">⚠️</div>
        <div class="stat-content">
          <div class="stat-value">{{ unresolvedCount }}</div>
          <div class="stat-label">未解决冲突</div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card stat-card--warning">
        <div class="stat-icon">🔴</div>
        <div class="stat-content">
          <div class="stat-value">{{ highSeverityCount }}</div>
          <div class="stat-label">高危冲突</div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card stat-card--success">
        <div class="stat-icon">✅</div>
        <div class="stat-content">
          <div class="stat-value">{{ resolvedCount }}</div>
          <div class="stat-label">已解决</div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card stat-card--info">
        <div class="stat-icon">📊</div>
        <div class="stat-content">
          <div class="stat-value">{{ totalConflicts }}</div>
          <div class="stat-label">总冲突数</div>
        </div>
      </AppleCard>
    </div>

    <!-- Tab导航 -->
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

    <!-- 内容区域 -->
    <div class="tab-content">
      <!-- Tab 1: 冲突列表 -->
      <div v-if="activeTab === 'conflicts'" class="tab-pane">
        <!-- 筛选栏 -->
        <div class="filter-bar">
          <div class="filter-group">
            <label class="filter-label">严重程度:</label>
            <select v-model="filters.severity" class="filter-select">
              <option value="">全部</option>
              <option value="low">低</option>
              <option value="medium">中</option>
              <option value="high">高</option>
              <option value="critical">紧急</option>
            </select>
          </div>

          <div class="filter-group">
            <label class="filter-label">状态:</label>
            <select v-model="filters.status" class="filter-select">
              <option value="">全部</option>
              <option value="unresolved">未解决</option>
              <option value="resolving">解决中</option>
              <option value="resolved">已解决</option>
              <option value="ignored">已忽略</option>
            </select>
          </div>

          <AppleButton variant="secondary" @click="handleRefresh">
            🔄 刷新
          </AppleButton>
        </div>

        <!-- 冲突列表 -->
        <div v-if="loading" class="loading-state">
          <div class="spinner" />
          <p>加载中...</p>
        </div>

        <div v-else-if="filteredConflicts.length === 0" class="empty-state">
          <p>暂无冲突数据</p>
        </div>

        <div v-else class="conflicts-list">
          <AppleCard
            v-for="conflict in filteredConflicts"
            :key="conflict.id"
            hoverable
            :class="[
              'conflict-card',
              `conflict-card--${conflict.severity}`
            ]"
          >
            <!-- 冲突头部 -->
            <div class="conflict-header">
              <div class="conflict-title-section">
                <h3 class="conflict-title">
                  {{ conflict.conflict_type_text || '飞行冲突' }}
                </h3>
                <span :class="`severity-badge severity-badge--${conflict.severity}`">
                  {{ conflict.severity_text }}
                </span>
                <span :class="`status-badge status-badge--${conflict.status}`">
                  {{ conflict.status_text }}
                </span>
              </div>
              <div class="conflict-time">
                {{ formatTime(conflict.detected_at) }}
              </div>
            </div>

            <!-- 冲突详情 -->
            <div class="conflict-body">
              <div class="info-section">
                <div class="info-row">
                  <span class="info-label">冲突ID:</span>
                  <span class="info-value">#{{ conflict.id }}</span>
                </div>
                <div v-if="conflict.flight_id_1" class="info-row">
                  <span class="info-label">涉及航班1:</span>
                  <span class="info-value">{{ conflict.flight_id_1 }}</span>
                </div>
                <div v-if="conflict.flight_id_2" class="info-row">
                  <span class="info-label">涉及航班2:</span>
                  <span class="info-value">{{ conflict.flight_id_2 }}</span>
                </div>
                <div v-if="conflict.description" class="info-row">
                  <span class="info-label">描述:</span>
                  <span class="info-value">{{ conflict.description }}</span>
                </div>
                <div v-if="conflict.resolved_at" class="info-row">
                  <span class="info-label">解决时间:</span>
                  <span class="info-value">{{ formatTime(conflict.resolved_at) }}</span>
                </div>
              </div>
            </div>

            <!-- 操作按钮 -->
            <div
              v-if="conflict.status === 'unresolved' && userRole === 'admin'"
              class="conflict-actions"
            >
              <AppleButton
                variant="primary"
                size="small"
                @click="handleResolve(conflict.id)"
              >
                标记为已解决
              </AppleButton>
              <AppleButton
                variant="secondary"
                size="small"
                @click="handleViewDetails(conflict)"
              >
                查看详情
              </AppleButton>
            </div>
          </AppleCard>
        </div>
      </div>

      <!-- Tab 2: 冲突检测 -->
      <div v-if="activeTab === 'detection'" class="tab-pane">
        <AppleCard class="detection-card">
          <h2 class="section-title">飞行冲突检测</h2>
          <p class="section-description">
            输入飞行任务信息,系统将检测是否存在潜在冲突
          </p>

          <form class="detection-form" @submit.prevent="handleDetectConflict">
            <AppleInput
              v-model.number="detectionData.task_id"
              type="number"
              label="飞行任务ID"
              placeholder="输入任务ID"
              required
              class="form-field"
            />

            <div class="form-row">
              <AppleInput
                v-model="detectionData.start_time"
                type="datetime-local"
                label="开始时间"
                required
                class="form-field"
              />
              <AppleInput
                v-model="detectionData.end_time"
                type="datetime-local"
                label="结束时间"
                required
                class="form-field"
              />
            </div>

            <div class="form-section">
              <h3 class="form-section-title">飞行路线(可选)</h3>
              <div class="form-row">
                <AppleInput
                  v-model.number="detectionData.route.start.lat"
                  type="number"
                  step="0.000001"
                  label="起点纬度"
                  placeholder="例: 41.796700"
                  class="form-field"
                />
                <AppleInput
                  v-model.number="detectionData.route.start.lng"
                  type="number"
                  step="0.000001"
                  label="起点经度"
                  placeholder="例: 123.451200"
                  class="form-field"
                />
              </div>

              <div class="form-row">
                <AppleInput
                  v-model.number="detectionData.route.end.lat"
                  type="number"
                  step="0.000001"
                  label="终点纬度"
                  placeholder="例: 41.806700"
                  class="form-field"
                />
                <AppleInput
                  v-model.number="detectionData.route.end.lng"
                  type="number"
                  step="0.000001"
                  label="终点经度"
                  placeholder="例: 123.461200"
                  class="form-field"
                />
              </div>
            </div>

            <AppleInput
              v-model.number="detectionData.altitude"
              type="number"
              label="飞行高度(米)"
              placeholder="例: 120"
              class="form-field"
            />

            <AppleButton
              type="submit"
              variant="primary"
              size="large"
              :loading="detecting"
              class="submit-button"
            >
              {{ detecting ? '检测中...' : '开始检测' }}
            </AppleButton>
          </form>

          <!-- 检测结果 -->
          <div v-if="detectionResult" class="detection-result">
            <div
              :class="[
                'result-header',
                detectionResult.has_conflicts ? 'result-header--danger' : 'result-header--success'
              ]"
            >
              <span class="result-icon">
                {{ detectionResult.has_conflicts ? '⚠️' : '✅' }}
              </span>
              <h3 class="result-title">
                {{ detectionResult.has_conflicts ? '发现冲突' : '未发现冲突' }}
              </h3>
            </div>

            <div v-if="detectionResult.conflicts && detectionResult.conflicts.length > 0" class="result-conflicts">
              <p class="result-subtitle">检测到 {{ detectionResult.conflicts.length }} 个潜在冲突:</p>
              <div class="result-list">
                <div
                  v-for="(conflict, index) in detectionResult.conflicts"
                  :key="index"
                  class="result-item"
                >
                  <div class="result-item-header">
                    <span :class="`severity-badge severity-badge--${conflict.severity}`">
                      {{ conflict.severity_text }}
                    </span>
                    <span class="result-item-type">{{ conflict.conflict_type_text }}</span>
                  </div>
                  <p class="result-item-description">{{ conflict.description }}</p>
                </div>
              </div>
            </div>
          </div>
        </AppleCard>
      </div>

      <!-- Tab 3: 统计分析 -->
      <div v-if="activeTab === 'analytics'" class="tab-pane">
        <AppleCard class="analytics-card">
          <h2 class="section-title">冲突统计分析</h2>
          <p class="section-description">系统冲突数据汇总与趋势分析</p>

          <div class="analytics-content">
            <!-- 按严重程度统计 -->
            <div class="analytics-section">
              <h3 class="analytics-subtitle">按严重程度分布</h3>
              <div class="analytics-bars">
                <div class="analytics-bar">
                  <div class="bar-label">低</div>
                  <div class="bar-track">
                    <div
                      class="bar-fill bar-fill--low"
                      :style="{ width: getPercentage(severityStats.low) + '%' }"
                    />
                  </div>
                  <div class="bar-value">{{ severityStats.low }}</div>
                </div>

                <div class="analytics-bar">
                  <div class="bar-label">中</div>
                  <div class="bar-track">
                    <div
                      class="bar-fill bar-fill--medium"
                      :style="{ width: getPercentage(severityStats.medium) + '%' }"
                    />
                  </div>
                  <div class="bar-value">{{ severityStats.medium }}</div>
                </div>

                <div class="analytics-bar">
                  <div class="bar-label">高</div>
                  <div class="bar-track">
                    <div
                      class="bar-fill bar-fill--high"
                      :style="{ width: getPercentage(severityStats.high) + '%' }"
                    />
                  </div>
                  <div class="bar-value">{{ severityStats.high }}</div>
                </div>

                <div class="analytics-bar">
                  <div class="bar-label">紧急</div>
                  <div class="bar-track">
                    <div
                      class="bar-fill bar-fill--critical"
                      :style="{ width: getPercentage(severityStats.critical) + '%' }"
                    />
                  </div>
                  <div class="bar-value">{{ severityStats.critical }}</div>
                </div>
              </div>
            </div>

            <!-- 按状态统计 -->
            <div class="analytics-section">
              <h3 class="analytics-subtitle">按状态分布</h3>
              <div class="status-stats">
                <div class="status-stat-item">
                  <div class="status-stat-value">{{ statusStats.unresolved }}</div>
                  <div class="status-stat-label">未解决</div>
                </div>
                <div class="status-stat-item">
                  <div class="status-stat-value">{{ statusStats.resolving }}</div>
                  <div class="status-stat-label">解决中</div>
                </div>
                <div class="status-stat-item">
                  <div class="status-stat-value">{{ statusStats.resolved }}</div>
                  <div class="status-stat-label">已解决</div>
                </div>
                <div class="status-stat-item">
                  <div class="status-stat-value">{{ statusStats.ignored }}</div>
                  <div class="status-stat-label">已忽略</div>
                </div>
              </div>
            </div>
          </div>
        </AppleCard>
      </div>
    </div>

    <!-- 冲突详情模态框 -->
    <AppleModal
      v-model="showDetailModal"
      title="冲突详情"
      :show-close="true"
    >
      <div v-if="selectedConflict" class="conflict-detail">
        <div class="detail-row">
          <span class="detail-label">冲突ID:</span>
          <span class="detail-value">#{{ selectedConflict.id }}</span>
        </div>
        <div class="detail-row">
          <span class="detail-label">严重程度:</span>
          <span :class="`severity-badge severity-badge--${selectedConflict.severity}`">
            {{ selectedConflict.severity_text }}
          </span>
        </div>
        <div class="detail-row">
          <span class="detail-label">状态:</span>
          <span :class="`status-badge status-badge--${selectedConflict.status}`">
            {{ selectedConflict.status_text }}
          </span>
        </div>
        <div class="detail-row">
          <span class="detail-label">检测时间:</span>
          <span class="detail-value">{{ formatTime(selectedConflict.detected_at) }}</span>
        </div>
        <div v-if="selectedConflict.description" class="detail-row">
          <span class="detail-label">描述:</span>
          <span class="detail-value">{{ selectedConflict.description }}</span>
        </div>
      </div>
    </AppleModal>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { useStore } from 'vuex'
import conflictDetectionApi from '@/services/conflictDetectionApi'
import { AppleButton, AppleCard, AppleInput, AppleModal } from '@/components/apple'

const store = useStore()

// 用户角色
const userRole = computed(() => store.state.user?.role || 'user')

// Tab配置
const tabs = computed(() => [
  { label: '冲突列表', value: 'conflicts', count: conflicts.value.length },
  { label: '冲突检测', value: 'detection' },
  { label: '统计分析', value: 'analytics' }
])

const activeTab = ref('conflicts')

// 数据状态
const loading = ref(false)
const conflicts = ref([])

// 筛选器
const filters = reactive({
  severity: '',
  status: ''
})

// 计算属性: 过滤后的冲突
const filteredConflicts = computed(() => {
  let filtered = conflicts.value

  if (filters.severity) {
    filtered = filtered.filter(c => c.severity === filters.severity)
  }

  if (filters.status) {
    filtered = filtered.filter(c => c.status === filters.status)
  }

  return filtered
})

// 统计数据
const unresolvedCount = computed(() =>
  conflicts.value.filter(c => c.status === 'unresolved').length
)

const highSeverityCount = computed(() =>
  conflicts.value.filter(c => c.severity === 'high' || c.severity === 'critical').length
)

const resolvedCount = computed(() =>
  conflicts.value.filter(c => c.status === 'resolved').length
)

const totalConflicts = computed(() => conflicts.value.length)

// 严重程度统计
const severityStats = computed(() => ({
  low: conflicts.value.filter(c => c.severity === 'low').length,
  medium: conflicts.value.filter(c => c.severity === 'medium').length,
  high: conflicts.value.filter(c => c.severity === 'high').length,
  critical: conflicts.value.filter(c => c.severity === 'critical').length
}))

// 状态统计
const statusStats = computed(() => ({
  unresolved: conflicts.value.filter(c => c.status === 'unresolved').length,
  resolving: conflicts.value.filter(c => c.status === 'resolving').length,
  resolved: conflicts.value.filter(c => c.status === 'resolved').length,
  ignored: conflicts.value.filter(c => c.status === 'ignored').length
}))

// 冲突检测表单
const detecting = ref(false)
const detectionData = reactive({
  task_id: null,
  start_time: '',
  end_time: '',
  route: {
    start: { lat: null, lng: null },
    end: { lat: null, lng: null }
  },
  altitude: null
})
const detectionResult = ref(null)

// 冲突详情模态框
const showDetailModal = ref(false)
const selectedConflict = ref(null)

// 加载冲突列表
const loadConflicts = async () => {
  try {
    loading.value = true
    const response = await conflictDetectionApi.getConflicts()

    if (response.success) {
      // 格式化冲突数据
      conflicts.value = (response.data || []).map(conflict =>
        conflictDetectionApi.formatConflictForDisplay(conflict)
      )
      console.log('成功加载冲突列表:', conflicts.value.length)
    } else {
      ElMessage.warning(response.message || '加载冲突失败')
    }
  } catch (error) {
    console.error('加载冲突失败:', error)
    ElMessage.error(error.message || '加载冲突失败')
  } finally {
    loading.value = false
  }
}

// 刷新列表
const handleRefresh = () => {
  loadConflicts()
  ElMessage.success('已刷新')
}

// 解决冲突
const handleResolve = async (conflictId) => {
  try {
    await ElMessageBox.confirm(
      '确认将此冲突标记为已解决?',
      '确认操作',
      {
        confirmButtonText: '确认',
        cancelButtonText: '取消',
        type: 'info'
      }
    )

    const response = await conflictDetectionApi.resolveConflict(conflictId, {
      resolution_note: '手动标记为已解决'
    })

    if (response.success) {
      ElMessage.success('已标记为已解决')
      await loadConflicts()
    } else {
      ElMessage.error(response.message || '操作失败')
    }
  } catch (error) {
    if (error !== 'cancel') {
      console.error('解决冲突失败:', error)
      ElMessage.error(error.message || '操作失败')
    }
  }
}

// 查看详情
const handleViewDetails = (conflict) => {
  selectedConflict.value = conflict
  showDetailModal.value = true
}

// 检测冲突
const handleDetectConflict = async () => {
  try {
    detecting.value = true
    detectionResult.value = null

    // 准备数据
    const flightData = {
      task_id: detectionData.task_id,
      start_time: detectionData.start_time,
      end_time: detectionData.end_time
    }

    // 添加路线(如果填写了)
    if (detectionData.route.start.lat && detectionData.route.start.lng &&
        detectionData.route.end.lat && detectionData.route.end.lng) {
      flightData.route = detectionData.route
    }

    // 添加高度(如果填写了)
    if (detectionData.altitude) {
      flightData.altitude = detectionData.altitude
    }

    const response = await conflictDetectionApi.detectFlightConflicts(flightData)

    if (response.success) {
      detectionResult.value = response.data

      // 格式化冲突数据
      if (detectionResult.value.conflicts) {
        detectionResult.value.conflicts = detectionResult.value.conflicts.map(conflict =>
          conflictDetectionApi.formatConflictForDisplay(conflict)
        )
      }

      if (detectionResult.value.has_conflicts) {
        ElMessage.warning(`检测到 ${detectionResult.value.conflicts.length} 个冲突`)
      } else {
        ElMessage.success('未检测到冲突')
      }

      // 重新加载冲突列表
      await loadConflicts()
    } else {
      ElMessage.error(response.message || '检测失败')
    }
  } catch (error) {
    console.error('检测冲突失败:', error)
    ElMessage.error(error.message || '检测失败')
  } finally {
    detecting.value = false
  }
}

// 格式化时间
const formatTime = (time) => {
  if (!time) return 'N/A'
  return new Date(time).toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit'
  })
}

// 计算百分比
const getPercentage = (value) => {
  if (totalConflicts.value === 0) return 0
  return Math.round((value / totalConflicts.value) * 100)
}

// 组件挂载时加载数据
onMounted(() => {
  loadConflicts()
})
</script>

<style scoped>
.flight-conflict-monitor {
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
  line-height: 1.2;
}

.stat-label {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  margin-top: var(--space-1);
}

/* Tab导航 */
.tabs {
  display: flex;
  gap: var(--space-2);
  margin-bottom: var(--space-6);
  border-bottom: 1px solid var(--color-border-default);
}

.tab-button {
  position: relative;
  padding: var(--space-3) var(--space-4);
  background: none;
  border: none;
  font-size: var(--font-size-base);
  font-weight: 500;
  color: var(--color-text-secondary);
  cursor: pointer;
  transition: var(--transition-button);
  display: flex;
  align-items: center;
  gap: var(--space-2);
}

.tab-button:hover {
  color: var(--color-text-primary);
}

.tab-button--active {
  color: var(--apple-blue);
  font-weight: 600;
}

.tab-button--active::after {
  content: '';
  position: absolute;
  bottom: -1px;
  left: 0;
  right: 0;
  height: 2px;
  background: var(--apple-blue);
}

.tab-badge {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  min-width: 20px;
  height: 20px;
  padding: 0 var(--space-2);
  background: var(--color-bg-tertiary);
  border-radius: var(--radius-full);
  font-size: var(--font-size-xs);
  font-weight: 600;
}

/* 筛选栏 */
.filter-bar {
  display: flex;
  gap: var(--space-4);
  margin-bottom: var(--space-6);
  align-items: center;
}

.filter-group {
  display: flex;
  align-items: center;
  gap: var(--space-2);
}

.filter-label {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  white-space: nowrap;
}

.filter-select {
  height: 36px;
  padding: 0 var(--space-3);
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: var(--transition-input);
}

.filter-select:hover {
  border-color: var(--color-border-hover);
}

/* 加载状态 */
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

/* 冲突列表 */
.conflicts-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-4);
}

.conflict-card {
  border-left: 4px solid transparent;
  transition: var(--transition-card);
}

.conflict-card--low {
  border-left-color: var(--apple-gray-400);
}

.conflict-card--medium {
  border-left-color: var(--apple-orange);
}

.conflict-card--high {
  border-left-color: var(--apple-red);
}

.conflict-card--critical {
  border-left-color: #d10000;
}

.conflict-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: var(--space-4);
  gap: var(--space-4);
}

.conflict-title-section {
  display: flex;
  align-items: center;
  gap: var(--space-3);
  flex-wrap: wrap;
}

.conflict-title {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0;
}

.conflict-time {
  font-size: var(--font-size-sm);
  color: var(--color-text-tertiary);
  white-space: nowrap;
}

/* 徽章样式 */
.severity-badge,
.status-badge {
  display: inline-flex;
  align-items: center;
  padding: var(--space-1) var(--space-3);
  border-radius: var(--radius-full);
  font-size: var(--font-size-xs);
  font-weight: 600;
  white-space: nowrap;
}

.severity-badge--low {
  background: rgba(134, 134, 139, 0.1);
  color: var(--apple-gray-500);
}

.severity-badge--medium {
  background: rgba(255, 149, 0, 0.1);
  color: var(--apple-orange);
}

.severity-badge--high {
  background: rgba(255, 59, 48, 0.1);
  color: var(--apple-red);
}

.severity-badge--critical {
  background: rgba(209, 0, 0, 0.1);
  color: #d10000;
}

.status-badge--unresolved {
  background: rgba(255, 149, 0, 0.1);
  color: var(--apple-orange);
}

.status-badge--resolving {
  background: rgba(0, 113, 227, 0.1);
  color: var(--apple-blue);
}

.status-badge--resolved {
  background: rgba(48, 209, 88, 0.1);
  color: var(--apple-green);
}

.status-badge--ignored {
  background: rgba(134, 134, 139, 0.1);
  color: var(--apple-gray-500);
}

.conflict-body {
  margin-bottom: var(--space-4);
}

.info-section {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.info-row {
  display: flex;
  gap: var(--space-2);
  font-size: var(--font-size-sm);
}

.info-label {
  color: var(--color-text-secondary);
  min-width: 100px;
}

.info-value {
  color: var(--color-text-primary);
  flex: 1;
}

.conflict-actions {
  display: flex;
  gap: var(--space-2);
  padding-top: var(--space-4);
  border-top: 1px solid var(--color-border-default);
}

/* 检测表单 */
.detection-card {
  max-width: 800px;
  margin: 0 auto;
}

.section-title {
  font-size: var(--font-size-2xl);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-2) 0;
}

.section-description {
  font-size: var(--font-size-base);
  color: var(--color-text-secondary);
  margin: 0 0 var(--space-6) 0;
}

.detection-form {
  display: flex;
  flex-direction: column;
  gap: var(--space-4);
}

.form-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-4);
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

.submit-button {
  width: 100%;
}

/* 检测结果 */
.detection-result {
  margin-top: var(--space-8);
  padding-top: var(--space-6);
  border-top: 1px solid var(--color-border-default);
}

.result-header {
  display: flex;
  align-items: center;
  gap: var(--space-3);
  padding: var(--space-4);
  border-radius: var(--radius-md);
  margin-bottom: var(--space-4);
}

.result-header--success {
  background: rgba(48, 209, 88, 0.1);
}

.result-header--danger {
  background: rgba(255, 59, 48, 0.1);
}

.result-icon {
  font-size: 32px;
}

.result-title {
  font-size: var(--font-size-xl);
  font-weight: 600;
  margin: 0;
}

.result-subtitle {
  font-size: var(--font-size-base);
  color: var(--color-text-secondary);
  margin: 0 0 var(--space-4) 0;
}

.result-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3);
}

.result-item {
  padding: var(--space-4);
  background: var(--color-bg-secondary);
  border-radius: var(--radius-md);
}

.result-item-header {
  display: flex;
  align-items: center;
  gap: var(--space-3);
  margin-bottom: var(--space-2);
}

.result-item-type {
  font-weight: 600;
  color: var(--color-text-primary);
}

.result-item-description {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  margin: 0;
}

/* 统计分析 */
.analytics-content {
  display: flex;
  flex-direction: column;
  gap: var(--space-8);
}

.analytics-section {
  padding: var(--space-6);
  background: var(--color-bg-secondary);
  border-radius: var(--radius-md);
}

.analytics-subtitle {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-4) 0;
}

.analytics-bars {
  display: flex;
  flex-direction: column;
  gap: var(--space-4);
}

.analytics-bar {
  display: grid;
  grid-template-columns: 60px 1fr 60px;
  gap: var(--space-3);
  align-items: center;
}

.bar-label {
  font-size: var(--font-size-sm);
  font-weight: 500;
  color: var(--color-text-secondary);
}

.bar-track {
  height: 24px;
  background: var(--color-bg-tertiary);
  border-radius: var(--radius-full);
  overflow: hidden;
}

.bar-fill {
  height: 100%;
  border-radius: var(--radius-full);
  transition: width 0.6s var(--easing-default);
}

.bar-fill--low {
  background: var(--apple-gray-400);
}

.bar-fill--medium {
  background: var(--apple-orange);
}

.bar-fill--high {
  background: var(--apple-red);
}

.bar-fill--critical {
  background: #d10000;
}

.bar-value {
  font-size: var(--font-size-base);
  font-weight: 600;
  color: var(--color-text-primary);
  text-align: right;
}

.status-stats {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
  gap: var(--space-4);
}

.status-stat-item {
  text-align: center;
  padding: var(--space-4);
  background: var(--color-bg-primary);
  border-radius: var(--radius-md);
}

.status-stat-value {
  font-size: var(--font-size-3xl);
  font-weight: 700;
  color: var(--apple-blue);
  margin-bottom: var(--space-2);
}

.status-stat-label {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
}

/* 冲突详情模态框 */
.conflict-detail {
  display: flex;
  flex-direction: column;
  gap: var(--space-4);
}

.detail-row {
  display: flex;
  align-items: center;
  gap: var(--space-3);
  padding: var(--space-3);
  border-radius: var(--radius-sm);
  background: var(--color-bg-secondary);
}

.detail-label {
  font-weight: 600;
  color: var(--color-text-secondary);
  min-width: 100px;
}

.detail-value {
  color: var(--color-text-primary);
  flex: 1;
}

/* 响应式 */
@media (max-width: 768px) {
  .flight-conflict-monitor {
    padding: var(--space-4);
  }

  .stats-cards {
    grid-template-columns: repeat(2, 1fr);
  }

  .filter-bar {
    flex-direction: column;
    align-items: stretch;
  }

  .form-row {
    grid-template-columns: 1fr;
  }

  .analytics-bar {
    grid-template-columns: 50px 1fr 50px;
  }

  .status-stats {
    grid-template-columns: repeat(2, 1fr);
  }
}
</style>
