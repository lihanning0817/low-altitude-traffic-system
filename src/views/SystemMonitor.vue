<template>
  <div class="system-monitor">
    <!-- 页面标题 -->
    <div class="page-header">
      <h1 class="page-title">
        <el-icon class="title-icon">
          <Monitor />
        </el-icon>
        系统监控
      </h1>
      <p class="page-description">实时监控系统状态、无人机设备和用户活动</p>
    </div>

    <!-- 系统状态卡片区域 -->
    <div class="status-cards">
      <!-- 健康状态卡片 -->
      <el-card class="status-card health-card" shadow="hover">
        <template #header>
          <div class="card-header">
            <el-icon class="card-icon">
              <CircleCheck />
            </el-icon>
            <span class="card-title">系统健康状态</span>
            <el-button
              type="text"
              size="small"
              @click="refreshHealthStatus"
              :loading="healthLoading"
            >
              <el-icon><Refresh /></el-icon>
            </el-button>
          </div>
        </template>

        <div class="health-content" v-if="healthData">
          <div class="health-indicator">
            <div class="status-light" :class="getHealthStatusClass(healthData.overall)"></div>
            <span class="status-text">{{ getHealthStatusText(healthData.overall) }}</span>
          </div>

          <div class="health-details">
            <div class="detail-item">
              <span class="label">运行时间:</span>
              <span class="value">{{ formatUptime(healthData.uptime) }}</span>
            </div>
            <div class="detail-item">
              <span class="label">数据库:</span>
              <el-tag :type="getServiceTagType(healthData.services.database)" size="small">
                {{ getServiceStatusText(healthData.services.database) }}
              </el-tag>
            </div>
            <div class="detail-item">
              <span class="label">外部API:</span>
              <el-tag :type="getServiceTagType(healthData.services.externalApis)" size="small">
                {{ getServiceStatusText(healthData.services.externalApis) }}
              </el-tag>
            </div>
          </div>
        </div>

        <div class="loading-state" v-else-if="healthLoading">
          <el-skeleton :rows="3" animated />
        </div>

        <div class="error-state" v-else>
          <el-empty description="健康状态数据加载失败" :image-size="60">
            <el-button type="primary" @click="refreshHealthStatus">重新加载</el-button>
          </el-empty>
        </div>
      </el-card>

      <!-- 系统信息卡片 -->
      <el-card class="status-card info-card" shadow="hover">
        <template #header>
          <div class="card-header">
            <el-icon class="card-icon">
              <InfoFilled />
            </el-icon>
            <span class="card-title">系统信息</span>
            <el-button
              type="text"
              size="small"
              @click="refreshSystemInfo"
              :loading="infoLoading"
            >
              <el-icon><Refresh /></el-icon>
            </el-button>
          </div>
        </template>

        <div class="info-content" v-if="systemInfo">
          <div class="info-details">
            <div class="detail-item">
              <span class="label">系统名称:</span>
              <span class="value">{{ systemInfo.name }}</span>
            </div>
            <div class="detail-item">
              <span class="label">版本:</span>
              <el-tag type="primary" size="small">{{ systemInfo.version }}</el-tag>
            </div>
            <div class="detail-item">
              <span class="label">环境:</span>
              <el-tag :type="getEnvironmentTagType(systemInfo.environment)" size="small">
                {{ getEnvironmentText(systemInfo.environment) }}
              </el-tag>
            </div>
            <div class="detail-item">
              <span class="label">启动时间:</span>
              <span class="value">{{ systemInfo.startTime || '未知' }}</span>
            </div>
          </div>
        </div>

        <div class="loading-state" v-else-if="infoLoading">
          <el-skeleton :rows="3" animated />
        </div>

        <div class="error-state" v-else>
          <el-empty description="系统信息加载失败" :image-size="60">
            <el-button type="primary" @click="refreshSystemInfo">重新加载</el-button>
          </el-empty>
        </div>
      </el-card>
    </div>

    <!-- 数据展示区域 -->
    <el-card class="data-card" shadow="hover">
      <template #header>
        <div class="card-header">
          <el-icon class="card-icon">
            <DataBoard />
          </el-icon>
          <span class="card-title">实时数据</span>
        </div>
      </template>

      <el-tabs v-model="activeTab" class="data-tabs" @tab-change="handleTabChange">
        <!-- 无人机数据标签页 -->
        <el-tab-pane label="无人机监控" name="drones">
          <template #label>
            <span class="tab-label">
              <el-icon><Drone /></el-icon>
              无人机监控
              <el-badge :value="dronesData.length" :hidden="dronesData.length === 0" />
            </span>
          </template>

          <div class="tab-content">
            <div class="table-toolbar">
              <div class="toolbar-left">
                <el-statistic title="总数" :value="dronesData.length" suffix="台" />
                <el-statistic
                  title="在线"
                  :value="getActiveDronesCount()"
                  suffix="台"
                  class="active-count"
                />
              </div>
              <div class="toolbar-right">
                <el-button
                  type="primary"
                  :icon="Refresh"
                  @click="refreshDronesData"
                  :loading="dronesLoading"
                >
                  刷新数据
                </el-button>
              </div>
            </div>

            <el-table
              :data="dronesData"
              v-loading="dronesLoading"
              empty-text="暂无无人机数据"
              stripe
              border
              class="data-table"
            >
              <el-table-column prop="droneId" label="设备ID" width="120" />
              <el-table-column prop="name" label="设备名称" width="150" />
              <el-table-column prop="model" label="型号" width="120" />
              <el-table-column label="状态" width="100">
                <template #default="{ row }">
                  <el-tag :color="row.statusColor" size="small">
                    {{ row.statusText }}
                  </el-tag>
                </template>
              </el-table-column>
              <el-table-column label="电量" width="120">
                <template #default="{ row }">
                  <div class="battery-info">
                    <el-progress
                      :percentage="row.battery"
                      :color="row.batteryColor"
                      :stroke-width="8"
                      :show-text="false"
                      style="width: 60px;"
                    />
                    <span class="battery-text">{{ row.battery }}%</span>
                  </div>
                </template>
              </el-table-column>
              <el-table-column label="位置" width="200">
                <template #default="{ row }">
                  <div class="location-info" v-if="row.location">
                    <div class="coordinate">
                      <el-icon><LocationInformation /></el-icon>
                      {{ row.location.lat.toFixed(4) }}, {{ row.location.lng.toFixed(4) }}
                    </div>
                  </div>
                  <span v-else class="no-location">位置未知</span>
                </template>
              </el-table-column>
              <el-table-column label="最后更新" width="160">
                <template #default="{ row }">
                  <span class="update-time">
                    {{ formatTimestamp(row.lastUpdate) }}
                  </span>
                </template>
              </el-table-column>
              <el-table-column label="操作" width="120" fixed="right">
                <template #default="{ row }">
                  <el-button type="text" size="small" @click="viewDroneDetails(row)">
                    <el-icon><View /></el-icon>
                    详情
                  </el-button>
                </template>
              </el-table-column>
            </el-table>
          </div>
        </el-tab-pane>

        <!-- 用户数据标签页 - 仅管理员可见 -->
        <el-tab-pane v-if="isAdmin()" label="用户管理" name="users">
          <template #label>
            <span class="tab-label">
              <el-icon><User /></el-icon>
              用户管理
              <el-badge :value="usersPagination.total" :hidden="usersPagination.total === 0" />
            </span>
          </template>

          <div class="tab-content">
            <div class="table-toolbar">
              <div class="toolbar-left">
                <el-statistic title="总用户" :value="usersPagination.total || 0" suffix="人" />
                <el-select
                  v-model="usersFilter.role"
                  placeholder="筛选角色"
                  clearable
                  @change="handleRoleFilterChange"
                  style="width: 150px; margin-left: 20px;"
                >
                  <el-option label="全部角色" value="" />
                  <el-option label="管理员" value="admin" />
                  <el-option label="操作员" value="operator" />
                  <el-option label="普通用户" value="user" />
                </el-select>
              </div>
              <div class="toolbar-right">
                <el-button
                  type="primary"
                  :icon="Refresh"
                  @click="refreshUsersData"
                  :loading="usersLoading"
                >
                  刷新数据
                </el-button>
              </div>
            </div>

            <el-table
              :data="usersData"
              v-loading="usersLoading"
              empty-text="暂无用户数据"
              stripe
              border
              class="data-table"
            >
              <el-table-column prop="id" label="ID" width="80" />
              <el-table-column prop="username" label="用户名" width="150" />
              <el-table-column prop="fullName" label="姓名" width="120" />
              <el-table-column prop="email" label="邮箱" width="200" />
              <el-table-column label="角色" width="100">
                <template #default="{ row }">
                  <el-tag :color="row.roleColor" size="small">
                    {{ row.roleText }}
                  </el-tag>
                </template>
              </el-table-column>
              <el-table-column label="状态" width="100">
                <template #default="{ row }">
                  <el-tag :color="row.statusColor" size="small">
                    {{ row.statusText }}
                  </el-tag>
                </template>
              </el-table-column>
              <el-table-column label="创建时间" width="160">
                <template #default="{ row }">
                  <span class="create-time">
                    {{ formatTimestamp(row.createdAt) }}
                  </span>
                </template>
              </el-table-column>
              <el-table-column label="最后登录" width="160">
                <template #default="{ row }">
                  <span class="login-time">
                    {{ row.lastLogin ? formatTimestamp(row.lastLogin) : '从未登录' }}
                  </span>
                </template>
              </el-table-column>
              <el-table-column label="操作" width="120" fixed="right">
                <template #default="{ row }">
                  <el-button type="text" size="small" @click="viewUserDetails(row)">
                    <el-icon><View /></el-icon>
                    详情
                  </el-button>
                </template>
              </el-table-column>
            </el-table>

            <!-- 分页组件 -->
            <div class="pagination-wrapper" v-if="usersPagination.total > 0">
              <el-pagination
                v-model:current-page="currentPage"
                v-model:page-size="pageSize"
                :page-sizes="[10, 20, 50, 100]"
                :total="usersPagination.total"
                layout="total, sizes, prev, pager, next, jumper"
                @size-change="handlePageSizeChange"
                @current-change="handleCurrentPageChange"
              />
            </div>
          </div>
        </el-tab-pane>
      </el-tabs>
    </el-card>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { useStore } from 'vuex'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  Monitor,
  CircleCheck,
  InfoFilled,
  DataBoard,
  Refresh,
  User,
  View,
  LocationInformation
} from '@element-plus/icons-vue'
import systemMonitorApi from '@/services/systemMonitorApi'

// 图标导入 - 这些图标可能需要安装额外的图标包
const Drone = Monitor // 暂时使用Monitor图标代替无人机图标

const store = useStore()

// 响应式数据
const activeTab = ref('drones')
const healthLoading = ref(false)
const infoLoading = ref(false)
const dronesLoading = ref(false)
const usersLoading = ref(false)

// 状态数据 - 从Vuex store获取
const healthData = computed(() => store.getters.systemHealthStatus)
const systemInfo = computed(() => store.getters.systemInfo)
const dronesData = computed(() => store.getters.monitorDrones)
const usersData = computed(() => store.getters.monitorUsers.users)

// 用户数据分页和筛选
const usersPagination = computed(() => {
  const vuexPagination = store.getters.monitorUsers.pagination || {}
  return {
    currentPage: vuexPagination.currentPage || 1,
    pageSize: vuexPagination.pageSize || 20,
    total: vuexPagination.total || 0
  }
})

// 分页控制状态
const currentPage = ref(1)
const pageSize = ref(20)

const usersFilter = ref({
  role: ''
})

// ========== 计算属性 ==========

const getActiveDronesCount = () => {
  return dronesData.value.filter(drone => drone.status === 'active').length
}

// ========== 页面加载 ==========

onMounted(async () => {
  // 初始加载所有数据
  await Promise.all([
    loadHealthStatus(),
    loadSystemInfo(),
    loadDronesData()
  ])

  // 如果用户有管理员权限，加载用户数据
  if (isAdmin()) {
    await loadUsersData()
  }
})

// ========== 数据加载方法 ==========

const loadHealthStatus = async (forceRefresh = false) => {
  healthLoading.value = true
  try {
    await store.dispatch('fetchSystemHealthStatus', { forceRefresh })
  } catch (error) {
    console.error('加载健康状态失败:', error)
    ElMessage.error('加载健康状态失败')
  } finally {
    healthLoading.value = false
  }
}

const loadSystemInfo = async (forceRefresh = false) => {
  infoLoading.value = true
  try {
    await store.dispatch('fetchSystemInfo', { forceRefresh })
  } catch (error) {
    console.error('加载系统信息失败:', error)
    ElMessage.error('加载系统信息失败')
  } finally {
    infoLoading.value = false
  }
}

const loadDronesData = async (forceRefresh = false) => {
  dronesLoading.value = true
  try {
    await store.dispatch('fetchMonitorDrones', { forceRefresh })
  } catch (error) {
    console.error('加载无人机数据失败:', error)

    // 检查是否是认证相关错误
    if (error.error_code === 'NOT_AUTHENTICATED' || error.requiresAuth) {
      ElMessage.warning(error.message || '请先登录后再访问无人机数据')
    } else if (error.error_code === 'AUTHENTICATION_EXPIRED') {
      ElMessage.warning('登录已过期，请重新登录')
    } else {
      ElMessage.error('加载无人机数据失败')
    }
  } finally {
    dronesLoading.value = false
  }
}

const loadUsersData = async (forceRefresh = false) => {
  usersLoading.value = true
  try {
    const params = {
      offset: (currentPage.value - 1) * pageSize.value,
      limit: pageSize.value
    }

    if (usersFilter.value.role) {
      params.role = usersFilter.value.role
    }

    await store.dispatch('fetchMonitorUsers', { params, forceRefresh })
  } catch (error) {
    console.error('加载用户数据失败:', error)

    // 检查是否是认证或权限相关错误
    if (error.error_code === 'NOT_AUTHENTICATED' || error.requiresAuth) {
      ElMessage.warning(error.message || '请先登录后再访问用户数据')
    } else if (error.error_code === 'AUTHENTICATION_EXPIRED') {
      ElMessage.warning('登录已过期，请重新登录')
    } else if (error.error_code === 'INSUFFICIENT_PRIVILEGES' || error.requiresAdmin) {
      // 显示详细的权限不足信息
      ElMessage({
        type: 'warning',
        message: error.message || '权限不足：只有管理员可以查看用户列表',
        duration: 5000,
        showClose: true
      })
    } else {
      ElMessage.error('加载用户数据失败')
    }
  } finally {
    usersLoading.value = false
  }
}

// ========== 事件处理 ==========

const handleTabChange = (tabName) => {
  activeTab.value = tabName

  // 切换到用户标签页时，检查权限并加载数据
  if (tabName === 'users' && isAdmin()) {
    loadUsersData()
  }
}

const handleRoleFilterChange = () => {
  currentPage.value = 1
  loadUsersData(true)
}

const handlePageSizeChange = (newPageSize) => {
  pageSize.value = newPageSize
  currentPage.value = 1
  loadUsersData(true)
}

const handleCurrentPageChange = (newCurrentPage) => {
  currentPage.value = newCurrentPage
  loadUsersData(true)
}

// ========== 刷新方法 ==========

const refreshHealthStatus = () => {
  loadHealthStatus(true)
}

const refreshSystemInfo = () => {
  loadSystemInfo(true)
}

const refreshDronesData = () => {
  loadDronesData(true)
}

const refreshUsersData = () => {
  loadUsersData(true)
}

// ========== 详情查看 ==========

const viewDroneDetails = (drone) => {
  ElMessageBox.alert(
    `
    <div style="text-align: left;">
      <p><strong>设备ID:</strong> ${drone.droneId}</p>
      <p><strong>名称:</strong> ${drone.name}</p>
      <p><strong>型号:</strong> ${drone.model}</p>
      <p><strong>状态:</strong> ${drone.statusText}</p>
      <p><strong>电量:</strong> ${drone.battery}%</p>
      <p><strong>位置:</strong> ${drone.location ? `${drone.location.lat.toFixed(6)}, ${drone.location.lng.toFixed(6)}` : '位置未知'}</p>
      <p><strong>最后更新:</strong> ${formatTimestamp(drone.lastUpdate)}</p>
    </div>
    `,
    `无人机详情 - ${drone.name}`,
    {
      dangerouslyUseHTMLString: true,
      confirmButtonText: '关闭'
    }
  )
}

const viewUserDetails = (user) => {
  ElMessageBox.alert(
    `
    <div style="text-align: left;">
      <p><strong>用户ID:</strong> ${user.id}</p>
      <p><strong>用户名:</strong> ${user.username}</p>
      <p><strong>姓名:</strong> ${user.fullName || '未设置'}</p>
      <p><strong>邮箱:</strong> ${user.email}</p>
      <p><strong>角色:</strong> ${user.roleText}</p>
      <p><strong>状态:</strong> ${user.statusText}</p>
      <p><strong>创建时间:</strong> ${formatTimestamp(user.createdAt)}</p>
      <p><strong>最后登录:</strong> ${user.lastLogin ? formatTimestamp(user.lastLogin) : '从未登录'}</p>
    </div>
    `,
    `用户详情 - ${user.username}`,
    {
      dangerouslyUseHTMLString: true,
      confirmButtonText: '关闭'
    }
  )
}

// ========== 工具方法 ==========

const isAdmin = () => {
  // 统一使用Vuex store的getter
  return store.getters.isAdmin
}

const getHealthStatusClass = (status) => {
  return {
    'status-ok': status === 'ok',
    'status-warning': status === 'degraded' || status === 'warning',
    'status-error': status === 'error' || status === 'down'
  }
}

const getHealthStatusText = (status) => {
  const statusMap = {
    'ok': '系统正常',
    'degraded': '部分异常',
    'warning': '警告',
    'error': '系统异常',
    'down': '系统下线'
  }
  return statusMap[status] || '状态未知'
}

const getServiceTagType = (status) => {
  const typeMap = {
    'ok': 'success',
    'warning': 'warning',
    'error': 'danger'
  }
  return typeMap[status] || 'info'
}

const getServiceStatusText = (status) => {
  const statusMap = {
    'ok': '正常',
    'warning': '警告',
    'error': '异常'
  }
  return statusMap[status] || '未知'
}

const getEnvironmentTagType = (env) => {
  const typeMap = {
    'production': 'danger',
    'staging': 'warning',
    'development': 'success'
  }
  return typeMap[env] || 'info'
}

const getEnvironmentText = (env) => {
  const envMap = {
    'production': '生产环境',
    'staging': '预发布',
    'development': '开发环境'
  }
  return envMap[env] || env
}

const formatUptime = (seconds) => {
  return systemMonitorApi.formatUptime(seconds)
}

const formatTimestamp = (timestamp) => {
  return systemMonitorApi.formatTimestamp(timestamp)
}
</script>

<style scoped>
.system-monitor {
  padding: 20px;
  background-color: #f5f7fa;
  min-height: calc(100vh - 120px);
}

/* 页面标题 */
.page-header {
  margin-bottom: 20px;
}

.page-title {
  display: flex;
  align-items: center;
  font-size: 24px;
  font-weight: 600;
  color: #1f2329;
  margin: 0 0 8px 0;
}

.title-icon {
  margin-right: 8px;
  color: #409eff;
}

.page-description {
  color: #606266;
  font-size: 14px;
  margin: 0;
}

/* 状态卡片布局 */
.status-cards {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
  margin-bottom: 20px;
}

.status-card {
  border-radius: 8px;
  transition: all 0.3s;
}

.status-card:hover {
  transform: translateY(-2px);
}

.card-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.card-icon {
  margin-right: 8px;
  color: #409eff;
}

.card-title {
  font-size: 16px;
  font-weight: 600;
  color: #1f2329;
}

/* 健康状态卡片 */
.health-content {
  padding: 10px 0;
}

.health-indicator {
  display: flex;
  align-items: center;
  margin-bottom: 20px;
  padding: 12px;
  background-color: #f8fafb;
  border-radius: 6px;
}

.status-light {
  width: 12px;
  height: 12px;
  border-radius: 50%;
  margin-right: 10px;
  position: relative;
  animation: pulse 2s infinite;
}

.status-light.status-ok {
  background-color: #67c23a;
  box-shadow: 0 0 8px rgba(103, 194, 58, 0.6);
}

.status-light.status-warning {
  background-color: #e6a23c;
  box-shadow: 0 0 8px rgba(230, 162, 60, 0.6);
}

.status-light.status-error {
  background-color: #f56c6c;
  box-shadow: 0 0 8px rgba(245, 108, 108, 0.6);
}

@keyframes pulse {
  0% { box-shadow: 0 0 8px currentColor; }
  50% { box-shadow: 0 0 16px currentColor; }
  100% { box-shadow: 0 0 8px currentColor; }
}

.status-text {
  font-size: 16px;
  font-weight: 600;
  color: #1f2329;
}

.health-details {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.detail-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 0;
  border-bottom: 1px solid #ebeef5;
}

.detail-item:last-child {
  border-bottom: none;
}

.label {
  color: #606266;
  font-size: 14px;
}

.value {
  color: #1f2329;
  font-weight: 500;
  font-size: 14px;
}

/* 系统信息卡片 */
.info-content {
  padding: 10px 0;
}

.info-details {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

/* 数据展示卡片 */
.data-card {
  border-radius: 8px;
}

.data-tabs {
  margin-top: 20px;
}

.tab-label {
  display: flex;
  align-items: center;
  gap: 6px;
}

.tab-content {
  padding: 20px 0;
}

/* 工具栏 */
.table-toolbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding: 16px;
  background-color: #f8fafb;
  border-radius: 6px;
  border: 1px solid #ebeef5;
}

.toolbar-left {
  display: flex;
  align-items: center;
  gap: 30px;
}

.active-count {
  color: #67c23a;
}

.toolbar-right {
  display: flex;
  gap: 12px;
}

/* 表格样式 */
.data-table {
  border-radius: 6px;
  overflow: hidden;
}

/* 无人机相关样式 */
.battery-info {
  display: flex;
  align-items: center;
  gap: 8px;
}

.battery-text {
  font-size: 12px;
  color: #606266;
  min-width: 30px;
}

.location-info {
  font-size: 12px;
}

.coordinate {
  display: flex;
  align-items: center;
  gap: 4px;
  color: #606266;
}

.no-location {
  color: #c0c4cc;
  font-style: italic;
}

.update-time, .create-time, .login-time {
  font-size: 12px;
  color: #606266;
}

/* 分页样式 */
.pagination-wrapper {
  display: flex;
  justify-content: center;
  margin-top: 20px;
  padding-top: 20px;
  border-top: 1px solid #ebeef5;
}

/* 加载和错误状态 */
.loading-state {
  padding: 20px;
}

.error-state {
  padding: 20px;
  text-align: center;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .status-cards {
    grid-template-columns: 1fr;
  }

  .table-toolbar {
    flex-direction: column;
    gap: 16px;
    align-items: stretch;
  }

  .toolbar-left {
    justify-content: space-between;
  }
}
</style>