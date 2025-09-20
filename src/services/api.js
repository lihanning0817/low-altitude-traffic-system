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

    const config = {
      method: options.method || 'GET',
      headers,
      ...options.body && { body: JSON.stringify(options.body) }
    }

    const response = await fetch(url, config);
    
    if (!response.ok) {
      const errorData = await response.json().catch(() => ({}));
      throw new Error(errorData.error || `HTTP error! status: ${response.status}`);
    }
    
    return await response.json();
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
  async getFlightTasks() {
    return this.request('/flight-tasks')
  }

  async createFlightTask(taskData) {
    return this.request('/flight-tasks', {
      method: 'POST',
      body: taskData
    })
  }

  async updateFlightTask(id, updates) {
    return this.request(`/flight-tasks/${id}`, {
      method: 'PUT',
      body: updates
    })
  }

  async deleteFlightTask(taskId) {
    return this.request(`/flight-tasks/${taskId}`, {
      method: 'DELETE'
    })
  }

  // 设备管理
  async getDevices() {
    return this.request('/devices')
  }

  // 空域管理
  async getAirspaces() {
    return this.request('/airspaces')
  }

  async applyFlightPermit(permitData) {
    return this.request('/flight-permits', {
      method: 'POST',
      body: permitData
    })
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
  async reportEmergency(emergencyData) {
    return this.request('/emergency-events', {
      method: 'POST',
      body: emergencyData
    })
  }

  async getEmergencyEvents() {
    return this.request('/emergency-events')
  }

  async getEmergencyLandingPoints() {
    return this.request('/emergency-landing-points')
  }

  // 交通管理
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
  async planRoute(routeData) {
    return this.request('/routes/plan', {
      method: 'POST',
      body: routeData
    })
  }

  // 获取所有路线
  async getRoutes() {
    return this.request('/routes')
  }
}

export default new ApiService()
