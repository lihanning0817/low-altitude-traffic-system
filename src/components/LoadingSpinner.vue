<template>
  <div
    class="loading-spinner"
    :class="{ 'full-screen': fullscreen }"
  >
    <div class="spinner-container">
      <!-- Apple 风格的加载动画 -->
      <div class="apple-spinner">
        <div class="spinner-circle" />
      </div>
      <p
        v-if="message"
        class="loading-message"
      >
        {{ message }}
      </p>
    </div>
  </div>
</template>

<script setup>
defineProps({
  message: {
    type: String,
    default: '加载中...'
  },
  fullscreen: {
    type: Boolean,
    default: false
  }
})
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.loading-spinner {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 100px;
}

.loading-spinner.full-screen {
  position: fixed;
  top: 0;
  left: 0;
  width: 100vw;
  height: 100vh;
  background-color: rgba(255, 255, 255, 0.85);
  z-index: 9999;
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
}

.spinner-container {
  text-align: center;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: var(--space-4);
}

/* Apple 风格的环形加载动画 */
.apple-spinner {
  width: 44px;
  height: 44px;
  position: relative;
}

.spinner-circle {
  width: 100%;
  height: 100%;
  border-radius: 50%;
  border: 3px solid transparent;
  border-top-color: var(--apple-blue);
  border-right-color: var(--apple-blue);
  animation: apple-spin 0.8s cubic-bezier(0.4, 0.0, 0.2, 1) infinite;
}

.loading-message {
  color: var(--color-text-secondary);
  font-size: var(--font-size-sm);
  font-weight: 500;
  margin: 0;
  letter-spacing: var(--letter-spacing-normal);
}

/* Apple 风格的旋转动画 */
@keyframes apple-spin {
  0% {
    transform: rotate(0deg);
    opacity: 1;
  }
  50% {
    opacity: 0.8;
  }
  100% {
    transform: rotate(360deg);
    opacity: 1;
  }
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .loading-spinner.full-screen {
    background-color: rgba(0, 0, 0, 0.85);
  }

  .spinner-circle {
    border-top-color: #0A84FF;
    border-right-color: #0A84FF;
  }

  .loading-message {
    color: #86868B;
  }
}
</style>
