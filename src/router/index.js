import { createRouter, createWebHistory } from 'vue-router'
import store from '@/store'
import HomePage from '@/views/HomePage.vue'
import LoginPage from '@/views/LoginPage.vue'
import HomeDashboard from '@/views/HomeDashboard.vue'
import LoginDashboard from '@/views/LoginDashboard.vue'
import RegisterDashboard from '@/views/RegisterDashboard.vue'
import RegisterPage from '@/components/RegisterPage.vue'
import AdminLogin from '@/views/AdminLogin.vue'
import AdminDashboard from '@/views/AdminDashboard.vue'
import TaskTracking from '@/views/TaskTracking.vue'
import ServiceForm from '@/views/ServiceForm.vue'
import OrderList from '@/views/OrderList.vue'

const routes = [
  {
    path: '/',
    name: 'home',
    component: HomePage
  },
  {
    path: '/login',
    name: 'login',
    component: LoginPage
  },
  {
    path: '/register',
    name: 'register',
    component: RegisterPage
  },
  {
    path: '/dashboard',
    name: 'dashboard',
    component: HomeDashboard
  },
  {
    path: '/admin/login',
    name: 'admin-login',
    component: AdminLogin
  },
  {
    path: '/admin/dashboard',
    name: 'admin-dashboard',
    component: AdminDashboard,
    meta: { requiresAdmin: true }
  },
  {
    path: '/task-tracking',
    name: 'task-tracking',
    component: TaskTracking
  },
  {
    path: '/service-form',
    name: 'service-form',
    component: ServiceForm
  },
  {
    path: '/order-list',
    name: 'order-list',
    component: OrderList
  },
  {
    path: '/tasks',
    name: 'tasks',
    component: () => import('@/components/FlightTaskList.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/map',
    name: 'map',
    component: () => import('@/components/MapComponent.vue')
  },
  {
    path: '/airspace',
    name: 'airspace',
    component: () => import('@/components/AirspaceManagement.vue')
  },
  {
    path: '/weather',
    name: 'weather',
    component: () => import('@/components/WeatherIntegration.vue')
  },
  {
    path: '/emergency',
    name: 'emergency',
    component: () => import('@/components/EmergencyResponse.vue')
  },
  {
    path: '/analytics',
    name: 'analytics',
    component: () => import('@/components/AnalyticsDashboard.vue')
  },
  {
    path: '/profile',
    name: 'profile',
    component: () => import('@/components/ProfileSettings.vue')
  },
  {
    path: '/settings',
    name: 'settings',
    component: () => import('@/components/SystemSettings.vue')
  },
  {
    path: '/api-test',
    name: 'api-test',
    component: () => import('@/components/ApiTest.vue')
  },
  {
    path: '/system-monitor',
    name: 'system-monitor',
    component: () => import('@/views/SystemMonitor.vue'),
    meta: { requiresAuth: true }
  },
  {
    path: '/:pathMatch(.*)*',
    redirect: '/'
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

// 路由守卫
router.beforeEach(async (to, from, next) => {
  try {
    // 轻量级认证状态检查，避免API调用
    const hasToken = !!localStorage.getItem('access_token')
    const storedUser = localStorage.getItem('user')
    let hasUser = false
    let userRole = 'guest'

    if (storedUser) {
      try {
        const user = JSON.parse(storedUser)
        hasUser = !!user
        userRole = user.role || 'guest'

        // 确保store中有用户信息
        if (!store.state.user && hasUser) {
          store.commit('SET_USER', user)
        }
      } catch (error) {
        console.warn('解析localStorage用户信息失败:', error)
        hasUser = false
      }
    }

    const isAuthenticated = hasToken && hasUser
    const isAdmin = userRole === 'admin'

    console.log('Route Guard Debug:', {
      route: to.path,
      hasToken,
      hasUser,
      userRole,
      isAuthenticated,
      requiresAuth: to.meta.requiresAuth
    })

    // 无需认证的页面
    const publicPages = ['home', 'login', 'register', 'admin-login']

    // 如果是公共页面，直接通过
    if (publicPages.includes(to.name)) {
      next()
      return
    }

    // 检查是否需要管理员权限
    if (to.meta.requiresAdmin && !isAdmin) {
      console.log('Redirecting to admin login: Admin required')
      next('/admin/login?redirect=' + encodeURIComponent(to.fullPath))
      return
    }

    // 检查是否需要认证
    if (to.meta.requiresAuth && !isAuthenticated) {
      console.log('Redirecting to login: Auth required')
      next('/login?redirect=' + encodeURIComponent(to.fullPath))
      return
    }

    // 如果已登录用户访问登录页，重定向到仪表板
    if (isAuthenticated && (to.name === 'login' || to.name === 'register')) {
      next('/dashboard')
      return
    }

    next()
  } catch (error) {
    console.error('Route guard error:', error)
    // 如果出现错误且不是公共页面，重定向到登录页
    const publicPages = ['home', 'login', 'register', 'admin-login']
    if (!publicPages.includes(to.name)) {
      next('/login')
    } else {
      next()
    }
  }
})

export default router
