<template>
  <div
    class="smart-card"
    :class="{ 'hoverable': hoverable, 'full-height': fullHeight }"
  >
    <div
      v-if="header"
      class="card-header"
    >
      <h3 class="card-title">
        {{ header }}
      </h3>
      <slot name="actions" />
    </div>
    
    <div class="card-body">
      <slot />
    </div>
    
    <div
      v-if="footer"
      class="card-footer"
    >
      <slot name="footer" />
    </div>
  </div>
</template>

<script setup>
import { defineProps } from 'vue'

defineProps({
  header: {
    type: String,
    default: ''
  },
  footer: {
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
  background: var(--bg-primary);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-lg);
  overflow: hidden;
  transition: all var(--transition-fast);
  display: flex;
  flex-direction: column;
  height: auto;
}

.smart-card.hoverable {
  cursor: pointer;
}

.smart-card.hoverable:hover {
  border-color: rgba(0, 113, 227, 0.2);
  transform: translateY(-1px);
}

.smart-card.full-height {
  height: 100%;
}

.card-header {
  padding: 1.5rem;
  border-bottom: 1px solid var(--border-color);
  display: flex;
  justify-content: space-between;
  align-items: center;
  background: var(--bg-tertiary);
}

.card-title {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
  color: var(--text-primary);
}

.card-body {
  flex: 1;
  padding: 1.5rem;
  overflow-y: auto;
}

.card-footer {
  padding: 1rem 1.5rem;
  border-top: 1px solid var(--border-color);
  background: var(--bg-tertiary);
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-size: 13px;
  color: var(--text-secondary);
}

/* 响应式设计 */
@media (max-width: 768px) {
  .smart-card {
    margin-bottom: 1rem;
  }
  
  .card-header {
    padding: 1rem;
  }
  
  .card-body {
    padding: 1rem;
  }
  
  .card-footer {
    padding: 0.75rem 1rem;
  }
}
</style>
