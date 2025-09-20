<template>
  <div class="page-header">
    <div class="header-content">
      <div class="title-section">
        <h2>实时地图监控</h2>
        <p>监控低空飞行器位置和交通状况</p>
      </div>
      <div class="status-indicators">
        <div class="status-item online">
          <div class="status-dot" />
          <span>系统在线</span>
        </div>
        <div class="status-item">
          <el-icon><Location /></el-icon>
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
.page-header {
  margin-bottom: 0;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 0;
}

.title-section h2 {
  margin: 0 0 8px 0;
  font-size: 28px;
  font-weight: 600;
  color: #2c3e50;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.title-section p {
  margin: 0;
  color: #7f8c8d;
  font-size: 16px;
}

.status-indicators {
  display: flex;
  gap: 24px;
  align-items: center;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  color: #606266;
}

.status-item.online .status-dot {
  width: 8px;
  height: 8px;
  background: #67c23a;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.5; }
  100% { opacity: 1; }
}
</style>
