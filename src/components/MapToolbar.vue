<template>
  <div class="toolbar">
    <div class="button-group">
      <button
        class="tool-button"
        :class="{ active: currentTool === 'select' }"
        @click="handleToolChange('select')"
      >
        <span class="icon">👆</span>
        <span>选择</span>
      </button>
      <button
        class="tool-button"
        :class="{ active: currentTool === 'measure' }"
        @click="handleToolChange('measure')"
      >
        <span class="icon">📏</span>
        <span>测距</span>
      </button>
      <button
        class="tool-button"
        :class="{ active: currentTool === 'area' }"
        @click="handleToolChange('area')"
      >
        <span class="icon">📐</span>
        <span>测面</span>
      </button>
    </div>
  </div>
</template>

<script setup>
defineProps({
  currentTool: {
    type: String,
    required: true
  }
})

const emit = defineEmits(['tool-change'])

const handleToolChange = (tool) => {
  emit('tool-change', tool)
}
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.toolbar {
  display: flex;
  gap: var(--space-3);
}

.button-group {
  display: flex;
  gap: var(--space-2);
  background: var(--color-bg-secondary);
  padding: var(--space-1);
  border-radius: var(--radius-sm);
}

.tool-button {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  padding: var(--space-2) var(--space-3);
  border: none;
  background: transparent;
  color: var(--color-text-primary);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-sm);
  font-weight: 500;
  border-radius: var(--radius-xs);
  cursor: pointer;
  transition: var(--transition-button);
}

.tool-button .icon {
  font-size: 18px;
}

.tool-button:hover:not(.active) {
  background: var(--color-bg-tertiary);
}

.tool-button.active {
  background: var(--apple-blue);
  color: white;
}

.tool-button:active {
  transform: scale(0.98);
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .button-group {
    background: #1C1C1E;
  }

  .tool-button:hover:not(.active) {
    background: #2C2C2E;
  }

  .tool-button.active {
    background: #0A84FF;
  }
}
</style>
