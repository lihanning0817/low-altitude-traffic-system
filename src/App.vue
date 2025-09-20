<template>
  <div id="app">
    <!-- 根据路由决定是否显示背景图案 -->
    <BackgroundPattern v-if="shouldShowBackground" />

    <!-- 根据路由决定显示哪种布局 -->
    <!-- 首页和登录页：使用直接渲染，无侧边栏 -->
    <router-view v-if="isPublicPage" />

    <!-- 后台管理页面：使用带侧边栏的 AppLayout -->
    <AppLayout v-else />

    <!-- 通知中心（全局显示） -->
    <NotificationCenter />
  </div>
</template>

<script>
import { computed, onMounted } from 'vue'
import { useRoute } from 'vue-router'
import { useStore } from 'vuex'
import AppLayout from './components/AppLayout.vue'
import NotificationCenter from './components/NotificationCenter.vue'
import BackgroundPattern from './components/BackgroundPattern.vue'

export default {
  name: 'App',
  components: {
    AppLayout,
    NotificationCenter,
    BackgroundPattern
  },
  setup() {
    const route = useRoute()
    const store = useStore()

    // 在应用启动时初始化主题
    onMounted(() => {
      store.dispatch('initTheme')
    })

    // 判断是否为公共页面（首页、登录页、注册页）
    const isPublicPage = computed(() => {
      const publicRoutes = ['home', 'login', 'register']
      return publicRoutes.includes(route.name)
    })

    // 判断是否需要显示背景图案（仅在后台管理页面显示）
    const shouldShowBackground = computed(() => {
      return !isPublicPage.value
    })

    return {
      isPublicPage,
      shouldShowBackground
    }
  }
}
</script>

<style>
/* 全局样式 */
@import './assets/styles/global.css';
</style>
