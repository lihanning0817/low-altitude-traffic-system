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

// 导入错误处理工具
import { initErrorHandler } from './utils/errorHandler'

// 初始化错误处理器
initErrorHandler()

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
  try {
    console.log('[Main] 初始化认证状态')

    // 使用Vuex store的action来统一处理认证状态同步
    await store.dispatch('initAuthState')

    // 初始化主题
    store.dispatch('initTheme')

    console.log('[Main] 认证状态初始化完成')
  } catch (error) {
    console.error('[Main] 认证状态初始化失败:', error)
    // 即使初始化失败也要清除状态，避免不一致
    store.commit('SET_USER', null)
  }
}

// 监听认证状态变化
window.addEventListener('auth:logout', () => {
  // 当token失效或登出时，重定向到登录页
  if (router.currentRoute.value.path !== '/login' && router.currentRoute.value.path !== '/register') {
    router.push('/login?redirect=' + encodeURIComponent(router.currentRoute.value.fullPath))
  }
})

// 初始化认证状态后再挂载应用
const init = async () => {
  await initAuth()
  app.mount('#app')
}

init()

// Vue全局错误处理
app.config.errorHandler = (err, instance, info) => {
  // 忽略ResizeObserver相关错误
  if (err.message && err.message.includes('ResizeObserver')) {
    return
  }
  console.error('Vue错误:', err, info)
  // 在生产环境中，这里可以发送错误报告到服务端
}
