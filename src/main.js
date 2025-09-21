import { createApp } from 'vue'
import App from './App.vue'
import router from './router'
import store from './store'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import * as ElementPlusIconsVue from '@element-plus/icons-vue'

// 导入全局样式
import './assets/styles/global.css'

// 导入认证API服务
import authApi from './services/authApi'

const app = createApp(App)

// 注册所有Element Plus图标
for (const [key, component] of Object.entries(ElementPlusIconsVue)) {
  app.component(key, component)
}

// 使用Element Plus
app.use(ElementPlus)

// 使用路由和存储
app.use(router)
app.use(store)

// 初始化主题（从localStorage中恢复）
if (store.state.theme === 'dark') {
  document.documentElement.setAttribute('data-theme', 'dark')
}

// 初始化认证状态
const initAuth = async () => {
  // 如果有存储的token，尝试获取用户信息
  if (authApi.getAccessToken()) {
    try {
      await store.dispatch('getCurrentUser')
    } catch (error) {
      console.warn('初始化时获取用户信息失败，可能token已过期:', error)
      // 清除无效的token
      authApi.clearTokens()
      store.commit('SET_USER', null)
    }
  }
}

// 监听认证状态变化
window.addEventListener('auth:logout', () => {
  // 当token失效或登出时，重定向到登录页
  if (router.currentRoute.value.path !== '/login' && router.currentRoute.value.path !== '/register') {
    router.push('/login?redirect=' + encodeURIComponent(router.currentRoute.value.fullPath))
  }
})

// 挂载应用并初始化认证状态
app.mount('#app')
initAuth()

// 全局错误处理
app.config.errorHandler = (err, instance, info) => {
  console.error('Vue错误:', err, info)
  // 在生产环境中，这里可以发送错误报告到服务端
}
