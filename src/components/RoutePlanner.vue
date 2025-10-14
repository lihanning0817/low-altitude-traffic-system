<template>
  <div class="route-planner">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h2>路径规划</h2>
          <p>为无人机规划基于城市主干道的最优飞行路径</p>
        </div>
      </div>
    </div>

    <!-- 主要内容区域 -->
    <div class="main-content">
      <!-- 左侧控制面板 -->
      <div class="left-panel">
        <!-- 规划控制面板 -->
        <SmartCard
          hover-effect
          class="planning-panel"
        >
          <el-card
            class="control-card"
            shadow="never"
          >
            <template #header>
              <div class="card-header">
                <span>路径规划设置</span>
              </div>
            </template>

            <el-form
              :model="planningForm"
              label-width="100px"
            >
              <el-row :gutter="20">
                <el-col :span="12">
                  <el-form-item label="起点">
                    <el-select
                      v-model="planningForm.start"
                      placeholder="请选择起点"
                      filterable
                    >
                      <el-option
                        v-for="node in roadNodes"
                        :key="node.id"
                        :label="node.name"
                        :value="node.id"
                      />
                    </el-select>
                  </el-form-item>
                </el-col>

                <el-col :span="12">
                  <el-form-item label="终点">
                    <el-select
                      v-model="planningForm.end"
                      placeholder="请选择终点"
                      filterable
                    >
                      <el-option
                        v-for="node in roadNodes"
                        :key="node.id"
                        :label="node.name"
                        :value="node.id"
                      />
                    </el-select>
                  </el-form-item>
                </el-col>
              </el-row>

              <el-row :gutter="20">
                <el-col :span="12">
                  <el-form-item label="优先级">
                    <el-select v-model="planningForm.priority">
                      <el-option
                        label="最短距离"
                        value="shortest"
                      />
                      <el-option
                        label="最快时间"
                        value="fastest"
                      />
                      <el-option
                        label="最安全"
                        value="safest"
                      />
                    </el-select>
                  </el-form-item>
                </el-col>

                <el-col :span="12">
                  <el-form-item label="飞行高度">
                    <el-slider
                      v-model="planningForm.altitude"
                      :min="50"
                      :max="300"
                      :step="10"
                      show-input
                    />
                  </el-form-item>
                </el-col>
              </el-row>

              <div class="form-actions">
                <el-button
                  type="primary"
                  :loading="planning"
                  size="large"
                  @click="planRoute"
                >
                  <el-icon><Position /></el-icon>
                  规划路径
                </el-button>

                <el-button
                  size="large"
                  @click="resetForm"
                >
                  <el-icon><Refresh /></el-icon>
                  重置
                </el-button>
              </div>
            </el-form>
          </el-card>
        </SmartCard>

        <!-- 历史路径 -->
        <SmartCard
          hover-effect
          class="history-section"
        >
          <el-card
            class="history-card"
            shadow="never"
          >
            <template #header>
              <div class="card-header">
                <span>历史路径</span>
              </div>
            </template>

            <el-table
              :data="routes"
              style="width: 100%"
            >
              <el-table-column
                prop="id"
                label="路径ID"
                width="120"
              />
              <el-table-column
                prop="start"
                label="起点"
              />
              <el-table-column
                prop="end"
                label="终点"
              />
              <el-table-column
                prop="distance"
                label="距离(km)"
                width="120"
              />
              <el-table-column
                prop="createTime"
                label="创建时间"
                width="180"
              />
              <el-table-column
                label="操作"
                width="150"
              >
                <template #default="{ row }">
                  <el-button
                    type="primary"
                    size="small"
                    @click="loadRoute(row)"
                  >
                    <el-icon><View /></el-icon>
                    查看
                  </el-button>
                </template>
              </el-table-column>
            </el-table>
          </el-card>
        </SmartCard>
      </div>

      <!-- 右侧地图和结果 -->
      <div class="right-panel">
        <!-- 地图容器 -->
        <SmartCard
          hover-effect
          class="map-container"
        >
          <el-card
            class="map-card"
            shadow="never"
          >
            <div
              ref="mapContainer"
              class="map-view"
            />
          </el-card>
        </SmartCard>

        <!-- 路径结果展示 -->
        <div
          v-if="currentRoute"
          class="route-results"
        >
          <el-card
            class="results-card"
            shadow="never"
          >
            <template #header>
              <div class="card-header">
                <span>规划结果</span>
                <div class="header-actions">
                  <el-button
                    type="success"
                    @click="saveRoute"
                  >
                    <el-icon><CircleCheck /></el-icon>
                    保存路径
                  </el-button>
                  <el-button @click="createFlightTask">
                    <el-icon><Promotion /></el-icon>
                    创建任务
                  </el-button>
                </div>
              </div>
            </template>

            <div class="route-info">
              <el-row :gutter="20">
                <el-col :span="8">
                  <div class="info-item">
                    <div class="info-label">
                      总距离
                    </div>
                    <div class="info-value">
                      {{ (currentRoute.totalDistance || 0).toFixed(2) }} km
                    </div>
                  </div>
                </el-col>

                <el-col :span="8">
                  <div class="info-item">
                    <div class="info-label">
                      预计时间
                    </div>
                    <div class="info-value">
                      {{ (currentRoute.totalTime || calculateETA()).toFixed(1) }} 分钟
                    </div>
                  </div>
                </el-col>

                <el-col :span="8">
                  <div class="info-item">
                    <div class="info-label">
                      节点数量
                    </div>
                    <div class="info-value">
                      {{ (currentRoute.nodes || []).length }} 个
                    </div>
                  </div>
                </el-col>
              </el-row>
            </div>

            <div class="route-details">
              <h4>路径详情</h4>
              <el-timeline>
                <el-timeline-item
                  v-for="(node, index) in (currentRoute.nodes || [])"
                  :key="index"
                  :timestamp="`节点 ${index + 1}`"
                  placement="top"
                  :type="index === 0 ? 'primary' : index === (currentRoute.nodes || []).length - 1 ? 'success' : ''"
                >
                  <el-card shadow="never">
                    <div class="node-info">
                      <div class="node-coords">
                        经纬度: {{ node.lat.toFixed(6) }}, {{ node.lng.toFixed(6) }}
                      </div>
                      <div
                        v-if="node.isMainRoad !== undefined"
                        class="node-type"
                      >
                        <el-tag :type="node.isMainRoad ? 'success' : 'warning'">
                          {{ node.isMainRoad ? '主干道' : '支路' }}
                        </el-tag>
                      </div>
                      <div
                        v-else
                        class="node-type"
                      >
                        <el-tag type="success">
                          路径点
                        </el-tag>
                      </div>
                    </div>
                  </el-card>
                </el-timeline-item>
              </el-timeline>
            </div>
          </el-card>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useStore } from 'vuex'
import { ElMessage, ElMessageBox } from 'element-plus'
import { debounce } from 'lodash-es'
import {
  Position, Refresh, CircleCheck, Promotion, View
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'

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

// 模拟道路节点数据
const roadNodes = ref([
  { id: 'A', name: '市中心广场', lat: 39.9042, lng: 116.4074, isMainRoad: true },
  { id: 'B', name: '火车站', lat: 39.9142, lng: 116.4174, isMainRoad: true },
  { id: 'C', name: '机场', lat: 39.9242, lng: 116.4274, isMainRoad: true },
  { id: 'D', name: '商业区', lat: 39.9342, lng: 116.4374, isMainRoad: true },
  { id: 'E', name: '大学城', lat: 39.9442, lng: 116.4474, isMainRoad: true },
  { id: 'F', name: '科技园', lat: 39.9542, lng: 116.4574, isMainRoad: true }
])

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
      // 这里可以处理地图点击事件，例如选择起点/终点
      // TODO: 实现地图点击选择起点/终点功能
    })

// 将地图实例存储到组件数据中，以便后续使用
    window.routePlannerMap = map
    
    // 监听地图缩放和移动事件
    const debouncedZoomChange = debounce(() => {
      // 可以在这里处理缩放变化
    }, 100)

    const debouncedMoveEnd = debounce(() => {
      // 可以在这里处理地图移动
    }, 100)

    // 移除之前的事件监听器，避免重复绑定
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
        ElMessageBox.alert(`
          <div style="text-align: left;">
            <h4>${title}</h4>
            <p><strong>位置:</strong> ${position[1].toFixed(6)}, ${position[0].toFixed(6)}</p>
            <p><strong>类型:</strong> ${isStart ? '起点' : '终点'}</p>
          </div>
        `, '节点信息', {
          dangerouslyUseHTMLString: true,
          confirmButtonText: '确定'
        })
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
      // 获取路径的边界
      const bounds = new AMap.Bounds(
        new AMap.LngLat(currentRoute.value.nodes[0].lng, currentRoute.value.nodes[0].lat),
        new AMap.LngLat(currentRoute.value.nodes[0].lng, currentRoute.value.nodes[0].lat)
      )

      // 扩展边界以包含所有节点
      currentRoute.value.nodes.forEach(node => {
        bounds.extend(new AMap.LngLat(node.lng, node.lat))
      })

      // 调整地图视野
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
    strokeColor: '#3366FF',
    strokeWeight: 6,
    strokeOpacity: 0.8,
    lineCap: 'round',
    lineJoin: 'round',
    zIndex: 10
  })

  // 添加到地图
  map.add(polyline)

  // 存储折线实例，便于后续清除
  window.routePolyline = polyline

  // 调整视野以包含整个路径
  map.setFitView([polyline])
}

const planRoute = async () => {
  if (!planningForm.value.start || !planningForm.value.end) {
    ElMessage.warning('请选择起点和终点')
    return
  }

  if (planningForm.value.start === planningForm.value.end) {
    ElMessage.warning('起点和终点不能相同')
    return
  }

  // 获取选中的起点和终点节点
  const startNode = roadNodes.value.find(node => node.id === planningForm.value.start)
  const endNode = roadNodes.value.find(node => node.id === planningForm.value.end)

  if (!startNode || !endNode) {
    ElMessage.warning('无法找到选定的起点或终点')
    return
  }

  try {
    planning.value = true
    await store.dispatch('planRoute', {
      start: { id: startNode.id, lng: startNode.lng, lat: startNode.lat },
      end: { id: endNode.id, lng: endNode.lng, lat: endNode.lat },
      useAMap: true // 使用高德地图API
    })
    ElMessage.success('路径规划成功')
    
    // 如果地图已初始化，则在地图上绘制路径
    if (window.routePlannerMap && currentRoute.value) {
      renderRouteOnMap(window.routePlannerMap, currentRoute.value)
    }
  } catch (error) {
    ElMessage.error(`路径规划失败: ${error.message}`)
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
  // 假设无人机平均速度为 50 km/h
  const speed = 50
  const time = (currentRoute.value.totalDistance / speed) * 60
  return time.toFixed(1)
}

const saveRoute = () => {
  ElMessage.success('路径已保存')
}

const createFlightTask = () => {
  ElMessageBox.prompt('请输入任务名称', '创建飞行任务', {
    confirmButtonText: '确定',
    cancelButtonText: '取消'
  }).then(({ value }) => {
    if (value) {
      // 这里应该创建飞行任务
      ElMessage.success(`飞行任务 "${value}" 创建成功`)
    }
  }).catch(() => {
    // 用户取消
  })
}

const loadRoute = (route) => {
  store.commit('SET_CURRENT_ROUTE', route)
  ElMessage.success('路径已加载')
}

// 生命周期
onMounted(() => {
  // 初始化一些示例数据
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
  
  // 初始化地图
  initMap()
})
</script>

<style scoped>
.route-planner {
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

/* 控制面板 */
.planning-panel {
  margin-bottom: 24px;
}

.control-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.form-actions {
  display: flex;
  gap: 12px;
  justify-content: center;
  margin-top: 20px;
}

/* 路径结果 */
.route-results {
  margin-bottom: 24px;
}

.results-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
}

.header-actions {
  display: flex;
  gap: 12px;
}

.route-info {
  margin-bottom: 24px;
}

.info-item {
  text-align: center;
  padding: 16px;
  background: #f8f9fa;
  border-radius: 8px;
}

.info-label {
  font-size: 14px;
  color: #7f8c8d;
  margin-bottom: 8px;
}

.info-value {
  font-size: 20px;
  font-weight: 600;
  color: #2c3e50;
}

.route-details h4 {
  margin: 0 0 16px 0;
  color: #2c3e50;
}

.node-info {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.node-coords {
  font-size: 13px;
  color: #7f8c8d;
}

/* 历史路径 */
.history-section {
  margin-bottom: 24px;
}

.history-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
  }

  .form-actions {
    flex-direction: column;
  }

  .header-actions {
    flex-direction: column;
    gap: 8px;
  }

  .node-info {
    flex-direction: column;
    gap: 8px;
    align-items: flex-start;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-card__body) {
  padding: 24px;
}

:deep(.el-form-item) {
  margin-bottom: 20px;
}

:deep(.el-button) {
  border-radius: 8px;
  font-weight: 500;
}
</style>
