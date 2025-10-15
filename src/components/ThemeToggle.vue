<template>
  <div class="theme-toggle">
    <button
      class="toggle-btn"
      :class="{ 'dark': isDarkMode }"
      :aria-label="isDarkMode ? '切换到浅色模式' : '切换到深色模式'"
      @click="toggleTheme"
    >
      <div class="toggle-track">
        <div class="toggle-thumb">
          <span class="toggle-icon">
            {{ isDarkMode ? '☀️' : '🌙' }}
          </span>
        </div>
      </div>
    </button>
  </div>
</template>

<script setup>
import { computed, onMounted } from 'vue'
import { useStore } from 'vuex'

const store = useStore()
const isDarkMode = computed(() => store.state.theme === 'dark')

// 组件挂载时初始化主题
onMounted(() => {
  store.dispatch('initTheme')
})

const toggleTheme = () => {
  store.dispatch('toggleTheme')
}
</script>

<style scoped>
/* 使用 Apple 设计系统 */
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
  transition: all 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
  padding: 2px;
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
}

.toggle-btn:hover {
  transform: scale(1.05);
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.toggle-btn:focus-visible {
  outline: 2px solid var(--apple-blue);
  outline-offset: 2px;
}

.toggle-track {
  position: relative;
  width: 100%;
  height: 100%;
  border-radius: 14px;
  background: var(--color-bg-secondary);
  transition: all 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

.toggle-btn.dark .toggle-track {
  background: #3A3A3C;
}

.toggle-thumb {
  position: absolute;
  top: 2px;
  left: 2px;
  width: 20px;
  height: 20px;
  background: #FFFFFF;
  border-radius: 50%;
  border: 1px solid var(--color-border-light);
  transition: all 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  transform: translateX(0);
}

.toggle-btn.dark .toggle-thumb {
  transform: translateX(22px);
  background: #1C1C1E;
  border-color: #3A3A3C;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
}

.toggle-icon {
  font-size: 12px;
  transition: all 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
  display: flex;
  align-items: center;
  justify-content: center;
}

.toggle-btn:active .toggle-thumb {
  transform: scale(0.92) translateX(0);
}

.toggle-btn.dark:active .toggle-thumb {
  transform: scale(0.92) translateX(22px);
}

/* Apple 风格的微动画 */
@keyframes bounce {
  0%, 100% { transform: scale(1); }
  50% { transform: scale(1.08); }
}

.toggle-btn:active {
  animation: bounce 0.2s ease-out;
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .toggle-btn {
    background: #1C1C1E;
    border-color: #3A3A3C;
  }

  .toggle-btn:hover {
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
  }
}
</style>
