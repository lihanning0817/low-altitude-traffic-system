<template>
  <div class="map-monitoring">
    <!-- 页面头部 -->
    <MapHeader :active-devices="activeDevices" />

    <!-- 主要内容区域 -->
    <div class="main-content">
      <!-- 地图容器 -->
      <AppleCard
        hover-effect
        class="map-container"
      >
        <!-- 地图工具面板 -->
        <div class="map-tools">
          <div class="zoom-controls">
            <button
              class="tool-button"
              @click="zoomIn"
            >
              <span>➕</span>
            </button>
            <button
              class="tool-button"
              @click="zoomOut"
            >
              <span>➖</span>
            </button>
          </div>

          <div class="layer-control">
            <button
              class="tool-button"
              @click="toggleMapTypeMenu"
            >
              <span>🗺️</span>
            </button>
            <Teleport to="body">
              <Transition name="dropdown">
                <div
                  v-if="showMapTypeMenu"
                  class="map-type-menu"
                  :style="{ top: mapTypeMenuPos.y + 'px', left: mapTypeMenuPos.x + 'px' }"
                  @click.stop
                >
                  <button
                    class="menu-item"
                    @click="changeMapType('satellite')"
                  >
                    卫星图
                  </button>
                  <button
                    class="menu-item"
                    @click="changeMapType('roadmap')"
                  >
                    路线图
                  </button>
                  <button
                    class="menu-item"
                    @click="changeMapType('hybrid')"
                  >
                    混合图
                  </button>
                </div>
              </Transition>
            </Teleport>
          </div>

          <div class="location-control">
            <button
              class="tool-button"
              @click="getCurrentLocation"
            >
              <span>📍</span>
            </button>
          </div>
        </div>

        <!-- 地图主体 -->
        <div
          id="map"
          ref="mapContainer"
          class="map-view"
        />

        <!-- 地图信息面板 -->
        <div class="map-info-panel">
          <div class="info-item">
            <span class="label">经度:</span>
            <span class="value">{{ mapCenter.lng.toFixed(6) }}</span>
          </div>
          <div class="info-item">
            <span class="label">纬度:</span>
            <span class="value">{{ mapCenter.lat.toFixed(6) }}</span>
          </div>
          <div class="info-item">
            <span class="label">缩放:</span>
            <span class="value">{{ mapZoom }}</span>
          </div>
        </div>

        <!-- 自定义右键菜单 -->
        <Teleport to="body">
          <Transition name="dropdown">
            <div
              v-if="contextMenu.visible"
              :style="{
                left: contextMenu.x + 'px',
                top: contextMenu.y + 'px'
              }"
              class="context-menu"
              @click.stop
            >
              <button
                class="menu-item"
                @click="setRoutePoint('start')"
              >
                <span class="menu-icon">🚩</span>
                <span>设置为起点</span>
              </button>
              <button
                class="menu-item"
                @click="setRoutePoint('waypoint')"
              >
                <span class="menu-icon">📌</span>
                <span>设置为途经点</span>
              </button>
              <button
                class="menu-item"
                @click="setRoutePoint('end')"
              >
                <span class="menu-icon">🎯</span>
                <span>设置为终点</span>
              </button>
              <div class="menu-divider" />
              <button
                class="menu-item danger"
                @click="clearRoute"
              >
                <span class="menu-icon">🗑️</span>
                <span>清除路线</span>
              </button>
            </div>
          </Transition>
        </Teleport>
      </AppleCard>

      <!-- 右侧侧边栏 -->
      <div class="right-sidebar">
        <!-- 设备状态面板 -->
        <AppleCard
          hover-effect
          class="device-panel"
        >
          <template #header>
            <div class="panel-header">
              <span class="panel-title">🖥️ 设备状态</span>
              <span class="status-badge online">{{ devices.length }} 在线</span>
            </div>
          </template>
          <div
            ref="deviceListRef"
            class="device-list"
          >
            <div
              v-for="device in devices"
              :key="device.id"
              class="device-item"
              :class="{ active: selectedDevice?.id === device.id }"
              @click="selectDevice(device)"
            >
              <div class="device-icon">
                <span>{{ getDeviceEmoji(device.type) }}</span>
              </div>
              <div class="device-info">
                <div class="device-name">
                  {{ device.name }}
                </div>
                <div class="device-status">
                  {{ device.status }}
                </div>
              </div>
              <div class="device-signal">
                <div class="signal-indicator" :style="{ '--signal': device.signal + '%' }">
                  <span class="signal-value">{{ device.signal }}%</span>
                </div>
              </div>
            </div>
          </div>
        </AppleCard>

        <!-- 路线规划面板 -->
        <AppleCard
          v-if="routeData.start || routeData.end || routeData.waypoints.length > 0"
          hover-effect
          class="route-panel"
        >
          <template #header>
            <div class="panel-header">
              <span class="panel-title">🗺️ 路线规划</span>
              <AppleButton
                variant="primary"
                size="small"
                :disabled="!canPlanRoute"
                @click="planRoute"
              >
                规划路线
              </AppleButton>
            </div>
          </template>

          <!-- 路线点位信息 -->
          <div class="route-points">
            <div
              v-if="routeData.start"
              class="route-point start"
            >
              <div class="point-header">
                <span>🚩 起点</span>
                <button
                  class="clear-button"
                  @click="clearRoutePoint('start')"
                >
                  ✕
                </button>
              </div>
              <div class="point-coords">
                {{ formatCoords(routeData.start) }}
              </div>
            </div>

            <div
              v-for="(waypoint, index) in routeData.waypoints"
              :key="index"
              class="route-point waypoint"
            >
              <div class="point-header">
                <span>📌 途经点 {{ index + 1 }}</span>
                <button
                  class="clear-button"
                  @click="clearWaypoint(index)"
                >
                  ✕
                </button>
              </div>
              <div class="point-coords">
                {{ formatCoords(waypoint) }}
              </div>
            </div>

            <div
              v-if="routeData.end"
              class="route-point end"
            >
              <div class="point-header">
                <span>🎯 终点</span>
                <button
                  class="clear-button"
                  @click="clearRoutePoint('end')"
                >
                  ✕
                </button>
              </div>
              <div class="point-coords">
                {{ formatCoords(routeData.end) }}
              </div>
            </div>
          </div>

          <!-- 路线信息 -->
          <div
            v-if="routeInfo"
            class="route-info"
          >
            <div class="info-title">
              📊 路线概要
            </div>
            <div class="info-details">
              <div class="detail-item">
                <span class="detail-label">总距离:</span>
                <span class="detail-value">{{ routeInfo.distance }}</span>
              </div>
              <div class="detail-item">
                <span class="detail-label">预计时间:</span>
                <span class="detail-value">{{ routeInfo.duration }}</span>
              </div>
              <div class="detail-item">
                <span class="detail-label">途经路段:</span>
                <span class="detail-value">{{ routeInfo.segments }}个</span>
              </div>
            </div>
          </div>

          <!-- 路线选项 -->
          <div class="route-options">
            <div class="form-group">
              <label>策略</label>
              <select
                v-model="routeOptions.strategy"
                class="route-select"
              >
                <option value="0">
                  最快路线
                </option>
                <option value="1">
                  最短路线
                </option>
                <option value="2">
                  避免拥堵
                </option>
              </select>
            </div>
            <div class="form-group">
              <label>避让</label>
              <div class="checkbox-group">
                <label class="checkbox-label">
                  <input
                    v-model="routeOptions.avoidTolls"
                    type="checkbox"
                  >
                  <span>避免收费</span>
                </label>
                <label class="checkbox-label">
                  <input
                    v-model="routeOptions.avoidHighway"
                    type="checkbox"
                  >
                  <span>避免高速</span>
                </label>
              </div>
            </div>
          </div>
        </AppleCard>
      </div>
    </div>

    <!-- 控制面板 -->
    <AppleCard
      hover-effect
      class="control-panel"
    >
      <div class="control-content">
        <!-- 左侧工具栏 -->
        <MapToolbar
          :current-tool="currentTool"
          @tool-change="setTool"
        />

        <!-- 中央控制区 -->
        <MapControls
          :show-traffic="showTraffic"
          @refresh="refreshMap"
          @center="centerMap"
          @toggle-traffic="toggleLayer"
        />

        <!-- 右侧操作区 -->
        <MapActions
          @fullscreen="showFullScreen"
          @settings="showSettings = true"
        />
      </div>
    </AppleCard>

    <!-- 设置对话框 -->
    <AppleModal
      v-model="showSettings"
      title="🔧 地图设置"
    >
      <div class="settings-form">
        <div class="form-group">
          <label>地图类型</label>
          <div class="radio-group">
            <label class="radio-label">
              <input
                v-model="mapType"
                type="radio"
                value="roadmap"
              >
              <span>路线图</span>
            </label>
            <label class="radio-label">
              <input
                v-model="mapType"
                type="radio"
                value="satellite"
              >
              <span>卫星图</span>
            </label>
            <label class="radio-label">
              <input
                v-model="mapType"
                type="radio"
                value="hybrid"
              >
              <span>混合图</span>
            </label>
          </div>
        </div>
        <div class="form-group">
          <label>显示交通</label>
          <div class="switch-wrapper">
            <label class="switch">
              <input
                v-model="showTraffic"
                type="checkbox"
              >
              <span class="slider" />
            </label>
          </div>
        </div>
        <div class="form-group">
          <label>自动刷新</label>
          <div class="switch-wrapper">
            <label class="switch">
              <input
                v-model="autoRefresh"
                type="checkbox"
              >
              <span class="slider" />
            </label>
          </div>
        </div>
      </div>
      <template #footer>
        <AppleButton
          variant="secondary"
          @click="showSettings = false"
        >
          取消
        </AppleButton>
        <AppleButton
          variant="primary"
          @click="applySettings"
        >
          应用
        </AppleButton>
      </template>
    </AppleModal>

    <!-- Toast Notification -->
    <Teleport to="body">
      <Transition name="toast">
        <div
          v-if="toastState.visible"
          class="toast-notification"
          :class="`toast-${toastState.type}`"
        >
          <div class="toast-icon">
            <span v-if="toastState.type === 'success'">✓</span>
            <span v-else-if="toastState.type === 'error'">✕</span>
            <span v-else-if="toastState.type === 'warning'">⚠</span>
            <span v-else>ℹ</span>
          </div>
          <div class="toast-message">{{ toastState.message }}</div>
        </div>
      </Transition>
    </Teleport>

    <!-- Loading Overlay -->
    <Teleport to="body">
      <Transition name="fade">
        <div
          v-if="loadingState.visible"
          class="loading-overlay"
        >
          <div class="loading-content">
            <div class="loading-spinner" />
            <div class="loading-text">{{ loadingState.text }}</div>
          </div>
        </div>
      </Transition>
    </Teleport>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted, onUnmounted, computed, nextTick } from 'vue'
import { AppleCard, AppleButton, AppleModal } from '@/components/apple'
import MapHeader from '@/components/MapHeader.vue'
import MapToolbar from '@/components/MapToolbar.vue'
import MapControls from '@/components/MapControls.vue'
import MapActions from '@/components/MapActions.vue'

// Toast notification state
const toastState = reactive({
  visible: false,
  message: '',
  type: 'info', // 'success', 'error', 'warning', 'info'
  duration: 3000
})

let toastTimer = null

// Show toast notification
const showToastNotification = (message, type = 'info', duration = 3000) => {
  if (toastTimer) {
    clearTimeout(toastTimer)
  }

  toastState.message = message
  toastState.type = type
  toastState.duration = duration
  toastState.visible = true

  toastTimer = setTimeout(() => {
    toastState.visible = false
  }, duration)
}

// Loading state
const loadingState = reactive({
  visible: false,
  text: ''
})

// Show loading
const showLoading = (text = '加载中...') => {
  loadingState.text = text
  loadingState.visible = true
  return {
    close: () => {
      loadingState.visible = false
    }
  }
}

// 响应式数据
const mapContainer = ref(null)
const deviceListRef = ref(null)
const currentTool = ref('select')
const showTraffic = ref(false)
const showSettings = ref(false)
const mapType = ref('roadmap')
const autoRefresh = ref(false)
const mapZoom = ref(11)
const selectedDevice = ref(null)
const showMapTypeMenu = ref(false)
const mapTypeMenuPos = reactive({ x: 0, y: 0 })

const mapCenter = reactive({
  lng: 116.397428,
  lat: 39.90923
})

// 设备数据
const devices = ref([
  {
    id: 'UAV001',
    name: '巡逻无人机-01',
    type: 'drone',
    status: '巡航中',
    signal: 85,
    position: [116.397428, 39.90923]
  },
  {
    id: 'CAM002',
    name: '监控摄像头-02',
    type: 'camera',
    status: '正常',
    signal: 92,
    position: [116.407428, 39.91923]
  },
  {
    id: 'RAD003',
    name: '雷达设备-03',
    type: 'radar',
    status: '运行中',
    signal: 78,
    position: [116.387428, 39.89923]
  }
])

const activeDevices = ref(devices.value.length)

// 自定义菜单
const contextMenu = reactive({
  visible: false,
  x: 0,
  y: 0,
  lnglat: null
})

// 路线数据
const routeData = reactive({
  start: null,
  end: null,
  waypoints: []
})

// 路线信息
const routeInfo = ref(null)

// 路线选项
const routeOptions = reactive({
  strategy: '0',
  avoidTolls: false,
  avoidHighway: false
})

// 地图相关变量
let map = null
let routePolyline = null
let startMarker = null
let endMarker = null
let waypointMarkers = []

// 计算属性
const canPlanRoute = computed(() => {
  return routeData.start && routeData.end
})

// 辅助函数
const getDeviceEmoji = (type) => {
  const emojiMap = {
    'drone': '✈️',
    'camera': '📹',
    'radar': '📡'
  }
  return emojiMap[type] || '🖥️'
}

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

// 地图类型菜单切换
const toggleMapTypeMenu = (event) => {
  showMapTypeMenu.value = !showMapTypeMenu.value
  if (showMapTypeMenu.value) {
    const rect = event.target.getBoundingClientRect()
    mapTypeMenuPos.x = rect.left
    mapTypeMenuPos.y = rect.bottom + 8
    nextTick(() => {
      document.addEventListener('click', () => {
        showMapTypeMenu.value = false
      }, { once: true })
    })
  }
}

// 地图初始化
const initMap = () => {
  if (window.AMap && mapContainer.value) {
    map = new window.AMap.Map(mapContainer.value, {
      zoom: mapZoom.value,
      center: [mapCenter.lng, mapCenter.lat],
      viewMode: '3D',
      features: ['bg', 'road', 'building', 'point'],
      resizeEnable: true
    })

    // 监听地图右键事件
    map.on('rightclick', (e) => {
      e.originEvent.preventDefault()
      showContextMenu(e)
    })

    // 也支持双击事件
    map.on('dblclick', (e) => {
      showContextMenu(e)
    })

    // 监听地图单击事件，隐藏菜单
    map.on('click', () => {
      hideContextMenu()
    })

    // 监听地图事件
    const debouncedZoomChange = debounce(() => {
      mapZoom.value = map.getZoom()
    }, 100)

    const debouncedMoveEnd = debounce(() => {
      const center = map.getCenter()
      mapCenter.lng = center.lng
      mapCenter.lat = center.lat
    }, 100)

    map.on('zoomchange', debouncedZoomChange)
    map.on('moveend', debouncedMoveEnd)

    // 初始化设备标记
    initDeviceMarkers()

    // 加载已有的地图标记点
    loadExistingMarkers()
  }
}

// 显示自定义菜单
const showContextMenu = (e) => {
  try {
    console.log('显示菜单，坐标:', e.lnglat)
    contextMenu.lnglat = e.lnglat

    const pixel = map.lngLatToContainer(e.lnglat)
    console.log('容器内坐标:', pixel)

    if (pixel && typeof pixel.x === 'number' && typeof pixel.y === 'number') {
      const mapElement = mapContainer.value
      const rect = mapElement.getBoundingClientRect()

      const pageX = pixel.x + rect.left + window.scrollX
      const pageY = pixel.y + rect.top + window.scrollY

      console.log('地图容器偏移:', rect.left, rect.top)
      console.log('页面滚动:', window.scrollX, window.scrollY)

      contextMenu.x = Math.max(0, Math.min(pageX, window.innerWidth + window.scrollX - 200))
      contextMenu.y = Math.max(0, Math.min(pageY, window.innerHeight + window.scrollY - 200))
      contextMenu.visible = true
      console.log('最终菜单位置:', contextMenu.x, contextMenu.y)

      nextTick(() => {
        document.addEventListener('click', hideContextMenu, { once: true })
      })
    } else {
      console.error('无法获取有效的屏幕坐标:', pixel)
    }
  } catch (error) {
    console.error('显示菜单时出错:', error)
  }
}

// 隐藏自定义菜单
const hideContextMenu = () => {
  contextMenu.visible = false
  document.removeEventListener('click', hideContextMenu)
}

// 设置路线点位
const setRoutePoint = async (type) => {
  if (!contextMenu.lnglat) return

  const { lng, lat } = contextMenu.lnglat
  const position = [lng, lat]

  try {
    const markerData = {
      lng: lng,
      lat: lat,
      title: type === 'start' ? '起点' : type === 'end' ? '终点' : '途经点',
      type: type,
      description: `${type === 'start' ? '起点' : type === 'end' ? '终点' : '途经点'}设置于 ${new Date().toLocaleString()}`
    }

    const response = await fetch('http://localhost:8081/api/v1/map/markers', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(markerData)
    })

    const result = await response.json()

    if (result.success) {
      console.log('标记点已保存到服务器:', result.data)
    } else {
      console.warn('保存标记点失败:', result.error)
    }
  } catch (error) {
    console.error('保存标记点时出错:', error)
  }

  if (type === 'start') {
    routeData.start = position
    updateStartMarker()
    showToastNotification('起点设置成功', 'success')
  } else if (type === 'end') {
    routeData.end = position
    updateEndMarker()
    showToastNotification('终点设置成功', 'success')
  } else if (type === 'waypoint') {
    routeData.waypoints.push(position)
    updateWaypointMarkers()
    showToastNotification('途经点设置成功', 'success')
  }

  hideContextMenu()
}

// 更新起点标记
const updateStartMarker = () => {
  if (startMarker) {
    map.remove(startMarker)
  }

  if (routeData.start) {
    startMarker = new AMap.Marker({
      position: routeData.start,
      icon: new AMap.Icon({
        size: new AMap.Size(25, 34),
        image: 'https://webapi.amap.com/theme/v1.3/markers/n/mark_g.png'
      }),
      title: '起点'
    })
    map.add(startMarker)
  }
}

// 更新终点标记
const updateEndMarker = () => {
  if (endMarker) {
    map.remove(endMarker)
  }

  if (routeData.end) {
    endMarker = new AMap.Marker({
      position: routeData.end,
      icon: new AMap.Icon({
        size: new AMap.Size(25, 34),
        image: 'https://webapi.amap.com/theme/v1.3/markers/n/mark_r.png'
      }),
      title: '终点'
    })
    map.add(endMarker)
  }
}

// 更新途经点标记
const updateWaypointMarkers = () => {
  waypointMarkers.forEach(marker => {
    map.remove(marker)
  })
  waypointMarkers = []

  routeData.waypoints.forEach((waypoint, index) => {
    const marker = new AMap.Marker({
      position: waypoint,
      icon: new AMap.Icon({
        size: new AMap.Size(25, 34),
        image: 'https://webapi.amap.com/theme/v1.3/markers/n/mark_b.png'
      }),
      title: `途经点 ${index + 1}`
    })
    map.add(marker)
    waypointMarkers.push(marker)
  })
}

// 规划路线
const planRoute = async () => {
  if (!canPlanRoute.value) {
    showToastNotification('请先设置起点和终点', 'warning')
    return
  }

  if (routePolyline) {
    map.remove(routePolyline)
    routePolyline = null
  }

  let loading = null
  try {
    loading = showLoading('正在规划路线...')

    const origin = `${routeData.start[0]},${routeData.start[1]}`
    const destination = `${routeData.end[0]},${routeData.end[1]}`
    const strategy = routeOptions.strategy

    console.log('路线规划参数:', {
      origin,
      destination,
      strategy,
      startCoords: routeData.start,
      endCoords: routeData.end
    })

    const params = new URLSearchParams({
      origin: origin,
      destination: destination,
      strategy: strategy
    })

    const requestUrl = `http://localhost:8081/api/v1/map/route?${params}`
    console.log('请求URL:', requestUrl)

    const response = await fetch(requestUrl, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
      },
    })

    console.log('响应状态:', response.status, response.statusText)

    if (!response.ok) {
      throw new Error(`HTTP错误: ${response.status} ${response.statusText}`)
    }

    const result = await response.json()
    console.log('API返回结果:', result)

    if (loading) {
      loading.close()
      loading = null
    }

    if (result.success && result.data) {
      console.log('路线数据:', result.data)

      routeInfo.value = {
        distance: result.data.distance,
        duration: result.data.duration,
        segments: result.data.steps ? result.data.steps.length : 0
      }

      const path = result.data.coordinates
      console.log('路线坐标点数量:', path ? path.length : 0)

      if (!path || !Array.isArray(path) || path.length < 2) {
        console.error('路线坐标数据无效:', path)
        showToastNotification('路线数据异常：至少需要2个坐标点', 'error')
        return
      }

      const isValidCoordinate = (coord) => {
        if (!coord || typeof coord !== 'object') return false
        const lng = typeof coord.lng === 'number' ? coord.lng : (typeof coord[0] === 'number' ? coord[0] : null)
        const lat = typeof coord.lat === 'number' ? coord.lat : (typeof coord[1] === 'number' ? coord[1] : null)

        return lng !== null && lat !== null &&
               lng >= -180 && lng <= 180 &&
               lat >= -90 && lat <= 90
      }

      const validPath = path.filter(isValidCoordinate)
      if (validPath.length < 2) {
        console.error('有效坐标点不足:', validPath.length)
        showToastNotification(`路线坐标验证失败：仅有${validPath.length}个有效坐标点`, 'error')
        return
      }

      if (validPath.length < path.length) {
        console.warn(`已过滤${path.length - validPath.length}个无效坐标点`)
      }

      routePolyline = new AMap.Polyline({
        path: validPath,
        strokeColor: '#0071E3',
        strokeWeight: 6,
        strokeOpacity: 0.8
      })

      map.add(routePolyline)

      map.setFitView([routePolyline])

      showToastNotification(`路线规划完成（${validPath.length}个坐标点）`, 'success')
    } else {
      console.error('路线规划API返回错误:', result)
      showToastNotification(result.error || '路线规划失败，请检查参数', 'error')
    }
  } catch (error) {
    console.error('路线规划请求失败详细错误:', error)

    if (loading) {
      loading.close()
    }

    showToastNotification(`路线规划失败: ${error.message}`, 'error')
  }
}

// 清除路线
const clearRoute = async () => {
  try {
    const response = await fetch('http://localhost:8081/api/v1/map/markers', {
      method: 'DELETE'
    })

    const result = await response.json()
    if (result.success) {
      console.log('服务器标记点已清除:', result.data)
    } else {
      console.warn('清除服务器标记点失败:', result.error)
    }
  } catch (error) {
    console.error('清除服务器标记点时出错:', error)
  }

  routeData.start = null
  routeData.end = null
  routeData.waypoints = []
  routeInfo.value = null

  if (startMarker) {
    map.remove(startMarker)
    startMarker = null
  }
  if (endMarker) {
    map.remove(endMarker)
    endMarker = null
  }
  waypointMarkers.forEach(marker => {
    map.remove(marker)
  })
  waypointMarkers = []

  if (routePolyline) {
    map.remove(routePolyline)
    routePolyline = null
  }

  hideContextMenu()
  showToastNotification('路线已清除', 'success')
}

// 清除单个路线点位
const clearRoutePoint = (type) => {
  if (type === 'start') {
    routeData.start = null
    if (startMarker) {
      map.remove(startMarker)
      startMarker = null
    }
  } else if (type === 'end') {
    routeData.end = null
    if (endMarker) {
      map.remove(endMarker)
      endMarker = null
    }
  }
}

// 清除途经点
const clearWaypoint = (index) => {
  routeData.waypoints.splice(index, 1)
  updateWaypointMarkers()
}

// 格式化坐标
const formatCoords = (coords) => {
  if (!coords) return ''
  return `${coords[1].toFixed(4)}, ${coords[0].toFixed(4)}`
}

// 加载已有的地图标记点
const loadExistingMarkers = async () => {
  try {
    const response = await fetch('http://localhost:8081/api/v1/map/markers')
    const result = await response.json()

    if (result.success && result.data) {
      console.log('从服务器加载的标记点:', result.data)

      result.data.forEach(marker => {
        const position = [marker.lng, marker.lat]

        if (marker.type === 'start') {
          routeData.start = position
          updateStartMarker()
        } else if (marker.type === 'end') {
          routeData.end = position
          updateEndMarker()
        } else if (marker.type === 'waypoint') {
          routeData.waypoints.push(position)
        }
      })

      if (routeData.waypoints.length > 0) {
        updateWaypointMarkers()
      }
    }
  } catch (error) {
    console.error('加载地图标记点失败:', error)
  }
}

// 初始化设备标记
const initDeviceMarkers = () => {
  devices.value.forEach(device => {
    const marker = new AMap.Marker({
      position: device.position,
      title: device.name,
      icon: getMarkerIcon(device.type),
      offset: new AMap.Pixel(-16, -16),
      zIndex: 100
    })

    marker.on('click', () => {
      selectDevice(device)
    })

    map.add(marker)
  })
}

// 其他方法
const setTool = (tool) => {
  currentTool.value = tool
}

const refreshMap = () => {
  if (map) {
    map.clearMap()
    initMap()
    showToastNotification('地图已刷新', 'success')
  }
}

const centerMap = () => {
  if (map) {
    map.setZoomAndCenter(11, [116.397428, 39.90923])
    showToastNotification('地图已居中', 'success')
  }
}

const toggleLayer = () => {
  showTraffic.value = !showTraffic.value
  showToastNotification(`${showTraffic.value ? '显示' : '隐藏'}交通图层`, 'info')
}

const showFullScreen = () => {
  const elem = mapContainer.value
  if (!document.fullscreenElement) {
    elem.requestFullscreen()
    showToastNotification('进入全屏模式', 'success')
  } else {
    document.exitFullscreen()
    showToastNotification('退出全屏模式', 'success')
  }
}

const zoomIn = () => {
  if (map) map.zoomIn()
}

const zoomOut = () => {
  if (map) map.zoomOut()
}

const changeMapType = (type) => {
  mapType.value = type
  if (map) {
    map.setMapStyle(`amap://styles/${type}`)
    showToastNotification(`切换到${type === 'satellite' ? '卫星图' : type === 'hybrid' ? '混合图' : '路线图'}模式`, 'success')
  }
  showMapTypeMenu.value = false
}

const getCurrentLocation = () => {
  if ('geolocation' in navigator) {
    navigator.geolocation.getCurrentPosition(
      (position) => {
        const { latitude, longitude } = position.coords
        if (map) {
          map.setZoomAndCenter(15, [longitude, latitude])
        }
        showToastNotification('定位成功', 'success')
      },
      () => {
        showToastNotification('获取位置失败', 'error')
      }
    )
  } else {
    showToastNotification('浏览器不支持定位功能', 'error')
  }
}

const selectDevice = (device) => {
  selectedDevice.value = device
  if (map) {
    map.setZoomAndCenter(15, device.position)
  }
}

const applySettings = () => {
  if (map && mapType.value) {
    changeMapType(mapType.value)
  }
  showSettings.value = false
  showToastNotification('设置已应用', 'success')
}

const getMarkerIcon = (type) => {
  const iconMap = {
    'drone': '/icons/drone.png',
    'camera': '/icons/camera.png',
    'radar': '/icons/radar.png'
  }
  return iconMap[type] || '/icons/default.png'
}

// 生命周期钩子
onMounted(() => {
  if (window.AMap) {
    initMap()
  } else {
    const script = document.createElement('script')
    script.type = 'text/javascript'
    script.src = `https://webapi.amap.com/maps?v=2.0&key=1872806f332dab32a1a3dc895b0ad542`
    script.onload = () => {
      initMap()
    }
    document.head.appendChild(script)
  }
})

onUnmounted(() => {
  console.log('[MapComponent] 组件卸载，清理地图资源')

  if (startMarker) {
    map.remove(startMarker)
    startMarker = null
  }
  if (endMarker) {
    map.remove(endMarker)
    endMarker = null
  }
  waypointMarkers.forEach(marker => {
    if (marker) map.remove(marker)
  })
  waypointMarkers = []

  if (routePolyline) {
    map.remove(routePolyline)
    routePolyline = null
  }

  if (map) {
    map.destroy()
    map = null
  }
})
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.map-monitoring {
  display: flex;
  flex-direction: column;
  min-height: 100vh;
  gap: var(--space-6);
  padding: var(--space-6);
  background: #FFFFFF;
}

.main-content {
  display: flex;
  flex: 1;
  gap: var(--space-6);
  min-height: 0;
}

/* 地图容器 */
.map-container {
  flex: 1;
  position: relative;
  min-height: 600px;
}

.map-view {
  width: 100%;
  height: 100%;
  min-height: 600px;
  border-radius: var(--radius-md);
  position: relative;
}

.map-tools {
  position: absolute;
  top: var(--space-4);
  right: var(--space-4);
  z-index: 1000;
  display: flex;
  flex-direction: column;
  gap: var(--space-3);
}

.zoom-controls,
.layer-control,
.location-control {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
  background: var(--color-bg-primary);
  backdrop-filter: blur(10px);
  border-radius: var(--radius-sm);
  padding: var(--space-2);
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  border: 1px solid var(--color-border-default);
}

.tool-button {
  width: 36px;
  height: 36px;
  border: none;
  background: var(--color-bg-secondary);
  border-radius: var(--radius-sm);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
  transition: var(--transition-button);
}

.tool-button:hover {
  background: var(--color-bg-tertiary);
  transform: translateY(-1px);
}

.tool-button:active {
  transform: translateY(0);
}

.map-type-menu {
  position: fixed;
  background: var(--color-bg-primary);
  border-radius: var(--radius-sm);
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.15);
  border: 1px solid var(--color-border-default);
  padding: var(--space-2);
  min-width: 120px;
  z-index: 9999;
}

.map-type-menu .menu-item {
  display: block;
  width: 100%;
  padding: var(--space-2) var(--space-3);
  border: none;
  background: none;
  cursor: pointer;
  text-align: left;
  border-radius: var(--radius-xs);
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
  transition: var(--transition-button);
}

.map-type-menu .menu-item:hover {
  background: var(--color-bg-secondary);
}

.map-info-panel {
  position: absolute;
  bottom: var(--space-4);
  left: var(--space-4);
  background: var(--color-bg-primary);
  backdrop-filter: blur(10px);
  border-radius: var(--radius-sm);
  padding: var(--space-3);
  display: flex;
  gap: var(--space-5);
  font-size: var(--font-size-xs);
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  border: 1px solid var(--color-border-default);
}

.info-item .label {
  color: var(--color-text-secondary);
  margin-right: var(--space-1);
}

.info-item .value {
  color: var(--color-text-primary);
  font-weight: 600;
}

/* 自定义右键菜单 */
.context-menu {
  background: var(--color-bg-primary);
  border-radius: var(--radius-md);
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.15);
  border: 1px solid var(--color-border-default);
  padding: var(--space-2);
  min-width: 160px;
  z-index: 9999;
  position: fixed;
  user-select: none;
}

.menu-item {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  padding: var(--space-2) var(--space-3);
  cursor: pointer;
  transition: var(--transition-button);
  border-radius: var(--radius-xs);
  color: var(--color-text-primary);
  font-size: var(--font-size-sm);
  border: none;
  background: none;
  width: 100%;
  text-align: left;
}

.menu-item:hover {
  background: var(--color-bg-secondary);
}

.menu-item.danger {
  color: #FF3B30;
}

.menu-item.danger:hover {
  background: rgba(255, 59, 48, 0.1);
}

.menu-icon {
  font-size: 16px;
}

.menu-divider {
  height: 1px;
  background: var(--color-border-default);
  margin: var(--space-1) 0;
}

/* 右侧侧边栏 */
.right-sidebar {
  width: 320px;
  display: flex;
  flex-direction: column;
  gap: var(--space-6);
  flex-shrink: 0;
}

/* 设备面板 */
.device-panel {
  flex-shrink: 0;
}

.panel-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-4);
}

.panel-title {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
}

.status-badge {
  display: inline-block;
  padding: 2px var(--space-2);
  border-radius: var(--radius-xs);
  font-size: var(--font-size-xs);
  font-weight: 500;
}

.status-badge.online {
  background: rgba(52, 199, 89, 0.1);
  color: #34C759;
}

.device-list {
  max-height: 300px;
  overflow-y: auto;
}

.device-item {
  display: flex;
  align-items: center;
  gap: var(--space-3);
  padding: var(--space-3);
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: var(--transition-button);
  margin-bottom: var(--space-2);
  border: 1px solid transparent;
}

.device-item:hover {
  background: var(--color-bg-secondary);
}

.device-item.active {
  background: var(--color-bg-secondary);
  border-color: var(--apple-blue);
}

.device-icon {
  width: 44px;
  height: 44px;
  border-radius: var(--radius-sm);
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--color-bg-secondary);
  font-size: 24px;
}

.device-info {
  flex: 1;
  min-width: 0;
}

.device-name {
  font-weight: 600;
  color: var(--color-text-primary);
  margin-bottom: 2px;
  font-size: var(--font-size-sm);
}

.device-status {
  font-size: var(--font-size-xs);
  color: var(--color-text-secondary);
}

.device-signal {
  flex-shrink: 0;
}

.signal-indicator {
  width: 48px;
  height: 48px;
  border-radius: 50%;
  background: conic-gradient(#34C759 0% var(--signal), var(--color-bg-secondary) var(--signal) 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  position: relative;
}

.signal-indicator::before {
  content: '';
  position: absolute;
  width: 36px;
  height: 36px;
  border-radius: 50%;
  background: var(--color-bg-primary);
}

.signal-value {
  position: relative;
  z-index: 1;
  font-size: var(--font-size-xs);
  font-weight: 600;
  color: var(--color-text-primary);
}

/* 路线面板 */
.route-panel {
  flex: 1;
  min-height: 0;
}

.route-points {
  margin-bottom: var(--space-4);
}

.route-point {
  margin-bottom: var(--space-3);
  padding: var(--space-3);
  border-radius: var(--radius-sm);
  border: 1px solid var(--color-border-default);
}

.route-point.start {
  border-color: #34C759;
  background: rgba(52, 199, 89, 0.05);
}

.route-point.waypoint {
  border-color: #0071E3;
  background: rgba(0, 113, 227, 0.05);
}

.route-point.end {
  border-color: #FF3B30;
  background: rgba(255, 59, 48, 0.05);
}

.point-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: var(--space-1);
  font-size: var(--font-size-sm);
  font-weight: 600;
  color: var(--color-text-primary);
}

.clear-button {
  width: 20px;
  height: 20px;
  border: none;
  background: var(--color-bg-secondary);
  border-radius: 50%;
  cursor: pointer;
  font-size: 12px;
  color: var(--color-text-secondary);
  display: flex;
  align-items: center;
  justify-content: center;
  transition: var(--transition-button);
}

.clear-button:hover {
  background: #FF3B30;
  color: white;
}

.point-coords {
  font-size: var(--font-size-xs);
  color: var(--color-text-secondary);
  margin-left: var(--space-5);
}

.route-info {
  margin-bottom: var(--space-4);
  padding: var(--space-3);
  background: var(--color-bg-secondary);
  border-radius: var(--radius-sm);
}

.info-title {
  font-size: var(--font-size-sm);
  font-weight: 600;
  color: var(--color-text-primary);
  margin-bottom: var(--space-2);
}

.info-details {
  display: flex;
  flex-direction: column;
  gap: var(--space-1);
}

.detail-item {
  display: flex;
  justify-content: space-between;
  font-size: var(--font-size-xs);
}

.detail-label {
  color: var(--color-text-secondary);
}

.detail-value {
  color: var(--color-text-primary);
  font-weight: 600;
}

.route-options {
  padding-top: var(--space-3);
  border-top: 1px solid var(--color-border-default);
}

.form-group {
  margin-bottom: var(--space-4);
}

.form-group label {
  display: block;
  font-size: var(--font-size-sm);
  font-weight: 500;
  color: var(--color-text-primary);
  margin-bottom: var(--space-2);
}

.route-select {
  width: 100%;
  height: 36px;
  padding: 0 var(--space-3);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  outline: none;
  transition: var(--transition-input);
  cursor: pointer;
}

.route-select:hover {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.route-select:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
}

.checkbox-group {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.checkbox-label {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  cursor: pointer;
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
}

.checkbox-label input[type="checkbox"] {
  width: 18px;
  height: 18px;
  cursor: pointer;
  accent-color: var(--apple-blue);
}

/* 控制面板 */
.control-panel {
  margin-top: auto;
  flex-shrink: 0;
}

.control-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: var(--space-6);
}

/* 设置对话框 */
.settings-form {
  padding: var(--space-4) 0;
}

.radio-group {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.radio-label {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  cursor: pointer;
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
}

.radio-label input[type="radio"] {
  width: 18px;
  height: 18px;
  cursor: pointer;
  accent-color: var(--apple-blue);
}

.switch-wrapper {
  display: flex;
  align-items: center;
}

.switch {
  position: relative;
  display: inline-block;
  width: 51px;
  height: 31px;
}

.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: var(--color-border-default);
  transition: var(--transition-input);
  border-radius: 31px;
}

.slider:before {
  position: absolute;
  content: "";
  height: 27px;
  width: 27px;
  left: 2px;
  bottom: 2px;
  background-color: white;
  transition: var(--transition-input);
  border-radius: 50%;
}

input:checked + .slider {
  background-color: var(--apple-blue);
}

input:checked + .slider:before {
  transform: translateX(20px);
}

/* 下拉菜单动画 */
.dropdown-enter-active,
.dropdown-leave-active {
  transition: all 0.2s ease;
}

.dropdown-enter-from {
  opacity: 0;
  transform: translateY(-8px);
}

.dropdown-leave-to {
  opacity: 0;
  transform: translateY(-8px);
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .right-sidebar {
    width: 280px;
  }
}

@media (max-width: 768px) {
  .map-monitoring {
    padding: var(--space-4);
    gap: var(--space-4);
  }

  .main-content {
    flex-direction: column;
  }

  .right-sidebar {
    width: 100%;
    flex-direction: row;
    gap: var(--space-3);
  }

  .device-panel,
  .route-panel {
    flex: 1;
  }

  .map-view {
    min-height: 400px;
  }
}

@media (max-width: 480px) {
  .right-sidebar {
    flex-direction: column;
  }
}

/* Toast Notification Styles */
.toast-notification {
  position: fixed;
  top: 24px;
  left: 50%;
  transform: translateX(-50%);
  z-index: 10000;
  display: flex;
  align-items: center;
  gap: var(--space-3);
  padding: var(--space-3) var(--space-5);
  border-radius: var(--radius-md);
  background: var(--color-bg-primary);
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.15);
  border: 1px solid var(--color-border-default);
  backdrop-filter: blur(20px);
  min-width: 280px;
  max-width: 500px;
}

.toast-icon {
  width: 24px;
  height: 24px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  font-weight: bold;
  flex-shrink: 0;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-sm);
  font-weight: 500;
  color: var(--color-text-primary);
  line-height: 1.4;
}

/* Toast Type Styles */
.toast-success .toast-icon {
  background: rgba(52, 199, 89, 0.15);
  color: #34C759;
}

.toast-error .toast-icon {
  background: rgba(255, 59, 48, 0.15);
  color: #FF3B30;
}

.toast-warning .toast-icon {
  background: rgba(255, 149, 0, 0.15);
  color: #FF9500;
}

.toast-info .toast-icon {
  background: rgba(0, 113, 227, 0.15);
  color: #0071E3;
}

/* Toast Animation */
.toast-enter-active,
.toast-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-enter-from {
  opacity: 0;
  transform: translateX(-50%) translateY(-20px) scale(0.95);
}

.toast-leave-to {
  opacity: 0;
  transform: translateX(-50%) translateY(-20px) scale(0.95);
}

/* Loading Overlay */
.loading-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  z-index: 9999;
  background: rgba(0, 0, 0, 0.7);
  backdrop-filter: blur(4px);
  display: flex;
  align-items: center;
  justify-content: center;
}

.loading-content {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: var(--space-4);
  padding: var(--space-8);
  background: var(--color-bg-primary);
  border-radius: var(--radius-lg);
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
}

.loading-spinner {
  width: 48px;
  height: 48px;
  border: 4px solid rgba(0, 113, 227, 0.1);
  border-top-color: #0071E3;
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
}

@keyframes spin {
  to {
    transform: rotate(360deg);
  }
}

.loading-text {
  font-size: var(--font-size-base);
  font-weight: 500;
  color: var(--color-text-primary);
}

/* Fade Animation */
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.3s ease;
}

.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .map-monitoring {
    background: #000000;
  }

  .tool-button,
  .map-info-panel,
  .zoom-controls,
  .layer-control,
  .location-control,
  .context-menu,
  .map-type-menu {
    background: #1C1C1E;
    border-color: #3A3A3C;
  }

  .tool-button:hover {
    background: #2C2C2E;
  }

  .device-icon {
    background: #2C2C2E;
  }

  .route-info {
    background: #1C1C1E;
  }

  .slider {
    background-color: #3A3A3C;
  }

  input:checked + .slider {
    background-color: #0A84FF;
  }

  .route-select {
    background: #1C1C1E;
    border-color: #3A3A3C;
    color: #F5F5F7;
  }

  .route-select:hover {
    background: #2C2C2E;
    border-color: #48484A;
  }

  .route-select:focus {
    border-color: #0A84FF;
    box-shadow: 0 0 0 3px rgba(10, 132, 255, 0.1);
  }

  /* Toast Dark Mode */
  .toast-notification {
    background: rgba(28, 28, 30, 0.95);
    border-color: #3A3A3C;
  }

  .toast-success .toast-icon {
    background: rgba(52, 199, 89, 0.2);
    color: #30D158;
  }

  .toast-error .toast-icon {
    background: rgba(255, 69, 58, 0.2);
    color: #FF453A;
  }

  .toast-warning .toast-icon {
    background: rgba(255, 159, 10, 0.2);
    color: #FF9F0A;
  }

  .toast-info .toast-icon {
    background: rgba(10, 132, 255, 0.2);
    color: #0A84FF;
  }

  /* Loading Dark Mode */
  .loading-content {
    background: #1C1C1E;
  }

  .loading-spinner {
    border-color: rgba(10, 132, 255, 0.1);
    border-top-color: #0A84FF;
  }
}
</style>
