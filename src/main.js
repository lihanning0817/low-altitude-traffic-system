import { createApp } from 'vue'
import App from './App.vue'
import router from './router'
import store from './store'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import * as ElementPlusIconsVue from '@element-plus/icons-vue'

// 导入全局样式
import './assets/styles/global.css'

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

// 挂载应用
app.mount('#app')

// 全局错误处理
app.config.errorHandler = (err, instance, info) => {
  console.error('Vue错误:', err, info)
  // 在生产环境中，这里可以发送错误报告到服务端
}
