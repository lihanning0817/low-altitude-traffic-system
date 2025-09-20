<template>
  <div class="task-tracking-container">
    <h1>实时任务追踪</h1>
    <div class="tracking-content">
      <div class="map-container">
        <div
          ref="mapContainer"
          class="map-placeholder"
        >
          <!-- 这里将集成高德地图 -->
          <p>高德地图集成区域</p>
        </div>
      </div>
      <div class="task-info">
        <h2>任务信息</h2>
        <div class="task-details">
          <p><strong>任务ID:</strong> TSK001</p>
          <p><strong>状态:</strong> <span class="status in-progress">进行中</span></p>
          <p><strong>无人机:</strong> DJI Mavic 3</p>
          <p><strong>当前位置:</strong> <span class="coordinates">39.9042°N, 116.4074°E</span></p>
          <p><strong>电池:</strong> <span class="battery">85%</span></p>
          <p><strong>预计到达时间:</strong> 15:30</p>
        </div>
        <div class="progress-section">
          <h3>任务进度</h3>
          <div class="progress-bar">
            <div
              class="progress-fill"
              :style="{ width: progress + '%' }"
            />
          </div>
          <p>{{ progress }}% 完成</p>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'TaskTracking',
  data() {
    return {
      progress: 65
    }
  },
  mounted() {
    // 模拟进度更新
    this.simulateProgress()
  },
  methods: {
    simulateProgress() {
      const interval = setInterval(() => {
        if (this.progress < 100) {
          this.progress += 5
        } else {
          clearInterval(interval)
        }
      }, 2000)
    }
  }
}
</script>

<style scoped>
.task-tracking-container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 2rem;
}

h1 {
  text-align: center;
  color: #333;
  margin-bottom: 2rem;
}

.tracking-content {
  display: flex;
  gap: 2rem;
}

.map-container {
  flex: 3;
}

.map-placeholder {
  height: 500px;
  background-color: #f0f0f0;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #666;
  font-size: 1.2rem;
}

.task-info {
  flex: 1;
  background: white;
  border-radius: 8px;
  padding: 1.5rem;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
}

.task-info h2 {
  margin-top: 0;
  color: #333;
  border-bottom: 1px solid #eee;
  padding-bottom: 1rem;
}

.task-details p {
  margin: 1rem 0;
  line-height: 1.5;
}

.status {
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  color: white;
  font-size: 0.9rem;
}

.status.in-progress {
  background-color: #409eff;
}

.coordinates {
  font-family: monospace;
  color: #409eff;
}

.battery {
  color: #67c23a;
  font-weight: 500;
}

.progress-section {
  margin-top: 2rem;
}

.progress-section h3 {
  margin-top: 0;
  color: #333;
}

.progress-bar {
  width: 100%;
  height: 10px;
  background-color: #e0e0e0;
  border-radius: 5px;
  overflow: hidden;
  margin: 1rem 0;
}

.progress-fill {
  height: 100%;
  background-color: #409eff;
  transition: width 0.3s;
}

@media (max-width: 768px) {
  .tracking-content {
    flex-direction: column;
  }

  .map-placeholder {
    height: 300px;
  }
}
</style>