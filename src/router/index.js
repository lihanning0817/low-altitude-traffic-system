import { createRouter, createWebHistory } from 'vue-router'
import HomePage from '@/views/HomePage.vue'
import LoginPage from '@/views/LoginPage.vue'
import HomeDashboard from '@/views/HomeDashboard.vue'
import LoginDashboard from '@/views/LoginDashboard.vue'
import RegisterDashboard from '@/views/RegisterDashboard.vue'
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
    component: RegisterDashboard
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
    component: () => import('@/components/FlightTaskList.vue')
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
    path: '/:pathMatch(.*)*',
    redirect: '/'
  }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

// 路由守卫
router.beforeEach((to, from, next) => {
  const isAuthenticated = localStorage.getItem('user') !== null
  const isAdmin = localStorage.getItem('user') && JSON.parse(localStorage.getItem('user')).role === 'admin'

  // 无需认证的页面
  const publicPages = ['home', 'login', 'register']

  if (to.meta.requiresAdmin && !isAdmin) {
    next('/login')
  } else if (!isAuthenticated && !publicPages.includes(to.name)) {
    next('/login')
  } else {
    next()
  }
})

export default router
