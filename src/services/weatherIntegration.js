/**
 * 气象数据集成模块
 * 负责获取和处理气象数据，为飞行路径规划提供支持
 */

class WeatherIntegrationService {
  constructor() {
    // 气象数据缓存
    this.weatherCache = new Map();
    // 缓存过期时间(毫秒)
    this.cacheExpiry = 30 * 60 * 1000; // 30分钟
    // 气象API配置
    this.apiConfig = {
      baseUrl: 'https://api.openweathermap.org/data/2.5',
      apiKey: process.env.WEATHER_API_KEY || 'your-api-key-here',
      units: 'metric'
    };
    // 风险阈值
    this.riskThresholds = {
      windSpeed: 15, // m/s
      visibility: 1000, // 米
      precipitation: 5, // mm/h
      temperature: {
        min: -10, // °C
        max: 45   // °C
      }
    };
  }

  /**
   * 获取指定区域的当前天气
   */
  async getCurrentWeather(lat, lng) {
    const cacheKey = `${lat.toFixed(2)},${lng.toFixed(2)}`;
    const cached = this.weatherCache.get(cacheKey);

    // 检查缓存
    if (cached && (Date.now() - cached.timestamp) < this.cacheExpiry) {
      return cached.data;
    }

    try {
      // 实际项目中这里会调用真实的气象API
      // 以下为模拟数据
      const weatherData = this.generateMockWeatherData(lat, lng);

      // 更新缓存
      this.weatherCache.set(cacheKey, {
        data: weatherData,
        timestamp: Date.now()
      });

      return weatherData;
    } catch (error) {
      console.error('获取天气数据失败:', error);
      throw new Error('无法获取天气数据');
    }
  }

  /**
   * 生成模拟天气数据(实际项目中应调用真实API)
   */
  generateMockWeatherData(lat, lng) {
    // 根据经纬度生成相对合理的天气数据
    const baseTemp = 20 - Math.abs(lat - 40) * 0.5; // 假设纬度40°附近温度最高
    const temperature = baseTemp + (Math.random() * 10 - 5);

    return {
      coord: { lat, lng },
      weather: [
        {
          id: 800,
          main: "Clear",
          description: "晴朗",
          icon: "01d"
        }
      ],
      main: {
        temp: parseFloat(temperature.toFixed(1)),
        feels_like: parseFloat((temperature - 1).toFixed(1)),
        temp_min: parseFloat((temperature - 3).toFixed(1)),
        temp_max: parseFloat((temperature + 3).toFixed(1)),
        pressure: 1013,
        humidity: 65
      },
      visibility: 10000,
      wind: {
        speed: parseFloat((Math.random() * 5).toFixed(1)),
        deg: Math.floor(Math.random() * 360)
      },
      clouds: {
        all: Math.floor(Math.random() * 20)
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
      name: "Location",
      cod: 200
    };
  }

  /**
   * 获取天气预报
   */
  async getWeatherForecast(lat, lng, hours = 24) {
    // 实际项目中这里会调用天气预报API
    // 以下为模拟数据
    const forecast = [];
    const now = new Date();

    for (let i = 1; i <= hours; i++) {
      const time = new Date(now.getTime() + i * 60 * 60 * 1000);
      const baseTemp = 20 - Math.abs(lat - 40) * 0.5;
      const temperature = baseTemp + (Math.random() * 8 - 4);

      forecast.push({
        dt: Math.floor(time.getTime() / 1000),
        main: {
          temp: parseFloat(temperature.toFixed(1)),
          feels_like: parseFloat((temperature - 1).toFixed(1)),
          temp_min: parseFloat((temperature - 2).toFixed(1)),
          temp_max: parseFloat((temperature + 2).toFixed(1)),
          pressure: 1013 + (Math.random() * 20 - 10),
          humidity: 50 + Math.floor(Math.random() * 30)
        },
        weather: [
          {
            id: Math.floor(Math.random() * 800) + 200,
            main: ["Thunderstorm", "Drizzle", "Rain", "Snow", "Clear", "Clouds"][Math.floor(Math.random() * 6)],
            description: ["雷暴", "毛毛雨", "雨", "雪", "晴朗", "多云"][Math.floor(Math.random() * 6)],
            icon: "01d"
          }
        ],
        clouds: {
          all: Math.floor(Math.random() * 100)
        },
        wind: {
          speed: parseFloat((Math.random() * 10).toFixed(1)),
          deg: Math.floor(Math.random() * 360),
          gust: parseFloat((Math.random() * 15).toFixed(1))
        },
        visibility: 5000 + Math.floor(Math.random() * 5000),
        pop: Math.random(),
        dt_txt: time.toISOString().replace('T', ' ').substring(0, 19)
      });
    }

    return {
      cod: "200",
      message: 0,
      cnt: hours,
      list: forecast,
      city: {
        id: 1816670,
        name: "Location",
        coord: { lat, lon: lng },
        country: "CN",
        population: 1000000,
        timezone: 28800,
        sunrise: Math.floor(Date.now() / 1000) + 21600,
        sunset: Math.floor(Date.now() / 1000) + 64800
      }
    };
  }

  /**
   * 评估飞行天气风险
   */
  assessFlightRisk(weatherData) {
    const risks = [];
    const warnings = [];

    // 风速风险评估
    if (weatherData.wind.speed > this.riskThresholds.windSpeed) {
      risks.push({
        type: 'wind',
        level: weatherData.wind.speed > this.riskThresholds.windSpeed * 1.5 ? 'high' : 'medium',
        value: weatherData.wind.speed,
        threshold: this.riskThresholds.windSpeed,
        description: `风速过高: ${weatherData.wind.speed} m/s`
      });
    }

    // 能见度风险评估
    if (weatherData.visibility < this.riskThresholds.visibility) {
      risks.push({
        type: 'visibility',
        level: weatherData.visibility < this.riskThresholds.visibility / 2 ? 'high' : 'medium',
        value: weatherData.visibility,
        threshold: this.riskThresholds.visibility,
        description: `能见度不足: ${weatherData.visibility} 米`
      });
    }

    // 降水风险评估
    const precipitation = this.extractPrecipitation(weatherData);
    if (precipitation > this.riskThresholds.precipitation) {
      risks.push({
        type: 'precipitation',
        level: precipitation > this.riskThresholds.precipitation * 2 ? 'high' : 'medium',
        value: precipitation,
        threshold: this.riskThresholds.precipitation,
        description: `降水强度过大: ${precipitation} mm/h`
      });
    }

    // 温度风险评估
    if (weatherData.main.temp < this.riskThresholds.temperature.min) {
      risks.push({
        type: 'temperature',
        level: 'high',
        value: weatherData.main.temp,
        threshold: this.riskThresholds.temperature.min,
        description: `温度过低: ${weatherData.main.temp}°C`
      });
    } else if (weatherData.main.temp > this.riskThresholds.temperature.max) {
      risks.push({
        type: 'temperature',
        level: 'high',
        value: weatherData.main.temp,
        threshold: this.riskThresholds.temperature.max,
        description: `温度过高: ${weatherData.main.temp}°C`
      });
    }

    // 天气状况警告
    const weatherCondition = weatherData.weather[0].main;
    if (['Thunderstorm', 'Snow'].includes(weatherCondition)) {
      warnings.push({
        type: 'severe_weather',
        level: 'high',
        condition: weatherCondition,
        description: `恶劣天气: ${weatherData.weather[0].description}`
      });
    } else if (['Rain', 'Drizzle'].includes(weatherCondition)) {
      warnings.push({
        type: 'moderate_weather',
        level: 'medium',
        condition: weatherCondition,
        description: `降水天气: ${weatherData.weather[0].description}`
      });
    }

    // 计算总体风险等级
    const highRisks = risks.filter(r => r.level === 'high').length;
    const mediumRisks = risks.filter(r => r.level === 'medium').length;
    const severeWarnings = warnings.filter(w => w.level === 'high').length;

    let overallRisk = 'low';
    if (highRisks > 0 || severeWarnings > 0) {
      overallRisk = 'high';
    } else if (mediumRisks > 1) {
      overallRisk = 'medium';
    } else if (mediumRisks > 0) {
      overallRisk = 'low_medium';
    }

    return {
      overallRisk: overallRisk,
      risks: risks,
      warnings: warnings,
      recommendations: this.generateRecommendations(risks, warnings)
    };
  }

  /**
   * 提取降水数据
   */
  extractPrecipitation(weatherData) {
    // 实际项目中可能从weatherData.rain或weatherData.snow中获取
    // 这里根据天气状况模拟降水数据
    const weatherMain = weatherData.weather[0].main;
    switch (weatherMain) {
      case 'Thunderstorm': return 10 + Math.random() * 15;
      case 'Rain': return 3 + Math.random() * 7;
      case 'Drizzle': return 1 + Math.random() * 2;
      case 'Snow': return 2 + Math.random() * 5;
      default: return 0;
    }
  }

  /**
   * 生成飞行建议
   */
  generateRecommendations(risks, warnings) {
    const recommendations = [];

    // 基于风险生成建议
    risks.forEach(risk => {
      switch (risk.type) {
        case 'wind':
          recommendations.push('建议降低飞行高度以减少风阻影响');
          if (risk.level === 'high') {
            recommendations.push('强风天气，建议推迟飞行任务');
          }
          break;
        case 'visibility':
          recommendations.push('能见度较低，建议开启所有导航灯光');
          if (risk.level === 'high') {
            recommendations.push('能见度严重不足，建议取消飞行任务');
          }
          break;
        case 'precipitation':
          recommendations.push('有降水，注意防水措施');
          if (risk.level === 'high') {
            recommendations.push('强降水天气，建议取消飞行任务');
          }
          break;
        case 'temperature':
          recommendations.push('注意温度对电池性能的影响');
          if (risk.level === 'high') {
            recommendations.push('极端温度，建议检查设备适应性');
          }
          break;
      }
    });

    // 基于警告生成建议
    warnings.forEach(warning => {
      if (warning.level === 'high') {
        recommendations.push('检测到恶劣天气，强烈建议推迟飞行');
      } else {
        recommendations.push('检测到不利天气条件，请谨慎飞行');
      }
    });

    // 如果没有风险，给出积极建议
    if (risks.length === 0 && warnings.length === 0) {
      recommendations.push('天气条件良好，适合飞行');
    }

    return recommendations;
  }

  /**
   * 获取路线天气信息
   */
  async getRouteWeather(route) {
    if (!route || !route.nodes || route.nodes.length === 0) {
      throw new Error('路线信息不完整');
    }

    const routeWeather = [];

    // 获取路线各节点的天气
    for (const node of route.nodes) {
      try {
        const weather = await this.getCurrentWeather(node.lat, node.lng);
        routeWeather.push({
          node: node,
          weather: weather,
          riskAssessment: this.assessFlightRisk(weather)
        });
      } catch (error) {
        console.warn(`无法获取节点(${node.lat}, ${node.lng})的天气数据:`, error.message);
        routeWeather.push({
          node: node,
          weather: null,
          riskAssessment: null,
          error: error.message
        });
      }
    }

    return routeWeather;
  }

  /**
   * 获取区域天气概况
   */
  async getAreaWeatherOverview(boundaries) {
    // 计算区域中心点
    const centerLat = (boundaries.minLat + boundaries.maxLat) / 2;
    const centerLng = (boundaries.minLng + boundaries.maxLng) / 2;

    try {
      const weather = await this.getCurrentWeather(centerLat, centerLng);
      const riskAssessment = this.assessFlightRisk(weather);

      return {
        center: { lat: centerLat, lng: centerLng },
        weather: weather,
        riskAssessment: riskAssessment,
        boundaries: boundaries
      };
    } catch (error) {
      throw new Error(`无法获取区域天气信息: ${error.message}`);
    }
  }

  /**
   * 设置风险阈值
   */
  setRiskThresholds(thresholds) {
    this.riskThresholds = {
      ...this.riskThresholds,
      ...thresholds
    };
  }

  /**
   * 清除过期缓存
   */
  clearExpiredCache() {
    const now = Date.now();
    for (const [key, value] of this.weatherCache) {
      if ((now - value.timestamp) >= this.cacheExpiry) {
        this.weatherCache.delete(key);
      }
    }
  }

  /**
   * 获取缓存统计
   */
  getCacheStats() {
    return {
      size: this.weatherCache.size,
      expiry: this.cacheExpiry
    };
  }
}

// 导出单例实例
const weatherIntegrationService = new WeatherIntegrationService();
export default weatherIntegrationService;

// 导出类以供需要多个实例时使用
export { WeatherIntegrationService };