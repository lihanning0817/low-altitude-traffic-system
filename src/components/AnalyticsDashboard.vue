<template>
  <div class="analytics-dashboard">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h2>数据分析仪表盘</h2>
          <p>多维度分析低空交通数据</p>
        </div>
        <div class="date-range">
          <input
            v-model="dateRange[0]"
            type="date"
            class="date-input"
            @change="loadData"
          >
          <span class="date-separator">至</span>
          <input
            v-model="dateRange[1]"
            type="date"
            class="date-input"
            @change="loadData"
          >
        </div>
      </div>
    </div>

    <!-- 概览卡片 -->
    <div class="overview-cards">
      <AppleCard
        hover-effect
        class="stat-card"
      >
        <div class="card-content">
          <div class="card-icon flight">
            <span>✈️</span>
          </div>
          <div class="card-info">
            <div class="card-title">
              总飞行次数
            </div>
            <div class="card-value">
              {{ summary.totalFlights }}
            </div>
            <div class="card-trend" :class="{ positive: summary.flightTrend >= 0, negative: summary.flightTrend < 0 }">
              <span class="trend-icon">{{ summary.flightTrend >= 0 ? '↗' : '↘' }}</span>
              <span class="trend-value">{{ Math.abs(summary.flightTrend) }}%</span>
            </div>
          </div>
        </div>
      </AppleCard>

      <AppleCard
        hover-effect
        class="stat-card"
      >
        <div class="card-content">
          <div class="card-icon timer">
            <span>⏱️</span>
          </div>
          <div class="card-info">
            <div class="card-title">
              平均飞行时长
            </div>
            <div class="card-value">
              {{ summary.avgFlightDuration }}
            </div>
            <div class="card-trend" :class="{ positive: summary.durationTrend >= 0, negative: summary.durationTrend < 0 }">
              <span class="trend-icon">{{ summary.durationTrend >= 0 ? '↗' : '↘' }}</span>
              <span class="trend-value">{{ Math.abs(summary.durationTrend) }}%</span>
            </div>
          </div>
        </div>
      </AppleCard>

      <AppleCard
        hover-effect
        class="stat-card"
      >
        <div class="card-content">
          <div class="card-icon warning">
            <span>⚠️</span>
          </div>
          <div class="card-info">
            <div class="card-title">
              紧急事件
            </div>
            <div class="card-value">
              {{ summary.emergencyCount }}
            </div>
            <div class="card-trend" :class="{ positive: summary.emergencyTrend >= 0, negative: summary.emergencyTrend < 0 }">
              <span class="trend-icon">{{ summary.emergencyTrend >= 0 ? '↗' : '↘' }}</span>
              <span class="trend-value">{{ Math.abs(summary.emergencyTrend) }}%</span>
            </div>
          </div>
        </div>
      </AppleCard>

      <AppleCard
        hover-effect
        class="stat-card"
      >
        <div class="card-content">
          <div class="card-icon health">
            <span>📊</span>
          </div>
          <div class="card-info">
            <div class="card-title">
              系统健康
            </div>
            <div class="card-value">
              {{ summary.systemHealth }}%
            </div>
            <div class="card-trend" :class="{ positive: summary.healthTrend >= 0, negative: summary.healthTrend < 0 }">
              <span class="trend-icon">{{ summary.healthTrend >= 0 ? '↗' : '↘' }}</span>
              <span class="trend-value">{{ Math.abs(summary.healthTrend) }}%</span>
            </div>
          </div>
        </div>
      </AppleCard>
    </div>

    <!-- 主要图表区域 -->
    <div class="main-charts">
      <!-- 左侧图表 -->
      <div class="left-charts">
        <!-- 飞行任务趋势 -->
        <AppleCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">📈 飞行任务趋势</span>
              <select
                v-model="flightTrendType"
                class="chart-select"
                @change="loadData"
              >
                <option value="day">
                  日趋势
                </option>
                <option value="week">
                  周趋势
                </option>
                <option value="month">
                  月趋势
                </option>
              </select>
            </div>
          </template>
          <div
            ref="flightTrendChart"
            class="chart-container"
          />
        </AppleCard>

        <!-- 空域使用率 -->
        <AppleCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">🗺️ 空域使用率</span>
              <select
                v-model="airspaceUsageType"
                class="chart-select"
                @change="loadData"
              >
                <option value="type">
                  按类型
                </option>
                <option value="area">
                  按区域
                </option>
              </select>
            </div>
          </template>
          <div
            ref="airspaceUsageChart"
            class="chart-container"
          />
        </AppleCard>
      </div>

      <!-- 右侧图表 -->
      <div class="right-charts">
        <!-- 紧急事件分布 -->
        <AppleCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">⚠️ 紧急事件分布</span>
              <select
                v-model="emergencyDistributionType"
                class="chart-select"
                @change="loadData"
              >
                <option value="type">
                  按类型
                </option>
                <option value="severity">
                  按严重程度
                </option>
              </select>
            </div>
          </template>
          <div
            ref="emergencyDistributionChart"
            class="chart-container"
          />
        </AppleCard>

        <!-- 设备状态 -->
        <AppleCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">🖥️ 设备状态</span>
              <AppleButton
                variant="secondary"
                size="small"
                @click="refreshData"
              >
                🔄 刷新
              </AppleButton>
            </div>
          </template>
          <div
            ref="deviceStatusChart"
            class="chart-container"
          />
        </AppleCard>
      </div>
    </div>

    <!-- 详细数据表格 -->
    <AppleCard
      hover-effect
      class="data-table"
    >
      <template #header>
        <div class="table-header">
          <span class="table-title">📋 详细数据</span>
          <AppleButton
            variant="secondary"
            size="small"
            @click="exportData"
          >
            💾 导出
          </AppleButton>
        </div>
      </template>
      <div class="table-wrapper">
        <table class="data-table-content">
          <thead>
            <tr>
              <th>日期</th>
              <th>飞行次数</th>
              <th>平均时长</th>
              <th>紧急事件</th>
              <th>系统健康</th>
              <th>天气</th>
              <th>交通密度</th>
              <th>备注</th>
            </tr>
          </thead>
          <tbody>
            <tr
              v-for="(item, index) in detailedData"
              :key="index"
            >
              <td>{{ item.date }}</td>
              <td>{{ item.totalFlights }}</td>
              <td>{{ item.avgDuration }}</td>
              <td>{{ item.emergencyCount }}</td>
              <td>
                <span class="health-badge" :class="getHealthClass(item.systemHealth)">
                  {{ item.systemHealth }}%
                </span>
              </td>
              <td>
                <span class="weather-badge">{{ getWeatherEmoji(item.weatherCondition) }} {{ item.weatherCondition }}</span>
              </td>
              <td>
                <span class="density-badge" :class="`density-${item.trafficDensity}`">
                  {{ item.trafficDensity }}
                </span>
              </td>
              <td class="notes-cell">{{ item.notes || '-' }}</td>
            </tr>
          </tbody>
        </table>
      </div>
    </AppleCard>

    <!-- Toast 通知 UI 组件 -->
    <div class="toast-container">
      <div
        v-for="toast in toastNotifications"
        :key="toast.id"
        :class="['toast-notification', `toast-${toast.type}`, { 'toast-show': toast.show }]"
      >
        <div class="toast-icon">
          <span v-if="toast.type === 'success'">✓</span>
          <span v-else-if="toast.type === 'error'">✕</span>
          <span v-else-if="toast.type === 'warning'">!</span>
          <span v-else>ℹ</span>
        </div>
        <div class="toast-message">{{ toast.message }}</div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import * as echarts from 'echarts'
import { AppleCard, AppleButton } from '@/components/apple'

// Toast 通知状态管理
const toastNotifications = ref([])
let toastIdCounter = 0

const showToastNotification = (message, type = 'success') => {
  const id = toastIdCounter++
  toastNotifications.value.push({
    id,
    message,
    type,
    show: true
  })

  setTimeout(() => {
    const index = toastNotifications.value.findIndex(t => t.id === id)
    if (index !== -1) {
      toastNotifications.value[index].show = false
      setTimeout(() => {
        toastNotifications.value = toastNotifications.value.filter(t => t.id !== id)
      }, 300)
    }
  }, 3000)
}

// 响应式数据
const dateRange = ref(['2024-01-01', '2024-01-31'])
const flightTrendType = ref('day')
const airspaceUsageType = ref('type')
const emergencyDistributionType = ref('type')

const summary = ref({
  totalFlights: 0,
  flightTrend: 0,
  avgFlightDuration: '0m',
  durationTrend: 0,
  emergencyCount: 0,
  emergencyTrend: 0,
  systemHealth: 0,
  healthTrend: 0
})

const detailedData = ref([])

// 图表实例
const flightTrendChart = ref(null)
const airspaceUsageChart = ref(null)
const emergencyDistributionChart = ref(null)
const deviceStatusChart = ref(null)

let flightTrendInstance = null
let airspaceUsageInstance = null
let emergencyDistributionInstance = null
let deviceStatusInstance = null

// 辅助函数
const getHealthClass = (health) => {
  if (health >= 90) return 'health-excellent'
  if (health >= 70) return 'health-good'
  return 'health-warning'
}

const getWeatherEmoji = (weather) => {
  const emojiMap = {
    '晴': '☀️',
    '多云': '☁️',
    '小雨': '🌧️',
    '阴': '☁️',
    '雨': '🌧️',
    '雪': '❄️'
  }
  return emojiMap[weather] || '🌤️'
}

// 方法
const loadData = () => {
  // 模拟加载数据
  loadSummaryData()
  loadDetailedData()
  renderCharts()
  showToastNotification('数据已加载', 'success')
}

const loadSummaryData = () => {
  // 模拟概览数据
  summary.value = {
    totalFlights: Math.floor(Math.random() * 100) + 200,
    flightTrend: (Math.random() * 20 - 10).toFixed(1),
    avgFlightDuration: `${Math.floor(Math.random() * 30) + 15}m`,
    durationTrend: (Math.random() * 20 - 10).toFixed(1),
    emergencyCount: Math.floor(Math.random() * 10),
    emergencyTrend: (Math.random() * 20 - 10).toFixed(1),
    systemHealth: Math.floor(Math.random() * 20) + 80,
    healthTrend: (Math.random() * 20 - 10).toFixed(1)
  }
}

const loadDetailedData = () => {
  // 模拟详细数据
  const data = []
  const startDate = new Date(dateRange.value[0])
  const endDate = new Date(dateRange.value[1])
  const days = Math.ceil((endDate - startDate) / (1000 * 60 * 60 * 24))

  for (let i = 0; i < days; i++) {
    const date = new Date(startDate)
    date.setDate(startDate.getDate() + i)
    const dateString = date.toISOString().split('T')[0]

    data.push({
      date: dateString,
      totalFlights: Math.floor(Math.random() * 50) + 100,
      avgDuration: `${Math.floor(Math.random() * 30) + 10}m`,
      emergencyCount: Math.floor(Math.random() * 5),
      systemHealth: Math.floor(Math.random() * 20) + 80,
      weatherCondition: ['晴', '多云', '小雨'][Math.floor(Math.random() * 3)],
      trafficDensity: ['低', '中', '高'][Math.floor(Math.random() * 3)],
      notes: ''
    })
  }

  detailedData.value = data
}

const renderCharts = () => {
  // 飞行任务趋势
  if (flightTrendChart.value) {
    flightTrendInstance = echarts.init(flightTrendChart.value)
    flightTrendInstance.setOption({
      tooltip: { trigger: 'axis' },
      grid: { left: '3%', right: '4%', bottom: '3%', containLabel: true },
      xAxis: {
        type: 'category',
        data: detailedData.value.map(d => d.date.substring(5)),
        axisLine: { lineStyle: { color: '#D1D1D6' } },
        axisLabel: { color: '#86868B' }
      },
      yAxis: {
        type: 'value',
        axisLine: { lineStyle: { color: '#D1D1D6' } },
        axisLabel: { color: '#86868B' },
        splitLine: { lineStyle: { color: '#F5F5F7' } }
      },
      series: [{
        data: detailedData.value.map(d => d.totalFlights),
        type: 'line',
        smooth: true,
        itemStyle: { color: '#0071E3' },
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgba(0, 113, 227, 0.2)' },
            { offset: 1, color: 'rgba(0, 113, 227, 0)' }
          ])
        }
      }]
    })
  }

  // 空域使用率
  if (airspaceUsageChart.value) {
    airspaceUsageInstance = echarts.init(airspaceUsageChart.value)
    airspaceUsageInstance.setOption({
      tooltip: { trigger: 'item' },
      legend: {
        top: '5%',
        left: 'center',
        textStyle: { color: '#1D1D1F' }
      },
      series: [{
        type: 'pie',
        radius: ['40%', '70%'],
        avoidLabelOverlap: false,
        itemStyle: { borderRadius: 10, borderColor: '#fff', borderWidth: 2 },
        label: { show: false },
        emphasis: { label: { show: true, fontWeight: 'bold', fontSize: 14 } },
        data: [
          { value: 40, name: '城区核心空域', itemStyle: { color: '#0071E3' } },
          { value: 25, name: '交通枢纽空域', itemStyle: { color: '#34C759' } },
          { value: 15, name: '学校周边空域', itemStyle: { color: '#FF9500' } },
          { value: 10, name: '工业区空域', itemStyle: { color: '#AF52DE' } },
          { value: 10, name: '公园空域', itemStyle: { color: '#FF3B30' } }
        ]
      }]
    })
  }

  // 紧急事件分布
  if (emergencyDistributionChart.value) {
    emergencyDistributionInstance = echarts.init(emergencyDistributionChart.value)
    emergencyDistributionInstance.setOption({
      tooltip: { trigger: 'axis', axisPointer: { type: 'shadow' } },
      grid: { left: '3%', right: '4%', bottom: '3%', containLabel: true },
      xAxis: {
        type: 'category',
        data: ['交通事故', '无人机异常', '气象预警', '非法飞行'],
        axisLine: { lineStyle: { color: '#D1D1D6' } },
        axisLabel: { color: '#86868B' }
      },
      yAxis: {
        type: 'value',
        axisLine: { lineStyle: { color: '#D1D1D6' } },
        axisLabel: { color: '#86868B' },
        splitLine: { lineStyle: { color: '#F5F5F7' } }
      },
      series: [{
        data: [
          { value: 8, itemStyle: { color: '#FF3B30' } },
          { value: 5, itemStyle: { color: '#FF9500' } },
          { value: 3, itemStyle: { color: '#FFCC00' } },
          { value: 2, itemStyle: { color: '#34C759' } }
        ],
        type: 'bar',
        barWidth: '50%',
        itemStyle: { borderRadius: [8, 8, 0, 0] }
      }]
    })
  }

  // 设备状态
  if (deviceStatusChart.value) {
    deviceStatusInstance = echarts.init(deviceStatusChart.value)
    deviceStatusInstance.setOption({
      tooltip: { trigger: 'item' },
      legend: {
        top: '5%',
        left: 'center',
        textStyle: { color: '#1D1D1F' }
      },
      series: [{
        type: 'pie',
        radius: ['50%', '70%'],
        avoidLabelOverlap: false,
        itemStyle: { borderRadius: 10, borderColor: '#fff', borderWidth: 2 },
        label: { show: false },
        emphasis: { label: { show: true, fontWeight: 'bold', fontSize: 14 } },
        data: [
          { value: 85, name: '正常', itemStyle: { color: '#34C759' } },
          { value: 10, name: '警告', itemStyle: { color: '#FF9500' } },
          { value: 5, name: '故障', itemStyle: { color: '#FF3B30' } }
        ]
      }]
    })
  }
}

const refreshData = () => {
  loadData()
}

const exportData = () => {
  showToastNotification('数据导出功能正在开发中', 'success')
}

// 生命周期钩子
onMounted(() => {
  // 初始化数据
  loadData()

  // 添加窗口大小变化监听器
  window.addEventListener('resize', () => {
    flightTrendInstance?.resize()
    airspaceUsageInstance?.resize()
    emergencyDistributionInstance?.resize()
    deviceStatusInstance?.resize()
  })
})

onUnmounted(() => {
  // 移除事件监听器
  window.removeEventListener('resize', () => {
    flightTrendInstance?.resize()
    airspaceUsageInstance?.resize()
    emergencyDistributionInstance?.resize()
    deviceStatusInstance?.resize()
  })

  // 销毁图表实例
  flightTrendInstance?.dispose()
  airspaceUsageInstance?.dispose()
  emergencyDistributionInstance?.dispose()
  deviceStatusInstance?.dispose()
})
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.analytics-dashboard {
  display: flex;
  flex-direction: column;
  min-height: 100vh;
  gap: var(--space-6);
  padding: var(--space-6);
  background: #FFFFFF;
}

/* 页面头部 */
.page-header {
  margin-bottom: 0;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.title-section h2 {
  margin: 0 0 var(--space-2) 0;
  font-size: var(--font-size-2xl);
  font-weight: 600;
  color: var(--color-text-primary);
  letter-spacing: var(--letter-spacing-tight);
}

.title-section p {
  margin: 0;
  color: var(--color-text-secondary);
  font-size: var(--font-size-base);
}

/* 日期选择器 */
.date-range {
  display: flex;
  align-items: center;
  gap: var(--space-3);
}

.date-input {
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
}

.date-input:hover {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.date-input:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
  background-color: var(--color-bg-primary);
}

.date-separator {
  color: var(--color-text-secondary);
  font-size: var(--font-size-sm);
}

/* 概览卡片 */
.overview-cards {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: var(--space-4);
}

.stat-card {
  min-height: 140px;
  display: flex;
  align-items: center;
}

.card-content {
  display: flex;
  align-items: center;
  width: 100%;
  gap: var(--space-4);
}

.card-icon {
  width: 64px;
  height: 64px;
  border-radius: var(--radius-md);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 32px;
  flex-shrink: 0;
}

.card-icon.flight {
  background: linear-gradient(135deg, #0071E3 0%, #005BB5 100%);
}

.card-icon.timer {
  background: linear-gradient(135deg, #34C759 0%, #28A745 100%);
}

.card-icon.warning {
  background: linear-gradient(135deg, #FF9500 0%, #FF8000 100%);
}

.card-icon.health {
  background: linear-gradient(135deg, #AF52DE 0%, #9A3FCC 100%);
}

.card-info {
  flex: 1;
  min-width: 0;
}

.card-title {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  margin-bottom: var(--space-1);
}

.card-value {
  font-size: var(--font-size-2xl);
  font-weight: 600;
  color: var(--color-text-primary);
  margin-bottom: var(--space-2);
  line-height: 1.2;
}

.card-trend {
  display: flex;
  align-items: center;
  gap: var(--space-1);
  font-size: var(--font-size-sm);
  font-weight: 500;
}

.card-trend.positive {
  color: #34C759;
}

.card-trend.negative {
  color: #FF3B30;
}

.trend-icon {
  font-size: var(--font-size-base);
}

/* 主要图表区域 */
.main-charts {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--space-6);
}

.left-charts,
.right-charts {
  display: flex;
  flex-direction: column;
  gap: var(--space-6);
}

.chart-card {
  flex: 1;
  min-height: 350px;
}

.chart-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-4);
}

.chart-title {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
}

.chart-select {
  height: 32px;
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

.chart-select:hover {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.chart-select:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
}

.chart-container {
  width: 100%;
  height: 280px;
}

/* 详细数据表格 */
.data-table {
  margin-top: var(--space-4);
}

.table-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-4);
}

.table-title {
  font-size: var(--font-size-lg);
  font-weight: 600;
  color: var(--color-text-primary);
}

.table-wrapper {
  overflow-x: auto;
  border-radius: var(--radius-sm);
  border: 1px solid var(--color-border-default);
}

.data-table-content {
  width: 100%;
  border-collapse: collapse;
  font-size: var(--font-size-sm);
}

.data-table-content thead {
  background: var(--color-bg-secondary);
}

.data-table-content th {
  padding: var(--space-3) var(--space-4);
  text-align: left;
  font-weight: 600;
  color: var(--color-text-primary);
  border-bottom: 1px solid var(--color-border-default);
  white-space: nowrap;
}

.data-table-content td {
  padding: var(--space-3) var(--space-4);
  color: var(--color-text-primary);
  border-bottom: 1px solid var(--color-border-light);
}

.data-table-content tbody tr:last-child td {
  border-bottom: none;
}

.data-table-content tbody tr:hover {
  background: var(--color-bg-secondary);
}

/* 表格徽章 */
.health-badge {
  display: inline-block;
  padding: 2px var(--space-2);
  border-radius: var(--radius-xs);
  font-size: var(--font-size-xs);
  font-weight: 500;
}

.health-badge.health-excellent {
  background: rgba(52, 199, 89, 0.1);
  color: #34C759;
}

.health-badge.health-good {
  background: rgba(255, 149, 0, 0.1);
  color: #FF9500;
}

.health-badge.health-warning {
  background: rgba(255, 59, 48, 0.1);
  color: #FF3B30;
}

.weather-badge {
  display: inline-flex;
  align-items: center;
  gap: var(--space-1);
  font-size: var(--font-size-sm);
}

.density-badge {
  display: inline-block;
  padding: 2px var(--space-2);
  border-radius: var(--radius-xs);
  font-size: var(--font-size-xs);
  font-weight: 500;
}

.density-badge.density-低 {
  background: rgba(52, 199, 89, 0.1);
  color: #34C759;
}

.density-badge.density-中 {
  background: rgba(255, 149, 0, 0.1);
  color: #FF9500;
}

.density-badge.density-高 {
  background: rgba(255, 59, 48, 0.1);
  color: #FF3B30;
}

.notes-cell {
  max-width: 200px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .main-charts {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 768px) {
  .analytics-dashboard {
    padding: var(--space-4);
    gap: var(--space-4);
  }

  .header-content {
    flex-direction: column;
    gap: var(--space-4);
    align-items: flex-start;
  }

  .date-range {
    width: 100%;
    flex-direction: column;
  }

  .date-input {
    width: 100%;
  }

  .overview-cards {
    grid-template-columns: 1fr;
  }

  .card-content {
    flex-direction: column;
    text-align: center;
  }

  .card-info {
    text-align: center;
  }

  .chart-header {
    flex-direction: column;
    gap: var(--space-3);
    align-items: flex-start;
  }

  .chart-select {
    width: 100%;
  }
}

@media (max-width: 480px) {
  .title-section h2 {
    font-size: var(--font-size-xl);
  }

  .table-wrapper {
    border-radius: 0;
    margin: 0 calc(-1 * var(--space-4));
  }
}

/* Toast 通知样式 */
.toast-container {
  position: fixed;
  top: 20px;
  right: 20px;
  z-index: 9999;
  display: flex;
  flex-direction: column;
  gap: var(--space-3);
  pointer-events: none;
}

.toast-notification {
  display: flex;
  align-items: center;
  gap: var(--space-3);
  min-width: 300px;
  padding: var(--space-4);
  background: #FFFFFF;
  border-radius: var(--radius-md);
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.1), 0 2px 8px rgba(0, 0, 0, 0.08);
  opacity: 0;
  transform: translateX(100px);
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
  pointer-events: auto;
  border-left: 4px solid;
}

.toast-notification.toast-show {
  opacity: 1;
  transform: translateX(0);
}

.toast-notification.toast-success {
  border-left-color: #34C759;
}

.toast-notification.toast-error {
  border-left-color: #FF3B30;
}

.toast-notification.toast-warning {
  border-left-color: #FF9500;
}

.toast-notification.toast-info {
  border-left-color: #0071E3;
}

.toast-icon {
  width: 24px;
  height: 24px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  font-weight: 600;
  color: white;
  flex-shrink: 0;
}

.toast-success .toast-icon {
  background: #34C759;
}

.toast-error .toast-icon {
  background: #FF3B30;
}

.toast-warning .toast-icon {
  background: #FF9500;
}

.toast-info .toast-icon {
  background: #0071E3;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
  font-weight: 500;
  line-height: 1.5;
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .analytics-dashboard {
    background: #000000;
  }

  .title-section h2 {
    color: #F5F5F7;
  }

  .title-section p {
    color: #86868B;
  }

  .date-input,
  .chart-select {
    background: #1C1C1E;
    border-color: #3A3A3C;
    color: #F5F5F7;
  }

  .date-input:hover,
  .chart-select:hover {
    background: #2C2C2E;
    border-color: #48484A;
  }

  .date-input:focus,
  .chart-select:focus {
    border-color: #0A84FF;
    box-shadow: 0 0 0 3px rgba(10, 132, 255, 0.1);
  }

  .data-table-content thead {
    background: #1C1C1E;
  }

  .data-table-content th,
  .data-table-content td {
    color: #F5F5F7;
    border-color: #3A3A3C;
  }

  .data-table-content tbody tr:hover {
    background: #1C1C1E;
  }

  .table-wrapper {
    border-color: #3A3A3C;
  }

  .toast-notification {
    background: #1C1C1E;
    box-shadow: 0 10px 40px rgba(0, 0, 0, 0.3), 0 2px 8px rgba(0, 0, 0, 0.2);
  }

  .toast-message {
    color: #F5F5F7;
  }
}
</style>
