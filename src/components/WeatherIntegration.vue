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
            <h3>{{ currentWeather?.main?.temp }}°C</h3>
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
            <h3>{{ currentWeather?.wind?.speed }} m/s</h3>
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
            <h3>{{ currentWeather?.weather?.[0]?.main }}</h3>
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
            <h3>{{ riskLevelText }}</h3>
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
            :type="getRiskTagType(riskLevel)"
            size="medium"
          >
            {{ riskLevelText }}
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
              <span class="value">{{ currentWeather?.main?.temp }}°C</span>
            </div>
            
            <div class="detail-item">
              <span class="label">体感温度:</span>
              <span class="value">{{ currentWeather?.main?.feels_like }}°C</span>
            </div>
            
            <div class="detail-item">
              <span class="label">湿度:</span>
              <span class="value">{{ currentWeather?.main?.humidity }}%</span>
            </div>
            
            <div class="detail-item">
              <span class="label">气压:</span>
              <span class="value">{{ currentWeather?.main?.pressure }} hPa</span>
            </div>
            
            <div class="detail-item">
              <span class="label">风速:</span>
              <span class="value">{{ currentWeather?.wind?.speed }} m/s</span>
            </div>
            
            <div class="detail-item">
              <span class="label">风向:</span>
              <span class="value">{{ getWindDirection(currentWeather?.wind?.deg) }}</span>
            </div>
            
            <div class="detail-item">
              <span class="label">能见度:</span>
              <span class="value">{{ currentWeather?.visibility > 0 ? (currentWeather?.visibility / 1000).toFixed(1) + 'km' : '未知' }}</span>
            </div>
            
            <div class="detail-item">
              <span class="label">云量:</span>
              <span class="value">{{ currentWeather?.clouds?.all }}%</span>
            </div>
          </div>
        </div>

        <div class="weather-condition">
          <div class="condition-icon">
            <img 
              :src="getWeatherIconUrl(currentWeather?.weather?.[0]?.icon)" 
              :alt="currentWeather?.weather?.[0]?.description"
              class="weather-icon"
            >
          </div>
          <div class="condition-description">
            <h3>{{ currentWeather?.weather?.[0]?.main }}</h3>
            <p>{{ currentWeather?.weather?.[0]?.description }}</p>
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
            class="risk-item"
          >
            <div
              class="risk-icon warning-icon"
              :class="getRiskIconClass(warning.level)"
            >
              <el-icon
                :size="18"
                :color="getRiskColor(warning.level)"
              >
                {{ getRiskIcon('warning') }}
              </el-icon>
            </div>
            <div class="risk-info">
              <h4>{{ warning.description }}</h4>
              <p class="risk-level">
                {{ getRiskLevelText(warning.level) }}
              </p>
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
              prop="node.lat"
              label="纬度"
              width="100"
              align="center"
            >
              <template #default="{ row }">
                {{ row.node.lat.toFixed(4) }}
              </template>
            </el-table-column>
            
            <el-table-column
              prop="node.lng"
              label="经度"
              width="100"
              align="center"
            >
              <template #default="{ row }">
                {{ row.node.lng.toFixed(4) }}
              </template>
            </el-table-column>
            
            <el-table-column
              prop="weather.main.temp"
              label="温度"
              width="80"
              align="center"
            >
              <template #default="{ row }">
                <span :class="getTempClass(row.weather.main.temp)">{{ row.weather.main.temp }}°C</span>
              </template>
            </el-table-column>
            
            <el-table-column
              prop="weather.weather[0].main"
              label="天气"
              width="120"
              align="center"
            >
              <template #default="{ row }">
                <div class="weather-status">
                  <img 
                    :src="getWeatherIconUrl(row.weather.weather[0].icon)" 
                    :alt="row.weather.weather[0].description"
                    class="weather-icon-small"
                  >
                  <span>{{ row.weather.weather[0].description }}</span>
                </div>
              </template>
            </el-table-column>
            
            <el-table-column
              prop="riskAssessment.overallRisk"
              label="风险等级"
              width="120"
              align="center"
            >
              <template #default="{ row }">
                <el-tag
                  :type="getRiskTagType(row.riskAssessment.overallRisk)"
                  size="small"
                >
                  {{ getRiskLevelText(row.riskAssessment.overallRisk) }}
                </el-tag>
              </template>
            </el-table-column>
            
            <el-table-column
              prop="riskAssessment.risks.length"
              label="风险项"
              width="100"
              align="center"
            >
              <template #default="{ row }">
                <span>{{ row.riskAssessment.risks.length }}</span>
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
          <span class="card-title">未来24小时天气预报</span>
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
              <span class="day">{{ getDayName(day.dt_txt) }}</span>
              <span class="date">{{ getDate(day.dt_txt) }}</span>
            </div>
            
            <div class="forecast-icon">
              <img 
                :src="getWeatherIconUrl(day.weather[0].icon)" 
                :alt="day.weather[0].description"
                class="forecast-icon-img"
              >
            </div>
            
            <div class="forecast-temp">
              <span class="high">{{ day.main.temp_max }}°C</span>
              <span class="low">{{ day.main.temp_min }}°C</span>
            </div>
            
            <div class="forecast-condition">
              {{ day.weather[0].main }}
            </div>
            
            <div class="forecast-detail">
              <div class="detail-row">
                <span class="label">风速:</span>
                <span class="value">{{ day.wind.speed }} m/s</span>
              </div>
              <div class="detail-row">
                <span class="label">降水概率:</span>
                <span class="value">{{ (day.pop * 100).toFixed(0) }}%</span>
              </div>
              <div class="detail-row">
                <span class="label">湿度:</span>
                <span class="value">{{ day.main.humidity }}%</span>
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
import { ref, computed } from 'vue'
import { ElMessage } from 'element-plus'
import {
  Temperature, Wind, Cloud, Warning, Location, Refresh,
  Map, Check
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'

// const store = useStore()

// 响应式数据
const loading = ref(false)
const showRiskDetails = ref(false)
const showRoutePlanner = ref(false)

const currentWeather = ref(null)
const riskAssessment = ref(null)
const routeWeather = ref([])
const forecast = ref([])
const selectedRoute = ref('')
const forecastTimeRange = ref([])

const routeStart = ref('')
const routeEnd = ref('')
const routeAltitude = ref(100)
const routeTime = ref(new Date().toISOString())
const routePreview = ref([])

// 模拟数据
const mockWeatherData = {
  coord: { lat: 39.90923, lng: 116.397428 },
  weather: [
    {
      id: 800,
      main: "Clear",
      description: "晴朗",
      icon: "01d"
    }
  ],
  main: {
    temp: 18.5,
    feels_like: 17.2,
    temp_min: 16.8,
    temp_max: 20.1,
    pressure: 1013,
    humidity: 65
  },
  visibility: 10000,
  wind: {
    speed: 3.5,
    deg: 180
  },
  clouds: {
    all: 10
  },
  dt: Math.floor(Date.now() / 1000),
  sys: {
    type: 1,
    id: 9412,
    country: "CN",
    sunrise: Math.floor(Date.now() / 1000) + 21600,
    sunset: Math.floor(Date.now() / 1000) + 64800
  },
  timezone: 28800,
  id: 1816670,
  name: "北京",
  cod: 200
}

const mockRiskAssessment = {
  overallRisk: 'medium',
  risks: [
    {
      type: 'wind',
      level: 'medium',
      value: 3.5,
      threshold: 15,
      description: '风速较高: 3.5 m/s'
    },
    {
      type: 'temperature',
      level: 'low',
      value: 18.5,
      threshold: -10,
      description: '温度适宜: 18.5°C'
    }
  ],
  warnings: [
    {
      type: 'moderate_weather',
      level: 'medium',
      condition: 'Clear',
      description: '检测到不利天气条件，请谨慎飞行'
    }
  ],
  recommendations: [
    '天气条件良好，适合飞行',
    '建议保持在较低高度以减少风的影响',
    '注意无人机电池性能受温度影响'
  ]
}

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

const mockRouteWeather = [
  {
    node: { lat: 39.90923, lng: 116.397428 },
    weather: {
      main: {
        temp: 18.5,
        feels_like: 17.2,
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
      visibility: 10000,
      wind: {
        speed: 3.5,
        deg: 180
      },
      clouds: {
        all: 10
      }
    },
    riskAssessment: {
      overallRisk: 'low',
      risks: [],
      warnings: [],
      recommendations: ['天气条件良好，适合飞行']
    }
  },
  {
    node: { lat: 39.91423, lng: 116.402428 },
    weather: {
      main: {
        temp: 18.2,
        feels_like: 16.9,
        temp_min: 16.5,
        temp_max: 19.8,
        pressure: 1012,
        humidity: 67
      },
      weather: [
        {
          id: 801,
          main: "Few clouds",
          description: "少云",
          icon: "02d"
        }
      ],
      visibility: 9800,
      wind: {
        speed: 4.1,
        deg: 175
      },
      clouds: {
        all: 20
      }
    },
    riskAssessment: {
      overallRisk: 'low',
      risks: [],
      warnings: [],
      recommendations: ['天气条件良好，适合飞行']
    }
  },
  {
    node: { lat: 39.91923, lng: 116.407428 },
    weather: {
      main: {
        temp: 17.8,
        feels_like: 16.5,
        temp_min: 16.1,
        temp_max: 19.4,
        pressure: 1011,
        humidity: 69
      },
      weather: [
        {
          id: 802,
          main: "Scattered clouds",
          description: "多云",
          icon: "03d"
        }
      ],
      visibility: 9500,
      wind: {
        speed: 4.8,
        deg: 170
      },
      clouds: {
        all: 40
      }
    },
    riskAssessment: {
      overallRisk: 'medium',
      risks: [
        {
          type: 'wind',
          level: 'medium',
          value: 4.8,
          threshold: 15,
          description: '风速较高: 4.8 m/s'
        }
      ],
      warnings: [],
      recommendations: ['风速较高，建议降低飞行高度']
    }
  },
  {
    node: { lat: 39.92423, lng: 116.412428 },
    weather: {
      main: {
        temp: 17.5,
        feels_like: 16.2,
        temp_min: 15.8,
        temp_max: 19.1,
        pressure: 1010,
        humidity: 70
      },
      weather: [
        {
          id: 803,
          main: "Broken clouds",
          description: "阴天",
          icon: "04d"
        }
      ],
      visibility: 9200,
      wind: {
        speed: 5.2,
        deg: 165
      },
      clouds: {
        all: 60
      }
    },
    riskAssessment: {
      overallRisk: 'medium',
      risks: [
        {
          type: 'wind',
          level: 'medium',
          value: 5.2,
          threshold: 15,
          description: '风速较高: 5.2 m/s'
        },
        {
          type: 'visibility',
          level: 'medium',
          value: 9200,
          threshold: 1000,
          description: '能见度不足: 9200米'
        }
      ],
      warnings: [],
      recommendations: ['风速较高，能见度一般，建议谨慎飞行']
    }
  },
  {
    node: { lat: 39.92923, lng: 116.417428 },
    weather: {
      main: {
        temp: 17.2,
        feels_like: 15.9,
        temp_min: 15.5,
        temp_max: 18.8,
        pressure: 1009,
        humidity: 72
      },
      weather: [
        {
          id: 804,
          main: "Overcast",
          description: "阴",
          icon: "04n"
        }
      ],
      visibility: 8500,
      wind: {
        speed: 4.5,
        deg: 160
      },
      clouds: {
        all: 80
      }
    },
    riskAssessment: {
      overallRisk: 'medium',
      risks: [
        {
          type: 'wind',
          level: 'medium',
          value: 4.5,
          threshold: 15,
          description: '风速较高: 4.5 m/s'
        },
        {
          type: 'visibility',
          level: 'medium',
          value: 8500,
          threshold: 1000,
          description: '能见度不足: 8500米'
        }
      ],
      warnings: [],
      recommendations: ['风速较高，能见度一般，建议谨慎飞行']
    }
  }
]

// 计算属性
const weatherLocation = computed(() => currentWeather.value?.name || '未知位置')
const riskLevel = computed(() => riskAssessment.value?.overallRisk || 'low')
const riskLevelText = computed(() => {
  const levels = {
    'low': '低风险',
    'low_medium': '低中风险',
    'medium': '中风险',
    'high': '高风险'
  }
  return levels[riskLevel.value] || '未知'
})

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
const refreshWeatherData = () => {
  loading.value = true
  
  // 模拟API调用
  setTimeout(() => {
    currentWeather.value = mockWeatherData
    riskAssessment.value = mockRiskAssessment
    forecast.value = mockForecast
    routeWeather.value = mockRouteWeather
    
    ElMessage.success('天气数据已刷新')
    loading.value = false
  }, 1500)
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

const getDayName = (dateTimeStr) => {
  const date = new Date(dateTimeStr)
  const days = ['周日', '周一', '周二', '周三', '周四', '周五', '周六']
  return days[date.getDay()]
}

const getDate = (dateTimeStr) => {
  const date = new Date(dateTimeStr)
  return `${date.getMonth() + 1}月${date.getDate()}日`
}

const getTempClass = (temp) => {
  if (temp < 0) return 'temp-cold'
  if (temp > 30) return 'temp-hot'
  return 'temp-normal'
}

const getRowClass = ({ row }) => {
  if (row.riskAssessment.overallRisk === 'high') return 'risk-high'
  if (row.riskAssessment.overallRisk === 'medium') return 'risk-medium'
  return ''
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
