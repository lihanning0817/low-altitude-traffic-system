<template>
  <div class="action-controls">
    <button
      class="action-button warning"
      @click="emit('fullscreen')"
    >
      <span class="icon">⛶</span>
      <span>全屏</span>
    </button>
    <button
      class="action-button"
      @click="emit('settings')"
    >
      <span class="icon">⚙️</span>
      <span>设置</span>
    </button>
  </div>
</template>

<script setup>
const emit = defineEmits(['fullscreen', 'settings'])
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.action-controls {
  display: flex;
  gap: var(--space-2);
}

.action-button {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  padding: var(--space-2) var(--space-4);
  border: 1px solid var(--color-border-default);
  background: var(--color-bg-primary);
  color: var(--color-text-primary);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-sm);
  font-weight: 500;
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: var(--transition-button);
}

.action-button .icon {
  font-size: 18px;
}

.action-button:hover {
  background: var(--color-bg-secondary);
  border-color: var(--color-border-hover);
  transform: translateY(-1px);
}

.action-button:active {
  transform: translateY(0);
}

.action-button.warning {
  background: #FF9500;
  color: white;
  border-color: #FF9500;
}

.action-button.warning:hover {
  background: #FF8000;
  border-color: #FF8000;
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .action-button {
    background: #1C1C1E;
    border-color: #3A3A3C;
    color: #F5F5F7;
  }

  .action-button:hover {
    background: #2C2C2E;
    border-color: #48484A;
  }

  .action-button.warning {
    background: #FF9F0A;
    border-color: #FF9F0A;
  }

  .action-button.warning:hover {
    background: #FF8000;
    border-color: #FF8000;
  }
}
</style>
