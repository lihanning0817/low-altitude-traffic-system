<template>
  <div
    class="smart-card"
    :class="{
      'hoverable': hoverable,
      'full-height': fullHeight
    }"
  >
    <div v-if="header" class="smart-card-header">
      {{ header }}
    </div>
    <div class="smart-card-body">
      <slot></slot>
    </div>
    <div v-if="$slots.footer" class="smart-card-footer">
      <slot name="footer"></slot>
    </div>
  </div>
</template>

<script setup>
defineProps({
  header: {
    type: String,
    default: ''
  },
  hoverable: {
    type: Boolean,
    default: false
  },
  fullHeight: {
    type: Boolean,
    default: false
  }
})
</script>

<style scoped>
.smart-card {
  background: var(--card-bg, #ffffff);
  border-radius: 12px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.08);
  overflow: hidden;
  transition: all 0.3s ease;
  border: 1px solid var(--border-color, #e4e7ed);
}

.smart-card.hoverable:hover {
  transform: translateY(-4px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.12);
}

.smart-card.full-height {
  height: 100%;
}

.smart-card-header {
  padding: 16px 20px;
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary, #303133);
  border-bottom: 1px solid var(--border-color, #e4e7ed);
  background: var(--bg-secondary, #f5f7fa);
}

.smart-card-body {
  padding: 20px;
}

.smart-card-footer {
  padding: 12px 20px;
  border-top: 1px solid var(--border-color, #e4e7ed);
  background: var(--bg-secondary, #f5f7fa);
}

/* 深色模式支持 */
@media (prefers-color-scheme: dark) {
  .smart-card {
    background: #1C1C1E;
    border-color: rgba(255, 255, 255, 0.1);
  }

  .smart-card-header,
  .smart-card-footer {
    background: #2C2C2E;
    border-color: rgba(255, 255, 255, 0.1);
    color: #F5F5F7;
  }
}
</style>
