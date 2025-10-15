<template>
  <div class="app-layout">
    <!-- 顶部导航栏 -->
    <header class="app-header">
      <div class="header-content">
        <div class="logo-section">
          <div class="logo-icon">✈️</div>
          <h1>智能低空交通系统</h1>
        </div>

        <div class="header-right">
          <div class="status-indicator">
            <div class="status-dot online" />
            <span>系统在线</span>
          </div>

          <ThemeToggle />

          <!-- 用户下拉菜单 -->
          <div
            class="user-menu-wrapper"
            @click="toggleUserMenu"
          >
            <div class="user-avatar">
              <span class="user-icon">👤</span>
            </div>

            <!-- 下拉菜单 -->
            <Transition name="dropdown">
              <div
                v-if="showUserMenu"
                class="dropdown-menu"
                @click.stop
              >
                <button
                  class="dropdown-item"
                  @click="handleCommand('profile')"
                >
                  <span class="item-icon">👤</span>
                  <span>个人资料</span>
                </button>
                <button
                  class="dropdown-item"
                  @click="handleCommand('settings')"
                >
                  <span class="item-icon">⚙️</span>
                  <span>系统设置</span>
                </button>
                <div class="dropdown-divider" />
                <button
                  class="dropdown-item danger"
                  @click="handleCommand('logout')"
                >
                  <span class="item-icon">🚪</span>
                  <span>退出登录</span>
                </button>
              </div>
            </Transition>
          </div>
        </div>
      </div>
    </header>

    <!-- 主内容区域 -->
    <div class="main-content">
      <!-- 侧边导航栏 -->
      <aside class="sidebar">
        <nav class="nav-menu">
          <ul>
            <li
              :class="{ active: currentRoute === 'dashboard' }"
              @click="navigateTo('dashboard')"
            >
              <span class="nav-icon">🏠</span>
              <span>仪表盘</span>
            </li>
            <li
              :class="{ active: currentRoute === 'tasks' }"
              @click="navigateTo('tasks')"
            >
              <span class="nav-icon">📋</span>
              <span>任务中心</span>
            </li>
            <li
              :class="{ active: currentRoute === 'map' }"
              @click="navigateTo('map')"
            >
              <span class="nav-icon">🗺️</span>
              <span>地图监控</span>
            </li>
            <li
              :class="{ active: currentRoute === 'airspace' }"
              @click="navigateTo('airspace')"
            >
              <span class="nav-icon">📍</span>
              <span>空域管理</span>
            </li>
            <li
              :class="{ active: currentRoute === 'weather' }"
              @click="navigateTo('weather')"
            >
              <span class="nav-icon">🌤️</span>
              <span>气象监测</span>
            </li>
            <li
              :class="{ active: currentRoute === 'emergency' }"
              @click="navigateTo('emergency')"
            >
              <span class="nav-icon">🚨</span>
              <span>应急响应</span>
            </li>
            <li
              :class="{ active: currentRoute === 'analytics' }"
              @click="navigateTo('analytics')"
            >
              <span class="nav-icon">📊</span>
              <span>数据分析</span>
            </li>
            <li
              :class="{ active: currentRoute === 'system-monitor' }"
              @click="navigateTo('system-monitor')"
            >
              <span class="nav-icon">🖥️</span>
              <span>系统监控</span>
            </li>
            <li
              :class="{ active: currentRoute === 'api-test' }"
              @click="navigateTo('api-test')"
            >
              <span class="nav-icon">🔌</span>
              <span>API测试</span>
            </li>
          </ul>
        </nav>

        <!-- 底部信息 -->
        <div class="sidebar-footer">
          <div class="version-info">
            v1.0.0
          </div>
        </div>
      </aside>

      <!-- 主内容区 -->
      <main class="main-area">
        <router-view />
      </main>
    </div>

    <!-- 底部状态栏 -->
    <footer class="app-footer">
      <div class="footer-content">
        <span>© 2025 智能低空交通系统 | 专业级城市低空交通管理解决方案</span>
        <div class="footer-links">
          <a
            href="#"
            @click.prevent
          >隐私政策</a>
          <span>·</span>
          <a
            href="#"
            @click.prevent
          >使用条款</a>
          <span>·</span>
          <a
            href="#"
            @click.prevent
          >技术支持</a>
        </div>
      </div>
    </footer>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div v-if="showToast" :class="['toast-notification', toastType]">
        <div class="toast-icon">{{ toastIcon }}</div>
        <div class="toast-message">{{ toastMessage }}</div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import { useRouter } from 'vue-router'
import { useStore } from 'vuex'
import ThemeToggle from '@/components/ThemeToggle.vue'

const router = useRouter()
const store = useStore()
const currentRoute = ref(router.currentRoute.value.name)
const showUserMenu = ref(false)

// Toast 通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

// Toast 通知方法
const showToastNotification = (message, type = 'success') => {
  toastMessage.value = message
  toastType.value = type
  showToast.value = true

  if (toastTimer) {
    clearTimeout(toastTimer)
  }

  toastTimer = setTimeout(() => {
    showToast.value = false
  }, 3000)
}

// 监听路由变化
router.afterEach((to) => {
  currentRoute.value = to.name
})

const navigateTo = (route) => {
  router.push(`/${route}`)
}

const toggleUserMenu = () => {
  showUserMenu.value = !showUserMenu.value
}

// 点击外部关闭菜单
const handleClickOutside = (e) => {
  const userMenuWrapper = document.querySelector('.user-menu-wrapper')
  if (userMenuWrapper && !userMenuWrapper.contains(e.target)) {
    showUserMenu.value = false
  }
}

onMounted(() => {
  document.addEventListener('click', handleClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', handleClickOutside)
})

const handleCommand = async (command) => {
  showUserMenu.value = false

  switch(command) {
    case 'logout':
      try {
        await store.dispatch('logout')
        showToastNotification('退出登录成功', 'success')
        router.push('/login')
      } catch (error) {
        console.error('退出登录失败:', error)
        showToastNotification('退出登录失败，请重试', 'error')

        try {
          store.commit('SET_USER', null)
          if (window.apiService && window.apiService.clearToken) {
            window.apiService.clearToken()
          }
          localStorage.removeItem('token')
          localStorage.removeItem('refreshToken')
          sessionStorage.removeItem('token')
          sessionStorage.removeItem('refreshToken')
          router.push('/login')
        } catch (fallbackError) {
          console.error('本地状态清除失败:', fallbackError)
        }
      }
      break
    case 'profile':
      router.push('/profile')
      break
    case 'settings':
      router.push('/settings')
      break
  }
}
</script>

<style scoped>
/* Apple 风格极简后台布局 */
.app-layout {
  display: flex;
  flex-direction: column;
  min-height: 100vh;
  background-color: #F5F5F7;
  font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Display', 'Segoe UI', Roboto, sans-serif;
}

/* 顶部导航栏 */
.app-header {
  background: rgba(255, 255, 255, 0.8);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  border-bottom: 1px solid rgba(0, 0, 0, 0.06);
  padding: 0 var(--space-8, 32px);
  position: sticky;
  top: 0;
  z-index: 100;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  height: 60px;
  max-width: 100%;
}

.logo-section {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
}

.logo-icon {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 20px;
  line-height: 1;
}

.logo-section h1 {
  margin: 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  letter-spacing: -0.02em;
  color: var(--color-text, #1D1D1F);
}

.header-right {
  display: flex;
  align-items: center;
  gap: var(--space-6, 24px);
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
  color: #6E6E73;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: var(--radius-full, 9999px);
  background: #34C759;
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.6; }
}

/* 用户菜单 */
.user-menu-wrapper {
  position: relative;
  cursor: pointer;
}

.user-avatar {
  width: 36px;
  height: 36px;
  border-radius: var(--radius-full, 9999px);
  background: #F5F5F7;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  border: 1px solid rgba(0, 0, 0, 0.06);
}

.user-avatar:hover {
  background: #E8E8ED;
  transform: scale(1.05);
}

.user-icon {
  font-size: 18px;
  line-height: 1;
}

/* 下拉菜单 */
.dropdown-menu {
  position: absolute;
  top: calc(100% + 8px);
  right: 0;
  min-width: 200px;
  background: #FFFFFF;
  border-radius: var(--radius-xl, 16px);
  box-shadow:
    0 20px 60px rgba(0, 0, 0, 0.15),
    0 0 0 0.5px rgba(0, 0, 0, 0.05);
  padding: var(--space-2, 8px);
  z-index: 1000;
}

.dropdown-item {
  width: 100%;
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-3, 12px) var(--space-4, 16px);
  background: transparent;
  border: none;
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 15px);
  font-weight: 500;
  color: var(--color-text, #1D1D1F);
  cursor: pointer;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  text-align: left;
}

.dropdown-item:hover {
  background: #F5F5F7;
}

.dropdown-item.danger {
  color: #FF3B30;
}

.dropdown-item.danger:hover {
  background: rgba(255, 59, 48, 0.1);
}

.item-icon {
  font-size: 18px;
  line-height: 1;
}

.dropdown-divider {
  height: 1px;
  background: rgba(0, 0, 0, 0.06);
  margin: var(--space-2, 8px) 0;
}

/* 下拉菜单动画 */
.dropdown-enter-active {
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.dropdown-leave-active {
  transition: all 0.15s cubic-bezier(0.4, 0, 1, 1);
}

.dropdown-enter-from {
  opacity: 0;
  transform: translateY(-10px) scale(0.95);
}

.dropdown-leave-to {
  opacity: 0;
  transform: translateY(-5px) scale(0.98);
}

/* 主内容区域 */
.main-content {
  display: flex;
  flex: 1;
  overflow: hidden;
}

/* 侧边导航栏 */
.sidebar {
  width: 240px;
  background: #FFFFFF;
  border-right: 1px solid rgba(0, 0, 0, 0.06);
  display: flex;
  flex-direction: column;
  position: relative;
}

.nav-menu {
  flex: 1;
  overflow-y: auto;
}

.nav-menu ul {
  list-style: none;
  margin: 0;
  padding: var(--space-4, 16px) var(--space-3, 12px);
}

.nav-menu li {
  padding: var(--space-3, 12px) var(--space-4, 16px);
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  cursor: pointer;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  color: #6E6E73;
  font-weight: 500;
  margin-bottom: var(--space-1, 4px);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 15px);
}

.nav-menu li:hover {
  background: #F5F5F7;
  color: var(--color-text, #1D1D1F);
}

.nav-menu li.active {
  background: #E8E8ED;
  color: #0071E3;
  font-weight: 600;
}

.nav-icon {
  width: 20px;
  height: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
  line-height: 1;
}

.nav-menu span {
  letter-spacing: -0.01em;
}

.sidebar-footer {
  padding: var(--space-4, 16px) var(--space-6, 24px);
  border-top: 1px solid rgba(0, 0, 0, 0.06);
  text-align: center;
  font-size: 12px;
  color: #86868B;
  background: #FAFAFA;
}

/* 主内容区 */
.main-area {
  flex: 1;
  overflow-y: auto;
  background: #F5F5F7;
  padding: var(--space-8, 32px);
}

/* 自定义滚动条 */
.main-area::-webkit-scrollbar,
.nav-menu::-webkit-scrollbar {
  width: 6px;
}

.main-area::-webkit-scrollbar-track,
.nav-menu::-webkit-scrollbar-track {
  background: transparent;
}

.main-area::-webkit-scrollbar-thumb,
.nav-menu::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.15);
  border-radius: var(--radius-full, 9999px);
}

.main-area::-webkit-scrollbar-thumb:hover,
.nav-menu::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.25);
}

/* 底部状态栏 */
.app-footer {
  background: #FFFFFF;
  padding: var(--space-4, 16px) var(--space-8, 32px);
  border-top: 1px solid rgba(0, 0, 0, 0.06);
  font-size: var(--font-size-sm, 14px);
  color: #6E6E73;
}

.footer-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.footer-links {
  display: flex;
  gap: var(--space-4, 16px);
  align-items: center;
}

.footer-links a {
  color: #6E6E73;
  text-decoration: none;
  transition: color 0.2s ease;
  font-weight: 500;
}

.footer-links a:hover {
  color: #0071E3;
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .sidebar {
    width: 220px;
  }
}

@media (max-width: 768px) {
  .app-header {
    padding: 0 var(--space-4, 16px);
  }

  .logo-section h1 {
    font-size: var(--font-size-base, 15px);
  }

  .sidebar {
    position: fixed;
    top: 60px;
    left: -240px;
    width: 240px;
    height: calc(100vh - 60px);
    z-index: 1000;
    transition: left 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    box-shadow: 2px 0 20px rgba(0, 0, 0, 0.1);
  }

  .sidebar.show {
    left: 0;
  }

  .main-content {
    flex-direction: column;
  }

  .main-area {
    padding: var(--space-4, 16px);
  }

  .footer-content {
    flex-direction: column;
    gap: var(--space-2, 8px);
    text-align: center;
  }

  .status-indicator span {
    display: none;
  }
}

/* 暗色模式 */
@media (prefers-color-scheme: dark) {
  .app-layout {
    background-color: #000000;
  }

  .app-header {
    background: rgba(28, 28, 30, 0.8);
    border-bottom-color: rgba(255, 255, 255, 0.1);
  }

  .logo-section h1 {
    color: #F5F5F7;
  }

  .status-indicator {
    color: #98989D;
  }

  .user-avatar {
    background: #2C2C2E;
    border-color: rgba(255, 255, 255, 0.1);
  }

  .user-avatar:hover {
    background: #3A3A3C;
  }

  .dropdown-menu {
    background: #1C1C1E;
    box-shadow:
      0 20px 60px rgba(0, 0, 0, 0.5),
      0 0 0 0.5px rgba(255, 255, 255, 0.1);
  }

  .dropdown-item {
    color: #F5F5F7;
  }

  .dropdown-item:hover {
    background: #2C2C2E;
  }

  .dropdown-item.danger {
    color: #FF453A;
  }

  .dropdown-item.danger:hover {
    background: rgba(255, 69, 58, 0.15);
  }

  .dropdown-divider {
    background: rgba(255, 255, 255, 0.1);
  }

  .sidebar {
    background: #1C1C1E;
    border-right-color: rgba(255, 255, 255, 0.1);
  }

  .nav-menu li {
    color: #98989D;
  }

  .nav-menu li:hover {
    background: #2C2C2E;
    color: #F5F5F7;
  }

  .nav-menu li.active {
    background: #2C2C2E;
    color: #0A84FF;
  }

  .sidebar-footer {
    background: #2C2C2E;
    border-top-color: rgba(255, 255, 255, 0.1);
    color: #8E8E93;
  }

  .main-area {
    background: #000000;
  }

  .app-footer {
    background: #1C1C1E;
    border-top-color: rgba(255, 255, 255, 0.1);
    color: #98989D;
  }

  .footer-links a {
    color: #98989D;
  }

  .footer-links a:hover {
    color: #0A84FF;
  }

  .main-area::-webkit-scrollbar-thumb,
  .nav-menu::-webkit-scrollbar-thumb {
    background: rgba(255, 255, 255, 0.2);
  }

  .main-area::-webkit-scrollbar-thumb:hover,
  .nav-menu::-webkit-scrollbar-thumb:hover {
    background: rgba(255, 255, 255, 0.3);
  }
}

/* Toast 通知样式 */
.toast-notification {
  position: fixed;
  top: var(--space-8, 32px);
  right: var(--space-6, 24px);
  z-index: 10000;
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px) var(--space-5, 20px);
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-lg, 12px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  min-width: 280px;
  max-width: 400px;
}

.toast-notification.success {
  border-left: 4px solid #34C759;
}

.toast-notification.error {
  border-left: 4px solid #FF3B30;
}

.toast-notification.warning {
  border-left: 4px solid #FF9500;
}

.toast-notification.info {
  border-left: 4px solid #007AFF;
}

.toast-icon {
  font-size: 24px;
  line-height: 1;
  flex-shrink: 0;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.5;
}

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0, 1, 1);
}

@keyframes slideInRight {
  from {
    transform: translateX(100%);
    opacity: 0;
  }
  to {
    transform: translateX(0);
    opacity: 1;
  }
}

@keyframes slideOutRight {
  from {
    transform: translateX(0);
    opacity: 1;
  }
  to {
    transform: translateX(100%);
    opacity: 0;
  }
}

/* Toast 深色模式 */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: #1C1C1E;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
  }

  .toast-message {
    color: #F5F5F7;
  }
}
</style>
