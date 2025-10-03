<template>
  <div class="route-editor">
    <el-card shadow="hover">
      <template #header>
        <div class="header">
          <span class="title">
            <el-icon><MapLocation /></el-icon>
            路径编辑器
          </span>
          <el-button
            v-if="route.points.length > 0"
            type="danger"
            size="small"
            @click="clearRoute"
          >
            清除路径
          </el-button>
        </div>
      </template>

      <!-- 路径点列表 -->
      <div class="route-points">
        <!-- 起点 -->
        <div class="point-item start-point">
          <div class="point-header">
            <el-icon class="point-icon"><Flag /></el-icon>
            <span class="point-label">起点</span>
          </div>
          <div v-if="route.startPoint" class="point-content">
            <div class="coordinates">
              <span class="coord">经度: {{ route.startPoint[0].toFixed(6) }}</span>
              <span class="coord">纬度: {{ route.startPoint[1].toFixed(6) }}</span>
            </div>
            <el-button
              type="text"
              size="small"
              @click="removePoint('start')"
            >
              <el-icon><Delete /></el-icon>
            </el-button>
          </div>
          <div v-else class="point-empty">
            <el-button
              type="primary"
              size="small"
              plain
              @click="startPointSelection('start')"
            >
              点击地图选择起点
            </el-button>
          </div>
        </div>

        <!-- 途经点列表 -->
        <div v-if="route.waypoints.length > 0" class="waypoints-section">
          <div class="section-label">
            <el-icon><Location /></el-icon>
            途经点 ({{ route.waypoints.length }})
          </div>
          <draggable
            v-model="route.waypoints"
            item-key="id"
            @end="onWaypointReorder"
            class="waypoints-list"
          >
            <template #item="{ element, index }">
              <div class="point-item waypoint">
                <div class="point-header">
                  <el-icon class="drag-handle"><Rank /></el-icon>
                  <span class="point-label">途经点 {{ index + 1 }}</span>
                </div>
                <div class="point-content">
                  <div class="coordinates">
                    <span class="coord">经度: {{ element.position[0].toFixed(6) }}</span>
                    <span class="coord">纬度: {{ element.position[1].toFixed(6) }}</span>
                  </div>
                  <el-button
                    type="text"
                    size="small"
                    @click="removeWaypoint(index)"
                  >
                    <el-icon><Delete /></el-icon>
                  </el-button>
                </div>
              </div>
            </template>
          </draggable>
        </div>

        <!-- 添加途经点按钮 -->
        <div class="add-waypoint-btn">
          <el-button
            type="primary"
            plain
            size="small"
            @click="startPointSelection('waypoint')"
          >
            <el-icon><Plus /></el-icon>
            添加途经点
          </el-button>
        </div>

        <!-- 终点 -->
        <div class="point-item end-point">
          <div class="point-header">
            <el-icon class="point-icon"><Position /></el-icon>
            <span class="point-label">终点</span>
          </div>
          <div v-if="route.endPoint" class="point-content">
            <div class="coordinates">
              <span class="coord">经度: {{ route.endPoint[0].toFixed(6) }}</span>
              <span class="coord">纬度: {{ route.endPoint[1].toFixed(6) }}</span>
            </div>
            <el-button
              type="text"
              size="small"
              @click="removePoint('end')"
            >
              <el-icon><Delete /></el-icon>
            </el-button>
          </div>
          <div v-else class="point-empty">
            <el-button
              type="primary"
              size="small"
              plain
              @click="startPointSelection('end')"
            >
              点击地图选择终点
            </el-button>
          </div>
        </div>
      </div>

      <!-- 路径统计信息 -->
      <div v-if="routeInfo" class="route-info">
        <el-divider />
        <div class="info-grid">
          <div class="info-item">
            <span class="label">总距离:</span>
            <span class="value">{{ (routeInfo.distance / 1000).toFixed(2) }} km</span>
          </div>
          <div class="info-item">
            <span class="label">预计时间:</span>
            <span class="value">{{ formatDuration(routeInfo.duration) }}</span>
          </div>
        </div>
      </div>

      <!-- 操作按钮 -->
      <div class="actions">
        <el-button
          type="primary"
          :disabled="!isRouteValid"
          @click="planRoute"
          :loading="planning"
        >
          <el-icon><Guide /></el-icon>
          规划路径
        </el-button>
        <el-button
          type="success"
          :disabled="!isRouteValid || !routeInfo"
          @click="saveRoute"
        >
          <el-icon><CircleCheck /></el-icon>
          保存路径
        </el-button>
      </div>
    </el-card>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'
import { ElMessage } from 'element-plus'
import draggable from 'vuedraggable'
import {
  MapLocation,
  Flag,
  Position,
  Location,
  Plus,
  Delete,
  Rank,
  Guide,
  CircleCheck
} from '@element-plus/icons-vue'

const props = defineProps({
  mapInstance: {
    type: Object,
    default: null
  }
})

const emit = defineEmits(['route-updated', 'route-saved', 'point-selecting'])

// 路径数据
const route = ref({
  startPoint: null,
  endPoint: null,
  waypoints: [],
  points: []
})

// 路径信息
const routeInfo = ref(null)

// 规划状态
const planning = ref(false)

// 点选择模式
const selectingMode = ref(null) // 'start', 'end', 'waypoint'

// 计算属性
const isRouteValid = computed(() => {
  return route.value.startPoint && route.value.endPoint
})

// 开始点选择
const startPointSelection = (type) => {
  selectingMode.value = type
  emit('point-selecting', type)
  ElMessage.info(`请在地图上点击选择${type === 'start' ? '起点' : type === 'end' ? '终点' : '途经点'}`)
}

// 设置点位
const setPoint = (lnglat, type) => {
  const position = [lnglat.lng, lnglat.lat]

  if (type === 'start') {
    route.value.startPoint = position
  } else if (type === 'end') {
    route.value.endPoint = position
  } else if (type === 'waypoint') {
    route.value.waypoints.push({
      id: Date.now(),
      position: position
    })
  }

  selectingMode.value = null
  updateRoutePoints()
  emit('route-updated', route.value)
}

// 移除点位
const removePoint = (type) => {
  if (type === 'start') {
    route.value.startPoint = null
  } else if (type === 'end') {
    route.value.endPoint = null
  }

  updateRoutePoints()
  emit('route-updated', route.value)
}

// 移除途经点
const removeWaypoint = (index) => {
  route.value.waypoints.splice(index, 1)
  updateRoutePoints()
  emit('route-updated', route.value)
}

// 途经点重新排序
const onWaypointReorder = () => {
  updateRoutePoints()
  emit('route-updated', route.value)
}

// 更新所有路径点
const updateRoutePoints = () => {
  const points = []

  if (route.value.startPoint) {
    points.push(route.value.startPoint)
  }

  route.value.waypoints.forEach(wp => {
    points.push(wp.position)
  })

  if (route.value.endPoint) {
    points.push(route.value.endPoint)
  }

  route.value.points = points
}

// 规划路径
const planRoute = async () => {
  if (!isRouteValid.value) {
    ElMessage.warning('请先设置起点和终点')
    return
  }

  planning.value = true

  try {
    // 这里应该调用地图服务的路径规划API
    // 暂时模拟数据
    await new Promise(resolve => setTimeout(resolve, 1000))

    const totalDistance = calculateDistance(route.value.points)

    routeInfo.value = {
      distance: totalDistance,
      duration: totalDistance / 15 // 假设速度15m/s
    }

    ElMessage.success('路径规划成功')
  } catch (error) {
    console.error('路径规划失败:', error)
    ElMessage.error('路径规划失败')
  } finally {
    planning.value = false
  }
}

// 计算距离（简化版）
const calculateDistance = (points) => {
  if (points.length < 2) return 0

  let total = 0
  for (let i = 1; i < points.length; i++) {
    const dx = points[i][0] - points[i - 1][0]
    const dy = points[i][1] - points[i - 1][1]
    total += Math.sqrt(dx * dx + dy * dy) * 111000 // 粗略转换为米
  }

  return total
}

// 格式化时长
const formatDuration = (seconds) => {
  const hours = Math.floor(seconds / 3600)
  const minutes = Math.floor((seconds % 3600) / 60)

  if (hours > 0) {
    return `${hours}小时${minutes}分钟`
  }
  return `${minutes}分钟`
}

// 保存路径
const saveRoute = () => {
  if (!isRouteValid.value || !routeInfo.value) {
    ElMessage.warning('请先规划路径')
    return
  }

  const routeData = {
    start: route.value.startPoint,
    end: route.value.endPoint,
    waypoints: route.value.waypoints.map(wp => wp.position),
    distance: routeInfo.value.distance,
    duration: routeInfo.value.duration
  }

  emit('route-saved', routeData)
  ElMessage.success('路径已保存')
}

// 清除路径
const clearRoute = () => {
  route.value = {
    startPoint: null,
    endPoint: null,
    waypoints: [],
    points: []
  }
  routeInfo.value = null
  emit('route-updated', route.value)
  ElMessage.info('路径已清除')
}

// 暴露方法给父组件
defineExpose({
  setPoint,
  clearRoute,
  selectingMode: () => selectingMode.value
})
</script>

<style scoped lang="scss">
.route-editor {
  .header {
    display: flex;
    justify-content: space-between;
    align-items: center;

    .title {
      display: flex;
      align-items: center;
      gap: 8px;
      font-weight: bold;
    }
  }

  .route-points {
    .point-item {
      margin-bottom: 16px;
      padding: 12px;
      border-radius: 8px;
      border: 2px solid #e4e7ed;

      &.start-point {
        border-color: #67C23A;
        background-color: #f0f9ff;
      }

      &.end-point {
        border-color: #F56C6C;
        background-color: #fef0f0;
      }

      &.waypoint {
        border-color: #409EFF;
        background-color: #ecf5ff;
      }

      .point-header {
        display: flex;
        align-items: center;
        gap: 8px;
        margin-bottom: 8px;

        .point-icon {
          font-size: 18px;
        }

        .point-label {
          font-weight: 600;
          flex: 1;
        }

        .drag-handle {
          cursor: move;
          color: #909399;
        }
      }

      .point-content {
        display: flex;
        justify-content: space-between;
        align-items: center;

        .coordinates {
          display: flex;
          flex-direction: column;
          gap: 4px;

          .coord {
            font-size: 12px;
            color: #606266;
          }
        }
      }

      .point-empty {
        text-align: center;
      }
    }

    .waypoints-section {
      .section-label {
        display: flex;
        align-items: center;
        gap: 8px;
        margin-bottom: 12px;
        font-weight: 600;
        color: #409EFF;
      }

      .waypoints-list {
        .point-item {
          cursor: move;
        }
      }
    }

    .add-waypoint-btn {
      text-align: center;
      margin: 16px 0;
    }
  }

  .route-info {
    .info-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 16px;

      .info-item {
        display: flex;
        flex-direction: column;
        gap: 4px;

        .label {
          font-size: 12px;
          color: #909399;
        }

        .value {
          font-size: 16px;
          font-weight: bold;
          color: #409EFF;
        }
      }
    }
  }

  .actions {
    display: flex;
    gap: 12px;
    margin-top: 16px;

    button {
      flex: 1;
    }
  }
}
</style>
