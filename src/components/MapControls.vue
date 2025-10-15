<template>
  <div class="central-controls">
    <div class="button-group">
      <button
        class="control-button success"
        @click="emit('refresh')"
      >
        <span class="icon">🔄</span>
        <span>刷新</span>
      </button>
      <button
        class="control-button"
        @click="emit('center')"
      >
        <span class="icon">🎯</span>
        <span>居中</span>
      </button>
      <button
        class="control-button"
        @click="emit('toggle-traffic')"
      >
        <span class="icon">👁️</span>
        <span>{{ showTraffic ? '隐藏' : '显示' }}交通</span>
      </button>
    </div>
  </div>
</template>

<script setup>
defineProps({
  showTraffic: {
    type: Boolean,
    required: true
  }
})

const emit = defineEmits(['refresh', 'center', 'toggle-traffic'])
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.central-controls {
  display: flex;
  gap: var(--space-3);
}

.button-group {
  display: flex;
  gap: var(--space-2);
}

.control-button {
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

.control-button .icon {
  font-size: 18px;
}

.control-button:hover {
  background: var(--color-bg-secondary);
  border-color: var(--color-border-hover);
  transform: translateY(-1px);
}

.control-button:active {
  transform: translateY(0);
}

.control-button.success {
  background: #34C759;
  color: white;
  border-color: #34C759;
}

.control-button.success:hover {
  background: #28A745;
  border-color: #28A745;
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .control-button {
    background: #1C1C1E;
    border-color: #3A3A3C;
    color: #F5F5F7;
  }

  .control-button:hover {
    background: #2C2C2E;
    border-color: #48484A;
  }

  .control-button.success {
    background: #30D158;
    border-color: #30D158;
  }

  .control-button.success:hover {
    background: #28A745;
    border-color: #28A745;
  }
}
</style>
