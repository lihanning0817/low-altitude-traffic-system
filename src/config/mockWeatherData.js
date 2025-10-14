/**
 * Mock天气数据配置文件
 * 用于天气集成功能的演示和降级方案
 *
 * @description 当外部天气API不可用时,使用此Mock数据作为降级方案
 */

/**
 * Mock预报数据 - 未来8个时段(每3小时一个时段)
 * 用于当日每时段天气变化表
 */
export const mockForecast = [
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

/**
 * Mock航线天气条件配置
 * 用于生成未来5天的航线天气预报
 */
export const mockRouteWeatherConditions = [
  { condition: '晴朗', icon: '01d', temp_max: 22, temp_min: 15, wind: 3.5, humidity: 55, score: 95, risk: 'low' },
  { condition: '少云', icon: '02d', temp_max: 21, temp_min: 14, wind: 4.2, humidity: 60, score: 88, risk: 'low' },
  { condition: '多云', icon: '03d', temp_max: 20, temp_min: 13, wind: 5.5, humidity: 65, score: 72, risk: 'low_medium' },
  { condition: '阴天', icon: '04d', temp_max: 18, temp_min: 12, wind: 6.8, humidity: 70, score: 58, risk: 'medium' },
  { condition: '小雨', icon: '10d', temp_max: 16, temp_min: 11, wind: 8.2, humidity: 80, score: 45, risk: 'medium' }
]

/**
 * 生成未来5天的航线天气预报Mock数据
 *
 * @param {Date|null} startDate - 起始日期,默认为今天
 * @returns {Array} 包含5天天气数据的数组
 */
export const generateMockRouteWeather = (startDate = null) => {
  const baseDate = startDate ? new Date(startDate) : new Date()

  return mockRouteWeatherConditions.map((weather, index) => {
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

/**
 * Mock当前天气数据
 * 用于降级方案的默认天气显示
 */
export const mockCurrentWeather = {
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

/**
 * Mock飞行安全数据
 * 用于降级方案的安全评估显示
 */
export const mockFlightSafetyData = {
  safety: {
    safe: true,
    score: 85,
    warnings: ['演示模式:天气条件良好']
  },
  weather: mockCurrentWeather.weather
}
