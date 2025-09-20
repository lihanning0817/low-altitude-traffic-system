<template>
  <div
    class="loading-spinner"
    :class="{ 'full-screen': fullscreen }"
  >
    <div class="spinner-container">
      <div class="spinner" />
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
import { defineProps } from 'vue'

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
  background-color: rgba(255, 255, 255, 0.8);
  z-index: 9999;
  backdrop-filter: blur(5px);
}

.spinner-container {
  text-align: center;
}

.spinner {
  width: 48px;
  height: 48px;
  border: 4px solid #f3f3f3;
  border-top: 4px solid var(--primary-color);
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-bottom: 16px;
}

.loading-message {
  color: var(--text-secondary);
  font-size: 14px;
  font-weight: 500;
  margin: 0;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}
</style>
