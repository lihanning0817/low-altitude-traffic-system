// API服务 - 连接后端数据库
class ApiService {
  constructor() {
    this.token = null
    this.baseURL = 'http://localhost:8081/api/v1'
  }

  setToken(token) {
    this.token = token
  }

  clearToken() {
    this.token = null
  }

  // 请求配置
  async request(endpoint, options = {}) {
    const url = `${this.baseURL}${endpoint}`;
    const headers = {
      'Content-Type': 'application/json',
      ...options.headers
    }

    if (this.token) {
      headers['Authorization'] = `Bearer ${this.token}`
    }

    // 设置超时时间（默认30秒）
    const timeout = options.timeout || 30000;
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), timeout);

    const config = {
      method: options.method || 'GET',
      headers,
      signal: controller.signal,
      ...options.body && { body: JSON.stringify(options.body) }
    }

    try {
      const response = await fetch(url, config);
      clearTimeout(timeoutId);

      if (!response.ok) {
        const errorData = await response.json().catch(() => ({}));
        throw new Error(errorData.error || `HTTP error! status: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      clearTimeout(timeoutId);
      if (error.name === 'AbortError') {
        throw new Error(`请求超时 (${timeout}ms)`);
      }
      throw error;
    }
  }

  // 系统健康检查
  async getHealth() {
    return this.request('/health')
  }

  // 获取系统信息
  async getSystemInfo() {
    return this.request('/info')
  }

  // 获取无人机列表
  async getDrones() {
    return this.request('/drones')
  }

  // 获取任务列表
  async getTasks() {
    return this.request('/tasks')
  }

  // 获取用户列表
  async getUsers() {
    return this.request('/users')
  }

  // 用户认证
  async login(userData) {
    return this.request('/users/login', {
      method: 'POST',
      body: userData
    })
  }

  async register(userData) {
    return this.request('/users/register', {
      method: 'POST',
      body: userData
    })
  }

  // 飞行任务管理
  // 修复：后端端点为 /tasks 而非 /flight-tasks
  async getFlightTasks() {
    return this.request('/tasks')
  }

  async createFlightTask(taskData) {
    return this.request('/tasks', {
      method: 'POST',
      body: taskData
    })
  }

  async updateFlightTask(id, updates) {
    return this.request(`/tasks/${id}`, {
      method: 'PUT',
      body: updates
    })
  }

  async deleteFlightTask(taskId) {
    return this.request(`/tasks/${taskId}`, {
      method: 'DELETE'
    })
  }

  // 设备管理
  // 已实现：后端API已完成
  async getDevices() {
    return this.request('/devices')
  }

  // 空域管理
  // 已实现：后端API已完成
  async getAirspaces() {
    return this.request('/airspaces')
  }

  // 飞行许可API
  // 已实现：后端API已完成
  async applyFlightPermit(permitData) {
    return this.request('/flight-permits', {
      method: 'POST',
      body: permitData
    })
  }

  async getFlightPermits() {
    return this.request('/flight-permits')
  }

  // 气象数据
  async getCurrentWeather(params) {
    const queryString = new URLSearchParams(params).toString();
    return this.request(`/weather?${queryString}`);
  }

  async getRouteWeather(route) {
    return this.request('/weather/route', {
      method: 'POST',
      body: route
    })
  }

  // 应急响应
  // 修复：后端端点为 /emergency/events 而非 /emergency-events
  async reportEmergency(emergencyData) {
    return this.request('/emergency/events', {
      method: 'POST',
      body: emergencyData
    })
  }

  async getEmergencyEvents() {
    return this.request('/emergency/events')
  }

  // 已实现：紧急降落点API
  async getEmergencyLandingPoints() {
    return this.request('/emergency-landing-points')
  }

  // 交通管理
  // 已实现：飞行注册和冲突检测API
  async registerFlight(flightData) {
    return this.request('/flights', {
      method: 'POST',
      body: flightData
    })
  }

  async getFlightConflicts() {
    return this.request('/flight-conflicts')
  }

  // 路径规划
  // 修复：后端端点为 /map/route 而非 /routes/plan
  // 后端使用GET方法通过查询参数传递origin和destination
  async planRoute(routeData) {
    const { origin, destination, strategy = '0' } = routeData
    const queryString = new URLSearchParams({ origin, destination, strategy }).toString()
    return this.request(`/map/route?${queryString}`, {
      method: 'GET'
    })
  }

  // 已实现：获取所有路线
  async getRoutes() {
    return this.request('/routes')
  }
}

export default new ApiService()
