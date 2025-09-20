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
          <el-date-picker
            v-model="dateRange"
            type="daterange"
            range-separator="至"
            start-placeholder="开始日期"
            end-placeholder="结束日期"
            format="YYYY-MM-DD"
            value-format="YYYY-MM-DD"
            @change="loadData"
          />
        </div>
      </div>
    </div>

    <!-- 概览卡片 -->
    <div class="overview-cards">
      <el-row :gutter="20">
        <el-col :span="6">
          <SmartCard
            hover-effect
            class="card"
          >
            <div class="card-content">
              <div class="card-icon">
                <el-icon><Position /></el-icon>
              </div>
              <div class="card-info">
                <div class="card-title">
                  总飞行次数
                </div>
                <div class="card-value">
                  {{ summary.totalFlights }}
                </div>
                <div class="card-trend">
                  <el-icon :color="summary.flightTrend >= 0 ? '#67c23a' : '#f56c6c'">
                    <component :is="summary.flightTrend >= 0 ? 'CaretTop' : 'CaretBottom'" />
                  </el-icon>
                  <span :style="{ color: summary.flightTrend >= 0 ? '#67c23a' : '#f56c6c' }">{{ summary.flightTrend }}%</span>
                </div>
              </div>
            </div>
          </SmartCard>
        </el-col>

        <el-col :span="6">
          <SmartCard
            hover-effect
            class="card"
          >
            <div class="card-content">
              <div class="card-icon">
                <el-icon><Timer /></el-icon>
              </div>
              <div class="card-info">
                <div class="card-title">
                  平均飞行时长
                </div>
                <div class="card-value">
                  {{ summary.avgFlightDuration }}
                </div>
                <div class="card-trend">
                  <el-icon :color="summary.durationTrend >= 0 ? '#67c23a' : '#f56c6c'">
                    <component :is="summary.durationTrend >= 0 ? 'CaretTop' : 'CaretBottom'" />
                  </el-icon>
                  <span :style="{ color: summary.durationTrend >= 0 ? '#67c23a' : '#f56c6c' }">{{ summary.durationTrend }}%</span>
                </div>
              </div>
            </div>
          </SmartCard>
        </el-col>

        <el-col :span="6">
          <SmartCard
            hover-effect
            class="card"
          >
            <div class="card-content">
              <div class="card-icon">
                <el-icon><Warning /></el-icon>
              </div>
              <div class="card-info">
                <div class="card-title">
                  紧急事件
                </div>
                <div class="card-value">
                  {{ summary.emergencyCount }}
                </div>
                <div class="card-trend">
                  <el-icon :color="summary.emergencyTrend >= 0 ? '#67c23a' : '#f56c6c'">
                    <component :is="summary.emergencyTrend >= 0 ? 'CaretTop' : 'CaretBottom'" />
                  </el-icon>
                  <span :style="{ color: summary.emergencyTrend >= 0 ? '#67c23a' : '#f56c6c' }">{{ summary.emergencyTrend }}%</span>
                </div>
              </div>
            </div>
          </SmartCard>
        </el-col>

        <el-col :span="6">
          <SmartCard
            hover-effect
            class="card"
          >
            <div class="card-content">
              <div class="card-icon">
                <el-icon><DataLine /></el-icon>
              </div>
              <div class="card-info">
                <div class="card-title">
                  系统健康
                </div>
                <div class="card-value">
                  {{ summary.systemHealth }}%
                </div>
                <div class="card-trend">
                  <el-icon :color="summary.healthTrend >= 0 ? '#67c23a' : '#f56c6c'">
                    <component :is="summary.healthTrend >= 0 ? 'CaretTop' : 'CaretBottom'" />
                  </el-icon>
                  <span :style="{ color: summary.healthTrend >= 0 ? '#67c23a' : '#f56c6c' }">{{ summary.healthTrend }}%</span>
                </div>
              </div>
            </div>
          </SmartCard>
        </el-col>
      </el-row>
    </div>

    <!-- 主要图表区域 -->
    <div class="main-charts">
      <!-- 左侧图表 -->
      <div class="left-charts">
        <!-- 飞行任务趋势 -->
        <SmartCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">飞行任务趋势</span>
              <el-select
                v-model="flightTrendType"
                size="small"
                style="width: 120px"
                @change="loadData"
              >
                <el-option
                  label="日趋势"
                  value="day"
                />
                <el-option
                  label="周趋势"
                  value="week"
                />
                <el-option
                  label="月趋势"
                  value="month"
                />
              </el-select>
            </div>
          </template>
          <div
            ref="flightTrendChart"
            class="chart-container"
          />
        </SmartCard>

        <!-- 空域使用率 -->
        <SmartCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">空域使用率</span>
              <el-select
                v-model="airspaceUsageType"
                size="small"
                style="width: 120px"
                @change="loadData"
              >
                <el-option
                  label="按类型"
                  value="type"
                />
                <el-option
                  label="按区域"
                  value="area"
                />
              </el-select>
            </div>
          </template>
          <div
            ref="airspaceUsageChart"
            class="chart-container"
          />
        </SmartCard>
      </div>

      <!-- 右侧图表 -->
      <div class="right-charts">
        <!-- 紧急事件分布 -->
        <SmartCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">紧急事件分布</span>
              <el-select
                v-model="emergencyDistributionType"
                size="small"
                style="width: 120px"
                @change="loadData"
              >
                <el-option
                  label="按类型"
                  value="type"
                />
                <el-option
                  label="按严重程度"
                  value="severity"
                />
              </el-select>
            </div>
          </template>
          <div
            ref="emergencyDistributionChart"
            class="chart-container"
          />
        </SmartCard>

        <!-- 设备状态 -->
        <SmartCard
          hover-effect
          class="chart-card"
        >
          <template #header>
            <div class="chart-header">
              <span class="chart-title">设备状态</span>
              <el-button
                size="small"
                @click="refreshData"
              >
                <el-icon><Refresh /></el-icon>
                刷新
              </el-button>
            </div>
          </template>
          <div
            ref="deviceStatusChart"
            class="chart-container"
          />
        </SmartCard>
      </div>
    </div>

    <!-- 详细数据表格 -->
    <SmartCard
      hover-effect
      class="data-table"
    >
      <template #header>
        <div class="table-header">
          <span class="table-title">详细数据</span>
          <el-button
            size="small"
            @click="exportData"
          >
            <el-icon><Download /></el-icon>
            导出
          </el-button>
        </div>
      </template>
      <el-table
        :data="detailedData"
        style="width: 100%"
        stripe
      >
        <el-table-column
          prop="date"
          label="日期"
          width="120"
        />
        <el-table-column
          prop="totalFlights"
          label="飞行次数"
          width="100"
        />
        <el-table-column
          prop="avgDuration"
          label="平均时长"
          width="100"
        />
        <el-table-column
          prop="emergencyCount"
          label="紧急事件"
          width="100"
        />
        <el-table-column
          prop="systemHealth"
          label="系统健康"
          width="100"
        />
        <el-table-column
          prop="weatherCondition"
          label="天气"
          width="100"
        />
        <el-table-column
          prop="trafficDensity"
          label="交通密度"
          width="100"
        />
        <el-table-column
          prop="notes"
          label="备注"
        />
      </el-table>
    </SmartCard>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import { Position, Timer, Warning, DataLine, Refresh, Download } from '@element-plus/icons-vue'
import * as echarts from 'echarts'
import SmartCard from '@/components/SmartCard.vue'


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

// 方法
const loadData = () => {
  // 模拟加载数据
  loadSummaryData()
  loadDetailedData()
  renderCharts()
  ElMessage.success('数据已加载')
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
      xAxis: { type: 'category', data: detailedData.value.map(d => d.date) },
      yAxis: { type: 'value' },
      series: [{ data: detailedData.value.map(d => d.totalFlights), type: 'line', smooth: true }]
    })
  }

  // 空域使用率
  if (airspaceUsageChart.value) {
    airspaceUsageInstance = echarts.init(airspaceUsageChart.value)
    airspaceUsageInstance.setOption({
      tooltip: { trigger: 'item' },
      legend: { top: '5%', left: 'center' },
      series: [{
        type: 'pie',
        radius: ['40%', '70%'],
        avoidLabelOverlap: false,
        itemStyle: { borderRadius: 10, borderColor: '#fff', borderWidth: 2 },
        label: { show: false },
        emphasis: { label: { show: true } },
        data: [
          { value: 40, name: '城区核心空域' },
          { value: 25, name: '交通枢纽空域' },
          { value: 15, name: '学校周边空域' },
          { value: 10, name: '工业区空域' },
          { value: 10, name: '公园空域' }
        ]
      }]
    })
  }

  // 紧急事件分布
  if (emergencyDistributionChart.value) {
    emergencyDistributionInstance = echarts.init(emergencyDistributionChart.value)
    emergencyDistributionInstance.setOption({
      tooltip: { trigger: 'axis', axisPointer: { type: 'shadow' } },
      xAxis: { type: 'category', data: ['交通事故', '无人机异常', '气象预警', '非法飞行'] },
      yAxis: { type: 'value' },
      series: [{ data: [8, 5, 3, 2], type: 'bar' }]
    })
  }

  // 设备状态
  if (deviceStatusChart.value) {
    deviceStatusInstance = echarts.init(deviceStatusChart.value)
    deviceStatusInstance.setOption({
      tooltip: { trigger: 'item' },
      legend: { top: '5%', left: 'center' },
      series: [{
        type: 'pie',
        radius: ['50%', '70%'],
        avoidLabelOverlap: false,
        itemStyle: { borderRadius: 10, borderColor: '#fff', borderWidth: 2 },
        label: { show: false },
        emphasis: { label: { show: true } },
        data: [
          { value: 85, name: '正常', itemStyle: { color: '#67c23a' } },
          { value: 10, name: '警告', itemStyle: { color: '#e6a23c' } },
          { value: 5, name: '故障', itemStyle: { color: '#f56c6c' } }
        ]
      }]
    })
  }
}

const refreshData = () => {
  loadData()
}

const exportData = () => {
  ElMessage.success('数据导出功能正在开发中')
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
.analytics-dashboard {
  display: flex;
  flex-direction: column;
  height: 100%;
  gap: 20px;
}

/* 页面头部 */
.page-header {
  margin-bottom: 0;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 0;
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

/* 概览卡片 */
.overview-cards {
  margin-bottom: 20px;
}

.card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
  height: 120px;
  display: flex;
  align-items: center;
  padding: 0;
}

.card-content {
  display: flex;
  align-items: center;
  width: 100%;
  padding: 20px;
}

.card-icon {
  width: 50px;
  height: 50px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  font-size: 24px;
  margin-right: 16px;
}

.card-info {
  flex: 1;
}

.card-title {
  font-size: 14px;
  color: #7f8c8d;
  margin-bottom: 4px;
}

.card-value {
  font-size: 24px;
  font-weight: 600;
  color: #2c3e50;
  margin-bottom: 4px;
}

.card-trend {
  display: flex;
  align-items: center;
  font-size: 12px;
}

/* 主要图表区域 */
.main-charts {
  display: flex;
  gap: 20px;
  flex: 1;
}

.left-charts,
.right-charts {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.chart-card {
  border-radius: 16px;
  border: 1px solid #e8eaec;
  flex: 1;
}

.chart-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.chart-title {
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
}

.chart-container {
  width: 100%;
  height: 250px;
  margin-top: 16px;
}

/* 详细数据表格 */
.data-table {
  border-radius: 16px;
  border: 1px solid #e8eaec;
}

.table-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.table-title {
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .main-charts {
    flex-direction: column;
  }
}

@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
  }

  .overview-cards .el-col {
    margin-bottom: 12px;
  }

  .date-range {
    width: 100%;
  }

  .card-content {
    flex-direction: column;
    text-align: center;
  }

  .card-icon {
    margin-right: 0;
    margin-bottom: 12px;
  }

  .card-info {
    text-align: center;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-card__body) {
  padding: 20px;
}

:deep(.el-table th) {
  background: #f8f9fa;
}
</style>
