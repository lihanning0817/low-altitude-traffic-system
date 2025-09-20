import { createStore } from 'vuex'
import { ElMessage } from 'element-plus'

import aMapRoutePlanner from '@/services/aMapRoutePlanner'
import apiService from '@/services/api'

// 本地存储工具
const storage = {
  get(key) {
    try {
      const value = localStorage.getItem(key)
      return value ? JSON.parse(value) : null
    } catch (error) {
      console.error('读取本地存储失败:', error)
      return null
    }
  },
  set(key, value) {
    try {
      localStorage.setItem(key, JSON.stringify(value))
    } catch (error) {
      console.error('写入本地存储失败:', error)
    }
  },
  remove(key) {
    try {
      localStorage.removeItem(key)
    } catch (error) {
      console.error('删除本地存储失败:', error)
    }
  }
}

export default createStore({
  state: {
    // 用户相关
    user: storage.get('user') || null,
    isAuthenticated: !!storage.get('user'),

    // 任务相关
    flightTasks: [],
    currentTask: null,

    // 设备相关
    devices: [],
    activeDevices: 0,

    // 路径规划相关
    routes: [],
    currentRoute: null,

    // 交通管理相关
    activeFlights: [],
    flightConflicts: [],

    // 空域管理相关
    airspaces: [],
    flightPermits: [],

    // 气象数据相关
    weatherData: null,
    routeWeather: [],

    // 应急响应相关
    emergencyEvents: [],
    emergencyLandingPoints: [],

    // 地图相关
    mapSettings: {
      center: [116.397428, 39.90923],
      zoom: 11,
      mapType: 'roadmap',
      showTraffic: false,
      autoRefresh: false,
      refreshInterval: 10
    },

    // UI状态
    sidebarCollapsed: false,
    theme: storage.get('theme') || 'light',
    notifications: [],

    // 系统状态
    loading: false,
    systemStatus: 'online'
  },

  getters: {
    // 用户相关
    isLoggedIn: state => state.isAuthenticated && state.user !== null,
    userName: state => state.user?.username || '未登录',
    userRole: state => state.user?.role || 'guest',

    // 任务统计
    totalTasks: state => state.flightTasks.length,
    runningTasks: state => state.flightTasks.filter(task => task.status === '进行中').length,
    completedTasks: state => state.flightTasks.filter(task => task.status === '已完成').length,
    pendingTasks: state => state.flightTasks.filter(task => task.status === '待执行').length,

    // 设备统计
    onlineDevices: state => state.devices.filter(device => device.status !== '离线').length,
    devicesByType: state => type => state.devices.filter(device => device.type === type),

    // 路径规划相关
    plannedRoutes: state => state.routes,
    currentRoute: state => state.currentRoute,

    // 交通管理相关
    activeFlightsCount: state => state.activeFlights.length,
    flightConflictsCount: state => state.flightConflicts.length,

    // 空域管理相关
    availableAirspaces: state => state.airspaces.filter(space => space.status === 'active'),
    restrictedAirspaces: state => state.airspaces.filter(space => space.status === 'restricted'),

    // 应急响应相关
    activeEmergencies: state => state.emergencyEvents.filter(event => event.status === 'reported' || event.status === 'responding'),
    emergencyEventsCount: state => state.emergencyEvents.length,

    // 通知相关
    unreadNotifications: state => state.notifications.filter(n => !n.read).length
  },

  mutations: {
    // 用户相关
    SET_USER(state, user) {
      state.user = user
      state.isAuthenticated = !!user
      if (user) {
        storage.set('user', user)
      } else {
        storage.remove('user')
      }
    },

    // 路径规划相关
    SET_ROUTES(state, routes) {
      state.routes = routes
    },

    ADD_ROUTE(state, route) {
      state.routes.push(route)
    },

    SET_CURRENT_ROUTE(state, route) {
      state.currentRoute = route
    },

    // 交通管理相关
    SET_ACTIVE_FLIGHTS(state, flights) {
      state.activeFlights = flights
    },

    ADD_ACTIVE_FLIGHT(state, flight) {
      state.activeFlights.push(flight)
    },

    REMOVE_ACTIVE_FLIGHT(state, flightId) {
      const index = state.activeFlights.findIndex(f => f.id === flightId)
      if (index !== -1) {
        state.activeFlights.splice(index, 1)
      }
    },

    SET_FLIGHT_CONFLICTS(state, conflicts) {
      state.flightConflicts = conflicts
    },

    // 空域管理相关
    SET_AIRSPACES(state, airspaces) {
      state.airspaces = airspaces
    },

    UPDATE_AIRSPACE(state, { id, updates }) {
      const index = state.airspaces.findIndex(a => a.id === id)
      if (index !== -1) {
        state.airspaces.splice(index, 1, { ...state.airspaces[index], ...updates })
      }
    },

    SET_FLIGHT_PERMITS(state, permits) {
      state.flightPermits = permits
    },

    ADD_FLIGHT_PERMIT(state, permit) {
      state.flightPermits.push(permit)
    },

    // 气象数据相关
    SET_WEATHER_DATA(state, weatherData) {
      state.weatherData = weatherData
    },

    SET_ROUTE_WEATHER(state, routeWeather) {
      state.routeWeather = routeWeather
    },

    // 应急响应相关
    SET_EMERGENCY_EVENTS(state, events) {
      state.emergencyEvents = events
    },

    ADD_EMERGENCY_EVENT(state, event) {
      state.emergencyEvents.push(event)
    },

    UPDATE_EMERGENCY_EVENT(state, { id, updates }) {
      const index = state.emergencyEvents.findIndex(e => e.id === id)
      if (index !== -1) {
        state.emergencyEvents.splice(index, 1, { ...state.emergencyEvents[index], ...updates })
      }
    },

    SET_EMERGENCY_LANDING_POINTS(state, points) {
      state.emergencyLandingPoints = points
    },

    // 任务相关
    SET_FLIGHT_TASKS(state, tasks) {
      state.flightTasks = tasks
    },

    ADD_FLIGHT_TASK(state, task) {
      state.flightTasks.push(task)
    },

    UPDATE_FLIGHT_TASK(state, { id, updates }) {
      const index = state.flightTasks.findIndex(task => task.id === id)
      if (index !== -1) {
        state.flightTasks.splice(index, 1, { ...state.flightTasks[index], ...updates })
      }
    },

    REMOVE_FLIGHT_TASK(state, taskId) {
      const index = state.flightTasks.findIndex(task => task.id === taskId)
      if (index !== -1) {
        state.flightTasks.splice(index, 1)
      }
    },

    SET_CURRENT_TASK(state, task) {
      state.currentTask = task
    },

    // 设备相关
    SET_DEVICES(state, devices) {
      state.devices = devices
      state.activeDevices = devices.filter(d => d.status !== '离线').length
    },

    UPDATE_DEVICE_STATUS(state, { deviceId, status, signal }) {
      const device = state.devices.find(d => d.id === deviceId)
      if (device) {
        device.status = status
        if (signal !== undefined) device.signal = signal
      }
      state.activeDevices = state.devices.filter(d => d.status !== '离线').length
    },

    // 更新设备位置（与高德地图集成）
    UPDATE_DEVICE_POSITION(state, { deviceId, position }) {
      const device = state.devices.find(d => d.id === deviceId)
      if (device) {
        device.position = position
      }
    },

    // 地图相关
    UPDATE_MAP_SETTINGS(state, settings) {
      state.mapSettings = { ...state.mapSettings, ...settings }
    },

    // UI状态
    TOGGLE_SIDEBAR(state) {
      state.sidebarCollapsed = !state.sidebarCollapsed
    },

    SET_SIDEBAR_COLLAPSED(state, collapsed) {
      state.sidebarCollapsed = collapsed
    },

    SET_THEME(state, theme) {
      state.theme = theme
      storage.set('theme', theme)

      // 应用主题到 HTML 元素
      const htmlElement = document.documentElement
      if (theme === 'dark') {
        htmlElement.classList.add('dark')
        htmlElement.classList.remove('light')
      } else {
        htmlElement.classList.add('light')
        htmlElement.classList.remove('dark')
      }
    },

    TOGGLE_THEME(state) {
      const newTheme = state.theme === 'light' ? 'dark' : 'light'
      state.theme = newTheme
      storage.set('theme', newTheme)

      // 应用主题到 HTML 元素
      const htmlElement = document.documentElement
      if (newTheme === 'dark') {
        htmlElement.classList.add('dark')
        htmlElement.classList.remove('light')
      } else {
        htmlElement.classList.add('light')
        htmlElement.classList.remove('dark')
      }
    },

    // 通知相关
    ADD_NOTIFICATION(state, notification) {
      const newNotification = {
        id: Date.now(),
        timestamp: new Date().toISOString(),
        read: false,
        ...notification
      }
      state.notifications.unshift(newNotification)

      // 保持通知数量在合理范围内
      if (state.notifications.length > 50) {
        state.notifications = state.notifications.slice(0, 50)
      }
    },

    MARK_NOTIFICATION_READ(state, notificationId) {
      const notification = state.notifications.find(n => n.id === notificationId)
      if (notification) {
        notification.read = true
      }
    },

    CLEAR_NOTIFICATIONS(state) {
      state.notifications = []
    },

    // 系统状态
    SET_LOADING(state, loading) {
      state.loading = loading
    },

    SET_SYSTEM_STATUS(state, status) {
      state.systemStatus = status
    }
  },

  actions: {
    // 用户认证
    async login({ commit, dispatch }, userData) {
      try {
        commit('SET_LOADING', true)

        // 调用API登录
        const response = await apiService.login(userData)
        const user = response.data.user
        const token = response.data.token

        // 设置API token
        apiService.setToken(token)

        commit('SET_USER', user)
        dispatch('addNotification', {
          type: 'success',
          title: '登录成功',
          message: `欢迎回来，${user.username}！`
        })

        // 登录成功后加载用户相关数据
        await dispatch('fetchUserData')

        return user
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '登录失败',
          message: error.message || '用户名或密码错误'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    async register({ commit, dispatch }, userData) {
      try {
        commit('SET_LOADING', true)

        // 调用API注册
        const response = await apiService.register(userData)
        const user = response.data.user

        dispatch('addNotification', {
          type: 'success',
          title: '注册成功',
          message: '账户创建成功，请登录'
        })

        return user
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '注册失败',
          message: error.message || '注册过程中出现错误'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    logout({ commit, dispatch }) {
      commit('SET_USER', null)
      commit('SET_FLIGHT_TASKS', [])
      commit('SET_DEVICES', [])
      commit('CLEAR_NOTIFICATIONS')

      // 清除API token
      apiService.clearToken()

      dispatch('addNotification', {
        type: 'info',
        title: '退出登录',
        message: '您已安全退出系统'
      })
    },

    // 数据获取
    async fetchUserData({ dispatch }) {
      try {
        await Promise.all([
          dispatch('fetchFlightTasks'),
          dispatch('fetchDevices')
        ])
      } catch (error) {
        console.error('获取用户数据失败:', error)
      }
    },

    async fetchFlightTasks({ commit }) {
      try {
        // 调用API获取飞行任务
        const response = await apiService.getFlightTasks()
        const tasks = response.data.tasks || []

        commit('SET_FLIGHT_TASKS', tasks)
      } catch (error) {
        console.error('获取任务列表失败:', error)
      }
    },

    async fetchDevices({ commit }) {
      try {
        // 模拟API调用
        const devices = [
          {
            id: 'UAV001',
            name: '巡逻无人机-01',
            type: 'drone',
            status: '巡航中',
            signal: 85,
            position: [116.397428, 39.90923],
            battery: 78,
            lastUpdate: new Date().toISOString()
          },
          {
            id: 'CAM002',
            name: '监控摄像头-02',
            type: 'camera',
            status: '正常',
            signal: 92,
            position: [116.407428, 39.91923],
            lastUpdate: new Date().toISOString()
          }
        ]

        commit('SET_DEVICES', devices)
      } catch (error) {
        console.error('获取设备列表失败:', error)
      }
    },

    // 任务管理
    async createTask({ commit, dispatch }, taskData) {
      try {
        // 调用API创建任务
        const response = await apiService.createFlightTask(taskData)
        const newTask = response.data.task

        commit('ADD_FLIGHT_TASK', newTask)

        dispatch('addNotification', {
          type: 'success',
          title: '任务创建成功',
          message: `任务 ${newTask.id} 已创建`
        })

        return newTask
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '任务创建失败',
          message: error.message
        })
        throw error
      }
    },

    async updateTask({ commit, dispatch }, { id, updates }) {
      try {
        // 调用API更新任务
        await apiService.updateFlightTask(id, updates)

        commit('UPDATE_FLIGHT_TASK', { id, updates })

        dispatch('addNotification', {
          type: 'success',
          title: '任务更新成功',
          message: `任务 ${id} 已更新`
        })
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '任务更新失败',
          message: error.message
        })
        throw error
      }
    },

    async deleteTask({ commit, dispatch }, taskId) {
      try {
        // 调用API删除任务
        await apiService.deleteFlightTask(taskId)

        commit('REMOVE_FLIGHT_TASK', taskId)

        dispatch('addNotification', {
          type: 'warning',
          title: '任务已删除',
          message: `任务 ${taskId} 已删除`
        })
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '任务删除失败',
          message: error.message
        })
        throw error
      }
    },

    // 通知管理
    addNotification({ commit }, notification) {
      commit('ADD_NOTIFICATION', notification)

      // 显示Element Plus消息
      if (notification.type && notification.message) {
        ElMessage({
          type: notification.type,
          message: notification.message,
          duration: 3000
        })
      }
    },

    markNotificationRead({ commit }, notificationId) {
      commit('MARK_NOTIFICATION_READ', notificationId)
    },

    clearNotifications({ commit }) {
      commit('CLEAR_NOTIFICATIONS')
    },

    // 设置管理
    updateMapSettings({ commit }, settings) {
      commit('UPDATE_MAP_SETTINGS', settings)
    },

    // 更新设备位置（与高德地图集成）
    updateDevicePosition({ commit }, { deviceId, position }) {
      commit('UPDATE_DEVICE_POSITION', { deviceId, position })
    },

    toggleSidebar({ commit }) {
      commit('TOGGLE_SIDEBAR')
    },

    setSidebarCollapsed({ commit }, collapsed) {
      commit('SET_SIDEBAR_COLLAPSED', collapsed)
    },

    setTheme({ commit }, theme) {
      commit('SET_THEME', theme)
    },

    toggleTheme({ commit }) {
      commit('TOGGLE_THEME')
    },

    initTheme({ commit, state }) {
      // 在应用启动时初始化主题
      const savedTheme = storage.get('theme') || 'light'

      // 检查系统偏好设置
      const prefersDark = window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches
      const theme = savedTheme === 'auto' ? (prefersDark ? 'dark' : 'light') : savedTheme

      commit('SET_THEME', theme)
    },

    // 路径规划相关
    async planRoute({ commit, dispatch }, { start, end, useAMap = true }) {
      try {
        let route;
        if (useAMap && window.AMap) {
          // 使用高德地图API进行路径规划
          route = await aMapRoutePlanner.planRoute(
            { lng: start.lng, lat: start.lat },
            { lng: end.lng, lat: end.lat }
          );
        } else {
          // 使用后端API进行路径规划
          const routeData = {
            start_lat: start.lat,
            start_lng: start.lng,
            end_lat: end.lat,
            end_lng: end.lng
          };

          const response = await apiService.planRoute(routeData);
          route = response.data.route;
        }

        if (route) {
          commit('ADD_ROUTE', route);
          commit('SET_CURRENT_ROUTE', route);
          return route;
        } else {
          throw new Error('无法找到合适的路径');
        }
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '路径规划失败',
          message: error.message
        });
        throw error;
      }
    },

    // 交通管理相关
    async registerFlight({ commit, dispatch }, flightData) {
      try {
        const response = await apiService.registerFlight(flightData)
        const result = response.data

        if (result.success) {
          commit('ADD_ACTIVE_FLIGHT', flightData)
          dispatch('addNotification', {
            type: 'success',
            title: '飞行器注册成功',
            message: `飞行器 ${flightData.id} 已注册`
          })
          return result
        } else {
          throw new Error(result.message)
        }
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '飞行器注册失败',
          message: error.message
        })
        throw error
      }
    },

    async detectFlightConflicts({ commit }) {
      try {
        const response = await apiService.getFlightConflicts()
        const conflicts = response.data.conflicts || []

        commit('SET_FLIGHT_CONFLICTS', conflicts)
        return conflicts
      } catch (error) {
        console.error('冲突检测失败:', error)
        return []
      }
    },

    // 空域管理相关
    async initializeAirspaces({ commit, dispatch }) {
      try {
        const response = await apiService.getAirspaces()
        const airspaces = response.data.airspaces || []

        commit('SET_AIRSPACES', airspaces)
        dispatch('addNotification', {
          type: 'success',
          title: '空域初始化成功',
          message: `已加载 ${airspaces.length} 个空域`
        })
        return airspaces
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '空域初始化失败',
          message: error.message
        })
        throw error
      }
    },

    async applyFlightPermit({ commit }, permitData) {
      try {
        const response = await apiService.applyFlightPermit(permitData)
        const result = response.data

        if (result.success) {
          commit('ADD_FLIGHT_PERMIT', {
            id: result.permitId,
            ...permitData,
            status: 'pending'
          })
          return result
        } else {
          throw new Error(result.message)
        }
      } catch (error) {
        throw new Error(error.message)
      }
    },

    // 气象数据相关
    async fetchWeatherData({ commit, dispatch }, { lat, lng }) {
      try {
        const response = await apiService.getCurrentWeather({ lat, lng })
        const weatherData = response.data.weather

        commit('SET_WEATHER_DATA', weatherData)
        return weatherData
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '获取天气数据失败',
          message: error.message
        })
        throw error
      }
    },

    async fetchRouteWeather({ commit, dispatch }, route) {
      try {
        const response = await apiService.getRouteWeather(route)
        const routeWeather = response.data.weather

        commit('SET_ROUTE_WEATHER', routeWeather)
        return routeWeather
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '获取路线天气失败',
          message: error.message
        })
        throw error
      }
    },

    // 应急响应相关
    async reportEmergency({ commit, dispatch }, emergencyData) {
      try {
        const response = await apiService.reportEmergency(emergencyData)
        const result = response.data

        if (result.success) {
          commit('ADD_EMERGENCY_EVENT', result.emergencyEvent)
          dispatch('addNotification', {
            type: 'warning',
            title: '紧急事件报告',
            message: `紧急事件已报告，ID: ${result.eventId}`
          })
          return result
        } else {
          throw new Error(result.message)
        }
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '紧急事件报告失败',
          message: error.message
        })
        throw error
      }
    },

    async initializeEmergencyLandingPoints({ commit, dispatch }) {
      try {
        const response = await apiService.getEmergencyLandingPoints()
        const landingPoints = response.data.landingPoints || []

        commit('SET_EMERGENCY_LANDING_POINTS', landingPoints)
        dispatch('addNotification', {
          type: 'success',
          title: '紧急降落点初始化成功',
          message: `已配置 ${landingPoints.length} 个紧急降落点`
        })
        return landingPoints
      } catch (error) {
        dispatch('addNotification', {
          type: 'error',
          title: '紧急降落点初始化失败',
          message: error.message
        })
        throw error
      }
    }
  }
})
