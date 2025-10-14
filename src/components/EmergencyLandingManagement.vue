<template>
  <div class="emergency-landing-management">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">应急降落点管理</h1>
      <p class="page-subtitle">管理和查看所有应急降落点位置</p>
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
      <!-- Tab 1: 降落点列表 -->
      <div v-if="activeTab === 'list'" class="tab-pane">
        <!-- 搜索和筛选栏 -->
        <div class="search-bar">
          <AppleInput
            v-model="searchQuery"
            type="text"
            placeholder="搜索降落点名称..."
            class="search-input"
            @input="handleSearch"
          />
          <div class="filter-group">
            <select v-model="statusFilter" class="status-filter">
              <option value="">所有状态</option>
              <option value="active">活跃</option>
              <option value="inactive">未启用</option>
              <option value="maintenance">维护中</option>
            </select>
            <AppleButton
              v-if="userRole === 'admin'"
              variant="primary"
              @click="showAddModal = true"
            >
              + 添加降落点
            </AppleButton>
          </div>
        </div>

        <!-- 降落点卡片网格 -->
        <div v-if="loading" class="loading-state">
          <div class="spinner" />
          <p>加载中...</p>
        </div>

        <div v-else-if="filteredLandingPoints.length === 0" class="empty-state">
          <p>暂无降落点数据</p>
        </div>

        <div v-else class="landing-points-grid">
          <AppleCard
            v-for="point in filteredLandingPoints"
            :key="point.id"
            hoverable
            class="landing-point-card"
          >
            <div class="card-header">
              <h3 class="point-name">{{ point.name || point.point_name }}</h3>
              <span :class="`status-badge status-badge--${point.status}`">
                {{ getStatusText(point.status) }}
              </span>
            </div>

            <div class="card-body">
              <div class="info-row">
                <span class="info-label">编码:</span>
                <span class="info-value">{{ point.point_code }}</span>
              </div>
              <div class="info-row">
                <span class="info-label">位置:</span>
                <span class="info-value">
                  {{ point.latitude?.toFixed(6) }}, {{ point.longitude?.toFixed(6) }}
                </span>
              </div>
              <div class="info-row">
                <span class="info-label">容量:</span>
                <span class="info-value">{{ point.capacity || 'N/A' }}</span>
              </div>
              <div v-if="point.facilities" class="info-row">
                <span class="info-label">设施:</span>
                <span class="info-value">{{ point.facilities }}</span>
              </div>
            </div>

            <div v-if="userRole === 'admin'" class="card-actions">
              <AppleButton
                variant="secondary"
                size="small"
                @click="handleEdit(point)"
              >
                编辑
              </AppleButton>
              <AppleButton
                variant="danger"
                size="small"
                @click="handleDelete(point.id)"
              >
                删除
              </AppleButton>
            </div>
          </AppleCard>
        </div>
      </div>

      <!-- Tab 2: 查找最近降落点 -->
      <div v-if="activeTab === 'nearest'" class="tab-pane">
        <AppleCard class="nearest-search-card">
          <h2 class="section-title">查找最近的应急降落点</h2>
          <p class="section-description">
            输入当前位置坐标,系统将为您找到最近的应急降落点
          </p>

          <form class="nearest-form" @submit.prevent="handleFindNearest">
            <div class="form-row">
              <AppleInput
                v-model="nearestSearch.latitude"
                type="number"
                step="0.000001"
                label="纬度"
                placeholder="例: 41.796700"
                required
                class="form-field"
              />
              <AppleInput
                v-model="nearestSearch.longitude"
                type="number"
                step="0.000001"
                label="经度"
                placeholder="例: 123.451200"
                required
                class="form-field"
              />
            </div>

            <div class="form-row">
              <AppleInput
                v-model.number="nearestSearch.limit"
                type="number"
                label="返回数量"
                placeholder="默认: 3"
                min="1"
                max="10"
                class="form-field"
              />
              <AppleInput
                v-model.number="nearestSearch.radius"
                type="number"
                label="搜索半径 (米)"
                placeholder="默认: 5000"
                min="100"
                max="50000"
                class="form-field"
              />
            </div>

            <AppleButton
              type="submit"
              variant="primary"
              size="large"
              :loading="searchingNearest"
              class="submit-button"
            >
              {{ searchingNearest ? '搜索中...' : '查找最近降落点' }}
            </AppleButton>
          </form>

          <!-- 搜索结果 -->
          <div v-if="nearestResults.length > 0" class="nearest-results">
            <h3 class="results-title">找到 {{ nearestResults.length }} 个降落点</h3>
            <div class="results-list">
              <div
                v-for="(result, index) in nearestResults"
                :key="result.id"
                class="result-item"
              >
                <div class="result-rank">{{ index + 1 }}</div>
                <div class="result-content">
                  <h4 class="result-name">{{ result.name || result.point_name }}</h4>
                  <p class="result-distance">
                    距离: {{ result.distance?.toFixed(2) }} 公里
                  </p>
                  <p class="result-location">
                    位置: {{ result.latitude?.toFixed(6) }}, {{ result.longitude?.toFixed(6) }}
                  </p>
                  <p v-if="result.facilities" class="result-facilities">
                    设施: {{ result.facilities }}
                  </p>
                </div>
                <span :class="`status-badge status-badge--${result.status}`">
                  {{ getStatusText(result.status) }}
                </span>
              </div>
            </div>
          </div>
        </AppleCard>
      </div>

      <!-- Tab 3: 地图视图 (占位) -->
      <div v-if="activeTab === 'map'" class="tab-pane">
        <AppleCard class="map-placeholder">
          <h2 class="section-title">地图视图</h2>
          <p class="section-description">地图集成功能开发中...</p>
          <div class="placeholder-content">
            <svg class="placeholder-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor">
              <path
                stroke-linecap="round"
                stroke-linejoin="round"
                stroke-width="2"
                d="M9 20l-5.447-2.724A1 1 0 013 16.382V5.618a1 1 0 011.447-.894L9 7m0 13l6-3m-6 3V7m6 10l4.553 2.276A1 1 0 0021 18.382V7.618a1 1 0 00-.553-.894L15 4m0 13V4m0 0L9 7"
              />
            </svg>
            <p>未来将支持在地图上可视化所有降落点位置</p>
          </div>
        </AppleCard>
      </div>
    </div>

    <!-- 添加/编辑降落点模态框 -->
    <AppleModal
      v-model="showAddModal"
      :title="editingPoint ? '编辑降落点' : '添加降落点'"
      :show-close="true"
    >
      <form class="landing-point-form" @submit.prevent="handleSubmit">
        <AppleInput
          v-model="formData.name"
          label="降落点名称"
          placeholder="输入降落点名称"
          required
          class="form-field"
        />

        <AppleInput
          v-model="formData.point_code"
          label="降落点编码"
          placeholder="例: ELP-SY-001"
          required
          :disabled="!!editingPoint"
          class="form-field"
        />

        <div class="form-row">
          <AppleInput
            v-model.number="formData.latitude"
            type="number"
            step="0.000001"
            label="纬度"
            placeholder="例: 41.796700"
            required
            class="form-field"
          />
          <AppleInput
            v-model.number="formData.longitude"
            type="number"
            step="0.000001"
            label="经度"
            placeholder="例: 123.451200"
            required
            class="form-field"
          />
        </div>

        <AppleInput
          v-model.number="formData.capacity"
          type="number"
          label="容量"
          placeholder="可容纳无人机数量"
          class="form-field"
        />

        <div class="form-field">
          <label class="form-label">状态</label>
          <select v-model="formData.status" class="form-select">
            <option value="active">活跃</option>
            <option value="inactive">未启用</option>
            <option value="maintenance">维护中</option>
          </select>
        </div>

        <AppleInput
          v-model="formData.facilities"
          label="设施描述"
          placeholder="描述可用设施..."
          class="form-field"
        />

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
            {{ submitting ? '提交中...' : (editingPoint ? '更新' : '创建') }}
          </AppleButton>
        </div>
      </form>
    </AppleModal>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { useStore } from 'vuex'
import emergencyLandingApi from '@/services/emergencyLandingApi'
import { AppleButton, AppleCard, AppleInput, AppleModal } from '@/components/apple'

const store = useStore()

// 用户角色
const userRole = computed(() => store.state.user?.role || 'user')

// Tab配置
const tabs = computed(() => [
  { label: '降落点列表', value: 'list', count: landingPoints.value.length },
  { label: '查找最近', value: 'nearest' },
  { label: '地图视图', value: 'map' }
])

const activeTab = ref('list')

// 数据状态
const loading = ref(false)
const landingPoints = ref([])
const searchQuery = ref('')
const statusFilter = ref('')

// 添加/编辑模态框
const showAddModal = ref(false)
const editingPoint = ref(null)
const submitting = ref(false)

const formData = reactive({
  name: '',
  point_code: '',
  latitude: null,
  longitude: null,
  capacity: null,
  status: 'active',
  facilities: ''
})

// 最近降落点搜索
const searchingNearest = ref(false)
const nearestSearch = reactive({
  latitude: null,
  longitude: null,
  limit: 3,
  radius: 5000
})
const nearestResults = ref([])

// 计算属性: 过滤后的降落点
const filteredLandingPoints = computed(() => {
  let filtered = landingPoints.value

  // 搜索过滤
  if (searchQuery.value) {
    const query = searchQuery.value.toLowerCase()
    filtered = filtered.filter(point =>
      (point.name?.toLowerCase().includes(query)) ||
      (point.point_name?.toLowerCase().includes(query)) ||
      (point.point_code?.toLowerCase().includes(query))
    )
  }

  // 状态过滤
  if (statusFilter.value) {
    filtered = filtered.filter(point => point.status === statusFilter.value)
  }

  return filtered
})

// 获取状态文本
const getStatusText = (status) => {
  const statusMap = {
    active: '活跃',
    inactive: '未启用',
    maintenance: '维护中'
  }
  return statusMap[status] || status
}

// 加载降落点列表
const loadLandingPoints = async () => {
  try {
    loading.value = true
    const response = await emergencyLandingApi.getLandingPoints()

    if (response.success) {
      landingPoints.value = response.data || []
      console.log('成功加载降落点列表:', landingPoints.value.length)
    } else {
      ElMessage.warning(response.message || '加载降落点失败')
    }
  } catch (error) {
    console.error('加载降落点失败:', error)
    ElMessage.error(error.message || '加载降落点失败')
  } finally {
    loading.value = false
  }
}

// 搜索处理
const handleSearch = () => {
  // 搜索由计算属性自动处理
}

// 编辑降落点
const handleEdit = (point) => {
  editingPoint.value = point
  formData.name = point.name || point.point_name
  formData.point_code = point.point_code
  formData.latitude = point.latitude
  formData.longitude = point.longitude
  formData.capacity = point.capacity
  formData.status = point.status
  formData.facilities = point.facilities || ''
  showAddModal.value = true
}

// 取消编辑
const handleCancelEdit = () => {
  showAddModal.value = false
  editingPoint.value = null
  resetForm()
}

// 重置表单
const resetForm = () => {
  formData.name = ''
  formData.point_code = ''
  formData.latitude = null
  formData.longitude = null
  formData.capacity = null
  formData.status = 'active'
  formData.facilities = ''
}

// 提交表单
const handleSubmit = async () => {
  try {
    submitting.value = true

    // 准备数据
    const submitData = {
      name: formData.name,
      point_code: formData.point_code,
      latitude: parseFloat(formData.latitude),
      longitude: parseFloat(formData.longitude),
      capacity: formData.capacity ? parseInt(formData.capacity) : null,
      status: formData.status,
      facilities: formData.facilities
    }

    let response
    if (editingPoint.value) {
      // 更新
      response = await emergencyLandingApi.updateLandingPoint(editingPoint.value.id, submitData)
    } else {
      // 创建
      response = await emergencyLandingApi.createLandingPoint(submitData)
    }

    if (response.success) {
      ElMessage.success(editingPoint.value ? '更新成功' : '创建成功')
      showAddModal.value = false
      editingPoint.value = null
      resetForm()
      await loadLandingPoints()
    } else {
      ElMessage.error(response.message || '操作失败')
    }
  } catch (error) {
    console.error('提交失败:', error)
    ElMessage.error(error.message || '提交失败')
  } finally {
    submitting.value = false
  }
}

// 删除降落点
const handleDelete = async (id) => {
  try {
    await ElMessageBox.confirm(
      '确定要删除此降落点吗?此操作无法撤销。',
      '确认删除',
      {
        confirmButtonText: '删除',
        cancelButtonText: '取消',
        type: 'warning'
      }
    )

    const response = await emergencyLandingApi.deleteLandingPoint(id)

    if (response.success) {
      ElMessage.success('删除成功')
      await loadLandingPoints()
    } else {
      ElMessage.error(response.message || '删除失败')
    }
  } catch (error) {
    if (error !== 'cancel') {
      console.error('删除失败:', error)
      ElMessage.error(error.message || '删除失败')
    }
  }
}

// 查找最近的降落点
const handleFindNearest = async () => {
  try {
    // 验证输入
    if (!nearestSearch.latitude || !nearestSearch.longitude) {
      ElMessage.warning('请输入纬度和经度')
      return
    }

    searchingNearest.value = true

    const response = await emergencyLandingApi.findNearestLandingPoints(
      parseFloat(nearestSearch.latitude),
      parseFloat(nearestSearch.longitude),
      nearestSearch.limit || 3,
      nearestSearch.radius || 5000
    )

    if (response.success) {
      nearestResults.value = response.data || []
      if (nearestResults.value.length === 0) {
        ElMessage.info('在指定范围内未找到降落点')
      } else {
        ElMessage.success(`找到 ${nearestResults.value.length} 个降落点`)
      }
    } else {
      ElMessage.warning(response.message || '搜索失败')
    }
  } catch (error) {
    console.error('搜索最近降落点失败:', error)
    ElMessage.error(error.message || '搜索失败')
  } finally {
    searchingNearest.value = false
  }
}

// 组件挂载时加载数据
onMounted(() => {
  loadLandingPoints()
})
</script>

<style scoped>
.emergency-landing-management {
  padding: var(--space-6);
  max-width: 1400px;
  margin: 0 auto;
}

/* 页面头部 */
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

/* Tab导航 */
.tabs {
  display: flex;
  gap: var(--space-2);
  margin-bottom: var(--space-6);
  border-bottom: 1px solid var(--color-border-default);
  padding-bottom: 0;
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
  border-radius: var(--radius-xs) var(--radius-xs) 0 0;
  display: flex;
  align-items: center;
  gap: var(--space-2);
}

.tab-button:hover {
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
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
  color: var(--color-text-secondary);
}

.tab-button--active .tab-badge {
  background: rgba(0, 113, 227, 0.1);
  color: var(--apple-blue);
}

/* 搜索栏 */
.search-bar {
  display: flex;
  gap: var(--space-4);
  margin-bottom: var(--space-6);
  align-items: center;
}

.search-input {
  flex: 1;
  max-width: 400px;
}

.filter-group {
  display: flex;
  gap: var(--space-3);
  align-items: center;
}

.status-filter {
  height: 44px;
  padding: 0 var(--space-4);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-base);
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  outline: none;
  transition: var(--transition-input);
  cursor: pointer;
}

.status-filter:hover {
  border-color: var(--color-border-hover);
}

.status-filter:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
}

/* 加载和空状态 */
.loading-state,
.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: var(--space-12) var(--space-4);
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
  to {
    transform: rotate(360deg);
  }
}

/* 降落点卡片网格 */
.landing-points-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
  gap: var(--space-4);
}

.landing-point-card {
  display: flex;
  flex-direction: column;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: var(--space-4);
  gap: var(--space-3);
}

.point-name {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0;
  flex: 1;
}

/* 状态徽章 */
.status-badge {
  display: inline-flex;
  align-items: center;
  padding: var(--space-1) var(--space-3);
  border-radius: var(--radius-full);
  font-size: var(--font-size-xs);
  font-weight: 600;
  white-space: nowrap;
}

.status-badge--active {
  background: rgba(48, 209, 88, 0.1);
  color: var(--apple-green);
}

.status-badge--inactive {
  background: rgba(134, 134, 139, 0.1);
  color: var(--apple-gray-500);
}

.status-badge--maintenance {
  background: rgba(255, 149, 0, 0.1);
  color: var(--apple-orange);
}

.card-body {
  flex: 1;
  margin-bottom: var(--space-4);
}

.info-row {
  display: flex;
  gap: var(--space-2);
  margin-bottom: var(--space-2);
  font-size: var(--font-size-sm);
}

.info-label {
  color: var(--color-text-secondary);
  min-width: 60px;
}

.info-value {
  color: var(--color-text-primary);
  flex: 1;
}

.card-actions {
  display: flex;
  gap: var(--space-2);
  padding-top: var(--space-4);
  border-top: 1px solid var(--color-border-default);
}

/* 表单样式 */
.landing-point-form,
.nearest-form {
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
  font-family: var(--font-family-primary);
  font-size: var(--font-size-base);
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  outline: none;
  transition: var(--transition-input);
  cursor: pointer;
}

.form-select:hover {
  border-color: var(--color-border-hover);
}

.form-select:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
}

.modal-actions {
  display: flex;
  justify-content: flex-end;
  gap: var(--space-3);
  margin-top: var(--space-6);
}

.submit-button {
  width: 100%;
}

/* 最近降落点搜索 */
.nearest-search-card {
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

.nearest-results {
  margin-top: var(--space-8);
  padding-top: var(--space-6);
  border-top: 1px solid var(--color-border-default);
}

.results-title {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-4) 0;
}

.results-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3);
}

.result-item {
  display: flex;
  align-items: flex-start;
  gap: var(--space-4);
  padding: var(--space-4);
  background: var(--color-bg-secondary);
  border-radius: var(--radius-md);
  transition: var(--transition-card);
}

.result-item:hover {
  background: var(--color-bg-tertiary);
}

.result-rank {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 32px;
  height: 32px;
  background: var(--apple-blue);
  color: white;
  border-radius: 50%;
  font-size: var(--font-size-base);
  font-weight: 700;
  flex-shrink: 0;
}

.result-content {
  flex: 1;
}

.result-name {
  font-size: var(--font-size-base);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0 0 var(--space-2) 0;
}

.result-distance {
  font-size: var(--font-size-sm);
  font-weight: 600;
  color: var(--apple-blue);
  margin: 0 0 var(--space-1) 0;
}

.result-location,
.result-facilities {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  margin: 0;
}

/* 地图占位符 */
.map-placeholder {
  text-align: center;
  padding: var(--space-12) var(--space-6);
}

.placeholder-content {
  margin-top: var(--space-8);
  color: var(--color-text-tertiary);
}

.placeholder-icon {
  width: 80px;
  height: 80px;
  margin: 0 auto var(--space-4);
  color: var(--color-text-tertiary);
}

/* 响应式 */
@media (max-width: 768px) {
  .emergency-landing-management {
    padding: var(--space-4);
  }

  .search-bar {
    flex-direction: column;
    align-items: stretch;
  }

  .search-input {
    max-width: none;
  }

  .filter-group {
    flex-direction: column;
    align-items: stretch;
  }

  .landing-points-grid {
    grid-template-columns: 1fr;
  }

  .form-row {
    grid-template-columns: 1fr;
  }

  .tabs {
    overflow-x: auto;
    -webkit-overflow-scrolling: touch;
  }
}
</style>
