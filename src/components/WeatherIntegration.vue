<template>
  <div class="weather-integration">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">☁️ 气象监测</h1>
      <p class="page-subtitle">实时天气数据与飞行风险评估</p>
      <div class="header-actions">
        <AppleButton variant="secondary" :loading="locating" @click="autoLocate">
          📍 自动定位
        </AppleButton>
        <AppleButton @click="refreshWeatherData">
          🔄 刷新数据
        </AppleButton>
      </div>
    </div>

    <!-- 天气统计卡片 -->
    <div class="stats-grid">
      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">🌡️</div>
          <div class="stat-info">
            <div class="stat-value">{{ currentWeather?.weather?.temperature ? currentWeather.weather.temperature.toFixed(1) : '--' }}°C</div>
            <div class="stat-label">当前温度</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">💨</div>
          <div class="stat-info">
            <div class="stat-value">{{ currentWeather?.weather?.wind_speed ? currentWeather.weather.wind_speed.toFixed(1) : '--' }} m/s</div>
            <div class="stat-label">风速</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">☁️</div>
          <div class="stat-info">
            <div class="stat-value">{{ currentWeather?.weather?.condition || '--' }}</div>
            <div class="stat-label">天气状况</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">⚠️</div>
          <div class="stat-info">
            <div class="stat-value" v-if="riskAssessment && riskAssessment.overallRisk">
              {{ getRiskLevelText(riskAssessment.overallRisk) }}
            </div>
            <div class="stat-value" v-else-if="loading">加载中...</div>
            <div class="stat-value" v-else>--</div>
            <div class="stat-label">飞行风险等级</div>
          </div>
        </div>
      </AppleCard>
    </div>

    <!-- 实时天气区域 -->
    <AppleCard class="current-weather-card">
      <div class="card-header">
        <h2 class="section-title">实时天气</h2>
        <span v-if="riskAssessment && riskAssessment.overallRisk"
              :class="['risk-badge', `risk-${riskAssessment.overallRisk}`]">
          {{ getRiskLevelText(riskAssessment.overallRisk) }}
        </span>
        <span v-else-if="loading" class="risk-badge risk-unknown">加载中...</span>
        <span v-else class="risk-badge risk-unknown">暂无数据</span>
      </div>

      <div class="weather-content">
        <div class="weather-main">
          <div class="weather-location">
            <span class="location-icon">📍</span>
            <span class="location-name">{{ weatherLocation }}</span>
          </div>

          <div class="weather-details-grid">
            <div class="weather-detail-item">
              <span class="detail-label">温度</span>
              <span class="detail-value">{{ weatherApi.formatTemperature(currentWeather?.weather?.temperature || 0) }}</span>
            </div>

            <div class="weather-detail-item">
              <span class="detail-label">体感温度</span>
              <span class="detail-value">{{ weatherApi.formatTemperature(currentWeather?.weather?.feels_like || 0) }}</span>
            </div>

            <div class="weather-detail-item">
              <span class="detail-label">湿度</span>
              <span class="detail-value">{{ weatherApi.formatHumidity(currentWeather?.weather?.humidity || 0) }}</span>
            </div>

            <div class="weather-detail-item">
              <span class="detail-label">气压</span>
              <span class="detail-value">{{ currentWeather?.weather?.pressure || '--' }} hPa</span>
            </div>

            <div class="weather-detail-item">
              <span class="detail-label">风速</span>
              <span class="detail-value">{{ weatherApi.formatWindSpeed(currentWeather?.weather?.wind_speed || 0) }}</span>
            </div>

            <div class="weather-detail-item">
              <span class="detail-label">风向</span>
              <span class="detail-value">{{ weatherApi.getWindDirection(currentWeather?.weather?.wind_direction || 0) }}</span>
            </div>

            <div class="weather-detail-item">
              <span class="detail-label">能见度</span>
              <span class="detail-value">{{ weatherApi.formatVisibility(currentWeather?.weather?.visibility || 0) }}</span>
            </div>

            <div class="weather-detail-item">
              <span class="detail-label">云量</span>
              <span class="detail-value">{{ currentWeather?.weather?.cloudiness || '--' }}%</span>
            </div>
          </div>
        </div>

        <div class="weather-visual">
          <div class="weather-icon-container">
            <img
              :src="weatherApi.getWeatherIconUrl(currentWeather?.weather?.icon || '01d')"
              :alt="currentWeather?.weather?.condition"
              class="weather-icon"
            >
          </div>
          <div class="weather-condition-text">
            <h3>{{ currentWeather?.weather?.condition || '--' }}</h3>
            <p>{{ weatherApi.getWeatherDescription(currentWeather?.weather?.condition || '') }}</p>
          </div>
        </div>
      </div>
    </AppleCard>

    <!-- 风险评估区域 -->
    <AppleCard class="risk-assessment-card">
      <div class="card-header">
        <h2 class="section-title">飞行风险评估</h2>
        <AppleButton variant="secondary" size="small" @click="showRiskDetails = true">
          详细分析
        </AppleButton>
      </div>

      <div class="risk-content">
        <div class="risk-items-list">
          <div
            v-for="(risk, index) in riskAssessment.risks"
            :key="index"
            class="risk-item"
          >
            <div :class="['risk-indicator', `risk-level-${risk.level}`]">
              <span class="risk-emoji">{{ getRiskEmoji(risk.type) }}</span>
            </div>
            <div class="risk-details">
              <h4>{{ risk.description }}</h4>
              <p class="risk-level-text">{{ getRiskLevelText(risk.level) }}</p>
            </div>
          </div>

          <div
            v-for="(warning, index) in riskAssessment.warnings"
            :key="'warning-' + index"
            class="risk-item warning-item"
          >
            <div class="risk-indicator warning-indicator">
              <span class="risk-emoji">⚠️</span>
            </div>
            <div class="risk-details">
              <h4>{{ warning.description }}</h4>
            </div>
          </div>
        </div>

        <div class="recommendations-section">
          <h3 class="recommendations-title">飞行建议</h3>
          <ul class="recommendations-list">
            <li v-for="(recommendation, index) in riskAssessment.recommendations" :key="index">
              <span class="recommendation-bullet">•</span>
              <span>{{ recommendation }}</span>
            </li>
          </ul>
        </div>
      </div>
    </AppleCard>

    <!-- 未来天气预报 -->
    <AppleCard class="forecast-card">
      <div class="card-header">
        <h2 class="section-title">当日每时段天气变化表（3h）</h2>
        <span class="update-badge">每3小时更新</span>
      </div>

      <div class="forecast-grid">
        <div
          v-for="(day, index) in forecast"
          :key="index"
          class="forecast-item"
          :class="{ 'forecast-today': index === 0 }"
        >
          <div class="forecast-time">
            <span class="time-slot">{{ getTimeSlot(index) }}</span>
            <span class="time-date">{{ getTimeSlotDate(index) }}</span>
          </div>

          <div class="forecast-icon">
            <img
              :src="weatherApi.getWeatherIconUrl(day.icon || '01d')"
              :alt="day.condition"
              class="forecast-weather-icon"
            >
          </div>

          <div class="forecast-temp">
            <span class="temp-high">{{ day.temp_max ? Math.round(day.temp_max) : '--' }}°</span>
            <span class="temp-divider">/</span>
            <span class="temp-low">{{ day.temp_min ? Math.round(day.temp_min) : '--' }}°</span>
          </div>

          <div class="forecast-condition">
            {{ day.condition || '--' }}
          </div>

          <div class="forecast-details">
            <div class="forecast-detail-row">
              <span class="detail-icon">💨</span>
              <span>{{ day.wind_speed ? day.wind_speed.toFixed(1) : '--' }} m/s</span>
            </div>
            <div class="forecast-detail-row">
              <span class="detail-icon">📊</span>
              <span>{{ day.flight_safety?.score || '--' }}</span>
            </div>
            <div class="forecast-detail-row">
              <span class="detail-icon">💧</span>
              <span>{{ day.humidity || '--' }}%</span>
            </div>
          </div>
        </div>
      </div>
    </AppleCard>

    <!-- 路线天气预测 -->
    <AppleCard class="route-weather-card">
      <div class="card-header">
        <h2 class="section-title">路线天气预测</h2>
        <AppleButton variant="secondary" size="small" @click="showRoutePlanner = true">
          规划路线
        </AppleButton>
      </div>

      <div class="route-weather-content">
        <div class="route-controls">
          <select v-model="selectedRoute" class="apple-select">
            <option value="">选择航线</option>
            <option value="R001">机场到市中心航线</option>
            <option value="R002">东城至西城航线</option>
            <option value="R003">城区巡逻航线</option>
            <option value="R004">工业区巡查航线</option>
          </select>
        </div>

        <div v-if="routeWeather.length > 0" class="route-weather-list">
          <div
            v-for="(day, index) in routeWeather"
            :key="index"
            class="route-weather-item"
            :class="{ 'risk-high': day.risk_level === 'high', 'risk-medium': day.risk_level === 'medium' }"
          >
            <div class="route-date">
              <div class="day-name">{{ day.dayName }}</div>
              <div class="date-str">{{ day.dateStr }}</div>
            </div>

            <div class="route-weather-info">
              <img :src="getWeatherIconUrl(day.icon)" :alt="day.condition" class="route-weather-icon">
              <span class="route-condition">{{ day.condition }}</span>
            </div>

            <div class="route-temp">
              <span class="temp-max">{{ day.temp_max }}°C</span>
              <span class="temp-sep">/</span>
              <span class="temp-min">{{ day.temp_min }}°C</span>
            </div>

            <div class="route-wind">
              💨 {{ day.wind_speed }} m/s
            </div>

            <div class="route-humidity">
              💧 {{ day.humidity }}%
            </div>

            <div class="route-safety">
              <div class="safety-bar">
                <div class="safety-fill" :style="{ width: day.safety_score + '%', backgroundColor: getSafetyScoreColor(day.safety_score) }"></div>
              </div>
              <span class="safety-text">{{ day.safety_score }}%</span>
            </div>

            <span :class="['route-risk-badge', `risk-${day.risk_level}`]">
              {{ getRiskLevelText(day.risk_level) }}
            </span>
          </div>
        </div>

        <div v-else class="empty-route">
          <div class="empty-icon">🗺️</div>
          <p class="empty-text">请选择航线查看天气预测</p>
        </div>
      </div>
    </AppleCard>

    <!-- 风险详情模态框 -->
    <AppleModal
      v-if="showRiskDetails"
      title="详细风险分析"
      @close="showRiskDetails = false"
    >
      <div class="risk-details-content">
        <div class="risk-section">
          <h3 class="risk-section-title">风险因素</h3>
          <div class="risk-factors-list">
            <div
              v-for="(risk, index) in riskAssessment.risks"
              :key="index"
              class="risk-factor-item"
            >
              <div class="risk-factor-header">
                <span :class="['risk-level-indicator', `level-${risk.level}`]">
                  {{ getRiskEmoji(risk.type) }}
                </span>
                <span class="risk-level-label">{{ getRiskLevelText(risk.level) }}</span>
              </div>
              <p class="risk-factor-description">{{ risk.description }}</p>
              <div class="risk-factor-values">
                <span>数值: {{ risk.value }}</span>
                <span>阈值: {{ risk.threshold }}</span>
              </div>
            </div>
          </div>
        </div>

        <div class="risk-section">
          <h3 class="risk-section-title">警告信息</h3>
          <div class="risk-factors-list">
            <div
              v-for="(warning, index) in riskAssessment.warnings"
              :key="index"
              class="risk-factor-item warning"
            >
              <div class="risk-factor-header">
                <span class="risk-level-indicator level-warning">⚠️</span>
                <span class="risk-level-label">警告</span>
              </div>
              <p class="risk-factor-description">{{ warning.description }}</p>
            </div>
          </div>
        </div>

        <div class="risk-section">
          <h3 class="risk-section-title">综合评估</h3>
          <div class="risk-overview">
            <div class="overview-metric">
              <span class="metric-label">总体风险等级</span>
              <span :class="['metric-value', `risk-${riskAssessment.overallRisk}`]">
                {{ getRiskLevelText(riskAssessment.overallRisk) }}
              </span>
            </div>
            <div class="overview-metric">
              <span class="metric-label">风险项总数</span>
              <span class="metric-value">{{ riskAssessment.risks.length + riskAssessment.warnings.length }}</span>
            </div>
            <div class="overview-metric">
              <span class="metric-label">高风险项</span>
              <span class="metric-value">{{ riskAssessment.risks.filter(r => r.level === 'high').length }}</span>
            </div>
          </div>
        </div>

        <div class="risk-section">
          <h3 class="risk-section-title">推荐措施</h3>
          <ul class="measures-list">
            <li v-for="(rec, index) in riskAssessment.recommendations" :key="index" class="measure-item">
              <span class="measure-check">✓</span>
              <span>{{ rec }}</span>
            </li>
          </ul>
        </div>
      </div>

      <template #footer>
        <AppleButton variant="secondary" @click="showRiskDetails = false">
          关闭
        </AppleButton>
      </template>
    </AppleModal>

    <!-- 路线规划模态框 -->
    <AppleModal
      v-if="showRoutePlanner"
      title="路线规划"
      @close="showRoutePlanner = false"
    >
      <form class="route-planner-form" @submit.prevent="saveRoute">
        <div class="form-group">
          <label class="form-label">起点位置</label>
          <input
            v-model="routeStart"
            type="text"
            class="apple-input"
            placeholder="输入起点坐标或地址"
          />
        </div>

        <div class="form-group">
          <label class="form-label">终点位置</label>
          <input
            v-model="routeEnd"
            type="text"
            class="apple-input"
            placeholder="输入终点坐标或地址"
          />
        </div>

        <div class="form-group">
          <label class="form-label">飞行高度 (米)</label>
          <input
            v-model.number="routeAltitude"
            type="number"
            class="apple-input"
            min="0"
            max="500"
            step="10"
          />
        </div>

        <div v-if="routePreview.length > 0" class="route-preview-section">
          <h3 class="preview-title">路线预览</h3>

          <div class="preview-map">
            <div class="map-placeholder">
              <span class="map-icon">🗺️</span>
              <p>路线预览地图</p>
            </div>
          </div>

          <div class="route-stats">
            <div class="stat-box">
              <span class="stat-label">总距离</span>
              <span class="stat-value">{{ routePreviewDistance.toFixed(1) }} km</span>
            </div>
            <div class="stat-box">
              <span class="stat-label">预计时间</span>
              <span class="stat-value">{{ routePreviewDuration }} 分钟</span>
            </div>
            <div class="stat-box">
              <span class="stat-label">平均速度</span>
              <span class="stat-value">{{ routePreviewSpeed.toFixed(1) }} km/h</span>
            </div>
            <div class="stat-box">
              <span class="stat-label">最大高度</span>
              <span class="stat-value">{{ routeAltitude }} m</span>
            </div>
          </div>
        </div>

        <div class="form-actions">
          <AppleButton
            variant="secondary"
            :disabled="!routeStart || !routeEnd"
            @click="generateRoutePreview"
          >
            生成预览
          </AppleButton>
        </div>
      </form>

      <template #footer>
        <AppleButton variant="secondary" @click="showRoutePlanner = false">
          取消
        </AppleButton>
        <AppleButton
          :disabled="!routeStart || !routeEnd"
          @click="saveRoute"
        >
          保存为新航线
        </AppleButton>
      </template>
    </AppleModal>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div v-if="showToast" :class="['toast-notification', toastType]">
        <div class="toast-icon">{{ toastIcon }}</div>
        <div class="toast-message">{{ toastMessage }}</div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed, watch, nextTick, onBeforeUnmount } from 'vue'
import AppleCard from '@/components/apple/AppleCard.vue'
import AppleButton from '@/components/apple/AppleButton.vue'
import AppleModal from '@/components/apple/AppleModal.vue'
import weatherApi from '@/services/weatherApi'
import {
  mockForecast,
  generateMockRouteWeather,
  mockCurrentWeather,
  mockFlightSafetyData
} from '@/config/mockWeatherData'

// 响应式数据
const loading = ref(false)
const locating = ref(false)
const showRiskDetails = ref(false)
const showRoutePlanner = ref(false)

// Toast 通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

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
const currentLocation = ref({ lat: 39.9042, lon: 116.4074 })

const activeRequests = new Set()

const routeStart = ref('')
const routeEnd = ref('')
const routeAltitude = ref(100)
const routePreview = ref([])

// 计算属性
const weatherLocation = computed(() => {
  if (currentWeather.value?.weather?.location) {
    return currentWeather.value.weather.location
  }
  return '未知位置'
})

const routePreviewDistance = computed(() => {
  if (routePreview.value.length === 0) return 0
  return routePreview.value.length - 1
})

const routePreviewDuration = computed(() => {
  if (routePreviewDistance.value === 0) return 0
  return Math.round(routePreviewDistance.value / 30 * 60)
})

const routePreviewSpeed = computed(() => {
  if (routePreviewDuration.value === 0) return 0
  return (routePreviewDistance.value / routePreviewDuration.value) * 60
})

const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

// 方法
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

const autoLocate = () => {
  if (!navigator.geolocation) {
    showToastNotification('您的浏览器不支持定位功能', 'error')
    return
  }

  locating.value = true
  showToastNotification('正在获取您的位置...', 'info')

  navigator.geolocation.getCurrentPosition(
    (position) => {
      const { latitude, longitude } = position.coords
      currentLocation.value = { lat: latitude, lon: longitude }
      showToastNotification(`定位成功: ${latitude.toFixed(4)}, ${longitude.toFixed(4)}`, 'success')
      locating.value = false
      refreshWeatherData()
    },
    (error) => {
      locating.value = false
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
      showToastNotification(errorMessage, 'error')
    },
    { enableHighAccuracy: true, timeout: 10000, maximumAge: 0 }
  )
}

const refreshWeatherData = async () => {
  loading.value = true

  try {
    const [weatherResponse, forecastResponse, safetyResponse] = await Promise.all([
      weatherApi.getCurrentWeatherByCoords(currentLocation.value.lat, currentLocation.value.lon),
      weatherApi.getForecast(currentLocation.value.lat, currentLocation.value.lon),
      weatherApi.checkFlightSafety(currentLocation.value.lat, currentLocation.value.lon)
    ])

    if (weatherResponse && weatherResponse.success) {
      currentWeather.value = weatherResponse.data
    } else {
      throw new Error('获取天气数据失败')
    }

    if (forecastResponse && forecastResponse.success) {
      forecast.value = forecastResponse.data.forecast || []
    } else {
      throw new Error('获取预报数据失败')
    }

    if (safetyResponse && safetyResponse.success) {
      flightSafetyData.value = safetyResponse.data
      buildRiskAssessment(safetyResponse.data)
    } else {
      throw new Error('获取安全评估数据失败')
    }

    showToastNotification('天气数据加载成功', 'success')
  } catch (error) {
    showToastNotification('外部天气API连接失败（可能被防火墙屏蔽），已切换到演示模式', 'warning')
    useMockData()
  } finally {
    loading.value = false
  }
}

const useMockData = () => {
  currentWeather.value = mockCurrentWeather

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
    flight_safety: { score: 85, safe: true }
  }))

  flightSafetyData.value = mockFlightSafetyData
  buildRiskAssessment(flightSafetyData.value)
}

const buildRiskAssessment = (safetyData) => {
  if (!safetyData || !safetyData.safety) return

  const safety = safetyData.safety
  const weather = safetyData.weather

  const risks = []
  const warnings = []
  const recommendations = []

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

  if (safety.warnings && safety.warnings.length > 0) {
    safety.warnings.forEach(warning => {
      warnings.push({ type: 'warning', description: warning })
    })
  }

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

  riskAssessment.value = { overallRisk, risks, warnings, recommendations }
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

const getRiskEmoji = (type) => {
  const emojis = {
    'wind': '💨',
    'temperature': '🌡️',
    'visibility': '👁️',
    'precipitation': '🌧️',
    'warning': '⚠️'
  }
  return emojis[type] || '⚠️'
}

const getWeatherIconUrl = (iconCode) => {
  return `https://openweathermap.org/img/wn/${iconCode}@2x.png`
}

const getTimeSlot = (index) => {
  const now = new Date()
  const currentHour = now.getHours()
  const baseHour = Math.floor(currentHour / 3) * 3
  const startHour = (baseHour + index * 3) % 24
  const endHour = (startHour + 3) % 24

  const formatHour = (hour) => `${hour.toString().padStart(2, '0')}:00`
  return `${formatHour(startHour)} ~ ${formatHour(endHour)}`
}

const getTimeSlotDate = (index) => {
  const now = new Date()
  const currentHour = now.getHours()
  const baseHour = Math.floor(currentHour / 3) * 3
  const actualStartHour = baseHour + index * 3
  const dayOffset = Math.floor(actualStartHour / 24)

  const slotDate = new Date(now)
  slotDate.setDate(slotDate.getDate() + dayOffset)

  const days = ['周日', '周一', '周二', '周三', '周四', '周五', '周六']
  const dayName = days[slotDate.getDay()]
  const dateStr = `${slotDate.getMonth() + 1}月${slotDate.getDate()}日`

  return `${dayName} ${dateStr}`
}

const getSafetyScoreColor = (score) => {
  if (score >= 80) return '#34C759'
  if (score >= 60) return '#FF9500'
  if (score >= 40) return '#FF3B30'
  return '#8E8E93'
}

const generateRoutePreview = () => {
  if (!routeStart.value || !routeEnd.value) return

  routePreview.value = Array.from({ length: 5 }, (_, i) => ({
    lat: 39.90923 + i * 0.005,
    lng: 116.397428 + i * 0.005
  }))

  showToastNotification('路线预览已生成', 'info')
}

const saveRoute = () => {
  if (!routeStart.value || !routeEnd.value) {
    showToastNotification('请填写起点和终点', 'warning')
    return
  }

  showToastNotification('航线已保存', 'success')
  showRoutePlanner.value = false
  resetRouteForm()
}

const resetRouteForm = () => {
  routeStart.value = ''
  routeEnd.value = ''
  routeAltitude.value = 100
  routePreview.value = []
}

const loadRouteWeatherData = () => {
  if (!selectedRoute.value) {
    routeWeather.value = []
    return
  }

  isLoadingRouteWeather = true

  try {
    const startDate = new Date()
    routeWeather.value = generateMockRouteWeather(startDate)
    const dateStr = `${startDate.getMonth() + 1}月${startDate.getDate()}日`
    showToastNotification(`已加载航线 ${selectedRoute.value} 从${dateStr}开始的5天天气预报`, 'success')
  } finally {
    nextTick(() => {
      isLoadingRouteWeather = false
    })
  }
}

let isLoadingRouteWeather = false

watch(selectedRoute, (newRoute) => {
  if (isLoadingRouteWeather) return
  if (newRoute) {
    loadRouteWeatherData()
  } else {
    routeWeather.value = []
  }
})

onBeforeUnmount(() => {
  activeRequests.forEach(controller => {
    controller.abort()
  })
  activeRequests.clear()
})

refreshWeatherData()
</script>

<style scoped>
.weather-integration {
  padding: var(--space-6, 24px);
  max-width: 1400px;
  margin: 0 auto;
}

/* 页面头部 */
.page-header {
  margin-bottom: var(--space-8, 32px);
}

.page-title {
  margin: 0 0 var(--space-2, 8px) 0;
  font-size: var(--font-size-3xl, 32px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  letter-spacing: -0.02em;
}

.page-subtitle {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
}

.header-actions {
  display: flex;
  gap: var(--space-3, 12px);
}

/* 统计卡片 */
.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: var(--space-4, 16px);
  margin-bottom: var(--space-6, 24px);
}

.stat-card {
  padding: var(--space-5, 20px);
  transition: all var(--transition-smooth, 0.3s ease);
}

.stat-card:hover {
  transform: translateY(-2px);
}

.stat-content {
  display: flex;
  align-items: center;
  gap: var(--space-4, 16px);
}

.stat-icon {
  font-size: 40px;
  line-height: 1;
}

.stat-info {
  flex: 1;
}

.stat-value {
  font-size: var(--font-size-2xl, 28px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.2;
  margin-bottom: var(--space-1, 4px);
}

.stat-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 实时天气卡片 */
.current-weather-card {
  padding: var(--space-6, 24px);
  margin-bottom: var(--space-6, 24px);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-5, 20px);
}

.section-title {
  margin: 0;
  font-size: var(--font-size-xl, 20px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.risk-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
}

.risk-low {
  background: #E8F5E9;
  color: #2E7D32;
}

.risk-low_medium {
  background: #E3F2FD;
  color: #1976D2;
}

.risk-medium {
  background: #FFF3E0;
  color: #F57C00;
}

.risk-high {
  background: #FFEBEE;
  color: #C62828;
}

.risk-unknown {
  background: var(--color-bg-secondary, #F5F5F7);
  color: var(--color-text-secondary, #86868B);
}

.weather-content {
  display: flex;
  gap: var(--space-8, 32px);
  align-items: flex-start;
}

.weather-main {
  flex: 1;
}

.weather-location {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
  margin-bottom: var(--space-5, 20px);
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.location-icon {
  font-size: 20px;
}

.weather-details-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: var(--space-4, 16px);
}

.weather-detail-item {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
}

.detail-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

.detail-value {
  font-size: var(--font-size-base, 16px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.weather-visual {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-5, 20px);
  border-left: 1px solid var(--color-border, #D2D2D7);
  min-width: 200px;
}

.weather-icon-container {
  width: 100px;
  height: 100px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
}

.weather-icon {
  width: 80px;
  height: 80px;
}

.weather-condition-text {
  text-align: center;
}

.weather-condition-text h3 {
  margin: 0 0 var(--space-1, 4px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.weather-condition-text p {
  margin: 0;
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 风险评估卡片 */
.risk-assessment-card {
  padding: var(--space-6, 24px);
  margin-bottom: var(--space-6, 24px);
}

.risk-content {
  margin-top: var(--space-5, 20px);
}

.risk-items-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3, 12px);
  margin-bottom: var(--space-6, 24px);
}

.risk-item {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  border-left: 3px solid var(--color-primary, #0071E3);
}

.risk-indicator {
  width: 40px;
  height: 40px;
  border-radius: var(--radius-md, 8px);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 20px;
  flex-shrink: 0;
}

.risk-level-low {
  background: #E8F5E9;
}

.risk-level-medium {
  background: #FFF3E0;
}

.risk-level-high {
  background: #FFEBEE;
}

.warning-indicator {
  background: #FFF3E0;
}

.risk-details h4 {
  margin: 0 0 var(--space-1, 4px) 0;
  font-size: var(--font-size-base, 16px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.risk-level-text {
  margin: 0;
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

.recommendations-section {
  padding: var(--space-5, 20px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
}

.recommendations-title {
  margin: 0 0 var(--space-3, 12px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.recommendations-list {
  list-style: none;
  padding: 0;
  margin: 0;
}

.recommendations-list li {
  display: flex;
  gap: var(--space-2, 8px);
  margin-bottom: var(--space-2, 8px);
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.6;
}

.recommendation-bullet {
  color: var(--color-primary, #0071E3);
  font-weight: bold;
}

/* 预报卡片 */
.forecast-card {
  padding: var(--space-6, 24px);
  margin-bottom: var(--space-6, 24px);
}

.update-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-xs, 12px);
  color: var(--color-text-secondary, #86868B);
}

.forecast-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: var(--space-4, 16px);
  margin-top: var(--space-5, 20px);
}

.forecast-item {
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  text-align: center;
  transition: all var(--transition-smooth, 0.3s ease);
}

.forecast-item:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08);
}

.forecast-today {
  background: var(--color-primary, #0071E3);
  color: white;
}

.forecast-time {
  margin-bottom: var(--space-2, 8px);
}

.time-slot {
  display: block;
  font-weight: 600;
  font-size: var(--font-size-sm, 14px);
}

.time-date {
  display: block;
  font-size: var(--font-size-xs, 12px);
  opacity: 0.8;
}

.forecast-icon {
  margin: var(--space-2, 8px) 0;
}

.forecast-weather-icon {
  width: 60px;
  height: 60px;
}

.forecast-temp {
  display: flex;
  justify-content: center;
  gap: var(--space-1, 4px);
  margin: var(--space-2, 8px) 0;
  font-weight: 600;
}

.temp-high {
  color: #FF3B30;
}

.temp-divider {
  opacity: 0.5;
}

.temp-low {
  color: #007AFF;
}

.forecast-condition {
  margin-bottom: var(--space-2, 8px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
}

.forecast-details {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
  font-size: var(--font-size-xs, 12px);
}

.forecast-detail-row {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: var(--space-1, 4px);
}

.detail-icon {
  font-size: 14px;
}

/* 路线天气卡片 */
.route-weather-card {
  padding: var(--space-6, 24px);
  margin-bottom: var(--space-6, 24px);
}

.route-weather-content {
  margin-top: var(--space-5, 20px);
}

.route-controls {
  margin-bottom: var(--space-5, 20px);
}

.apple-select {
  width: 100%;
  max-width: 300px;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  background: var(--color-bg-primary, #FFFFFF);
  cursor: pointer;
  transition: all var(--transition-input, 0.2s ease);
  appearance: none;
  background-image: url("data:image/svg+xml,%3Csvg width='12' height='8' viewBox='0 0 12 8' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M1 1.5L6 6.5L11 1.5' stroke='%2386868B' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/%3E%3C/svg%3E");
  background-repeat: no-repeat;
  background-position: right 12px center;
  padding-right: 36px;
}

.apple-select:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.route-weather-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3, 12px);
}

.route-weather-item {
  display: grid;
  grid-template-columns: 100px 150px 120px 100px 80px 150px 120px;
  align-items: center;
  gap: var(--space-4, 16px);
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  transition: all var(--transition-smooth, 0.3s ease);
}

.route-weather-item:hover {
  transform: translateX(4px);
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
}

.route-date {
  display: flex;
  flex-direction: column;
}

.day-name {
  font-weight: 600;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
}

.date-str {
  font-size: var(--font-size-xs, 12px);
  color: var(--color-text-secondary, #86868B);
}

.route-weather-info {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
}

.route-weather-icon {
  width: 40px;
  height: 40px;
}

.route-condition {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-primary, #1D1D1F);
}

.route-temp {
  display: flex;
  gap: var(--space-1, 4px);
  font-size: var(--font-size-sm, 14px);
}

.temp-max {
  color: #FF3B30;
  font-weight: 600;
}

.temp-sep {
  color: var(--color-text-secondary, #86868B);
}

.temp-min {
  color: #007AFF;
}

.route-wind,
.route-humidity {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-primary, #1D1D1F);
}

.route-safety {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
}

.safety-bar {
  flex: 1;
  height: 8px;
  background: #E5E5EA;
  border-radius: var(--radius-full, 9999px);
  overflow: hidden;
}

.safety-fill {
  height: 100%;
  border-radius: var(--radius-full, 9999px);
  transition: width 0.3s ease;
}

.safety-text {
  font-size: var(--font-size-xs, 12px);
  font-weight: 600;
  color: var(--color-text-secondary, #86868B);
}

.route-risk-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-xs, 12px);
  font-weight: 500;
  text-align: center;
}

.empty-route {
  text-align: center;
  padding: var(--space-12, 48px);
}

.empty-icon {
  font-size: 64px;
  margin-bottom: var(--space-4, 16px);
  opacity: 0.5;
}

.empty-text {
  margin: 0;
  font-size: var(--font-size-lg, 18px);
  color: var(--color-text-secondary, #86868B);
}

/* 风险详情模态框 */
.risk-details-content {
  padding: var(--space-6, 24px) 0;
  max-height: 600px;
  overflow-y: auto;
}

.risk-section {
  margin-bottom: var(--space-6, 24px);
}

.risk-section:last-child {
  margin-bottom: 0;
}

.risk-section-title {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  padding-bottom: var(--space-2, 8px);
  border-bottom: 1px solid var(--color-border, #D2D2D7);
}

.risk-factors-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3, 12px);
}

.risk-factor-item {
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  border-left: 3px solid var(--color-primary, #0071E3);
}

.risk-factor-header {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
  margin-bottom: var(--space-2, 8px);
}

.risk-level-indicator {
  width: 32px;
  height: 32px;
  border-radius: var(--radius-md, 8px);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
}

.level-low {
  background: #E8F5E9;
}

.level-medium {
  background: #FFF3E0;
}

.level-high {
  background: #FFEBEE;
}

.level-warning {
  background: #FFF3E0;
}

.risk-level-label {
  font-weight: 600;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
}

.risk-factor-description {
  margin: 0 0 var(--space-2, 8px) 0;
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.6;
}

.risk-factor-values {
  display: flex;
  gap: var(--space-4, 16px);
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

.risk-overview {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: var(--space-4, 16px);
}

.overview-metric {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
}

.metric-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

.metric-value {
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.measures-list {
  list-style: none;
  padding: 0;
  margin: 0;
}

.measure-item {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
  margin-bottom: var(--space-2, 8px);
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.6;
}

.measure-check {
  color: #34C759;
  font-weight: bold;
  font-size: 18px;
}

/* 路线规划模态框 */
.route-planner-form {
  padding: var(--space-6, 24px) 0;
}

.form-group {
  margin-bottom: var(--space-5, 20px);
}

.form-label {
  display: block;
  margin-bottom: var(--space-2, 8px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.apple-input {
  width: 100%;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  transition: all var(--transition-input, 0.2s ease);
  background: var(--color-bg-primary, #FFFFFF);
  font-family: inherit;
}

.apple-input:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.route-preview-section {
  margin-top: var(--space-6, 24px);
  padding-top: var(--space-6, 24px);
  border-top: 1px solid var(--color-border, #D2D2D7);
}

.preview-title {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.preview-map {
  margin-bottom: var(--space-4, 16px);
}

.map-placeholder {
  height: 200px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  border: 2px dashed var(--color-border, #D2D2D7);
}

.map-icon {
  font-size: 48px;
  margin-bottom: var(--space-2, 8px);
  opacity: 0.5;
}

.map-placeholder p {
  margin: 0;
  color: var(--color-text-secondary, #86868B);
}

.route-stats {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
  gap: var(--space-3, 12px);
}

.stat-box {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: var(--space-3, 12px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-md, 8px);
}

.stat-label {
  font-size: var(--font-size-xs, 12px);
  color: var(--color-text-secondary, #86868B);
  margin-bottom: var(--space-1, 4px);
}

.stat-value {
  font-size: var(--font-size-base, 16px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.form-actions {
  margin-top: var(--space-4, 16px);
}

/* 响应式设计 */
@media (max-width: 768px) {
  .weather-integration {
    padding: var(--space-4, 16px);
  }

  .page-title {
    font-size: var(--font-size-2xl, 28px);
  }

  .header-actions {
    flex-direction: column;
  }

  .stats-grid {
    grid-template-columns: repeat(2, 1fr);
  }

  .weather-content {
    flex-direction: column;
  }

  .weather-visual {
    border-left: none;
    border-top: 1px solid var(--color-border, #D2D2D7);
    padding-top: var(--space-5, 20px);
  }

  .forecast-grid {
    grid-template-columns: 1fr;
  }

  .route-weather-item {
    grid-template-columns: 1fr;
    gap: var(--space-2, 8px);
  }

  .route-stats {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 480px) {
  .stats-grid {
    grid-template-columns: 1fr;
  }

  .weather-details-grid {
    grid-template-columns: 1fr;
  }
}

/* 深色模式 */
@media (prefers-color-scheme: dark) {
  .page-title,
  .section-title,
  .weather-location,
  .detail-value {
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .page-subtitle,
  .stat-label,
  .detail-label {
    color: var(--color-text-secondary-dark, #A1A1A6);
  }

  .apple-input,
  .apple-select {
    background: var(--color-bg-secondary-dark, #1C1C1E);
    border-color: var(--color-border-dark, #38383A);
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .forecast-item,
  .risk-item,
  .route-weather-item {
    background: var(--color-bg-secondary-dark, #1C1C1E);
  }
}

/* Toast 通知样式 */
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

.toast-icon {
  font-size: 24px;
  line-height: 1;
  flex-shrink: 0;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.5;
}

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0, 1, 1);
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

/* Toast 深色模式 */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: #1C1C1E;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
  }

  .toast-message {
    color: #F5F5F7;
  }
}
</style>
