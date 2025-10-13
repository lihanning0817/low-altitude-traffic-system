<template>
  <div class="weather-integration">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h2>气象监测</h2>
          <p>实时天气数据与飞行风险评估</p>
        </div>
        <div class="action-section">
          <el-button
            type="success"
            size="large"
            @click="autoLocate"
            :loading="locating"
          >
            <el-icon><Location /></el-icon>
            自动定位
          </el-button>
          <el-button
            type="primary"
            size="large"
            @click="refreshWeatherData"
          >
            <el-icon><Refresh /></el-icon>
            刷新数据
          </el-button>
        </div>
      </div>
    </div>

    <!-- 天气统计卡片 -->
    <div class="stats-section">
      <div class="stats-grid">
        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Temperature />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ currentWeather?.weather?.temperature ? currentWeather.weather.temperature.toFixed(1) : '--' }}°C</h3>
            <p>当前温度</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Wind />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ currentWeather?.weather?.wind_speed ? currentWeather.weather.wind_speed.toFixed(1) : '--' }} m/s</h3>
            <p>风速</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Cloud />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ currentWeather?.weather?.condition || '--' }}</h3>
            <p>天气状况</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Warning />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3 v-if="riskAssessment && riskAssessment.overallRisk">{{ getRiskLevelText(riskAssessment.overallRisk) }}</h3>
            <h3 v-else-if="loading">加载中...</h3>
            <h3 v-else>--</h3>
            <p>飞行风险等级</p>
          </div>
        </SmartCard>
      </div>
    </div>

    <!-- 实时天气区域 -->
    <SmartCard
      hover-effect
      class="current-weather-card"
    >
      <template #header>
        <div class="card-header">
          <span class="card-title">实时天气</span>
          <el-tag
            v-if="riskAssessment && riskAssessment.overallRisk"
            :type="getRiskTagType(riskAssessment.overallRisk)"
            size="medium"
          >
            {{ getRiskLevelText(riskAssessment.overallRisk) }}
          </el-tag>
          <el-tag
            v-else-if="loading"
            type="info"
            size="medium"
          >
            加载中...
          </el-tag>
          <el-tag
            v-else
            type="info"
            size="medium"
          >
            暂无数据
          </el-tag>
        </div>
      </template>
      <div class="weather-content">
        <div class="weather-info">
          <div class="weather-location">
            <el-icon
              size="20"
              color="#667eea"
            >
              <Location />
            </el-icon>
            <span>{{ weatherLocation }}</span>
          </div>

          <div class="weather-details">
            <div class="detail-item">
              <span class="label">温度:</span>
              <span class="value">{{ weatherApi.formatTemperature(currentWeather?.weather?.temperature || 0) }}</span>
            </div>

            <div class="detail-item">
              <span class="label">体感温度:</span>
              <span class="value">{{ weatherApi.formatTemperature(currentWeather?.weather?.feels_like || 0) }}</span>
            </div>

            <div class="detail-item">
              <span class="label">湿度:</span>
              <span class="value">{{ weatherApi.formatHumidity(currentWeather?.weather?.humidity || 0) }}</span>
            </div>

            <div class="detail-item">
              <span class="label">气压:</span>
              <span class="value">{{ currentWeather?.weather?.pressure || '--' }} hPa</span>
            </div>

            <div class="detail-item">
              <span class="label">风速:</span>
              <span class="value">{{ weatherApi.formatWindSpeed(currentWeather?.weather?.wind_speed || 0) }}</span>
            </div>

            <div class="detail-item">
              <span class="label">风向:</span>
              <span class="value">{{ weatherApi.getWindDirection(currentWeather?.weather?.wind_direction || 0) }}</span>
            </div>

            <div class="detail-item">
              <span class="label">能见度:</span>
              <span class="value">{{ weatherApi.formatVisibility(currentWeather?.weather?.visibility || 0) }}</span>
            </div>

            <div class="detail-item">
              <span class="label">云量:</span>
              <span class="value">{{ currentWeather?.weather?.cloudiness || '--' }}%</span>
            </div>
          </div>
        </div>

        <div class="weather-condition">
          <div class="condition-icon">
            <img
              :src="weatherApi.getWeatherIconUrl(currentWeather?.weather?.icon || '01d')"
              :alt="currentWeather?.weather?.condition"
              class="weather-icon"
            >
          </div>
          <div class="condition-description">
            <h3>{{ currentWeather?.weather?.condition || '--' }}</h3>
            <p>{{ weatherApi.getWeatherDescription(currentWeather?.weather?.condition || '') }}</p>
          </div>
        </div>
      </div>
    </SmartCard>

    <!-- 风险评估区域 -->
    <SmartCard
      hover-effect
      class="risk-assessment-card"
    >
      <template #header>
        <div class="card-header">
          <span class="card-title">飞行风险评估</span>
          <el-button
            type="primary"
            size="small"
            @click="showRiskDetails = true"
          >
            详细分析
          </el-button>
        </div>
      </template>

      <div class="risk-content">
        <div class="risk-summary">
          <div
            v-for="(risk, index) in riskAssessment.risks"
            :key="index"
            class="risk-item"
          >
            <div
              class="risk-icon"
              :class="getRiskIconClass(risk.level)"
            >
              <el-icon
                :size="18"
                :color="getRiskColor(risk.level)"
              >
                {{ getRiskIcon(risk.type) }}
              </el-icon>
            </div>
            <div class="risk-info">
              <h4>{{ risk.description }}</h4>
              <p class="risk-level">
                {{ getRiskLevelText(risk.level) }}
              </p>
            </div>
          </div>

          <div
            v-for="(warning, index) in riskAssessment.warnings"
            :key="index"
            class="risk-item warning-item"
          >
            <div class="risk-icon warning-icon">
              <el-icon
                :size="18"
                color="#E6A23C"
              >
                {{ getRiskIcon('warning') }}
              </el-icon>
            </div>
            <div class="risk-info">
              <h4>{{ warning.description }}</h4>
            </div>
          </div>
        </div>

        <div class="recommendations">
          <h4>飞行建议</h4>
          <ul>
            <li
              v-for="(recommendation, index) in riskAssessment.recommendations"
              :key="index"
            >
              {{ recommendation }}
            </li>
          </ul>
        </div>
      </div>
    </SmartCard>

    <!-- 路线天气预测 -->
    <SmartCard
      hover-effect
      class="route-weather-card"
    >
      <template #header>
        <div class="card-header">
          <span class="card-title">路线天气预测</span>
          <el-button
            type="primary"
            size="small"
            @click="showRoutePlanner = true"
          >
            规划路线
          </el-button>
        </div>
      </template>

      <div class="route-weather-content">
        <div class="route-selector">
          <el-select
            v-model="selectedRoute"
            placeholder="选择航线"
            size="large"
            clearable
            style="width: 300px;"
          >
            <el-option
              label="机场到市中心航线"
              value="R001"
            />
            <el-option
              label="东城至西城航线"
              value="R002"
            />
            <el-option
              label="城区巡逻航线"
              value="R003"
            />
            <el-option
              label="工业区巡查航线"
              value="R004"
            />
          </el-select>
          
          <el-date-picker
            v-model="forecastTimeRange"
            type="datetimerange"
            range-separator="至"
            start-placeholder="开始时间"
            end-placeholder="结束时间"
            value-format="YYYY-MM-DD HH:mm:ss"
            style="margin-left: 20px;"
            width="300px"
          />
        </div>

        <div
          v-if="routeWeather.length > 0"
          class="route-weather-list"
        >
          <el-table
            :data="routeWeather"
            style="width: 100%"
            :row-class-name="getRowClass"
          >
            <el-table-column
              prop="date"
              label="日期"
              width="120"
              align="center"
            >
              <template #default="{ row }">
                <div>
                  <div style="font-weight: 500;">{{ row.dayName }}</div>
                  <div style="font-size: 12px; color: #909399;">{{ row.dateStr }}</div>
                </div>
              </template>
            </el-table-column>

            <el-table-column
              prop="weather"
              label="天气状况"
              width="150"
              align="center"
            >
              <template #default="{ row }">
                <div class="weather-status">
                  <img
                    :src="getWeatherIconUrl(row.icon)"
                    :alt="row.condition"
                    class="weather-icon-small"
                  >
                  <span>{{ row.condition }}</span>
                </div>
              </template>
            </el-table-column>

            <el-table-column
              prop="temperature"
              label="温度范围"
              width="120"
              align="center"
            >
              <template #default="{ row }">
                <div>
                  <span style="color: #F56C6C; font-weight: 500;">{{ row.temp_max }}°C</span>
                  <span style="margin: 0 5px;">/</span>
                  <span style="color: #409EFF;">{{ row.temp_min }}°C</span>
                </div>
              </template>
            </el-table-column>

            <el-table-column
              prop="wind_speed"
              label="风速"
              width="100"
              align="center"
            >
              <template #default="{ row }">
                <span>{{ row.wind_speed }} m/s</span>
              </template>
            </el-table-column>

            <el-table-column
              prop="humidity"
              label="湿度"
              width="80"
              align="center"
            >
              <template #default="{ row }">
                <span>{{ row.humidity }}%</span>
              </template>
            </el-table-column>

            <el-table-column
              prop="safety_score"
              label="安全评分"
              width="100"
              align="center"
            >
              <template #default="{ row }">
                <el-progress
                  :percentage="row.safety_score"
                  :color="getSafetyScoreColor(row.safety_score)"
                  :stroke-width="8"
                />
              </template>
            </el-table-column>

            <el-table-column
              prop="risk_level"
              label="风险等级"
              width="120"
              align="center"
            >
              <template #default="{ row }">
                <el-tag
                  :type="getRiskTagType(row.risk_level)"
                  size="small"
                >
                  {{ getRiskLevelText(row.risk_level) }}
                </el-tag>
              </template>
            </el-table-column>
          </el-table>
        </div>

        <div
          v-else
          class="no-data"
        >
          <el-empty description="请选择航线查看天气预测" />
        </div>
      </div>
    </SmartCard>

    <!-- 未来天气预报 -->
    <SmartCard
      hover-effect
      class="forecast-card"
    >
      <template #header>
        <div class="card-header">
          <span class="card-title">当日每时段天气变化表（3h）</span>
          <el-tag type="info" size="small">每3小时更新</el-tag>
        </div>
      </template>

      <div class="forecast-content">
        <div class="forecast-grid">
          <div
            v-for="(day, index) in forecast"
            :key="index"
            class="forecast-item"
            :class="{ 'forecast-today': index === 0 }"
          >
            <div class="forecast-date">
              <span class="day">{{ getTimeSlot(index) }}</span>
              <span class="date">{{ getTimeSlotDate(index) }}</span>
            </div>
            
            <div class="forecast-icon">
              <img
                :src="weatherApi.getWeatherIconUrl(day.icon || '01d')"
                :alt="day.condition"
                class="forecast-icon-img"
              >
            </div>

            <div class="forecast-temp">
              <span class="high">{{ day.temp_max ? Math.round(day.temp_max) : '--' }}°C</span>
              <span class="low">{{ day.temp_min ? Math.round(day.temp_min) : '--' }}°C</span>
            </div>

            <div class="forecast-condition">
              {{ day.condition || '--' }}
            </div>

            <div class="forecast-detail">
              <div class="detail-row">
                <span class="label">风速:</span>
                <span class="value">{{ day.wind_speed ? day.wind_speed.toFixed(1) : '--' }} m/s</span>
              </div>
              <div class="detail-row">
                <span class="label">安全评分:</span>
                <span class="value">{{ day.flight_safety?.score || '--' }}</span>
              </div>
              <div class="detail-row">
                <span class="label">湿度:</span>
                <span class="value">{{ day.humidity || '--' }}%</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </SmartCard>

    <!-- 风险详情对话框 -->
    <el-dialog
      v-model="showRiskDetails"
      title="详细风险分析"
      width="800px"
      :close-on-click-modal="false"
    >
      <div class="risk-details-content">
        <div class="risk-section">
          <h3>风险因素</h3>
          <div class="risk-factors">
            <div
              v-for="(risk, index) in riskAssessment.risks"
              :key="index"
              class="risk-factor"
            >
              <div class="risk-type">
                <el-icon
                  :size="20"
                  :color="getRiskColor(risk.level)"
                >
                  {{ getRiskIcon(risk.type) }}
                </el-icon>
                <span>{{ getRiskLevelText(risk.level) }}</span>
              </div>
              <p class="risk-description">
                {{ risk.description }}
              </p>
              <div class="risk-value">
                <span>数值: {{ risk.value }}</span>
                <span>阈值: {{ risk.threshold }}</span>
              </div>
            </div>
          </div>
        </div>

        <div class="risk-section">
          <h3>警告信息</h3>
          <div class="risk-factors">
            <div
              v-for="(warning, index) in riskAssessment.warnings"
              :key="index"
              class="risk-factor"
            >
              <div class="risk-type">
                <el-icon
                  :size="20"
                  :color="getRiskColor(warning.level)"
                >
                  {{ getRiskIcon('warning') }}
                </el-icon>
                <span>{{ getRiskLevelText(warning.level) }}</span>
              </div>
              <p class="risk-description">
                {{ warning.description }}
              </p>
            </div>
          </div>
        </div>

        <div class="risk-section">
          <h3>综合评估</h3>
          <div class="risk-overview">
            <div class="overview-item">
              <span class="label">总体风险等级:</span>
              <el-tag
                :type="getRiskTagType(riskAssessment.overallRisk)"
                size="large"
              >
                {{ getRiskLevelText(riskAssessment.overallRisk) }}
              </el-tag>
            </div>
            <div class="overview-item">
              <span class="label">风险项总数:</span>
              <span class="value">{{ riskAssessment.risks.length + riskAssessment.warnings.length }}</span>
            </div>
            <div class="overview-item">
              <span class="label">高风险项:</span>
              <span class="value">{{ riskAssessment.risks.filter(r => r.level === 'high').length }}</span>
            </div>
          </div>
        </div>

        <div class="risk-section">
          <h3>推荐措施</h3>
          <div class="recommendations-list">
            <div
              v-for="(rec, index) in riskAssessment.recommendations"
              :key="index"
              class="recommendation-item"
            >
              <el-icon
                size="16"
                color="#67c23a"
              >
                <Check />
              </el-icon>
              <span>{{ rec }}</span>
            </div>
          </div>
        </div>
      </div>
      
      <template #footer>
        <el-button @click="showRiskDetails = false">
          关闭
        </el-button>
      </template>
    </el-dialog>

    <!-- 路线规划对话框 -->
    <el-dialog
      v-model="showRoutePlanner"
      title="路线规划"
      width="800px"
      :close-on-click-modal="false"
    >
      <div class="route-planner-content">
        <div class="route-form">
          <el-form label-width="120px">
            <el-form-item label="起点位置">
              <el-input
                v-model="routeStart"
                placeholder="输入起点坐标或地址"
              />
            </el-form-item>
            
            <el-form-item label="终点位置">
              <el-input
                v-model="routeEnd"
                placeholder="输入终点坐标或地址"
              />
            </el-form-item>
            
            <el-form-item label="飞行高度(m)">
              <el-input-number
                v-model="routeAltitude"
                :min="0"
                :max="500"
                :step="10"
                controls-position="right"
                style="width: 100%"
              />
            </el-form-item>
            
            <el-form-item label="预计飞行时间">
              <el-date-picker
                v-model="routeTime"
                type="datetime"
                value-format="YYYY-MM-DD HH:mm:ss"
                style="width: 100%"
              />
            </el-form-item>
          </el-form>
        </div>
        
        <div class="route-preview">
          <div class="preview-header">
            <h4>预览路线</h4>
            <el-button
              type="primary"
              :disabled="!routeStart || !routeEnd"
              @click="generateRoutePreview"
            >
              生成预览
            </el-button>
          </div>
          
          <div
            v-if="routePreview.length > 0"
            class="preview-map"
          >
            <div class="map-container">
              <div class="map-placeholder">
                <el-icon
                  size="48"
                  color="#909399"
                >
                  <Map />
                </el-icon>
                <p>路线预览地图</p>
              </div>
            </div>
          </div>
          
          <div
            v-if="routePreview.length > 0"
            class="route-stats"
          >
            <div class="stat-item">
              <span class="label">总距离:</span>
              <span class="value">{{ routePreviewDistance.toFixed(1) }} km</span>
            </div>
            <div class="stat-item">
              <span class="label">预计时间:</span>
              <span class="value">{{ routePreviewDuration }} 分钟</span>
            </div>
            <div class="stat-item">
              <span class="label">平均速度:</span>
              <span class="value">{{ routePreviewSpeed.toFixed(1) }} km/h</span>
            </div>
            <div class="stat-item">
              <span class="label">最大高度:</span>
              <span class="value">{{ routeAltitude }} m</span>
            </div>
          </div>
        </div>
      </div>
      
      <template #footer>
        <el-button @click="showRoutePlanner = false">
          取消
        </el-button>
        <el-button
          type="primary"
          :disabled="!routeStart || !routeEnd"
          @click="saveRoute"
        >
          保存为新航线
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, computed, watch, nextTick, onBeforeUnmount } from 'vue'
import { ElMessage } from 'element-plus'
import {
  Temperature, Wind, Cloudy, Warning, Location, Refresh,
  Map, Check
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'
import weatherApi from '@/services/weatherApi'

// const store = useStore()

// 响应式数据
const loading = ref(false)
const locating = ref(false)
const showRiskDetails = ref(false)
const showRoutePlanner = ref(false)

const currentWeather = ref(null)
const flightSafetyData = ref(null)
const riskAssessment = ref({
  overallRisk: 'unknown',
  risks: [],
  warnings: [],
  recommendations: ['正在加载天气数据...']
})
const routeWeather = ref([])
const forecast = ref([])
const selectedRoute = ref('')
const forecastTimeRange = ref([])
const currentLocation = ref({ lat: 39.9042, lon: 116.4074 }) // 默认北京坐标

// 用于跟踪正在进行的HTTP请求的AbortController集合
const activeRequests = new Set()

// 创建一个包装器来跟踪Promise请求
const trackRequest = async (promise) => {
  const controller = new AbortController()
  activeRequests.add(controller)

  try {
    const result = await promise
    return result
  } finally {
    activeRequests.delete(controller)
  }
}

const routeStart = ref('')
const routeEnd = ref('')
const routeAltitude = ref(100)
const routeTime = ref(new Date().toISOString())
const routePreview = ref([])

// Mock forecast data for fallback when API fails
const mockForecast = [
  {
    dt: Math.floor(Date.now() / 1000),
    dt_txt: new Date().toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 18.5,
      temp_min: 16.8,
      temp_max: 20.1,
      pressure: 1013,
      humidity: 65
    },
    weather: [
      {
        id: 800,
        main: "Clear",
        description: "晴朗",
        icon: "01d"
      }
    ],
    clouds: {
      all: 10
    },
    wind: {
      speed: 3.5,
      deg: 180,
      gust: 5.2
    },
    visibility: 10000,
    pop: 0.1
  },
  {
    dt: Math.floor((Date.now() + 3600000) / 1000),
    dt_txt: new Date(Date.now() + 3600000).toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 19.2,
      temp_min: 17.5,
      temp_max: 21.0,
      pressure: 1012,
      humidity: 62
    },
    weather: [
      {
        id: 801,
        main: "Few clouds",
        description: "少云",
        icon: "02d"
      }
    ],
    clouds: {
      all: 20
    },
    wind: {
      speed: 4.1,
      deg: 175,
      gust: 6.1
    },
    visibility: 9800,
    pop: 0.15
  },
  {
    dt: Math.floor((Date.now() + 7200000) / 1000),
    dt_txt: new Date(Date.now() + 7200000).toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 19.8,
      temp_min: 18.1,
      temp_max: 21.5,
      pressure: 1011,
      humidity: 58
    },
    weather: [
      {
        id: 802,
        main: "Scattered clouds",
        description: "多云",
        icon: "03d"
      }
    ],
    clouds: {
      all: 40
    },
    wind: {
      speed: 4.8,
      deg: 170,
      gust: 6.8
    },
    visibility: 9500,
    pop: 0.2
  },
  {
    dt: Math.floor((Date.now() + 10800000) / 1000),
    dt_txt: new Date(Date.now() + 10800000).toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 20.1,
      temp_min: 18.5,
      temp_max: 22.0,
      pressure: 1010,
      humidity: 55
    },
    weather: [
      {
        id: 803,
        main: "Broken clouds",
        description: "阴天",
        icon: "04d"
      }
    ],
    clouds: {
      all: 60
    },
    wind: {
      speed: 5.2,
      deg: 165,
      gust: 7.5
    },
    visibility: 9200,
    pop: 0.3
  },
  {
    dt: Math.floor((Date.now() + 14400000) / 1000),
    dt_txt: new Date(Date.now() + 14400000).toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 19.5,
      temp_min: 17.8,
      temp_max: 21.5,
      pressure: 1009,
      humidity: 58
    },
    weather: [
      {
        id: 804,
        main: "Overcast",
        description: "阴",
        icon: "04n"
      }
    ],
    clouds: {
      all: 80
    },
    wind: {
      speed: 4.5,
      deg: 160,
      gust: 6.5
    },
    visibility: 8500,
    pop: 0.4
  },
  {
    dt: Math.floor((Date.now() + 18000000) / 1000),
    dt_txt: new Date(Date.now() + 18000000).toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 18.2,
      temp_min: 16.5,
      temp_max: 20.0,
      pressure: 1008,
      humidity: 65
    },
    weather: [
      {
        id: 800,
        main: "Clear",
        description: "晴朗",
        icon: "01n"
      }
    ],
    clouds: {
      all: 10
    },
    wind: {
      speed: 3.8,
      deg: 170,
      gust: 5.8
    },
    visibility: 10000,
    pop: 0.1
  },
  {
    dt: Math.floor((Date.now() + 21600000) / 1000),
    dt_txt: new Date(Date.now() + 21600000).toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 17.8,
      temp_min: 15.9,
      temp_max: 19.5,
      pressure: 1007,
      humidity: 68
    },
    weather: [
      {
        id: 801,
        main: "Few clouds",
        description: "少云",
        icon: "02n"
      }
    ],
    clouds: {
      all: 20
    },
    wind: {
      speed: 4.2,
      deg: 175,
      gust: 6.2
    },
    visibility: 9800,
    pop: 0.15
  },
  {
    dt: Math.floor((Date.now() + 25200000) / 1000),
    dt_txt: new Date(Date.now() + 25200000).toISOString().replace('T', ' ').substring(0, 19),
    main: {
      temp: 17.5,
      temp_min: 15.5,
      temp_max: 19.0,
      pressure: 1006,
      humidity: 70
    },
    weather: [
      {
        id: 802,
        main: "Scattered clouds",
        description: "多云",
        icon: "03n"
      }
    ],
    clouds: {
      all: 40
    },
    wind: {
      speed: 4.6,
      deg: 180,
      gust: 6.7
    },
    visibility: 9500,
    pop: 0.2
  }
]

// 生成未来5天的航线天气预报Mock数据
const generateMockRouteWeather = (startDate = null) => {
  const baseDate = startDate ? new Date(startDate) : new Date()
  const weatherConditions = [
    { condition: '晴朗', icon: '01d', temp_max: 22, temp_min: 15, wind: 3.5, humidity: 55, score: 95, risk: 'low' },
    { condition: '少云', icon: '02d', temp_max: 21, temp_min: 14, wind: 4.2, humidity: 60, score: 88, risk: 'low' },
    { condition: '多云', icon: '03d', temp_max: 20, temp_min: 13, wind: 5.5, humidity: 65, score: 72, risk: 'low_medium' },
    { condition: '阴天', icon: '04d', temp_max: 18, temp_min: 12, wind: 6.8, humidity: 70, score: 58, risk: 'medium' },
    { condition: '小雨', icon: '10d', temp_max: 16, temp_min: 11, wind: 8.2, humidity: 80, score: 45, risk: 'medium' }
  ]

  return weatherConditions.map((weather, index) => {
    const date = new Date(baseDate)
    date.setDate(date.getDate() + index)

    // 计算相对于今天的天数差
    const today = new Date()
    today.setHours(0, 0, 0, 0)
    const targetDate = new Date(date)
    targetDate.setHours(0, 0, 0, 0)
    const daysDiff = Math.floor((targetDate - today) / (1000 * 60 * 60 * 24))

    let dayName = ''
    if (daysDiff === 0) {
      dayName = '今天'
    } else if (daysDiff === 1) {
      dayName = '明天'
    } else if (daysDiff === 2) {
      dayName = '后天'
    } else if (daysDiff > 0) {
      dayName = `${daysDiff}天后`
    } else if (daysDiff === -1) {
      dayName = '昨天'
    } else {
      dayName = `${Math.abs(daysDiff)}天前`
    }

    const dateStr = `${date.getMonth() + 1}月${date.getDate()}日`

    return {
      date: date.getTime(),
      dayName: dayName,
      dateStr: dateStr,
      condition: weather.condition,
      icon: weather.icon,
      temp_max: weather.temp_max,
      temp_min: weather.temp_min,
      wind_speed: weather.wind.toFixed(1),
      humidity: weather.humidity,
      safety_score: weather.score,
      risk_level: weather.risk
    }
  })
}

// 计算属性
const weatherLocation = computed(() => {
  if (currentWeather.value?.weather?.location) {
    return currentWeather.value.weather.location
  }
  return '未知位置'
})

// 移除了 riskLevel 和 riskLevelText computed 属性
// 现在统一使用 riskAssessment.overallRisk 来确保一致性

const routePreviewDistance = computed(() => {
  if (routePreview.value.length === 0) return 0
  // 简化计算：假设每段距离为1公里
  return routePreview.value.length - 1
})

const routePreviewDuration = computed(() => {
  if (routePreviewDistance.value === 0) return 0
  // 假设平均速度为30km/h
  return Math.round(routePreviewDistance.value / 30 * 60)
})

const routePreviewSpeed = computed(() => {
  if (routePreviewDuration.value === 0) return 0
  return (routePreviewDistance.value / routePreviewDuration.value) * 60
})

// 方法
/**
 * 自动定位当前位置
 */
const autoLocate = () => {
  if (!navigator.geolocation) {
    ElMessage.error('您的浏览器不支持定位功能')
    return
  }

  locating.value = true
  ElMessage.info('正在获取您的位置...')

  navigator.geolocation.getCurrentPosition(
    // 成功回调
    (position) => {
      const { latitude, longitude } = position.coords
      console.log('[AutoLocate] 获取位置成功:', latitude, longitude)

      // 更新当前位置
      currentLocation.value = {
        lat: latitude,
        lon: longitude
      }

      ElMessage.success(`定位成功: ${latitude.toFixed(4)}, ${longitude.toFixed(4)}`)

      // 自动刷新天气数据
      locating.value = false
      refreshWeatherData()
    },
    // 错误回调
    (error) => {
      locating.value = false
      console.error('[AutoLocate] 定位失败:', error)

      let errorMessage = '定位失败'
      switch (error.code) {
        case error.PERMISSION_DENIED:
          errorMessage = '您拒绝了定位请求，请在浏览器设置中允许定位权限'
          break
        case error.POSITION_UNAVAILABLE:
          errorMessage = '位置信息不可用，请检查您的设备定位服务是否开启'
          break
        case error.TIMEOUT:
          errorMessage = '定位请求超时，请重试'
          break
        default:
          errorMessage = '定位失败: ' + error.message
      }

      ElMessage.error({
        message: errorMessage,
        duration: 5000,
        showClose: true
      })
    },
    // 选项
    {
      enableHighAccuracy: true, // 使用高精度定位
      timeout: 10000,          // 10秒超时
      maximumAge: 0            // 不使用缓存的位置
    }
  )
}

const refreshWeatherData = async () => {
  loading.value = true

  try {
    // 并发获取当前天气、预报和飞行安全评估
    const [weatherResponse, forecastResponse, safetyResponse] = await Promise.all([
      weatherApi.getCurrentWeatherByCoords(currentLocation.value.lat, currentLocation.value.lon),
      weatherApi.getForecast(currentLocation.value.lat, currentLocation.value.lon),
      weatherApi.checkFlightSafety(currentLocation.value.lat, currentLocation.value.lon)
    ])

    // 处理天气数据
    if (weatherResponse && weatherResponse.success) {
      currentWeather.value = weatherResponse.data
    } else {
      throw new Error('获取天气数据失败')
    }

    // 处理预报数据
    if (forecastResponse && forecastResponse.success) {
      forecast.value = forecastResponse.data.forecast || []
    } else {
      throw new Error('获取预报数据失败')
    }

    // 处理安全评估数据
    if (safetyResponse && safetyResponse.success) {
      flightSafetyData.value = safetyResponse.data
      buildRiskAssessment(safetyResponse.data)
    } else {
      throw new Error('获取安全评估数据失败')
    }

    ElMessage.success('天气数据加载成功')
  } catch (error) {
    console.error('刷新天气数据失败:', error)
    ElMessage.warning({
      message: '外部天气API连接失败（可能被防火墙屏蔽），已切换到演示模式',
      duration: 5000,
      showClose: true
    })
    // 使用Mock数据作为降级方案
    useMockData()
  } finally {
    loading.value = false
  }
}

// 使用Mock数据的降级函数
const useMockData = () => {
  currentWeather.value = {
    weather: {
      location: '北京',
      temperature: 18.5,
      feels_like: 17.2,
      temp_min: 16.8,
      temp_max: 20.1,
      humidity: 65,
      pressure: 1013,
      wind_speed: 3.5,
      wind_direction: 180,
      visibility: 10000,
      cloudiness: 10,
      condition: '晴朗',
      icon: '01d',
      timestamp: Math.floor(Date.now() / 1000)
    }
  }

  forecast.value = mockForecast.slice(0, 8).map(item => ({
    timestamp: item.dt,
    temperature: item.main.temp,
    temp_min: item.main.temp_min,
    temp_max: item.main.temp_max,
    humidity: item.main.humidity,
    pressure: item.main.pressure,
    wind_speed: item.wind.speed,
    condition: item.weather[0].description,
    icon: item.weather[0].icon,
    cloudiness: item.clouds.all,
    visibility: item.visibility,
    flight_safety: {
      score: 85,
      safe: true
    }
  }))

  flightSafetyData.value = {
    safety: {
      safe: true,
      score: 85,
      warnings: ['演示模式：天气条件良好']
    },
    weather: currentWeather.value.weather
  }

  buildRiskAssessment(flightSafetyData.value)
}

/**
 * 根据后端返回的飞行安全数据构建前端风险评估
 */
const buildRiskAssessment = (safetyData) => {
  if (!safetyData || !safetyData.safety) return

  const safety = safetyData.safety
  const weather = safetyData.weather

  const risks = []
  const warnings = []
  const recommendations = []

  // 添加风险项
  if (weather.wind_speed) {
    const windSpeed = weather.wind_speed
    let riskLevel = 'low'
    if (windSpeed > 15) {
      riskLevel = 'high'
    } else if (windSpeed > 10) {
      riskLevel = 'medium'
    }

    if (windSpeed > 10) {
      risks.push({
        type: 'wind',
        level: riskLevel,
        value: windSpeed,
        threshold: 15,
        description: `风速: ${windSpeed.toFixed(1)} m/s`
      })
    }
  }

  if (weather.visibility) {
    const visibility = weather.visibility
    let riskLevel = 'low'
    if (visibility < 1000) {
      riskLevel = 'high'
    } else if (visibility < 5000) {
      riskLevel = 'medium'
    }

    if (visibility < 5000) {
      risks.push({
        type: 'visibility',
        level: riskLevel,
        value: visibility,
        threshold: 1000,
        description: `能见度: ${(visibility / 1000).toFixed(1)} km`
      })
    }
  }

  // 温度评估
  if (weather.temperature) {
    const temp = weather.temperature
    let riskLevel = 'low'
    if (temp < -10 || temp > 40) {
      riskLevel = 'medium'
    }

    risks.push({
      type: 'temperature',
      level: riskLevel,
      value: temp,
      threshold: -10,
      description: `温度: ${temp.toFixed(1)}°C`
    })
  }

  // 添加警告和建议（警告不单独显示风险等级，只作为提示信息）
  if (safety.warnings && safety.warnings.length > 0) {
    safety.warnings.forEach(warning => {
      warnings.push({
        type: 'warning',
        description: warning
      })
    })
  }

  // 根据评分生成建议和总体风险等级
  const score = safety.score || 100
  let overallRisk = 'low'

  if (score >= 80) {
    overallRisk = 'low'
    recommendations.push('天气条件良好，适合飞行')
  } else if (score >= 60) {
    overallRisk = 'low_medium'
    recommendations.push('天气条件一般，建议谨慎飞行')
    recommendations.push('注意观察天气变化')
  } else if (score >= 40) {
    overallRisk = 'medium'
    recommendations.push('天气条件较差，不建议飞行')
    recommendations.push('如必须飞行，请降低飞行高度和速度')
  } else {
    overallRisk = 'high'
    recommendations.push('天气条件恶劣，禁止飞行')
  }

  console.log('[buildRiskAssessment] 计算结果:', {
    score: score,
    overallRisk: overallRisk,
    risks: risks,
    warnings: warnings,
    recommendations: recommendations
  })

  riskAssessment.value = {
    overallRisk: overallRisk,
    risks,
    warnings,
    recommendations
  }

  console.log('[buildRiskAssessment] riskAssessment设置完成:', riskAssessment.value)
}

const getRiskTagType = (level) => {
  const types = {
    'low': 'success',
    'low_medium': 'info',
    'medium': 'warning',
    'high': 'danger'
  }
  return types[level] || 'info'
}

const getRiskLevelText = (level) => {
  const texts = {
    'low': '低风险',
    'low_medium': '低中风险',
    'medium': '中风险',
    'high': '高风险'
  }
  return texts[level] || '未知'
}

const getRiskColor = (level) => {
  const colors = {
    'low': '#67c23a',
    'low_medium': '#409eff',
    'medium': '#e6a23c',
    'high': '#f56c6c'
  }
  return colors[level] || '#909399'
}

const getRiskIcon = (type) => {
  const icons = {
    'wind': 'Wind',
    'temperature': 'Temperature',
    'visibility': 'Visibility',
    'precipitation': 'Rain',
    'warning': 'Warning',
    'none': ''
  }
  return icons[type] || 'Warning'
}

const getRiskIconClass = (level) => {
  return level === 'high' ? 'risk-icon-high' : level === 'medium' ? 'risk-icon-medium' : 'risk-icon-low'
}

const getWindDirection = (deg) => {
  if (!deg) return '未知'
  const directions = ['北', '东北', '东', '东南', '南', '西南', '西', '西北']
  const index = Math.round(deg / 45) % 8
  return directions[index]
}

const getWeatherIconUrl = (iconCode) => {
  return `https://openweathermap.org/img/wn/${iconCode}@2x.png`
}

const getDayName = (timestamp) => {
  if (!timestamp) return ''
  const date = new Date(timestamp * 1000) // 后端返回Unix时间戳（秒）
  const days = ['周日', '周一', '周二', '周三', '周四', '周五', '周六']
  return days[date.getDay()]
}

const getDate = (timestamp) => {
  if (!timestamp) return ''
  const date = new Date(timestamp * 1000) // 后端返回Unix时间戳（秒）
  return `${date.getMonth() + 1}月${date.getDate()}日`
}

const getTimeSlot = (index) => {
  // 每个时段为3小时，从当前时间开始
  const now = new Date()
  const currentHour = now.getHours()

  // 计算起始小时（向下取整到3的倍数）
  const baseHour = Math.floor(currentHour / 3) * 3

  // 计算该索引对应的起始和结束小时
  const startHour = (baseHour + index * 3) % 24
  const endHour = (startHour + 3) % 24

  // 格式化为 HH:MM 格式
  const formatHour = (hour) => {
    return `${hour.toString().padStart(2, '0')}:00`
  }

  return `${formatHour(startHour)} ~ ${formatHour(endHour)}`
}

const getTimeSlotDate = (index) => {
  // 计算该时段对应的日期
  const now = new Date()
  const currentHour = now.getHours()
  const baseHour = Math.floor(currentHour / 3) * 3

  // 计算该时段的起始小时（实际小时数，不取模）
  const actualStartHour = baseHour + index * 3

  // 计算日期偏移（每24小时为一天）
  const dayOffset = Math.floor(actualStartHour / 24)

  // 创建新日期
  const slotDate = new Date(now)
  slotDate.setDate(slotDate.getDate() + dayOffset)

  const days = ['周日', '周一', '周二', '周三', '周四', '周五', '周六']
  const dayName = days[slotDate.getDay()]
  const dateStr = `${slotDate.getMonth() + 1}月${slotDate.getDate()}日`

  return `${dayName} ${dateStr}`
}

const getTempClass = (temp) => {
  if (temp < 0) return 'temp-cold'
  if (temp > 30) return 'temp-hot'
  return 'temp-normal'
}

const getRowClass = ({ row }) => {
  if (row.risk_level === 'high') return 'risk-high'
  if (row.risk_level === 'medium') return 'risk-medium'
  return ''
}

const getSafetyScoreColor = (score) => {
  if (score >= 80) return '#67C23A' // 绿色 - 安全
  if (score >= 60) return '#E6A23C' // 橙色 - 警告
  if (score >= 40) return '#F56C6C' // 红色 - 危险
  return '#909399' // 灰色 - 禁止
}

const generateRoutePreview = () => {
  if (!routeStart.value || !routeEnd.value) return
  
  // 模拟路线生成
  routePreview.value = Array.from({ length: 5 }, (_, i) => ({
    lat: 39.90923 + i * 0.005,
    lng: 116.397428 + i * 0.005
  }))
  
  routePreviewDistance.value = routePreview.value.length - 1
  routePreviewDuration.value = Math.round(routePreviewDistance.value / 30 * 60)
  routePreviewSpeed.value = (routePreviewDistance.value / routePreviewDuration.value) * 60
  
  ElMessage.info('路线预览已生成')
}

const saveRoute = () => {
  if (!routeStart.value || !routeEnd.value) {
    ElMessage.warning('请填写起点和终点')
    return
  }
  
  ElMessage.success('航线已保存')
  showRoutePlanner.value = false
  resetRouteForm()
}

const resetRouteForm = () => {
  routeStart.value = ''
  routeEnd.value = ''
  routeAltitude.value = 100
  routeTime.value = new Date().toISOString()
  routePreview.value = []
  routePreviewDistance.value = 0
  routePreviewDuration.value = 0
  routePreviewSpeed.value = 0
}

// 加载航线天气数据的函数
const loadRouteWeatherData = () => {
  if (!selectedRoute.value) {
    routeWeather.value = []
    return
  }

  // 设置标志，防止watch触发无限循环
  isLoadingRouteWeather = true

  try {
    // 确定起始日期
    let startDate = null
    if (forecastTimeRange.value && forecastTimeRange.value.length > 0) {
      // 如果用户选择了日期范围，使用起始日期
      startDate = new Date(forecastTimeRange.value[0])
      console.log('[WeatherIntegration] 使用选定起始日期:', forecastTimeRange.value[0])
    } else {
      // 否则使用今天
      startDate = new Date()
      console.log('[WeatherIntegration] 使用今天作为起始日期')
    }

    console.log('[WeatherIntegration] 加载航线天气:', selectedRoute.value, '起始日期:', startDate)

    // 生成从起始日期开始的未来5天天气预报
    routeWeather.value = generateMockRouteWeather(startDate)

    const dateStr = `${startDate.getMonth() + 1}月${startDate.getDate()}日`
    ElMessage.success(`已加载航线 ${selectedRoute.value} 从${dateStr}开始的5天天气预报`)
  } finally {
    // 使用nextTick确保在DOM更新后才清除标志
    nextTick(() => {
      isLoadingRouteWeather = false
    })
  }
}

// 防止watch无限循环的标志
let isLoadingRouteWeather = false

// 监听航线选择变化
watch(selectedRoute, (newRoute) => {
  if (isLoadingRouteWeather) return

  if (newRoute) {
    loadRouteWeatherData()
  } else {
    routeWeather.value = []
  }
})

// 监听日期范围变化（深度监听数组内容变化）
watch(forecastTimeRange, (newRange) => {
  if (isLoadingRouteWeather) return

  // 只有在已选择航线的情况下才重新加载数据
  if (selectedRoute.value) {
    loadRouteWeatherData()
  }
}, { deep: true })

// 组件卸载时取消所有未完成的HTTP请求，防止内存泄漏
onBeforeUnmount(() => {
  console.log('[WeatherIntegration] 组件卸载，取消所有待处理的HTTP请求:', activeRequests.size)

  // 取消所有活动请求
  activeRequests.forEach(controller => {
    controller.abort()
  })

  // 清空请求集合
  activeRequests.clear()
})

// 初始化数据
refreshWeatherData()
</script>

<style scoped>
.weather-integration {
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

.action-section {
  display: flex;
  gap: 12px;
}

/* 统计卡片 */
.stats-section {
  margin-bottom: 24px;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 20px;
}

.stat-card {
  padding: 24px;
  display: flex;
  align-items: center;
  gap: 16px;
  transition: all 0.3s ease;
}

.stat-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 8px 30px rgba(0, 0, 0, 0.12);
}

.stat-icon {
  width: 60px;
  height: 60px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.stat-content h3 {
  margin: 0 0 4px 0;
  font-size: 28px;
  font-weight: 700;
  color: #2c3e50;
}

.stat-content p {
  margin: 0;
  font-size: 14px;
  color: #7f8c8d;
  font-weight: 500;
}

/* 当前天气卡片 */
.current-weather-card {
  margin-bottom: 24px;
}

.weather-content {
  display: flex;
  gap: 30px;
  padding: 20px 0;
}

.weather-info {
  flex: 1;
}

.weather-location {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 20px;
  color: #667eea;
  font-weight: 500;
}

.weather-details {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 15px;
}

.detail-item {
  display: flex;
  flex-direction: column;
}

.label {
  font-size: 13px;
  color: #7f8c8d;
  margin-bottom: 4px;
}

.value {
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
}

.weather-condition {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 10px;
  padding: 20px;
  border-left: 1px solid #eee;
  min-width: 200px;
}

.condition-icon {
  width: 80px;
  height: 80px;
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: #f8f9fa;
  border-radius: 12px;
}

.weather-icon {
  width: 80px;
  height: 80px;
}

.condition-description h3 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
  color: #2c3e50;
}

.condition-description p {
  margin: 0;
  color: #7f8c8d;
  font-size: 14px;
}

/* 风险评估卡片 */
.risk-assessment-card {
  margin-bottom: 24px;
}

.risk-content {
  padding: 20px 0;
}

.risk-summary {
  margin-bottom: 24px;
}

.risk-item {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 12px;
  padding: 12px;
  border-radius: 8px;
  background-color: #f8f9fa;
  border-left: 4px solid #667eea;
}

.risk-icon {
  width: 32px;
  height: 32px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.risk-icon.high {
  background-color: #f56c6c;
  color: white;
}

.risk-icon.medium {
  background-color: #e6a23c;
  color: white;
}

.risk-icon.low {
  background-color: #67c23a;
  color: white;
}

.warning-icon {
  background-color: #f093fb;
  color: white;
}

.risk-info h4 {
  margin: 0;
  font-size: 14px;
  font-weight: 600;
  color: #2c3e50;
}

.risk-level {
  font-size: 12px;
  font-weight: 500;
  color: #667eea;
}

.recommendations {
  margin-top: 20px;
}

.recommendations h4 {
  margin: 0 0 12px 0;
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
}

.recommendations ul {
  padding-left: 20px;
  margin: 0;
}

.recommendations li {
  margin-bottom: 8px;
  color: #606266;
  line-height: 1.6;
}

/* 路线天气卡片 */
.route-weather-card {
  margin-bottom: 24px;
}

.route-weather-content {
  padding: 20px 0;
}

.route-selector {
  display: flex;
  align-items: center;
  margin-bottom: 20px;
  gap: 20px;
}

.no-data {
  text-align: center;
  padding: 40px;
  color: #909399;
}

.route-weather-list {
  max-height: 400px;
  overflow-y: auto;
}

.risk-high {
  background-color: #ffebee;
}

.risk-medium {
  background-color: #fff3e0;
}

/* 未来天气预报卡片 */
.forecast-card {
  margin-bottom: 24px;
}

.forecast-content {
  padding: 20px 0;
}

.forecast-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 15px;
}

.forecast-item {
  padding: 16px;
  border-radius: 12px;
  background-color: #f8f9fa;
  text-align: center;
  border: 1px solid #e8eaec;
  transition: all 0.3s ease;
}

.forecast-item:hover {
  transform: translateY(-4px);
  box-shadow: 0 8px 30px rgba(0, 0, 0, 0.1);
}

.forecast-today {
  background-color: #667eea;
  color: white;
  border-color: #667eea;
}

.forecast-today .forecast-date,
.forecast-today .forecast-icon,
.forecast-today .forecast-temp,
.forecast-today .forecast-condition,
.forecast-today .forecast-detail {
  color: white;
}

.forecast-date {
  margin-bottom: 10px;
}

.day {
  font-weight: 600;
  font-size: 14px;
}

.date {
  font-size: 12px;
  opacity: 0.9;
}

.forecast-icon {
  margin: 10px 0;
}

.forecast-icon-img {
  width: 60px;
  height: 60px;
}

.forecast-temp {
  display: flex;
  justify-content: center;
  gap: 8px;
  margin: 8px 0;
}

.high {
  font-weight: 700;
  color: #e64a19;
}

.low {
  font-weight: 500;
  color: #2196f3;
}

.forecast-condition {
  font-weight: 600;
  margin-bottom: 8px;
  font-size: 14px;
}

.forecast-detail {
  font-size: 12px;
  color: #606266;
}

.detail-row {
  display: flex;
  justify-content: space-between;
  margin: 4px 0;
}

.label {
  font-weight: 500;
}

.value {
  font-weight: 600;
}

/* 风险详情对话框 */
.risk-details-content {
  padding: 20px 0;
  max-height: 600px;
  overflow-y: auto;
}

.risk-section {
  margin-bottom: 30px;
}

.risk-section h3 {
  margin: 0 0 16px 0;
  font-size: 18px;
  font-weight: 600;
  color: #2c3e50;
  border-bottom: 1px solid #eee;
  padding-bottom: 10px;
}

.risk-factors {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.risk-factor {
  display: flex;
  flex-direction: column;
  gap: 8px;
  padding: 12px;
  border-radius: 8px;
  background-color: #f8f9fa;
  border-left: 4px solid #667eea;
}

.risk-type {
  display: flex;
  align-items: center;
  gap: 8px;
  font-weight: 600;
  color: #2c3e50;
}

.risk-description {
  margin: 0;
  color: #606266;
  line-height: 1.6;
}

.risk-value {
  display: flex;
  gap: 16px;
  font-size: 12px;
  color: #909399;
}

.risk-overview {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 16px;
  margin-top: 16px;
}

.overview-item {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.overview-item .label {
  font-size: 14px;
  color: #7f8c8d;
}

.overview-item .value {
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
}

.recommendations-list {
  margin-top: 16px;
}

.recommendation-item {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 8px;
  color: #606266;
  line-height: 1.6;
}

/* 路线规划对话框 */
.route-planner-content {
  padding: 20px 0;
}

.route-form {
  margin-bottom: 30px;
}

.route-preview {
  margin-top: 20px;
}

.preview-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.preview-header h4 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
  color: #2c3e50;
}

.preview-map {
  margin-bottom: 20px;
}

.map-container {
  width: 100%;
  height: 200px;
  border: 1px dashed #ddd;
  border-radius: 8px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background-color: #f8f9fa;
  margin-bottom: 16px;
}

.map-placeholder {
  text-align: center;
  color: #909399;
}

.map-placeholder el-icon {
  margin-bottom: 10px;
}

.route-stats {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
  gap: 16px;
  margin-top: 16px;
}

.stat-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 12px;
  border-radius: 8px;
  background-color: #f8f9fa;
  border: 1px solid #eee;
}

.stat-item .label {
  font-size: 12px;
  color: #7f8c8d;
  margin-bottom: 4px;
}

.stat-item .value {
  font-size: 16px;
  font-weight: 600;
  color: #2c3e50;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
  }

  .weather-content {
    flex-direction: column;
    gap: 20px;
  }

  .weather-condition {
    border-left: none;
    border-top: 1px solid #eee;
    padding: 20px;
  }

  .forecast-grid {
    grid-template-columns: 1fr;
  }

  .route-selector {
    flex-direction: column;
    gap: 16px;
  }

  .preview-header {
    flex-direction: column;
    align-items: flex-start;
    gap: 16px;
  }

  .route-stats {
    grid-template-columns: 1fr;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-card__body) {
  padding: 24px;
}

:deep(.el-table__row:hover) {
  background-color: #f8f9fa;
  cursor: pointer;
}

:deep(.el-table__header) {
  background-color: #fafbfc;
}

:deep(.el-input__wrapper) {
  border-radius: 12px;
  transition: all 0.3s ease;
}

:deep(.el-input__wrapper:hover) {
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.1);
}

:deep(.el-button) {
  border-radius: 8px;
  font-weight: 500;
}

:deep(.el-date-editor) {
  border-radius: 12px;
}

:deep(.el-select) {
  border-radius: 12px;
}

:deep(.el-radio-group) {
  display: flex;
  gap: 20px;
}

:deep(.el-radio) {
  margin-right: 20px;
}

:deep(.el-alert) {
  margin-bottom: 15px;
}

:deep(.el-message-box__content) {
  padding: 20px;
  }
  </style>
