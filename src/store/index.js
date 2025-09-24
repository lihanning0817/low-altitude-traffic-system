import { createStore } from 'vuex'
import { ElMessage } from 'element-plus'

import aMapRoutePlanner from '@/services/aMapRoutePlanner'
import apiService from '@/services/api'
import authApi from '@/services/authApi'
import flightTaskApi from '@/services/flightTaskApi'
import systemMonitorApi from '@/services/systemMonitorApi'

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
    isAuthenticated: !!storage.get('user') && !!localStorage.getItem('access_token'),

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
    systemStatus: 'online',

    // 系统监控相关
    systemMonitor: {
      healthStatus: null,
      systemInfo: null,
      drones: [],
      users: {
        users: [],
        pagination: {}
      },
      lastUpdated: {
        health: null,
        info: null,
        drones: null,
        users: null
      },
      cacheExpiry: {
        health: 30000,    // 30秒
        info: 300000,     // 5分钟
        drones: 60000,    // 1分钟
        users: 120000     // 2分钟
      }
    }
  },

  getters: {
    // 用户相关
    isLoggedIn: state => state.isAuthenticated && state.user !== null && !!localStorage.getItem('access_token'),
    userName: state => state.user?.username || '未登录',
    userRole: state => state.user?.role || 'guest',
    userEmail: state => state.user?.email || '',
    userId: state => state.user?.id || null,
    userFullName: state => state.user?.full_name || state.user?.fullName || '',

    // 权限相关
    isAdmin: state => state.user?.role === 'admin',
    isOperator: state => state.user?.role === 'operator',
    isUser: state => state.user?.role === 'user',
    hasAdminPrivileges: state => state.user?.role === 'admin',
    hasOperatorPrivileges: state => state.user?.role === 'admin' || state.user?.role === 'operator',

    // 用户状态检查
    isUserActive: state => state.user?.status === 'active',

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
    unreadNotifications: state => state.notifications.filter(n => !n.read).length,

    // 系统监控相关
    systemHealthStatus: state => state.systemMonitor.healthStatus,
    systemInfo: state => state.systemMonitor.systemInfo,
    monitorDrones: state => state.systemMonitor.drones,
    monitorUsers: state => state.systemMonitor.users,
    systemMonitorLastUpdated: state => state.systemMonitor.lastUpdated,

    // 判断缓存是否需要刷新
    needsRefresh: state => (type) => {
      const lastUpdated = state.systemMonitor.lastUpdated[type]
      const cacheExpiry = state.systemMonitor.cacheExpiry[type]
      if (!lastUpdated) return true
      return (Date.now() - lastUpdated) > cacheExpiry
    }
  },

  mutations: {
    // 用户相关
    SET_USER(state, user) {
      state.user = user

      // 更新认证状态
      const hasToken = !!localStorage.getItem('access_token')
      state.isAuthenticated = !!user && hasToken

      if (user) {
        // 确保用户对象包含所有必要字段
        const normalizedUser = {
          id: user.id,
          username: user.username,
          email: user.email,
          role: user.role || 'user',
          status: user.status || 'active',
          full_name: user.full_name || user.fullName || '',
          created_at: user.created_at || user.createdAt,
          updated_at: user.updated_at || user.updatedAt
        }

        state.user = normalizedUser
        storage.set('user', normalizedUser)

        console.log('[Vuex] 用户信息已更新:', normalizedUser)
      } else {
        storage.remove('user')
        storage.remove('access_token')
        storage.remove('refresh_token')

        console.log('[Vuex] 用户信息已清除')
      }
    },

    // 更新用户信息（不清除token）
    UPDATE_USER(state, updates) {
      if (state.user) {
        const updatedUser = { ...state.user, ...updates }
        state.user = updatedUser
        storage.set('user', updatedUser)
        console.log('[Vuex] 用户信息已更新:', updatedUser)
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
    },

    // 系统监控相关
    SET_SYSTEM_HEALTH_STATUS(state, healthStatus) {
      state.systemMonitor.healthStatus = healthStatus
      state.systemMonitor.lastUpdated.health = Date.now()
    },

    SET_SYSTEM_INFO(state, systemInfo) {
      state.systemMonitor.systemInfo = systemInfo
      state.systemMonitor.lastUpdated.info = Date.now()
    },

    SET_MONITOR_DRONES(state, drones) {
      state.systemMonitor.drones = drones
      state.systemMonitor.lastUpdated.drones = Date.now()
    },

    SET_MONITOR_USERS(state, users) {
      state.systemMonitor.users = users
      state.systemMonitor.lastUpdated.users = Date.now()
    },

    CLEAR_SYSTEM_MONITOR_CACHE(state) {
      state.systemMonitor.healthStatus = null
      state.systemMonitor.systemInfo = null
      state.systemMonitor.drones = []
      state.systemMonitor.users = { users: [], pagination: {} }
      state.systemMonitor.lastUpdated = {
        health: null,
        info: null,
        drones: null,
        users: null
      }
    }
  },

  actions: {
    // 用户认证
    async login({ commit, dispatch }, userData) {
      try {
        commit('SET_LOADING', true)

        // 调用认证API登录
        const response = await authApi.login(userData)

        if (response.success) {
          const user = response.data.user
          // 注意：token已在authApi.login()中设置，无需重复设置

          commit('SET_USER', user)
          dispatch('addNotification', {
            type: 'success',
            title: '登录成功',
            message: `欢迎回来，${user.username}！`
          })

          // 登录成功后加载用户相关数据
          await dispatch('fetchUserData')

          return user
        } else {
          throw new Error(response.message || '登录失败')
        }
      } catch (error) {
        const errorMessage = error.message || error.error_code || '用户名或密码错误'
        dispatch('addNotification', {
          type: 'error',
          title: '登录失败',
          message: errorMessage
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    async register({ commit, dispatch }, userData) {
      try {
        commit('SET_LOADING', true)

        // 调用认证API注册
        const response = await authApi.register(userData)

        if (response.success) {
          const user = response.data.user

          dispatch('addNotification', {
            type: 'success',
            title: '注册成功',
            message: '账户创建成功，请登录'
          })

          return user
        } else {
          throw new Error(response.message || '注册失败')
        }
      } catch (error) {
        const errorMessage = error.message || error.error_code || '注册过程中出现错误'
        dispatch('addNotification', {
          type: 'error',
          title: '注册失败',
          message: errorMessage
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    async logout({ commit, dispatch }) {
      try {
        // 调用后端登出API
        await authApi.logout()
      } catch (error) {
        console.warn('后端登出失败:', error)
        // 即使后端登出失败也继续本地清除
      }

      // 清除本地状态
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

    // 设置用户信息（用于登录后保存用户状态）
    setUser({ commit }, user) {
      commit('SET_USER', user)
    },

    // 强制同步认证状态
    async syncAuthState({ commit, dispatch, getters }) {
      try {
        const hasToken = !!localStorage.getItem('access_token')
        const storedUser = storage.get('user')

        console.log('[Store] syncAuthState Debug:', {
          hasToken,
          hasStoredUser: !!storedUser,
          currentUser: getters.userName,
          tokenValue: hasToken ? '***存在***' : '无',
          userValue: storedUser?.username || '无'
        })

        if (hasToken && storedUser) {
          // 确保 Vuex state 与 localStorage 同步
          if (!getters.isLoggedIn || getters.userId !== storedUser.id) {
            commit('SET_USER', storedUser)
          }

          // 验证token是否仍然有效
          try {
            await dispatch('getCurrentUser')
            console.log('[Store] Token验证成功，用户状态已同步')
          } catch (error) {
            console.warn('[Store] Token验证失败，清除认证状态:', error)
            // Token无效，清除状态
            commit('SET_USER', null)
            dispatch('addNotification', {
              type: 'warning',
              title: '登录已过期',
              message: '请重新登录以继续使用系统'
            })
          }
        } else if (!hasToken) {
          console.log('[Store] 无有效Token，清除用户状态')
          commit('SET_USER', null)
        }
      } catch (error) {
        console.error('[Store] syncAuthState error:', error)
        commit('SET_USER', null)
      }
    },

    // 初始化认证状态（应用启动时调用）
    initAuthState({ dispatch }) {
      console.log('[Store] 初始化认证状态')
      return dispatch('syncAuthState')
    },

    // 获取当前用户信息
    async getCurrentUser({ commit, dispatch }) {
      try {
        const response = await authApi.getCurrentUser()

        if (response.success) {
          const user = response.data
          commit('SET_USER', user)
          return user
        } else {
          throw new Error(response.message || '获取用户信息失败')
        }
      } catch (error) {
        console.error('获取用户信息失败:', error)
        // 如果token无效，清除用户状态
        if (error.status === 401) {
          commit('SET_USER', null)
          authApi.clearTokens()
        }
        throw error
      }
    },

    // 修改密码
    async changePassword({ dispatch }, passwordData) {
      try {
        const response = await authApi.changePassword(passwordData)

        if (response.success) {
          dispatch('addNotification', {
            type: 'success',
            title: '密码修改成功',
            message: '您的密码已成功修改'
          })
          return response
        } else {
          throw new Error(response.message || '密码修改失败')
        }
      } catch (error) {
        const errorMessage = error.message || error.error_code || '密码修改失败'
        dispatch('addNotification', {
          type: 'error',
          title: '密码修改失败',
          message: errorMessage
        })
        throw error
      }
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

    async fetchFlightTasks({ commit, dispatch }) {
      try {
        commit('SET_LOADING', true)

        // 调用FlightTask API获取飞行任务
        const response = await flightTaskApi.getFlightTasks()

        if (response.success) {
          const tasks = response.data.tasks || []
          // 格式化任务数据
          const formattedTasks = tasks.map(task => flightTaskApi.formatTask(task))
          commit('SET_FLIGHT_TASKS', formattedTasks)
          return formattedTasks
        } else {
          throw new Error(response.message || '获取任务列表失败')
        }
      } catch (error) {
        console.error('获取任务列表失败:', error)
        dispatch('addNotification', {
          type: 'error',
          title: '获取任务失败',
          message: error.message || '无法获取飞行任务列表'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
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
        commit('SET_LOADING', true)

        // 验证任务数据
        const validation = flightTaskApi.validateTaskData(taskData)
        if (!validation.isValid) {
          throw new Error(validation.errors.join(', '))
        }

        // 调用FlightTask API创建任务
        const response = await flightTaskApi.createFlightTask(taskData)

        if (response.success) {
          const newTask = flightTaskApi.formatTask(response.data.task)
          commit('ADD_FLIGHT_TASK', newTask)

          dispatch('addNotification', {
            type: 'success',
            title: '任务创建成功',
            message: `任务"${newTask.name}"已创建`
          })

          return newTask
        } else {
          throw new Error(response.message || '任务创建失败')
        }
      } catch (error) {
        console.error('创建任务失败:', error)
        dispatch('addNotification', {
          type: 'error',
          title: '任务创建失败',
          message: error.message || '创建任务时发生错误'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    async updateTask({ commit, dispatch }, { id, updates }) {
      try {
        commit('SET_LOADING', true)

        // 验证更新数据
        if (updates.name !== undefined || updates.description !== undefined ||
            updates.scheduled_time !== undefined || updates.status !== undefined) {
          const validation = flightTaskApi.validateTaskData(updates, true) // 传递isUpdate=true
          if (!validation.isValid) {
            throw new Error(validation.errors.join(', '))
          }
        }

        // 调用FlightTask API更新任务
        const response = await flightTaskApi.updateFlightTask(id, updates)

        if (response.success) {
          const updatedTask = flightTaskApi.formatTask(response.data.task)
          commit('UPDATE_FLIGHT_TASK', { id, updates: updatedTask })

          dispatch('addNotification', {
            type: 'success',
            title: '任务更新成功',
            message: `任务"${updatedTask.name}"已更新`
          })

          return updatedTask
        } else {
          throw new Error(response.message || '任务更新失败')
        }
      } catch (error) {
        console.error('更新任务失败:', error)
        dispatch('addNotification', {
          type: 'error',
          title: '任务更新失败',
          message: error.message || '更新任务时发生错误'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    async deleteTask({ commit, dispatch }, taskId) {
      try {
        commit('SET_LOADING', true)

        // 调用FlightTask API删除任务
        const response = await flightTaskApi.deleteFlightTask(taskId)

        if (response.success) {
          commit('REMOVE_FLIGHT_TASK', taskId)

          dispatch('addNotification', {
            type: 'warning',
            title: '任务已删除',
            message: response.message || `任务 ${taskId} 已删除`
          })

          return true
        } else {
          throw new Error(response.message || '任务删除失败')
        }
      } catch (error) {
        console.error('删除任务失败:', error)
        dispatch('addNotification', {
          type: 'error',
          title: '任务删除失败',
          message: error.message || '删除任务时发生错误'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    // 获取单个任务详情
    async fetchTask({ commit, dispatch }, taskId) {
      try {
        commit('SET_LOADING', true)

        const response = await flightTaskApi.getFlightTask(taskId)

        if (response.success) {
          const task = flightTaskApi.formatTask(response.data.task)
          commit('SET_CURRENT_TASK', task)
          return task
        } else {
          throw new Error(response.message || '获取任务详情失败')
        }
      } catch (error) {
        console.error('获取任务详情失败:', error)
        dispatch('addNotification', {
          type: 'error',
          title: '获取任务详情失败',
          message: error.message || '无法获取任务详情'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
      }
    },

    // 批量更新任务状态
    async batchUpdateTaskStatus({ commit, dispatch }, { taskIds, status }) {
      try {
        commit('SET_LOADING', true)

        const response = await flightTaskApi.batchUpdateStatus(taskIds, status)

        if (response.success) {
          // 刷新任务列表
          await dispatch('fetchFlightTasks')

          dispatch('addNotification', {
            type: 'success',
            title: '批量更新成功',
            message: response.message
          })

          return response
        } else {
          throw new Error(response.message || '批量更新失败')
        }
      } catch (error) {
        console.error('批量更新失败:', error)
        dispatch('addNotification', {
          type: 'error',
          title: '批量更新失败',
          message: error.message || '批量更新任务状态时发生错误'
        })
        throw error
      } finally {
        commit('SET_LOADING', false)
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
    },

    // ========== 系统监控相关Actions ==========

    /**
     * 获取系统健康状态（带缓存）
     */
    async fetchSystemHealthStatus({ commit, getters }, { forceRefresh = false } = {}) {
      try {
        // 检查是否需要刷新缓存
        if (!forceRefresh && !getters.needsRefresh('health')) {
          return getters.systemHealthStatus
        }

        const response = await systemMonitorApi.getHealthStatus()

        if (response.success) {
          const healthStatus = systemMonitorApi.formatHealthStatus(response)
          commit('SET_SYSTEM_HEALTH_STATUS', healthStatus)
          return healthStatus
        } else {
          throw new Error(response.message || '获取系统健康状态失败')
        }
      } catch (error) {
        console.error('获取系统健康状态失败:', error)
        throw error
      }
    },

    /**
     * 获取系统基本信息（带缓存）
     */
    async fetchSystemInfo({ commit, getters }, { forceRefresh = false } = {}) {
      try {
        // 检查是否需要刷新缓存
        if (!forceRefresh && !getters.needsRefresh('info')) {
          return getters.systemInfo
        }

        const response = await systemMonitorApi.getSystemInfo()

        if (response.success) {
          const systemInfo = systemMonitorApi.formatSystemInfo(response)
          commit('SET_SYSTEM_INFO', systemInfo)
          return systemInfo
        } else {
          throw new Error(response.message || '获取系统信息失败')
        }
      } catch (error) {
        console.error('获取系统信息失败:', error)
        throw error
      }
    },

    /**
     * 获取无人机列表（带缓存）
     */
    async fetchMonitorDrones({ commit, getters }, { forceRefresh = false } = {}) {
      try {
        // 检查是否需要刷新缓存
        if (!forceRefresh && !getters.needsRefresh('drones')) {
          return getters.monitorDrones
        }

        const response = await systemMonitorApi.getDronesList()

        if (response.success) {
          const drones = systemMonitorApi.formatDronesData(response)
          commit('SET_MONITOR_DRONES', drones)
          return drones
        } else {
          // 如果是认证相关错误，直接抛出包含错误代码的对象
          if (response.requiresAuth || response.error_code === 'NOT_AUTHENTICATED' || response.error_code === 'AUTHENTICATION_EXPIRED') {
            const authError = new Error(response.message || '获取无人机列表失败')
            authError.error_code = response.error_code
            authError.requiresAuth = response.requiresAuth
            throw authError
          }
          throw new Error(response.message || '获取无人机列表失败')
        }
      } catch (error) {
        console.error('获取无人机列表失败:', error)
        throw error
      }
    },

    /**
     * 获取用户列表（带缓存和分页）
     */
    async fetchMonitorUsers({ commit, getters }, { params = {}, forceRefresh = false } = {}) {
      try {
        // 检查是否需要刷新缓存（如果有新参数则强制刷新）
        const currentUsers = getters.monitorUsers
        const hasNewParams = JSON.stringify(params) !== JSON.stringify(currentUsers.lastParams || {})

        if (!forceRefresh && !hasNewParams && !getters.needsRefresh('users')) {
          return currentUsers
        }

        const response = await systemMonitorApi.getUsersList(params)

        if (response.success) {
          const usersData = systemMonitorApi.formatUsersData(response)
          usersData.lastParams = params  // 保存查询参数
          commit('SET_MONITOR_USERS', usersData)
          return usersData
        } else {
          // 如果是认证或权限相关错误，直接抛出包含错误代码的对象
          if (response.requiresAuth || response.requiresAdmin ||
              ['NOT_AUTHENTICATED', 'AUTHENTICATION_EXPIRED', 'INSUFFICIENT_PRIVILEGES', 'INVALID_USER_DATA', 'MISSING_USER_DATA'].includes(response.error_code)) {
            const authError = new Error(response.message || '获取用户列表失败')
            authError.error_code = response.error_code
            authError.requiresAuth = response.requiresAuth
            authError.requiresAdmin = response.requiresAdmin
            throw authError
          }
          throw new Error(response.message || '获取用户列表失败')
        }
      } catch (error) {
        console.error('获取用户列表失败:', error)
        throw error
      }
    },

    /**
     * 刷新所有系统监控数据
     */
    async refreshAllSystemMonitorData({ dispatch }) {
      try {
        await Promise.all([
          dispatch('fetchSystemHealthStatus', { forceRefresh: true }),
          dispatch('fetchSystemInfo', { forceRefresh: true }),
          dispatch('fetchMonitorDrones', { forceRefresh: true }),
          dispatch('fetchMonitorUsers', { forceRefresh: true })
        ])
      } catch (error) {
        console.error('刷新系统监控数据失败:', error)
        throw error
      }
    },

    /**
     * 清除系统监控缓存
     */
    clearSystemMonitorCache({ commit }) {
      commit('CLEAR_SYSTEM_MONITOR_CACHE')
    }
  }
})
