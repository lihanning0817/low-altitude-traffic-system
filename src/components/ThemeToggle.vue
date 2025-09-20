<template>
  <div class="theme-toggle">
    <button
      class="toggle-btn"
      :class="{ 'dark': isDarkMode }"
      @click="toggleTheme"
      :aria-label="isDarkMode ? '切换到浅色模式' : '切换到深色模式'"
    >
      <div class="toggle-track">
        <div class="toggle-thumb">
          <el-icon :size="14" class="toggle-icon">
            <component :is="iconComponent" />
          </el-icon>
        </div>
      </div>
    </button>
  </div>
</template>

<script setup>
import { computed, onMounted } from 'vue'
import { useStore } from 'vuex'
import { Moon, Sun } from '@element-plus/icons-vue'

const store = useStore()
const isDarkMode = computed(() => store.state.theme === 'dark')

const iconComponent = computed(() => {
  return isDarkMode.value ? Sun : Moon
})

// 组件挂载时初始化主题
onMounted(() => {
  store.dispatch('initTheme')
})

const toggleTheme = () => {
  store.dispatch('toggleTheme')
}
</script>

<style scoped>
.theme-toggle {
  display: inline-flex;
  align-items: center;
  justify-content: center;
}

.toggle-btn {
  position: relative;
  width: 52px;
  height: 28px;
  background: transparent;
  border: none;
  border-radius: 16px;
  cursor: pointer;
  outline: none;
  transition: var(--transition-theme);
  padding: 2px;
  background: var(--bg-secondary);
  border: 1px solid var(--border-color);
}

.toggle-btn:hover {
  transform: scale(1.02);
  box-shadow: var(--shadow-sm);
}

.toggle-btn:focus-visible {
  outline: 2px solid var(--primary-color);
  outline-offset: 2px;
}

.toggle-track {
  position: relative;
  width: 100%;
  height: 100%;
  border-radius: 14px;
  background: var(--bg-secondary);
  transition: var(--transition-theme);
}

.toggle-btn.dark .toggle-track {
  background: var(--bg-tertiary);
}

.toggle-thumb {
  position: absolute;
  top: 2px;
  left: 2px;
  width: 20px;
  height: 20px;
  background: var(--bg-primary);
  border-radius: 50%;
  border: 1px solid var(--border-light);
  transition: var(--transition-theme);
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: var(--shadow-sm);
  transform: translateX(0);
}

.toggle-btn.dark .toggle-thumb {
  transform: translateX(22px);
  background: var(--bg-primary);
  border-color: var(--border-color);
  box-shadow: var(--shadow-md);
}

.toggle-icon {
  color: var(--text-secondary);
  transition: var(--transition-theme);
}

.toggle-btn.dark .toggle-icon {
  color: var(--primary-color);
}

.toggle-btn:active .toggle-thumb {
  transform: scale(0.95) translateX(0);
}

.toggle-btn.dark:active .toggle-thumb {
  transform: scale(0.95) translateX(22px);
}

/* 苹果风格的微动画 */
@keyframes bounce {
  0%, 100% { transform: scale(1); }
  50% { transform: scale(1.05); }
}

.toggle-btn:active {
  animation: bounce 0.2s ease-out;
}
</style>
