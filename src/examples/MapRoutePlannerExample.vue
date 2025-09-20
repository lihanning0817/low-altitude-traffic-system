<template>
  <div class="map-route-example">
    <h2>地图路径规划功能演示</h2>

    <div class="example-content">
      <div class="instructions">
        <h3>使用说明：</h3>
        <ol>
          <li><strong>双击地图</strong> 任意位置弹出右键菜单</li>
          <li>选择 <strong>"设置为起点"</strong> 设置路线起点</li>
          <li>选择 <strong>"设置为终点"</strong> 设置路线终点</li>
          <li>可选择 <strong>"设置为途经点"</strong> 添加途经点</li>
          <li>点击右侧面板的 <strong>"规划路线"</strong> 按钮生成路线</li>
          <li>在路线选项中可以选择不同的规划策略</li>
          <li>使用 <strong>"清除路线"</strong> 清空所有点位和路线</li>
        </ol>
      </div>

      <div class="features">
        <h3>功能特色：</h3>
        <ul>
          <li>✅ 双击弹出自定义菜单</li>
          <li>✅ 支持起点、终点、途经点设置</li>
          <li>✅ 高德地图路径规划 API 集成</li>
          <li>✅ 绿色路线渲染</li>
          <li>✅ 路线信息显示（距离、时间、路段数）</li>
          <li>✅ 多种规划策略（最快、最短、避免拥堵）</li>
          <li>✅ 避让选项（避免收费、避免高速）</li>
          <li>✅ 响应式 UI 设计</li>
          <li>✅ 深色/浅色主题支持</li>
        </ul>
      </div>

      <div class="api-info">
        <h3>API 信息：</h3>
        <p><strong>高德地图 API Key:</strong> 1872806f332dab32a1a3dc895b0ad542</p>
        <p><strong>版本:</strong> v1.4.15</p>
        <p><strong>插件:</strong> AMap.Driving (驾车路径规划)</p>
      </div>
    </div>

    <!-- 嵌入地图组件 -->
    <div class="map-wrapper">
      <MapComponent />
    </div>
  </div>
</template>

<script setup>
import MapComponent from '@/components/MapComponent.vue'
</script>

<style scoped>
.map-route-example {
  padding: 20px;
  background: var(--bg-secondary);
  min-height: 100vh;
}

.map-route-example h2 {
  color: var(--text-primary);
  margin-bottom: 20px;
  text-align: center;
}

.example-content {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
  margin-bottom: 30px;
}

.instructions,
.features,
.api-info {
  background: var(--card-bg);
  border: 1px solid var(--card-border);
  border-radius: var(--radius-lg);
  padding: 20px;
  box-shadow: var(--shadow-sm);
}

.instructions h3,
.features h3,
.api-info h3 {
  color: var(--primary-color);
  margin-bottom: 15px;
  font-size: 18px;
}

.instructions ol {
  padding-left: 20px;
}

.instructions ol li {
  margin-bottom: 8px;
  color: var(--text-primary);
  line-height: 1.5;
}

.instructions ol li strong {
  color: var(--primary-color);
}

.features ul {
  list-style: none;
  padding: 0;
}

.features ul li {
  margin-bottom: 8px;
  color: var(--text-primary);
  display: flex;
  align-items: center;
  gap: 8px;
}

.api-info p {
  margin-bottom: 8px;
  color: var(--text-secondary);
}

.api-info p strong {
  color: var(--text-primary);
}

.map-wrapper {
  border-radius: var(--radius-lg);
  overflow: hidden;
  box-shadow: var(--shadow-lg);
  border: 1px solid var(--card-border);
}

/* 响应式设计 */
@media (max-width: 768px) {
  .example-content {
    grid-template-columns: 1fr;
  }

  .map-route-example {
    padding: 10px;
  }
}
</style>