<template>
  <div class="icon-library">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">🎨 图标库</h1>
      <p class="page-subtitle">本项目使用的 Emoji 图标集合</p>
    </div>

    <!-- 图标分类展示 -->
    <div class="categories-container">
      <!-- 导航菜单图标 -->
      <AppleCard class="category-card">
        <h2 class="category-title">🧭 导航菜单</h2>
        <div class="icons-grid">
          <div
            v-for="icon in navigationIcons"
            :key="icon.name"
            class="icon-item"
          >
            <div class="icon-display">{{ icon.emoji }}</div>
            <div class="icon-info">
              <div class="icon-name">{{ icon.name }}</div>
              <div class="icon-description">{{ icon.description }}</div>
            </div>
          </div>
        </div>
      </AppleCard>

      <!-- 状态与操作图标 -->
      <AppleCard class="category-card">
        <h2 class="category-title">⚡ 状态与操作</h2>
        <div class="icons-grid">
          <div
            v-for="icon in actionIcons"
            :key="icon.name"
            class="icon-item"
          >
            <div class="icon-display">{{ icon.emoji }}</div>
            <div class="icon-info">
              <div class="icon-name">{{ icon.name }}</div>
              <div class="icon-description">{{ icon.description }}</div>
            </div>
          </div>
        </div>
      </AppleCard>

      <!-- 交通与飞行图标 -->
      <AppleCard class="category-card">
        <h2 class="category-title">✈️ 交通与飞行</h2>
        <div class="icons-grid">
          <div
            v-for="icon in transportIcons"
            :key="icon.name"
            class="icon-item"
          >
            <div class="icon-display">{{ icon.emoji }}</div>
            <div class="icon-info">
              <div class="icon-name">{{ icon.name }}</div>
              <div class="icon-description">{{ icon.description }}</div>
            </div>
          </div>
        </div>
      </AppleCard>

      <!-- 天气与环境图标 -->
      <AppleCard class="category-card">
        <h2 class="category-title">🌤️ 天气与环境</h2>
        <div class="icons-grid">
          <div
            v-for="icon in weatherIcons"
            :key="icon.name"
            class="icon-item"
          >
            <div class="icon-display">{{ icon.emoji }}</div>
            <div class="icon-info">
              <div class="icon-name">{{ icon.name }}</div>
              <div class="icon-description">{{ icon.description }}</div>
            </div>
          </div>
        </div>
      </AppleCard>

      <!-- 数据与分析图标 -->
      <AppleCard class="category-card">
        <h2 class="category-title">📊 数据与分析</h2>
        <div class="icons-grid">
          <div
            v-for="icon in dataIcons"
            :key="icon.name"
            class="icon-item"
          >
            <div class="icon-display">{{ icon.emoji }}</div>
            <div class="icon-info">
              <div class="icon-name">{{ icon.name }}</div>
              <div class="icon-description">{{ icon.description }}</div>
            </div>
          </div>
        </div>
      </AppleCard>

      <!-- 系统与工具图标 -->
      <AppleCard class="category-card">
        <h2 class="category-title">🔧 系统与工具</h2>
        <div class="icons-grid">
          <div
            v-for="icon in systemIcons"
            :key="icon.name"
            class="icon-item"
          >
            <div class="icon-display">{{ icon.emoji }}</div>
            <div class="icon-info">
              <div class="icon-name">{{ icon.name }}</div>
              <div class="icon-description">{{ icon.description }}</div>
            </div>
          </div>
        </div>
      </AppleCard>
    </div>

    <!-- 使用说明 -->
    <AppleCard class="usage-card">
      <h2 class="section-title">📖 使用说明</h2>
      <div class="usage-content">
        <p>本项目采用 Emoji 图标系统，优势包括：</p>
        <ul class="advantages-list">
          <li>✅ 无需额外的图标库依赖，减少包体积</li>
          <li>✅ 跨平台兼容性好，所有设备原生支持</li>
          <li>✅ 符合 Apple 设计规范的简洁风格</li>
          <li>✅ 易于维护和更新</li>
          <li>✅ 提升页面加载速度</li>
        </ul>
        <div class="code-example">
          <h3 class="example-title">代码示例</h3>
          <pre class="code-block"><code>&lt;span class="icon"&gt;🏠&lt;/span&gt; 首页
&lt;button&gt;✈️ 飞行任务&lt;/button&gt;
&lt;div class="status"&gt;✅ 成功&lt;/div&gt;</code></pre>
        </div>
      </div>
    </AppleCard>
  </div>
</template>

<script setup>
import AppleCard from './apple/AppleCard.vue'

// 导航菜单图标
const navigationIcons = [
  { emoji: '🏠', name: 'Home', description: '首页/主页' },
  { emoji: '📊', name: 'Dashboard', description: '仪表盘/数据面板' },
  { emoji: '🗺️', name: 'Map', description: '地图' },
  { emoji: '✈️', name: 'Flight', description: '飞行/航班' },
  { emoji: '📋', name: 'List', description: '列表' },
  { emoji: '👤', name: 'User', description: '用户/个人' },
  { emoji: '⚙️', name: 'Settings', description: '设置' },
  { emoji: '🔔', name: 'Notification', description: '通知' }
]

// 状态与操作图标
const actionIcons = [
  { emoji: '✅', name: 'Success', description: '成功/完成' },
  { emoji: '❌', name: 'Error', description: '错误/失败' },
  { emoji: '⚠️', name: 'Warning', description: '警告' },
  { emoji: 'ℹ️', name: 'Info', description: '信息' },
  { emoji: '🔄', name: 'Refresh', description: '刷新/重新加载' },
  { emoji: '➕', name: 'Add', description: '添加/新增' },
  { emoji: '✏️', name: 'Edit', description: '编辑' },
  { emoji: '🗑️', name: 'Delete', description: '删除' },
  { emoji: '💾', name: 'Save', description: '保存' },
  { emoji: '🔍', name: 'Search', description: '搜索' },
  { emoji: '🔗', name: 'Link', description: '链接' },
  { emoji: '📤', name: 'Upload', description: '上传' },
  { emoji: '📥', name: 'Download', description: '下载' }
]

// 交通与飞行图标
const transportIcons = [
  { emoji: '✈️', name: 'Airplane', description: '飞机' },
  { emoji: '🚁', name: 'Helicopter', description: '直升机' },
  { emoji: '🛸', name: 'UFO', description: 'UFO/飞碟' },
  { emoji: '🛫', name: 'Takeoff', description: '起飞' },
  { emoji: '🛬', name: 'Landing', description: '降落' },
  { emoji: '📍', name: 'Location', description: '位置/定位' },
  { emoji: '🎯', name: 'Target', description: '目标' },
  { emoji: '🗺️', name: 'WorldMap', description: '世界地图' },
  { emoji: '🧭', name: 'Compass', description: '指南针' },
  { emoji: '📡', name: 'Satellite', description: '卫星/信号' },
  { emoji: '🚦', name: 'TrafficLight', description: '红绿灯/交通' }
]

// 天气与环境图标
const weatherIcons = [
  { emoji: '☀️', name: 'Sunny', description: '晴天' },
  { emoji: '🌤️', name: 'PartlyCloudy', description: '局部多云' },
  { emoji: '☁️', name: 'Cloudy', description: '多云' },
  { emoji: '🌧️', name: 'Rainy', description: '雨天' },
  { emoji: '⛈️', name: 'Thunderstorm', description: '雷暴' },
  { emoji: '🌨️', name: 'Snowy', description: '下雪' },
  { emoji: '🌪️', name: 'Tornado', description: '龙卷风' },
  { emoji: '🌫️', name: 'Foggy', description: '雾' },
  { emoji: '💨', name: 'Wind', description: '风' },
  { emoji: '🌡️', name: 'Temperature', description: '温度' },
  { emoji: '💧', name: 'Droplet', description: '水滴/湿度' },
  { emoji: '🌙', name: 'Moon', description: '月亮/夜晚' }
]

// 数据与分析图标
const dataIcons = [
  { emoji: '📊', name: 'BarChart', description: '柱状图' },
  { emoji: '📈', name: 'TrendingUp', description: '上升趋势' },
  { emoji: '📉', name: 'TrendingDown', description: '下降趋势' },
  { emoji: '💹', name: 'ChartIncreasing', description: '增长图表' },
  { emoji: '📋', name: 'Clipboard', description: '剪贴板/记录' },
  { emoji: '📝', name: 'Memo', description: '备忘录/笔记' },
  { emoji: '📄', name: 'Document', description: '文档' },
  { emoji: '📁', name: 'Folder', description: '文件夹' },
  { emoji: '🗂️', name: 'CardIndex', description: '索引/分类' },
  { emoji: '🔢', name: 'Numbers', description: '数字' }
]

// 系统与工具图标
const systemIcons = [
  { emoji: '⚙️', name: 'Settings', description: '设置/配置' },
  { emoji: '🔧', name: 'Wrench', description: '工具/修复' },
  { emoji: '🛠️', name: 'Tools', description: '工具箱' },
  { emoji: '🔑', name: 'Key', description: '密钥/权限' },
  { emoji: '🔒', name: 'Lock', description: '锁定/加密' },
  { emoji: '🔓', name: 'Unlock', description: '解锁' },
  { emoji: '🛡️', name: 'Shield', description: '防护/安全' },
  { emoji: '⚡', name: 'Lightning', description: '闪电/快速' },
  { emoji: '💡', name: 'Bulb', description: '灯泡/想法' },
  { emoji: '🔌', name: 'Plug', description: '插头/连接' },
  { emoji: '📱', name: 'Mobile', description: '手机/移动设备' },
  { emoji: '💻', name: 'Laptop', description: '笔记本电脑' },
  { emoji: '🖥️', name: 'Desktop', description: '台式电脑' },
  { emoji: '⌚', name: 'Watch', description: '手表/时间' },
  { emoji: '⏱️', name: 'Stopwatch', description: '秒表/计时' }
]
</script>

<style scoped>
.icon-library {
  min-height: 100vh;
  background: var(--color-bg-secondary, #F5F5F7);
  padding: var(--space-6, 24px);
}

/* 页面头部 */
.page-header {
  max-width: 1400px;
  margin: 0 auto var(--space-8, 32px);
  text-align: center;
}

.page-title {
  margin: 0 0 var(--space-2, 8px) 0;
  font-size: var(--font-size-3xl, 32px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  letter-spacing: var(--letter-spacing-tight, -0.02em);
}

.page-subtitle {
  margin: 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
}

/* 分类容器 */
.categories-container {
  max-width: 1400px;
  margin: 0 auto;
  display: flex;
  flex-direction: column;
  gap: var(--space-6, 24px);
}

/* 分类卡片 */
.category-card {
  padding: var(--space-6, 24px);
}

.category-title {
  margin: 0 0 var(--space-5, 20px) 0;
  font-size: var(--font-size-xl, 24px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  padding-bottom: var(--space-3, 12px);
  border-bottom: 2px solid var(--color-border, #E5E5E7);
}

/* 图标网格 */
.icons-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
  gap: var(--space-4, 16px);
}

/* 图标项 */
.icon-item {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  transition: all var(--transition-smooth, 0.3s ease);
  cursor: pointer;
}

.icon-item:hover {
  background: var(--color-bg-tertiary, #E8E8ED);
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08);
}

.icon-display {
  font-size: 36px;
  line-height: 1;
  flex-shrink: 0;
  width: 48px;
  height: 48px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-md, 10px);
}

.icon-info {
  flex: 1;
  min-width: 0;
}

.icon-name {
  font-size: var(--font-size-base, 16px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin-bottom: var(--space-1, 4px);
}

.icon-description {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
  line-height: 1.4;
}

/* 使用说明卡片 */
.usage-card {
  max-width: 1400px;
  margin: 0 auto;
  padding: var(--space-6, 24px);
}

.section-title {
  margin: 0 0 var(--space-5, 20px) 0;
  font-size: var(--font-size-xl, 24px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.usage-content {
  color: var(--color-text-primary, #1D1D1F);
}

.usage-content > p {
  font-size: var(--font-size-base, 16px);
  margin: 0 0 var(--space-4, 16px) 0;
  line-height: 1.6;
}

.advantages-list {
  margin: 0 0 var(--space-6, 24px) 0;
  padding-left: var(--space-5, 20px);
  list-style: none;
}

.advantages-list li {
  font-size: var(--font-size-base, 16px);
  line-height: 1.8;
  margin-bottom: var(--space-2, 8px);
  color: var(--color-text-primary, #1D1D1F);
}

.code-example {
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-lg, 12px);
  padding: var(--space-5, 20px);
  border-left: 4px solid var(--color-primary, #0071E3);
}

.example-title {
  margin: 0 0 var(--space-3, 12px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.code-block {
  margin: 0;
  padding: var(--space-4, 16px);
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-md, 8px);
  overflow-x: auto;
}

.code-block code {
  font-family: 'SF Mono', 'Monaco', 'Inconsolata', 'Fira Code', 'Droid Sans Mono', 'Source Code Pro', monospace;
  font-size: var(--font-size-sm, 14px);
  line-height: 1.6;
  color: var(--color-text-primary, #1D1D1F);
}

/* 响应式设计 */
@media (max-width: 768px) {
  .icon-library {
    padding: var(--space-4, 16px);
  }

  .page-title {
    font-size: var(--font-size-2xl, 28px);
  }

  .category-title {
    font-size: var(--font-size-lg, 20px);
  }

  .icons-grid {
    grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
    gap: var(--space-3, 12px);
  }

  .icon-display {
    font-size: 28px;
    width: 40px;
    height: 40px;
  }

  .icon-name {
    font-size: var(--font-size-sm, 14px);
  }

  .icon-description {
    font-size: var(--font-size-xs, 12px);
  }
}

@media (max-width: 480px) {
  .icons-grid {
    grid-template-columns: 1fr;
  }
}

/* 深色模式 */
@media (prefers-color-scheme: dark) {
  .icon-library {
    background: #000000;
  }

  .page-title,
  .category-title,
  .section-title,
  .icon-name,
  .usage-content > p,
  .advantages-list li {
    color: #F5F5F7;
  }

  .page-subtitle,
  .icon-description {
    color: #98989D;
  }

  .icon-item {
    background: #1C1C1E;
  }

  .icon-item:hover {
    background: #2C2C2E;
  }

  .icon-display {
    background: #2C2C2E;
  }

  .code-example {
    background: #1C1C1E;
  }

  .code-block {
    background: #2C2C2E;
  }

  .code-block code {
    color: #F5F5F7;
  }
}
</style>
