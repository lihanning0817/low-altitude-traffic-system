<template>
  <div class="app-layout">
    <!-- 顶部导航栏 -->
    <header class="app-header">
      <div class="header-content">
        <div class="logo-section">
          <div class="logo-icon">
            <svg
              width="24"
              height="24"
              viewBox="0 0 24 24"
              fill="none"
              xmlns="http://www.w3.org/2000/svg"
            >
              <path
                d="M12 2L15.09 8.26L22 9.27L17 14.14L18.18 21.02L12 17.77L5.82 21.02L7 14.14L2 9.27L8.91 8.26L12 2Z"
                stroke="currentColor"
                stroke-width="2"
                stroke-linecap="round"
                stroke-linejoin="round"
              />
              <path
                d="M12 12L12 16"
                stroke="currentColor"
                stroke-width="2"
                stroke-linecap="round"
              />
              <path
                d="M12 8H12.01"
                stroke="currentColor"
                stroke-width="2"
                stroke-linecap="round"
              />
            </svg>
          </div>
          <h1>智能低空交通系统</h1>
        </div>
        
        <div class="header-right">
          <div class="status-indicator">
            <div class="status-dot online" />
            <span>系统在线</span>
          </div>
          
          <ThemeToggle />
          
          <el-dropdown
            trigger="click"
            @command="handleCommand"
          >
            <div class="user-avatar">
              <el-icon><User /></el-icon>
            </div>
            <template #dropdown>
              <el-dropdown-menu>
                <el-dropdown-item command="profile">
                  个人资料
                </el-dropdown-item>
                <el-dropdown-item command="settings">
                  系统设置
                </el-dropdown-item>
                <el-dropdown-item
                  command="logout"
                  divided
                >
                  退出登录
                </el-dropdown-item>
              </el-dropdown-menu>
            </template>
          </el-dropdown>
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
              <el-icon><House /></el-icon>
              <span>仪表盘</span>
            </li>
            <li
              :class="{ active: currentRoute === 'tasks' }"
              @click="navigateTo('tasks')"
            >
              <el-icon><DataLine /></el-icon>
              <span>任务中心</span>
            </li>
            <li
              :class="{ active: currentRoute === 'map' }"
              @click="navigateTo('map')"
            >
              <el-icon><MapLocation /></el-icon>
              <span>地图监控</span>
            </li>
            <li
              :class="{ active: currentRoute === 'airspace' }"
              @click="navigateTo('airspace')"
            >
              <el-icon><Location /></el-icon>
              <span>空域管理</span>
            </li>
            <li
              :class="{ active: currentRoute === 'weather' }"
              @click="navigateTo('weather')"
            >
              <el-icon><Sunny /></el-icon>
              <span>气象监测</span>
            </li>
            <li
              :class="{ active: currentRoute === 'emergency' }"
              @click="navigateTo('emergency')"
            >
              <el-icon><Warning /></el-icon>
              <span>应急响应</span>
            </li>
            <li
              :class="{ active: currentRoute === 'analytics' }"
              @click="navigateTo('analytics')"
            >
              <el-icon><TrendCharts /></el-icon>
              <span>数据分析</span>
            </li>
            <li
              :class="{ active: currentRoute === 'api-test' }"
              @click="navigateTo('api-test')"
            >
              <el-icon><Connection /></el-icon>
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
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { User, House, DataLine, MapLocation, Pointer, Location, Sunny, Warning, TrendCharts, Connection } from '@element-plus/icons-vue'
import ThemeToggle from '@/components/ThemeToggle.vue'

const router = useRouter()
const currentRoute = ref(router.currentRoute.value.name)

// 监听路由变化
router.afterEach((to) => {
  currentRoute.value = to.name
})

const navigateTo = (route) => {
  router.push(`/${route}`)
}

const handleCommand = (command) => {
  switch(command) {
    case 'logout':
      // 清除用户会话
      localStorage.removeItem('user')
      router.push('/login')
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
/* 苹果风格极简后台布局 */
.app-layout {
  display: flex;
  flex-direction: column;
  min-height: 100vh;
  background-color: var(--bg-primary);
  font-family: var(--font-family);
}

/* 顶部导航栏 - 苹果风格极简设计 */
.app-header {
  background: var(--header-bg);
  backdrop-filter: blur(20px);
  border-bottom: 1px solid var(--header-border);
  padding: 0 2rem;
  position: sticky;
  top: 0;
  z-index: 100;
  transition: all var(--transition-theme);
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  height: 60px;
}

.logo-section {
  display: flex;
  align-items: center;
  gap: 12px;
}

.logo-icon {
  width: 28px;
  height: 28px;
  background: var(--bg-secondary);
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--header-text);
  transition: all var(--transition-fast);
}

.logo-section h1 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
  letter-spacing: -0.3px;
  color: var(--header-text);
}

.header-right {
  display: flex;
  align-items: center;
  gap: 1.5rem;
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 13px;
  font-weight: 500;
  color: var(--text-secondary);
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: var(--success-color);
  animation: pulse 2s infinite;
}

.user-avatar {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  background: var(--bg-secondary);
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  transition: all var(--transition-fast);
  border: 1px solid var(--border-color);
  color: var(--header-text);
}

.user-avatar:hover {
  background: var(--bg-tertiary);
  transform: translateY(-1px);
  box-shadow: var(--shadow-sm);
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.7; }
  100% { opacity: 1; }
}

/* 主内容区域 */
.main-content {
  display: flex;
  flex: 1;
  overflow: hidden;
}

/* 侧边导航栏 - 苹果风格极简设计 */
.sidebar {
  width: 220px;
  background: var(--sidebar-bg);
  border-right: 1px solid var(--border-color);
  display: flex;
  flex-direction: column;
  position: relative;
  transition: background-color var(--transition-theme);
}

.nav-menu ul {
  list-style: none;
  margin: 0;
  padding: 1rem 0;
  flex: 1;
}

.nav-menu li {
  padding: 12px 24px;
  display: flex;
  align-items: center;
  gap: 12px;
  cursor: pointer;
  transition: all var(--transition-fast);
  color: var(--sidebar-text-secondary);
  font-weight: 500;
  margin: 0 12px;
  border-radius: 8px;
}

.nav-menu li:hover {
  background: var(--sidebar-item-hover);
  color: var(--sidebar-text);
}

.nav-menu li.active {
  background: var(--sidebar-item-hover);
  color: var(--sidebar-item-active);
  font-weight: 600;
}

.nav-menu li .el-icon {
  width: 20px;
  height: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
}

.nav-menu span {
  font-size: 14px;
  letter-spacing: -0.1px;
}

.sidebar-footer {
  padding: 1rem 1.5rem;
  border-top: 1px solid var(--border-color);
  text-align: center;
  font-size: 12px;
  color: var(--text-tertiary);
  background: var(--bg-secondary);
  transition: all var(--transition-theme);
}

/* 主内容区 */
.main-area {
  flex: 1;
  overflow-y: auto;
  background: var(--bg-secondary);
  padding: 2rem;
  transition: background-color var(--transition-theme);
}

/* 底部状态栏 */
.app-footer {
  background: var(--bg-primary);
  padding: 1rem 2rem;
  border-top: 1px solid var(--border-color);
  font-size: 13px;
  color: var(--text-secondary);
}

.footer-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.footer-links {
  display: flex;
  gap: 1rem;
}

.footer-links a {
  color: var(--text-secondary);
  text-decoration: none;
  transition: color 0.2s ease;
}

.footer-links a:hover {
  color: var(--primary-color);
}

/* 响应式设计 */
@media (max-width: 1200px) {
  .sidebar {
    width: 200px;
  }
  
  .nav-menu span {
    font-size: 14px;
  }
}

@media (max-width: 768px) {
  .app-header {
    padding: 0 1rem;
  }
  
  .header-content {
    flex-direction: column;
    gap: 1rem;
    text-align: center;
  }
  
  .sidebar {
    position: fixed;
    top: 64px;
    left: -240px;
    width: 240px;
    height: calc(100vh - 64px);
    z-index: 1000;
    transition: left 0.3s ease;
  }
  
  .sidebar.show {
    left: 0;
  }
  
  .main-content {
    flex-direction: column;
  }
  
  .main-area {
    padding: 1rem;
  }
  
  .footer-content {
    flex-direction: column;
    gap: 0.5rem;
    text-align: center;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-dropdown) {
  cursor: pointer;
}

:deep(.el-dropdown-menu) {
  border-radius: 12px;
  box-shadow: 0 8px 30px rgba(0, 0, 0, 0.15);
  border: 1px solid rgba(0, 0, 0, 0.05);
}

:deep(.el-dropdown-menu__item) {
  border-radius: 8px;
  margin: 4px 8px;
  transition: all 0.2s ease;
}

:deep(.el-dropdown-menu__item:hover) {
  background: var(--bg-tertiary);
  color: var(--primary-color);
}

:deep(.el-dropdown-menu__item.divided) {
  border-top: 1px solid var(--border-color);
  margin-top: 8px;
}

/* 全局滚动条样式 */
::-webkit-scrollbar {
  width: 6px;
  height: 6px;
}

::-webkit-scrollbar-track {
  background: transparent;
}

::-webkit-scrollbar-thumb {
  background: rgba(0, 113, 227, 0.3);
  border-radius: 3px;
}

::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 113, 227, 0.5);
}
</style>
