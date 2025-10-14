<template>
  <div class="home-dashboard">
    <!-- 标题和状态 -->
    <div class="dashboard-header">
      <h1>智能低空交通系统</h1>
      <p>实时监控、智能规划、高效管理</p>
    </div>

    <!-- 统计卡片 -->
    <div class="stat-grid">
      <SmartCard
        header="任务统计"
        hoverable
      >
        <div class="stat-item">
          <div class="stat-icon">
            <el-icon><DataLine /></el-icon>
          </div>
          <div class="stat-value">
            {{ totalTasks }}
          </div>
          <div class="stat-label">
            总任务数
          </div>
        </div>
        
        <div class="stat-subgrid">
          <div class="stat-subitem">
            <span class="stat-sublabel">进行中</span>
            <span class="stat-subvalue">{{ runningTasks }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">已完成</span>
            <span class="stat-subvalue">{{ completedTasks }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">待执行</span>
            <span class="stat-subvalue">{{ pendingTasks }}</span>
          </div>
        </div>
      </SmartCard>

      <SmartCard
        header="设备状态"
        hoverable
      >
        <div class="stat-item">
          <div class="stat-icon">
            <el-icon><Pointer /></el-icon>
          </div>
          <div class="stat-value">
            {{ onlineDevices }}
          </div>
          <div class="stat-label">
            在线设备
          </div>
        </div>
        
        <div class="stat-subgrid">
          <div class="stat-subitem">
            <span class="stat-sublabel">无人机</span>
            <span class="stat-subvalue">{{ droneCount }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">摄像头</span>
            <span class="stat-subvalue">{{ cameraCount }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">雷达</span>
            <span class="stat-subvalue">{{ radarCount }}</span>
          </div>
        </div>
      </SmartCard>

      <SmartCard
        header="空域管理"
        hoverable
      >
        <div class="stat-item">
          <div class="stat-icon">
            <el-icon><Location /></el-icon>
          </div>
          <div class="stat-value">
            {{ availableAirspaces.length }}
          </div>
          <div class="stat-label">
            可用空域
          </div>
        </div>
        
        <div class="stat-subgrid">
          <div class="stat-subitem">
            <span class="stat-sublabel">受限制</span>
            <span class="stat-subvalue">{{ restrictedAirspaces.length }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">保护区域</span>
            <span class="stat-subvalue">{{ protectedAirspaces.length }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">开放区域</span>
            <span class="stat-subvalue">{{ permittedAirspaces.length }}</span>
          </div>
        </div>
      </SmartCard>

      <SmartCard
        header="系统健康"
        hoverable
      >
        <div class="stat-item">
          <div class="stat-icon">
            <el-icon><TrendCharts /></el-icon>
          </div>
          <div class="stat-value">
            {{ systemStatusText }}
          </div>
          <div class="stat-label">
            系统状态
          </div>
        </div>
        
        <div class="stat-subgrid">
          <div class="stat-subitem">
            <span class="stat-sublabel">冲突检测</span>
            <span class="stat-subvalue">{{ flightConflictsCount }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">紧急事件</span>
            <span class="stat-subvalue">{{ emergencyEventsCount }}</span>
          </div>
          <div class="stat-subitem">
            <span class="stat-sublabel">未读通知</span>
            <span class="stat-subvalue">{{ unreadNotifications }}</span>
          </div>
        </div>
      </SmartCard>
    </div>

    <!-- 最近活动 -->
    <div class="recent-activities">
      <SmartCard
        header="最近活动"
        full-height
      >
        <div class="activity-list">
          <div
            v-for="(activity, index) in recentActivities"
            :key="index"
            class="activity-item"
          >
            <div class="activity-icon">
              <el-icon :size="20">
                <component :is="getActivityIcon(activity.type)" />
              </el-icon>
            </div>
            <div class="activity-content">
              <div class="activity-title">
                {{ activity.title }}
              </div>
              <div class="activity-description">
                {{ activity.description }}
              </div>
              <div class="activity-time">
                {{ formatTime(activity.timestamp) }}
              </div>
            </div>
            <div class="activity-status">
              <el-tag :type="activity.status === 'success' ? 'success' : 'warning'">
                {{ activity.statusText }}
              </el-tag>
            </div>
          </div>
        </div>
      </SmartCard>
    </div>

    <!-- 快捷操作 -->
    <div class="quick-actions">
      <SmartCard
        header="快捷操作"
        full-height
      >
        <div class="action-grid">
          <div
            class="action-card"
            @click="navigateTo('tasks')"
          >
            <el-icon size="36">
              <DataLine />
            </el-icon>
            <div class="action-label">
              任务中心
            </div>
            <div class="action-desc">
              查看和管理所有飞行任务
            </div>
          </div>
          
          <div
            class="action-card"
            @click="navigateTo('map')"
          >
            <el-icon size="36">
              <MapLocation />
            </el-icon>
            <div class="action-label">
              地图监控
            </div>
            <div class="action-desc">
              实时监控无人机位置和状态
            </div>
          </div>
          
          <div
            class="action-card"
            @click="navigateTo('route-planner')"
          >
            <el-icon size="36">
              <Guide />
            </el-icon>
            <div class="action-label">
              路径规划
            </div>
            <div class="action-desc">
              规划最优飞行路线
            </div>
          </div>
          
          <div
            class="action-card"
            @click="navigateTo('emergency')"
          >
            <el-icon size="36">
              <Warning />
            </el-icon>
            <div class="action-label">
              应急响应
            </div>
            <div class="action-desc">
              处理紧急情况和事故
            </div>
          </div>
          
          <div
            class="action-card"
            @click="navigateTo('airspace')"
          >
            <el-icon size="36">
              <Location />
            </el-icon>
            <div class="action-label">
              空域管理
            </div>
            <div class="action-desc">
              配置和管理空域限制
            </div>
          </div>
          
          <div
            class="action-card"
            @click="navigateTo('weather')"
          >
            <el-icon size="36">
              <Sunny />
            </el-icon>
            <div class="action-label">
              气象监测
            </div>
            <div class="action-desc">
              获取实时天气信息
            </div>
          </div>
        </div>
      </SmartCard>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useStore } from 'vuex'
import { useRouter } from 'vue-router'
import SmartCard from '@/components/SmartCard.vue'
import { DataLine, Pointer, Location, Warning, Sunny, MapLocation, TrendCharts, Guide } from '@element-plus/icons-vue'

const store = useStore()
const router = useRouter()

// 计算属性
const totalTasks = computed(() => store.getters.totalTasks)
const runningTasks = computed(() => store.getters.runningTasks)
const completedTasks = computed(() => store.getters.completedTasks)
const pendingTasks = computed(() => store.getters.pendingTasks)

const onlineDevices = computed(() => store.getters.onlineDevices)
const droneCount = computed(() => store.getters.devicesByType('drone').length)
const cameraCount = computed(() => store.getters.devicesByType('camera').length)
const radarCount = computed(() => store.getters.devicesByType('radar').length)

const availableAirspaces = computed(() => store.getters.availableAirspaces)
const restrictedAirspaces = computed(() => store.getters.restrictedAirspaces)
const protectedAirspaces = computed(() => store.getters.airspaces.filter(a => a.type === 'protected'))
const permittedAirspaces = computed(() => store.getters.airspaces.filter(a => a.type === 'permitted'))

const flightConflictsCount = computed(() => store.getters.flightConflictsCount)
const emergencyEventsCount = computed(() => store.getters.emergencyEventsCount)
const unreadNotifications = computed(() => store.getters.unreadNotifications)
const systemStatusText = computed(() => {
  return store.state.systemStatus === 'online' ? '正常运行' : '异常'
})

// 最近活动数据
const recentActivities = [
  {
    type: 'task',
    title: '新任务创建',
    description: '已创建城区巡航任务T007',
    timestamp: new Date().toISOString(),
    status: 'success',
    statusText: '成功'
  },
  {
    type: 'device',
    title: '设备上线',
    description: 'UAV006无人机已上线并开始巡逻',
    timestamp: new Date(Date.now() - 300000).toISOString(),
    status: 'success',
    statusText: '成功'
  },
  {
    type: 'conflict',
    title: '潜在碰撞风险',
    description: 'UAV001与UAV004在高度120米处存在潜在碰撞风险',
    timestamp: new Date(Date.now() - 600000).toISOString(),
    status: 'warning',
    statusText: '警告'
  },
  {
    type: 'weather',
    title: '气象预警',
    description: '预计未来2小时将有强风天气，风速超过15m/s',
    timestamp: new Date(Date.now() - 900000).toISOString(),
    status: 'warning',
    statusText: '警告'
  },
  {
    type: 'emergency',
    title: '紧急事件报告',
    description: '道路交通事故发生在京港澳高速与二环交汇处',
    timestamp: new Date(Date.now() - 1200000).toISOString(),
    status: 'warning',
    statusText: '警告'
  }
]

// 获取活动图标
const getActivityIcon = (type) => {
  switch(type) {
    case 'task':
      return DataLine
    case 'device':
      return Pointer
    case 'conflict':
      return Warning
    case 'weather':
      return Sunny
    case 'emergency':
      return Warning
    default:
      return TrendCharts
  }
}

// 格式化时间
const formatTime = (timestamp) => {
  const date = new Date(timestamp)
  const now = new Date()
  const diff = now - date
  
  if (diff < 60000) { // 1分钟内
    return '刚刚'
  } else if (diff < 3600000) { // 1小时内
    return `${Math.floor(diff / 60000)}分钟前`
  } else if (diff < 86400000) { // 24小时内
    return `${Math.floor(diff / 3600000)}小时前`
  } else {
    return date.toLocaleDateString() + ' ' + date.toLocaleTimeString()
  }
}

// 导航到页面
const navigateTo = (route) => {
  router.push(`/${route}`)
}
</script>

<style scoped>
.home-dashboard {
  padding: 2rem;
  max-width: 1600px;
  margin: 0 auto;
  width: 100%;
}

.dashboard-header {
  text-align: center;
  margin-bottom: 2.5rem;
}

.dashboard-header h1 {
  font-size: 36px;
  font-weight: 700;
  color: var(--text-primary);
  margin-bottom: 0.5rem;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.dashboard-header p {
  font-size: 18px;
  color: var(--text-secondary);
  font-weight: 500;
}

.stat-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
  gap: 2rem;
  margin-bottom: 2.5rem;
}

.stat-item {
  display: flex;
  align-items: center;
  margin-bottom: 1.5rem;
}

.stat-icon {
  width: 56px;
  height: 56px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  margin-right: 1rem;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
}

.stat-value {
  font-size: 32px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0;
}

.stat-label {
  font-size: 16px;
  color: var(--text-secondary);
  font-weight: 500;
}

.stat-subgrid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 1rem;
  margin-top: 1rem;
}

.stat-subitem {
  text-align: center;
  padding: 0.75rem;
  border-radius: 8px;
  background: rgba(102, 126, 234, 0.05);
}

.stat-sublabel {
  font-size: 13px;
  color: var(--text-tertiary);
  display: block;
}

.stat-subvalue {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  display: block;
}

.recent-activities {
  margin-bottom: 2.5rem;
}

.activity-list {
  max-height: 300px;
  overflow-y: auto;
}

.activity-item {
  display: flex;
  align-items: center;
  padding: 1rem;
  border-bottom: 1px solid var(--border-color);
  transition: background-color 0.2s ease;
}

.activity-item:hover {
  background-color: var(--bg-secondary);
}

.activity-icon {
  width: 40px;
  height: 40px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  margin-right: 1rem;
  background: var(--primary-color);
  color: white;
}

.activity-content {
  flex: 1;
  min-width: 0;
}

.activity-title {
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 4px;
  font-size: 15px;
  line-height: 1.4;
}

.activity-description {
  color: var(--text-secondary);
  font-size: 14px;
  line-height: 1.4;
  margin-bottom: 4px;
  word-wrap: break-word;
  overflow-wrap: break-word;
}

.activity-time {
  color: var(--text-tertiary);
  font-size: 12px;
  margin-top: 4px;
}

.activity-status {
  flex-shrink: 0;
}

.quick-actions {
  margin-bottom: 2rem;
}

.action-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 1.5rem;
}

.action-card {
  padding: 1.5rem;
  border-radius: 12px;
  background: var(--bg-secondary);
  text-align: center;
  cursor: pointer;
  transition: all 0.3s ease;
  border: 1px solid var(--border-color);
}

.action-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 12px 40px rgba(0, 0, 0, 0.1);
  border-color: var(--primary-color);
}

.action-card i {
  color: var(--primary-color);
}

.action-label {
  font-weight: 600;
  color: var(--text-primary);
  margin: 1rem 0 0.5rem 0;
  font-size: 15px;
}

.action-desc {
  font-size: 13px;
  color: var(--text-secondary);
  line-height: 1.4;
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .home-dashboard {
    padding: 1.5rem;
  }
  
  .dashboard-header h1 {
    font-size: 30px;
  }
  
  .dashboard-header p {
    font-size: 16px;
  }
  
  .stat-grid {
    grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  }
  
  .stat-value {
    font-size: 28px;
  }
}

@media (max-width: 768px) {
  .home-dashboard {
    padding: 1rem;
  }
  
  .dashboard-header h1 {
    font-size: 24px;
  }
  
  .dashboard-header p {
    font-size: 14px;
  }
  
  .stat-grid {
    grid-template-columns: 1fr;
  }
  
  .stat-item {
    flex-direction: column;
    text-align: center;
  }
  
  .stat-icon {
    margin-right: 0;
    margin-bottom: 1rem;
  }
  
  .stat-subgrid {
    grid-template-columns: repeat(3, 1fr);
  }
  
  .action-grid {
    grid-template-columns: repeat(2, 1fr);
  }
  
  .action-card {
    padding: 1rem;
  }
  
  .action-label {
    font-size: 14px;
  }
  
  .action-desc {
    font-size: 12px;
  }
}
</style>
