<template>
  <div class="page-header">
    <div class="header-content">
      <div class="title-section">
        <h2>🗺️ 实时地图监控</h2>
        <p>监控低空飞行器位置和交通状况</p>
      </div>
      <div class="status-indicators">
        <div class="status-item online">
          <div class="status-dot" />
          <span>系统在线</span>
        </div>
        <div class="status-item">
          <span class="icon">📍</span>
          <span>{{ activeDevices }} 设备活跃</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
defineProps({
  activeDevices: {
    type: Number,
    required: true
  }
})
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.page-header {
  margin-bottom: 0;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: var(--space-5) 0;
}

.title-section h2 {
  margin: 0 0 var(--space-2) 0;
  font-size: var(--font-size-2xl);
  font-weight: 600;
  color: var(--color-text-primary);
  letter-spacing: var(--letter-spacing-tight);
}

.title-section p {
  margin: 0;
  color: var(--color-text-secondary);
  font-size: var(--font-size-base);
}

.status-indicators {
  display: flex;
  gap: var(--space-6);
  align-items: center;
}

.status-item {
  display: flex;
  align-items: center;
  gap: var(--space-2);
  font-size: var(--font-size-sm);
  color: var(--color-text-primary);
  font-weight: 500;
}

.status-item .icon {
  font-size: 18px;
}

.status-item.online .status-dot {
  width: 8px;
  height: 8px;
  background: #34C759;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.5; }
  100% { opacity: 1; }
}

/* Responsive design */
@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: var(--space-4);
    align-items: flex-start;
  }

  .status-indicators {
    gap: var(--space-4);
  }
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .title-section h2 {
    color: #F5F5F7;
  }

  .title-section p {
    color: #86868B;
  }

  .status-item {
    color: #F5F5F7;
  }

  .status-item.online .status-dot {
    background: #30D158;
  }
}
</style>
