<template>
  <div class="map-monitoring">
    <!-- 页面头部 -->
    <MapHeader :active-devices="activeDevices" />

    <!-- 主要内容区域 -->
    <div class="main-content">
      <!-- 地图容器 -->
      <SmartCard hover-effect class="map-container">
        <el-card shadow="never" class="map-card">
          <!-- 地图工具面板 -->
          <div class="map-tools">
            <div class="zoom-controls">
              <el-button circle size="small" @click="zoomIn">
                <el-icon><Plus /></el-icon>
              </el-button>
              <el-button circle size="small" @click="zoomOut">
                <el-icon><Minus /></el-icon>
              </el-button>
            </div>

            <div class="layer-control">
              <el-dropdown @command="changeMapType">
                <el-button circle size="small">
                  <el-icon><Grid /></el-icon>
                </el-button>
                <template #dropdown>
                  <el-dropdown-menu>
                    <el-dropdown-item command="satellite">卫星图</el-dropdown-item>
                    <el-dropdown-item command="roadmap">路线图</el-dropdown-item>
                    <el-dropdown-item command="hybrid">混合图</el-dropdown-item>
                  </el-dropdown-menu>
                </template>
              </el-dropdown>
            </div>

            <div class="location-control">
              <el-button circle size="small" @click="getCurrentLocation">
                <el-icon><LocationInformation /></el-icon>
              </el-button>
            </div>
          </div>

          <!-- 地图主体 -->
          <div id="map" ref="mapContainer" class="map-view" />

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
          <div
            v-show="contextMenu.visible"
            :style="{
              left: contextMenu.x + 'px',
              top: contextMenu.y + 'px'
            }"
            class="context-menu"
            @click.stop
          >
            <div class="menu-item" @click="setRoutePoint('start')">
              <el-icon><Flag /></el-icon>
              <span>设置为起点</span>
            </div>
            <div class="menu-item" @click="setRoutePoint('waypoint')">
              <el-icon><Location /></el-icon>
              <span>设置为途经点</span>
            </div>
            <div class="menu-item" @click="setRoutePoint('end')">
              <el-icon><Position /></el-icon>
              <span>设置为终点</span>
            </div>
            <div class="menu-divider"></div>
            <div class="menu-item danger" @click="clearRoute">
              <el-icon><Delete /></el-icon>
              <span>清除路线</span>
            </div>
          </div>
        </el-card>
      </SmartCard>

      <!-- 右侧侧边栏 -->
      <div class="right-sidebar">
        <!-- 设备状态面板 -->
        <SmartCard hover-effect class="device-panel">
          <el-card shadow="never" class="device-card">
            <template #header>
              <div class="panel-header">
                <span class="panel-title">设备状态</span>
                <el-tag type="success">{{ devices.length }} 在线</el-tag>
              </div>
            </template>
            <div ref="deviceListRef" class="device-list">
              <div
                v-for="device in devices"
                :key="device.id"
                class="device-item"
                :class="{ active: selectedDevice?.id === device.id }"
                @click="selectDevice(device)"
              >
                <div class="device-icon">
                  <el-icon :size="20" :color="getDeviceColor(device.status)">
                    <component :is="getDeviceIcon(device.type)" />
                  </el-icon>
                </div>
                <div class="device-info">
                  <div class="device-name">{{ device.name }}</div>
                  <div class="device-status">{{ device.status }}</div>
                </div>
                <div class="device-signal">
                  <el-progress
                    type="circle"
                    :percentage="device.signal"
                    :width="30"
                    :show-text="false"
                    :color="getSignalColor(device.signal)"
                  />
                </div>
              </div>
            </div>
          </el-card>
        </SmartCard>

        <!-- 路线规划面板 -->
        <SmartCard hover-effect class="route-panel" v-if="routeData.start || routeData.end || routeData.waypoints.length > 0">
          <el-card shadow="never" class="route-card">
            <template #header>
              <div class="panel-header">
                <span class="panel-title">路线规划</span>
                <el-button size="small" type="primary" @click="planRoute" :disabled="!canPlanRoute">
                  规划路线
                </el-button>
              </div>
            </template>

            <!-- 路线点位信息 -->
            <div class="route-points">
              <div v-if="routeData.start" class="route-point start">
                <div class="point-header">
                  <el-icon><Flag /></el-icon>
                  <span>起点</span>
                  <el-button size="small" text @click="clearRoutePoint('start')">
                    <el-icon><Close /></el-icon>
                  </el-button>
                </div>
                <div class="point-coords">
                  {{ formatCoords(routeData.start) }}
                </div>
              </div>

              <div v-for="(waypoint, index) in routeData.waypoints" :key="index" class="route-point waypoint">
                <div class="point-header">
                  <el-icon><Location /></el-icon>
                  <span>途经点 {{ index + 1 }}</span>
                  <el-button size="small" text @click="clearWaypoint(index)">
                    <el-icon><Close /></el-icon>
                  </el-button>
                </div>
                <div class="point-coords">
                  {{ formatCoords(waypoint) }}
                </div>
              </div>

              <div v-if="routeData.end" class="route-point end">
                <div class="point-header">
                  <el-icon><Position /></el-icon>
                  <span>终点</span>
                  <el-button size="small" text @click="clearRoutePoint('end')">
                    <el-icon><Close /></el-icon>
                  </el-button>
                </div>
                <div class="point-coords">
                  {{ formatCoords(routeData.end) }}
                </div>
              </div>
            </div>

            <!-- 路线信息 -->
            <div v-if="routeInfo" class="route-info">
              <div class="info-title">路线概要</div>
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
              <el-form label-width="60px">
                <el-form-item label="策略">
                  <el-select v-model="routeOptions.strategy" size="small">
                    <el-option label="最快路线" value="0" />
                    <el-option label="最短路线" value="1" />
                    <el-option label="避免拥堵" value="2" />
                  </el-select>
                </el-form-item>
                <el-form-item label="避让">
                  <el-checkbox-group v-model="routeOptions.restrictions">
                    <el-checkbox label="避免收费" size="small" />
                    <el-checkbox label="避免高速" size="small" />
                  </el-checkbox-group>
                </el-form-item>
              </el-form>
            </div>
          </el-card>
        </SmartCard>
      </div>
    </div>

    <!-- 控制面板 -->
    <SmartCard hover-effect class="control-panel">
      <el-card shadow="never" class="control-card">
        <div class="control-content">
          <!-- 左侧工具栏 -->
          <MapToolbar :current-tool="currentTool" @tool-change="setTool" />

          <!-- 中央控制区 -->
          <MapControls
            :show-traffic="showTraffic"
            @refresh="refreshMap"
            @center="centerMap"
            @toggle-traffic="toggleLayer"
          />

          <!-- 右侧操作区 -->
          <MapActions @fullscreen="showFullScreen" @settings="showSettings = true" />
        </div>
      </el-card>
    </SmartCard>

    <!-- 设置对话框 -->
    <el-dialog v-model="showSettings" title="地图设置" width="500px">
      <el-form label-width="120px">
        <el-form-item label="地图类型">
          <el-radio-group v-model="mapType">
            <el-radio value="roadmap">路线图</el-radio>
            <el-radio value="satellite">卫星图</el-radio>
            <el-radio value="hybrid">混合图</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item label="显示交通">
          <el-switch v-model="showTraffic" />
        </el-form-item>
        <el-form-item label="自动刷新">
          <el-switch v-model="autoRefresh" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="showSettings = false">取消</el-button>
        <el-button type="primary" @click="applySettings">应用</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted, onUnmounted, computed, nextTick } from 'vue'
import { ElMessage, ElMessageBox, ElLoading } from 'element-plus'
import {
  Plus, Minus, Grid, LocationInformation, Position, VideoCamera, Radar,
  Flag, Location, Delete, Close
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'
import MapHeader from '@/components/MapHeader.vue'
import MapToolbar from '@/components/MapToolbar.vue'
import MapControls from '@/components/MapControls.vue'
import MapActions from '@/components/MapActions.vue'

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
  strategy: '0', // 0: 最快路线, 1: 最短路线, 2: 避免拥堵
  restrictions: []
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

    // 移除前端路径规划服务初始化，改为调用后端接口

    // 监听地图右键事件（更符合用户习惯）
    map.on('rightclick', (e) => {
      e.originEvent.preventDefault() // 阻止浏览器默认右键菜单
      showContextMenu(e)
    })

    // 也支持双击事件（保持兼容）
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
    console.log('显示菜单，坐标:', e.lnglat) // 调试日志
    contextMenu.lnglat = e.lnglat

    // 使用高德地图 v2.0 推荐的方法将地图坐标转换为屏幕坐标
    const pixel = map.lngLatToContainer(e.lnglat)
    console.log('屏幕坐标:', pixel) // 调试日志

    // 确保坐标有效
    if (pixel && typeof pixel.x === 'number' && typeof pixel.y === 'number') {
      contextMenu.x = Math.max(0, Math.min(pixel.x, window.innerWidth - 200)) // 防止菜单超出屏幕
      contextMenu.y = Math.max(0, Math.min(pixel.y, window.innerHeight - 200))
      contextMenu.visible = true
      console.log('菜单位置:', contextMenu.x, contextMenu.y) // 调试日志

      // 添加全局点击事件监听，点击其他地方时隐藏菜单
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
    // 保存标记点到后端
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
    ElMessage.success('起点设置成功')
  } else if (type === 'end') {
    routeData.end = position
    updateEndMarker()
    ElMessage.success('终点设置成功')
  } else if (type === 'waypoint') {
    routeData.waypoints.push(position)
    updateWaypointMarkers()
    ElMessage.success('途经点设置成功')
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
    ElMessage.warning('请先设置起点和终点')
    return
  }

  // 清除之前的路线
  if (routePolyline) {
    map.remove(routePolyline)
    routePolyline = null
  }

  let loading = null
  try {
    loading = ElLoading.service({
      lock: true,
      text: '正在规划路线...',
      background: 'rgba(0, 0, 0, 0.7)'
    })

    // 构建请求参数
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

    // 调用后端路线规划接口
    const response = await fetch(requestUrl, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
      },
    })

    console.log('响应状态:', response.status, response.statusText)
    console.log('响应头:', Object.fromEntries(response.headers.entries()))

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

      // 创建路线信息
      routeInfo.value = {
        distance: result.data.distance,
        duration: result.data.duration,
        segments: result.data.steps ? result.data.steps.length : 0
      }

      // 绘制路线
      const path = result.data.coordinates
      console.log('路线坐标点数量:', path ? path.length : 0)

      if (path && path.length > 0) {
        routePolyline = new AMap.Polyline({
          path: path,
          strokeColor: '#00AA00',
          strokeWeight: 6,
          strokeOpacity: 0.8
        })

        map.add(routePolyline)

        // 调整地图视野
        map.setFitView([routePolyline])

        ElMessage.success('路线规划完成')
      } else {
        console.error('路线坐标数据为空')
        ElMessage.error('路线数据异常，无法绘制路线')
      }
    } else {
      console.error('路线规划API返回错误:', result)
      ElMessage.error(result.error || '路线规划失败，请检查参数')
    }
  } catch (error) {
    console.error('路线规划请求失败详细错误:', error)
    console.error('错误类型:', error.constructor.name)
    console.error('错误消息:', error.message)
    console.error('错误堆栈:', error.stack)

    if (loading) {
      loading.close()
    }

    ElMessage.error(`路线规划失败: ${error.message}`)
  }
}

// 清除路线
const clearRoute = async () => {
  try {
    // 清除后端标记点
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

  // 清除路线数据
  routeData.start = null
  routeData.end = null
  routeData.waypoints = []
  routeInfo.value = null

  // 清除地图标记
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

  // 清除路线
  if (routePolyline) {
    map.remove(routePolyline)
    routePolyline = null
  }

  hideContextMenu()
  ElMessage.success('路线已清除')
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

      // 更新所有途经点标记
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
    ElMessage.success('地图已刷新')
  }
}

const centerMap = () => {
  if (map) {
    map.setZoomAndCenter(11, [116.397428, 39.90923])
    ElMessage.success('地图已居中')
  }
}

const toggleLayer = () => {
  showTraffic.value = !showTraffic.value
  ElMessage.info(`${showTraffic.value ? '显示' : '隐藏'}交通图层`)
}

const showFullScreen = () => {
  const elem = mapContainer.value
  if (!document.fullscreenElement) {
    elem.requestFullscreen()
    ElMessage.success('进入全屏模式')
  } else {
    document.exitFullscreen()
    ElMessage.success('退出全屏模式')
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
    ElMessage.success(`切换到${type}模式`)
  }
}

const getCurrentLocation = () => {
  if ('geolocation' in navigator) {
    navigator.geolocation.getCurrentPosition(
      (position) => {
        const { latitude, longitude } = position.coords
        if (map) {
          map.setZoomAndCenter(15, [longitude, latitude])
        }
        ElMessage.success('定位成功')
      },
      () => {
        ElMessage.error('获取位置失败')
      }
    )
  } else {
    ElMessage.error('浏览器不支持定位功能')
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
  ElMessage.success('设置已应用')
}

const getDeviceIcon = (type) => {
  const iconMap = {
    'drone': Position,
    'camera': VideoCamera,
    'radar': Radar
  }
  return iconMap[type] || Position
}

const getDeviceColor = (status) => {
  const colorMap = {
    '巡航中': '#409eff',
    '正常': '#67c23a',
    '运行中': '#e6a23c'
  }
  return colorMap[status] || '#909399'
}

const getSignalColor = (signal) => {
  if (signal >= 80) return '#67c23a'
  if (signal >= 60) return '#e6a23c'
  return '#f56c6c'
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
  // 加载高德地图API
  if (window.AMap) {
    initMap()
  } else {
    // 动态加载高德地图API v2.0（移除Driving插件，因为使用后端代理）
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
  if (map) {
    map.destroy()
  }
})
</script>

<style scoped>
.map-monitoring {
  display: flex;
  flex-direction: column;
  height: 100vh;
  gap: 20px;
  padding: 20px;
  background: var(--bg-secondary);
}

.main-content {
  display: flex;
  flex: 1;
  gap: 20px;
  min-height: 0;
}

/* 地图容器 */
.map-container {
  flex: 1;
  position: relative;
}

.map-card {
  border-radius: var(--radius-lg);
  border: 1px solid var(--card-border);
  height: 100%;
  background: var(--card-bg);
}

.map-view {
  width: 100%;
  height: 500px;
  border-radius: var(--radius-md);
  position: relative;
}

.map-tools {
  position: absolute;
  top: 20px;
  right: 20px;
  z-index: 1000;
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.zoom-controls,
.layer-control,
.location-control {
  display: flex;
  flex-direction: column;
  gap: 8px;
  background: var(--card-bg);
  backdrop-filter: blur(10px);
  border-radius: var(--radius-sm);
  padding: 8px;
  box-shadow: var(--shadow-md);
  border: 1px solid var(--border-color);
}

.map-info-panel {
  position: absolute;
  bottom: 20px;
  left: 20px;
  background: var(--card-bg);
  backdrop-filter: blur(10px);
  border-radius: var(--radius-sm);
  padding: 12px;
  display: flex;
  gap: 20px;
  font-size: 12px;
  box-shadow: var(--shadow-md);
  border: 1px solid var(--border-color);
}

.info-item .label {
  color: var(--text-secondary);
  margin-right: 4px;
}

.info-item .value {
  color: var(--text-primary);
  font-weight: 600;
}

/* 自定义右键菜单 */
.context-menu {
  background: var(--card-bg);
  border-radius: var(--radius-md);
  box-shadow: var(--shadow-lg);
  border: 1px solid var(--card-border);
  padding: 8px;
  min-width: 160px;
  z-index: 9999;
  position: fixed;
  user-select: none;
}

.menu-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 12px;
  cursor: pointer;
  transition: all var(--transition-fast);
  border-radius: var(--radius-sm);
  color: var(--text-primary);
  font-size: 14px;
}

.menu-item:hover {
  background: var(--sidebar-item-hover);
  color: var(--primary-color);
}

.menu-item.danger {
  color: var(--danger-color);
}

.menu-item.danger:hover {
  background: rgba(244, 67, 54, 0.1);
}

.menu-divider {
  height: 1px;
  background: var(--border-color);
  margin: 4px 0;
}

/* 右侧侧边栏 */
.right-sidebar {
  width: 300px;
  display: flex;
  flex-direction: column;
  gap: 20px;
  flex-shrink: 0;
}

/* 设备面板 */
.device-panel {
  flex-shrink: 0;
}

.device-card {
  border-radius: var(--radius-lg);
  border: 1px solid var(--card-border);
  background: var(--card-bg);
  max-height: 300px;
  overflow: hidden;
}

.panel-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.panel-title {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
}

.device-list {
  max-height: 200px;
  overflow-y: auto;
}

.device-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px;
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: all var(--transition-fast);
  margin-bottom: 8px;
}

.device-item:hover {
  background: var(--sidebar-item-hover);
}

.device-item.active {
  background: var(--sidebar-item-hover);
  border: 1px solid var(--primary-color);
}

.device-icon {
  width: 40px;
  height: 40px;
  border-radius: var(--radius-sm);
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--bg-secondary);
}

.device-info {
  flex: 1;
}

.device-name {
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 4px;
}

.device-status {
  font-size: 12px;
  color: var(--text-secondary);
}

.device-signal {
  flex-shrink: 0;
}

/* 路线面板 */
.route-panel {
  flex: 1;
  min-height: 0;
}

.route-card {
  border-radius: var(--radius-lg);
  border: 1px solid var(--card-border);
  background: var(--card-bg);
  height: 100%;
  display: flex;
  flex-direction: column;
}

.route-points {
  margin-bottom: 20px;
}

.route-point {
  margin-bottom: 12px;
  padding: 8px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--border-color);
}

.route-point.start {
  border-color: #67c23a;
  background: rgba(103, 194, 58, 0.1);
}

.route-point.waypoint {
  border-color: #409eff;
  background: rgba(64, 158, 255, 0.1);
}

.route-point.end {
  border-color: #f56c6c;
  background: rgba(245, 108, 108, 0.1);
}

.point-header {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 4px;
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
}

.point-coords {
  font-size: 12px;
  color: var(--text-secondary);
  margin-left: 24px;
}

.route-info {
  margin-bottom: 20px;
  padding: 12px;
  background: var(--bg-secondary);
  border-radius: var(--radius-sm);
}

.info-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 8px;
}

.info-details {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.detail-item {
  display: flex;
  justify-content: space-between;
  font-size: 12px;
}

.detail-label {
  color: var(--text-secondary);
}

.detail-value {
  color: var(--text-primary);
  font-weight: 600;
}

.route-options {
  padding-top: 12px;
  border-top: 1px solid var(--border-color);
}

/* 控制面板 */
.control-panel {
  margin-top: auto;
  flex-shrink: 0;
}

.control-card {
  border-radius: var(--radius-lg);
  border: 1px solid var(--card-border);
  background: var(--card-bg);
}

.control-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 20px;
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .right-sidebar {
    width: 280px;
  }
}

@media (max-width: 768px) {
  .main-content {
    flex-direction: column;
  }

  .right-sidebar {
    width: 100%;
    flex-direction: row;
    gap: 12px;
  }

  .device-panel,
  .route-panel {
    flex: 1;
  }

  .map-view {
    height: 400px;
  }
}
</style>