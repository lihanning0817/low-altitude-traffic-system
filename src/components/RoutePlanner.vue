<template>
  <div class="route-planner">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">🗺️ 路径规划</h1>
      <p class="page-subtitle">为无人机规划基于城市主干道的最优飞行路径</p>
    </div>

    <!-- 主要内容区域 -->
    <div class="main-content">
      <!-- 规划表单卡片 -->
      <AppleCard class="planning-card">
        <h2 class="section-title">规划设置</h2>

        <form class="planning-form" @submit.prevent="planRoute">
          <div class="form-row">
            <div class="form-group">
              <label class="form-label">起点</label>
              <select
                v-model="planningForm.start"
                class="apple-select"
                required
              >
                <option value="">请选择起点</option>
                <option
                  v-for="node in roadNodes"
                  :key="node.id"
                  :value="node.id"
                >
                  {{ node.name }}
                </option>
              </select>
            </div>

            <div class="form-group">
              <label class="form-label">终点</label>
              <select
                v-model="planningForm.end"
                class="apple-select"
                required
              >
                <option value="">请选择终点</option>
                <option
                  v-for="node in roadNodes"
                  :key="node.id"
                  :value="node.id"
                >
                  {{ node.name }}
                </option>
              </select>
            </div>
          </div>

          <div class="form-row">
            <div class="form-group">
              <label class="form-label">优先级</label>
              <select v-model="planningForm.priority" class="apple-select">
                <option value="shortest">最短距离</option>
                <option value="fastest">最快时间</option>
                <option value="safest">最安全</option>
              </select>
            </div>

            <div class="form-group">
              <label class="form-label">飞行高度 ({{ planningForm.altitude }}m)</label>
              <input
                type="range"
                v-model.number="planningForm.altitude"
                min="50"
                max="300"
                step="10"
                class="apple-slider"
              />
              <div class="slider-labels">
                <span>50m</span>
                <span>300m</span>
              </div>
            </div>
          </div>

          <div class="form-actions">
            <AppleButton
              variant="secondary"
              type="button"
              @click="resetForm"
            >
              重置
            </AppleButton>
            <AppleButton
              variant="primary"
              type="submit"
              :disabled="planning"
            >
              {{ planning ? '规划中...' : '规划路径' }}
            </AppleButton>
          </div>
        </form>
      </AppleCard>

      <!-- 地图容器 -->
      <AppleCard class="map-card">
        <div
          ref="mapContainer"
          class="map-view"
        />
      </AppleCard>

      <!-- 路径结果展示 -->
      <AppleCard v-if="currentRoute" class="results-card">
        <div class="results-header">
          <h2 class="section-title">📊 规划结果</h2>
          <div class="results-actions">
            <AppleButton
              variant="secondary"
              size="small"
              @click="saveRoute"
            >
              保存路径
            </AppleButton>
            <AppleButton
              variant="primary"
              size="small"
              @click="createFlightTask"
            >
              创建任务
            </AppleButton>
          </div>
        </div>

        <!-- 统计信息 -->
        <div class="stats-grid">
          <div class="stat-item">
            <div class="stat-icon">📏</div>
            <div class="stat-content">
              <span class="stat-label">总距离</span>
              <span class="stat-value">{{ (currentRoute.totalDistance || 0).toFixed(2) }} km</span>
            </div>
          </div>

          <div class="stat-item">
            <div class="stat-icon">⏱️</div>
            <div class="stat-content">
              <span class="stat-label">预计时间</span>
              <span class="stat-value">{{ (currentRoute.totalTime || calculateETA()).toFixed(1) }} 分钟</span>
            </div>
          </div>

          <div class="stat-item">
            <div class="stat-icon">📍</div>
            <div class="stat-content">
              <span class="stat-label">节点数量</span>
              <span class="stat-value">{{ (currentRoute.nodes || []).length }} 个</span>
            </div>
          </div>
        </div>

        <!-- 路径详情 -->
        <div class="route-details">
          <h3 class="details-title">路径详情</h3>
          <div class="timeline">
            <div
              v-for="(node, index) in (currentRoute.nodes || [])"
              :key="index"
              class="timeline-item"
            >
              <div :class="['timeline-dot', getNodeClass(index)]"></div>
              <div class="timeline-content">
                <div class="node-header">
                  <span class="node-index">节点 {{ index + 1 }}</span>
                  <span :class="['node-badge', node.isMainRoad ? 'main-road' : 'side-road']">
                    {{ node.isMainRoad ? '主干道' : '支路' }}
                  </span>
                </div>
                <p class="node-coords">
                  经纬度: {{ node.lat.toFixed(6) }}, {{ node.lng.toFixed(6) }}
                </p>
              </div>
            </div>
          </div>
        </div>
      </AppleCard>

      <!-- 历史路径 -->
      <AppleCard v-if="routes.length > 0" class="history-card">
        <h2 class="section-title">📋 历史路径</h2>
        <div class="history-list">
          <div
            v-for="route in routes"
            :key="route.id"
            class="history-item"
            @click="loadRoute(route)"
          >
            <div class="history-info">
              <div class="history-route">
                <span class="route-id">{{ route.id }}</span>
                <span class="route-path">{{ route.start }} → {{ route.end }}</span>
              </div>
              <div class="history-meta">
                <span class="meta-distance">{{ route.distance }} km</span>
                <span class="meta-time">{{ route.createTime }}</span>
              </div>
            </div>
            <div class="history-action">
              <span class="action-icon">→</span>
            </div>
          </div>
        </div>
      </AppleCard>
    </div>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div v-if="showToast" :class="['toast-notification', toastType]">
        <div class="toast-icon">{{ toastIcon }}</div>
        <div class="toast-message">{{ toastMessage }}</div>
      </div>
    </Transition>

    <!-- 确认对话框 -->
    <Transition name="modal">
      <div v-if="showConfirmDialog" class="modal-overlay" @click="handleConfirmNo">
        <div class="modal-container confirm-dialog" @click.stop>
          <h3 class="modal-title">{{ confirmDialogTitle }}</h3>
          <p class="modal-message">{{ confirmDialogMessage }}</p>
          <div class="modal-actions">
            <AppleButton variant="secondary" @click="handleConfirmNo">取消</AppleButton>
            <AppleButton @click="handleConfirmYes">确定</AppleButton>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 输入对话框 -->
    <Transition name="modal">
      <div v-if="showInputDialog" class="modal-overlay" @click="handlePromptCancel">
        <div class="modal-container input-dialog" @click.stop>
          <h3 class="modal-title">{{ inputDialogTitle }}</h3>
          <input
            v-model="inputDialogValue"
            type="text"
            class="modal-input"
            :placeholder="inputDialogPlaceholder"
            @keyup.enter="handlePromptConfirm"
          />
          <div class="modal-actions">
            <AppleButton variant="secondary" @click="handlePromptCancel">取消</AppleButton>
            <AppleButton @click="handlePromptConfirm">确定</AppleButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useStore } from 'vuex'
import { debounce } from 'lodash-es'
import AppleCard from '@/components/apple/AppleCard.vue'
import AppleButton from '@/components/apple/AppleButton.vue'

// 状态管理
const store = useStore()

// 响应式数据
const planning = ref(false)
const currentRoute = computed(() => store.state.currentRoute)
const routes = computed(() => store.state.routes)
const mapContainer = ref(null)

const planningForm = ref({
  start: '',
  end: '',
  priority: 'shortest',
  altitude: 100
})

// Toast通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

// 确认对话框状态
const showConfirmDialog = ref(false)
const confirmDialogTitle = ref('')
const confirmDialogMessage = ref('')
const confirmDialogCallback = ref(null)

// 输入对话框状态
const showInputDialog = ref(false)
const inputDialogTitle = ref('')
const inputDialogPlaceholder = ref('')
const inputDialogValue = ref('')
const inputDialogCallback = ref(null)

// 模拟道路节点数据
const roadNodes = ref([
  { id: 'A', name: '市中心广场', lat: 39.9042, lng: 116.4074, isMainRoad: true },
  { id: 'B', name: '火车站', lat: 39.9142, lng: 116.4174, isMainRoad: true },
  { id: 'C', name: '机场', lat: 39.9242, lng: 116.4274, isMainRoad: true },
  { id: 'D', name: '商业区', lat: 39.9342, lng: 116.4374, isMainRoad: true },
  { id: 'E', name: '大学城', lat: 39.9442, lng: 116.4474, isMainRoad: true },
  { id: 'F', name: '科技园', lat: 39.9542, lng: 116.4574, isMainRoad: true }
])

// Toast通知图标
const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

// Toast通知方法
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

// 确认对话框方法
const showConfirm = (title, message) => {
  return new Promise((resolve) => {
    confirmDialogTitle.value = title
    confirmDialogMessage.value = message
    showConfirmDialog.value = true
    confirmDialogCallback.value = resolve
  })
}

const handleConfirmYes = () => {
  showConfirmDialog.value = false
  if (confirmDialogCallback.value) {
    confirmDialogCallback.value(true)
  }
}

const handleConfirmNo = () => {
  showConfirmDialog.value = false
  if (confirmDialogCallback.value) {
    confirmDialogCallback.value(false)
  }
}

// 输入对话框方法
const showPrompt = (title, placeholder = '') => {
  return new Promise((resolve) => {
    inputDialogTitle.value = title
    inputDialogPlaceholder.value = placeholder
    inputDialogValue.value = ''
    showInputDialog.value = true
    inputDialogCallback.value = resolve
  })
}

const handlePromptConfirm = () => {
  showInputDialog.value = false
  if (inputDialogCallback.value) {
    inputDialogCallback.value({ value: inputDialogValue.value })
  }
}

const handlePromptCancel = () => {
  showInputDialog.value = false
  if (inputDialogCallback.value) {
    inputDialogCallback.value(null)
  }
}

// 方法
const initMap = () => {
  if (window.AMap && mapContainer.value) {
    // 创建地图实例
    const map = new AMap.Map(mapContainer.value, {
      zoom: 11,
      center: [116.397428, 39.90923],
      viewMode: '3D',
      features: ['bg', 'road', 'building', 'point'],
      resizeEnable: true
    })

    // 添加点击事件监听器
    map.on('click', () => {
      // 这里可以处理地图点击事件
    })

    // 将地图实例存储到组件数据中
    window.routePlannerMap = map

    // 监听地图缩放和移动事件
    const debouncedZoomChange = debounce(() => {
      // 处理缩放变化
    }, 100)

    const debouncedMoveEnd = debounce(() => {
      // 处理地图移动
    }, 100)

    map.off('zoomchange', debouncedZoomChange)
    map.off('moveend', debouncedMoveEnd)

    map.on('zoomchange', debouncedZoomChange)
    map.on('moveend', debouncedMoveEnd)

    // 如果有当前路径，则绘制路径
    if (currentRoute.value && currentRoute.value.nodes && currentRoute.value.nodes.length > 0) {
      renderRouteOnMap(map, currentRoute.value)
    }

    // 添加起点和终点标记
    const addWaypointMarker = (position, title, isStart = false) => {
      const marker = new AMap.Marker({
        position: position,
        title: title,
        icon: isStart ?
          'https://webapi.amap.com/theme/v1.3/markers/n/start.png' :
          'https://webapi.amap.com/theme/v1.3/markers/n/end.png',
        offset: new AMap.Pixel(-13, -30),
        zIndex: 200
      })

      marker.on('click', () => {
        showToastNotification(`${title}: ${position[1].toFixed(6)}, ${position[0].toFixed(6)} (${isStart ? '起点' : '终点'})`, 'info')
      })

      map.add(marker)
    }

    // 添加起点和终点标记
    if (currentRoute.value && currentRoute.value.startPoint && currentRoute.value.endPoint) {
      addWaypointMarker([currentRoute.value.startPoint.lng, currentRoute.value.startPoint.lat], '起点', true)
      addWaypointMarker([currentRoute.value.endPoint.lng, currentRoute.value.endPoint.lat], '终点', false)
    }

    // 调整地图视野以包含整个路径
    if (currentRoute.value && currentRoute.value.nodes && currentRoute.value.nodes.length > 0) {
      const bounds = new AMap.Bounds(
        new AMap.LngLat(currentRoute.value.nodes[0].lng, currentRoute.value.nodes[0].lat),
        new AMap.LngLat(currentRoute.value.nodes[0].lng, currentRoute.value.nodes[0].lat)
      )

      currentRoute.value.nodes.forEach(node => {
        bounds.extend(new AMap.LngLat(node.lng, node.lat))
      })

      map.setFitView([new AMap.Polyline({ path: currentRoute.value.nodes.map(n => [n.lng, n.lat]) })], false, [100, 100, 100, 100])
    }
  }
}

const renderRouteOnMap = (map, route) => {
  // 清除之前绘制的路径
  if (window.routePolyline) {
    map.remove(window.routePolyline)
  }

  if (!route.nodes || route.nodes.length < 2) return

  // 提取路径点坐标
  const path = route.nodes.map(node => [node.lng, node.lat])

  // 创建折线
  const polyline = new AMap.Polyline({
    path: path,
    strokeColor: '#0071E3',
    strokeWeight: 6,
    strokeOpacity: 0.8,
    lineCap: 'round',
    lineJoin: 'round',
    zIndex: 10
  })

  map.add(polyline)
  window.routePolyline = polyline
  map.setFitView([polyline])
}

const planRoute = async () => {
  if (!planningForm.value.start || !planningForm.value.end) {
    showToastNotification('请选择起点和终点', 'warning')
    return
  }

  if (planningForm.value.start === planningForm.value.end) {
    showToastNotification('起点和终点不能相同', 'warning')
    return
  }

  const startNode = roadNodes.value.find(node => node.id === planningForm.value.start)
  const endNode = roadNodes.value.find(node => node.id === planningForm.value.end)

  if (!startNode || !endNode) {
    showToastNotification('无法找到选定的起点或终点', 'warning')
    return
  }

  try {
    planning.value = true
    await store.dispatch('planRoute', {
      start: { id: startNode.id, lng: startNode.lng, lat: startNode.lat },
      end: { id: endNode.id, lng: endNode.lng, lat: endNode.lat },
      useAMap: true
    })
    showToastNotification('路径规划成功', 'success')

    if (window.routePlannerMap && currentRoute.value) {
      renderRouteOnMap(window.routePlannerMap, currentRoute.value)
    }
  } catch (error) {
    showToastNotification(`路径规划失败: ${error.message}`, 'error')
  } finally {
    planning.value = false
  }
}

const resetForm = () => {
  planningForm.value = {
    start: '',
    end: '',
    priority: 'shortest',
    altitude: 100
  }
}

const calculateETA = () => {
  if (!currentRoute.value || !currentRoute.value.totalDistance) return 0
  const speed = 50
  const time = (currentRoute.value.totalDistance / speed) * 60
  return time.toFixed(1)
}

const saveRoute = () => {
  showToastNotification('路径已保存', 'success')
}

const createFlightTask = async () => {
  const result = await showPrompt('创建飞行任务', '请输入任务名称')
  if (result && result.value) {
    showToastNotification(`飞行任务 "${result.value}" 创建成功`, 'success')
  }
}

const loadRoute = (route) => {
  store.commit('SET_CURRENT_ROUTE', route)
  showToastNotification('路径已加载', 'success')
}

const getNodeClass = (index) => {
  if (index === 0) return 'start-node'
  if (currentRoute.value && index === currentRoute.value.nodes.length - 1) return 'end-node'
  return 'normal-node'
}

// 生命周期
onMounted(() => {
  const sampleRoutes = [
    {
      id: 'R001',
      start: 'A',
      end: 'C',
      distance: 5.2,
      createTime: '2024-01-15 10:30:00'
    },
    {
      id: 'R002',
      start: 'B',
      end: 'E',
      distance: 8.7,
      createTime: '2024-01-14 14:20:00'
    }
  ]

  store.commit('SET_ROUTES', sampleRoutes)
  initMap()
})
</script>

<style scoped>
.route-planner {
  min-height: 100vh;
  background: var(--color-bg-secondary, #F5F5F7);
  padding: var(--space-6, 24px);
}

/* 页面头部 */
.page-header {
  max-width: 1200px;
  margin: 0 auto var(--space-6, 24px);
}

.page-title {
  font-size: var(--font-size-3xl, 32px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin: 0 0 var(--space-2, 8px) 0;
}

.page-subtitle {
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #6E6E73);
  margin: 0;
}

/* 主内容 */
.main-content {
  max-width: 1200px;
  margin: 0 auto;
  display: flex;
  flex-direction: column;
  gap: var(--space-5, 20px);
}

/* 规划卡片 */
.planning-card {
  padding: var(--space-6, 24px);
}

.section-title {
  font-size: var(--font-size-xl, 24px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin: 0 0 var(--space-5, 20px) 0;
}

.planning-form {
  display: flex;
  flex-direction: column;
  gap: var(--space-5, 20px);
}

.form-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-4, 16px);
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: var(--space-2, 8px);
}

.form-label {
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.apple-select {
  width: 100%;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
  background: var(--color-bg-primary, #FFFFFF);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  outline: none;
  transition: all var(--transition-input, 0.2s ease);
  appearance: none;
  background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='8' viewBox='0 0 12 8'%3E%3Cpath fill='%236E6E73' d='M6 8L0 2l1.5-1.5L6 5 10.5.5 12 2z'/%3E%3C/svg%3E");
  background-repeat: no-repeat;
  background-position: right 12px center;
  padding-right: 40px;
}

.apple-select:hover {
  border-color: var(--color-text-secondary, #6E6E73);
}

.apple-select:focus {
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.apple-slider {
  width: 100%;
  height: 4px;
  border-radius: 2px;
  background: var(--color-border, #D2D2D7);
  outline: none;
  appearance: none;
}

.apple-slider::-webkit-slider-thumb {
  appearance: none;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: var(--color-primary, #0071E3);
  cursor: pointer;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  transition: transform 0.2s ease;
}

.apple-slider::-webkit-slider-thumb:hover {
  transform: scale(1.1);
}

.apple-slider::-moz-range-thumb {
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: var(--color-primary, #0071E3);
  cursor: pointer;
  border: none;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
}

.slider-labels {
  display: flex;
  justify-content: space-between;
  font-size: var(--font-size-xs, 12px);
  color: var(--color-text-tertiary, #86868B);
  margin-top: var(--space-1, 4px);
}

.form-actions {
  display: flex;
  gap: var(--space-3, 12px);
  justify-content: flex-end;
  padding-top: var(--space-4, 16px);
  border-top: 1px solid var(--color-border, #E5E5E7);
}

/* 地图卡片 */
.map-card {
  padding: 0;
  overflow: hidden;
  height: 500px;
}

.map-view {
  width: 100%;
  height: 100%;
  min-height: 500px;
}

/* 结果卡片 */
.results-card {
  padding: var(--space-6, 24px);
}

.results-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-5, 20px);
}

.results-actions {
  display: flex;
  gap: var(--space-3, 12px);
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: var(--space-4, 16px);
  margin-bottom: var(--space-6, 24px);
}

.stat-item {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
}

.stat-icon {
  font-size: 32px;
  width: 48px;
  height: 48px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-md, 10px);
}

.stat-content {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
}

.stat-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #6E6E73);
}

.stat-value {
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

/* 路径详情 */
.route-details {
  padding-top: var(--space-5, 20px);
  border-top: 1px solid var(--color-border, #E5E5E7);
}

.details-title {
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin: 0 0 var(--space-4, 16px) 0;
}

.timeline {
  display: flex;
  flex-direction: column;
  gap: var(--space-4, 16px);
}

.timeline-item {
  display: flex;
  gap: var(--space-3, 12px);
  position: relative;
}

.timeline-item:not(:last-child)::before {
  content: '';
  position: absolute;
  left: 11px;
  top: 28px;
  bottom: -16px;
  width: 2px;
  background: var(--color-border, #E5E5E7);
}

.timeline-dot {
  width: 24px;
  height: 24px;
  border-radius: 50%;
  border: 2px solid var(--color-border, #E5E5E7);
  background: var(--color-bg-primary, #FFFFFF);
  flex-shrink: 0;
  position: relative;
  z-index: 1;
}

.timeline-dot.start-node {
  border-color: var(--color-primary, #0071E3);
  background: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.timeline-dot.end-node {
  border-color: #00A86B;
  background: #00A86B;
  box-shadow: 0 0 0 4px rgba(0, 168, 107, 0.1);
}

.timeline-dot.normal-node {
  border-color: var(--color-text-tertiary, #86868B);
}

.timeline-content {
  flex: 1;
  padding-top: 2px;
}

.node-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-2, 8px);
}

.node-index {
  font-size: var(--font-size-base, 16px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.node-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-xs, 12px);
  font-weight: 500;
}

.node-badge.main-road {
  background: #E6F9F0;
  color: #00A86B;
}

.node-badge.side-road {
  background: #FFF4E6;
  color: #E67E00;
}

.node-coords {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #6E6E73);
  margin: 0;
}

/* 历史记录 */
.history-card {
  padding: var(--space-6, 24px);
}

.history-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3, 12px);
}

.history-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  cursor: pointer;
  transition: all var(--transition-smooth, 0.2s ease);
}

.history-item:hover {
  background: var(--color-border, #E5E5E7);
  transform: translateX(4px);
}

.history-info {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: var(--space-2, 8px);
}

.history-route {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
}

.route-id {
  padding: var(--space-1, 4px) var(--space-2, 8px);
  background: var(--color-primary, #0071E3);
  color: white;
  border-radius: var(--radius-sm, 6px);
  font-size: var(--font-size-xs, 12px);
  font-weight: 600;
}

.route-path {
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.history-meta {
  display: flex;
  gap: var(--space-4, 16px);
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #6E6E73);
}

.history-action {
  color: var(--color-text-tertiary, #86868B);
  font-size: 20px;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .route-planner {
    padding: var(--space-4, 16px);
  }

  .page-title {
    font-size: var(--font-size-2xl, 28px);
  }

  .form-row {
    grid-template-columns: 1fr;
  }

  .form-actions {
    flex-direction: column;
  }

  .results-header {
    flex-direction: column;
    align-items: flex-start;
    gap: var(--space-3, 12px);
  }

  .results-actions {
    width: 100%;
    flex-direction: column;
  }

  .stats-grid {
    grid-template-columns: 1fr;
  }

  .history-item {
    flex-direction: column;
    align-items: flex-start;
    gap: var(--space-3, 12px);
  }
}

/* 深色模式 */
@media (prefers-color-scheme: dark) {
  .route-planner {
    background: #000000;
  }

  .page-title,
  .section-title,
  .stat-value,
  .node-index,
  .route-path {
    color: #F5F5F7;
  }

  .page-subtitle,
  .stat-label,
  .node-coords,
  .history-meta {
    color: #98989D;
  }

  .apple-select {
    background: #1C1C1E;
    border-color: #38383A;
    color: #F5F5F7;
  }

  .stat-item {
    background: #1C1C1E;
  }

  .stat-icon {
    background: #2C2C2E;
  }

  .history-item {
    background: #1C1C1E;
  }

  .history-item:hover {
    background: #2C2C2E;
  }

  .timeline-dot {
    background: #1C1C1E;
    border-color: #38383A;
  }

  .timeline-item:not(:last-child)::before {
    background: #38383A;
  }
}

/* Toast 通知 */
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

.toast-icon {
  font-size: 24px;
  line-height: 1;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
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

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
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

/* 模态对话框 */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  width: 100vw;
  height: 100vh;
  background: rgba(0, 0, 0, 0.5);
  backdrop-filter: blur(10px);
  -webkit-backdrop-filter: blur(10px);
  z-index: 9999;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: var(--space-4, 16px);
}

.modal-container {
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-lg, 12px);
  padding: var(--space-6, 24px);
  max-width: 400px;
  width: 100%;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
}

.modal-title {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-xl, 24px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.modal-message {
  margin: 0 0 var(--space-5, 20px) 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
  line-height: 1.5;
}

.modal-input {
  width: 100%;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  margin-bottom: var(--space-5, 20px);
  transition: all var(--transition-input, 0.2s ease);
  background: var(--color-bg-primary, #FFFFFF);
  color: var(--color-text-primary, #1D1D1F);
}

.modal-input:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.modal-actions {
  display: flex;
  gap: var(--space-3, 12px);
  justify-content: flex-end;
}

/* 模态框动画 */
.modal-enter-active, .modal-leave-active {
  transition: opacity 0.3s ease;
}

.modal-enter-active .modal-container,
.modal-leave-active .modal-container {
  transition: transform 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

.modal-enter-from, .modal-leave-to {
  opacity: 0;
}

.modal-enter-from .modal-container {
  transform: scale(0.95) translateY(20px);
}

.modal-leave-to .modal-container {
  transform: scale(0.95) translateY(-20px);
}

/* 深色模式下的Toast和Modal */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: rgba(28, 28, 30, 0.95);
  }

  .toast-message {
    color: #F5F5F7;
  }

  .modal-container {
    background: #1C1C1E;
  }

  .modal-title {
    color: #F5F5F7;
  }

  .modal-message {
    color: #98989D;
  }

  .modal-input {
    background: #2C2C2E;
    border-color: #38383A;
    color: #F5F5F7;
  }
}
</style>
