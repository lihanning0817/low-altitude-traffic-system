<template>
  <div class="flight-task-detail">
    <!-- 顶部导航栏 -->
    <div class="detail-header">
      <button class="back-button" @click="goBack">
        <span class="back-icon">←</span>
        <span>返回</span>
      </button>
      <h1 class="page-title">✈️ 飞行任务详情</h1>
    </div>

    <!-- 主内容区域 -->
    <div class="detail-container">
      <!-- 任务状态卡片 -->
      <AppleCard class="status-card">
        <div class="card-header">
          <h2 class="card-title">{{ task.name }}</h2>
          <span :class="['status-badge', `status-${task.status}`]">
            {{ getStatusText(task.status) }}
          </span>
        </div>

        <div class="task-meta">
          <div class="meta-item">
            <span class="meta-label">任务编号</span>
            <span class="meta-value">{{ task.id || 'N/A' }}</span>
          </div>
          <div class="meta-item">
            <span class="meta-label">创建时间</span>
            <span class="meta-value">{{ formatDate(task.createdAt) }}</span>
          </div>
          <div class="meta-item">
            <span class="meta-label">预计时长</span>
            <span class="meta-value">{{ task.duration || 'N/A' }}</span>
          </div>
        </div>
      </AppleCard>

      <!-- 飞行信息卡片 -->
      <AppleCard class="info-card">
        <h3 class="section-title">📍 飞行信息</h3>
        <div class="info-grid">
          <div class="info-item">
            <div class="info-icon">🛫</div>
            <div class="info-content">
              <span class="info-label">起飞地点</span>
              <span class="info-value">{{ task.start }}</span>
            </div>
          </div>

          <div class="info-divider"></div>

          <div class="info-item">
            <div class="info-icon">🛬</div>
            <div class="info-content">
              <span class="info-label">降落地点</span>
              <span class="info-value">{{ task.end }}</span>
            </div>
          </div>
        </div>

        <div class="info-detail">
          <div class="detail-row">
            <span class="detail-label">飞行器型号</span>
            <span class="detail-value">{{ task.aircraft }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">飞行高度</span>
            <span class="detail-value">{{ task.altitude || 'N/A' }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">飞行速度</span>
            <span class="detail-value">{{ task.speed || 'N/A' }}</span>
          </div>
        </div>
      </AppleCard>

      <!-- 任务进度时间线 -->
      <AppleCard class="timeline-card">
        <h3 class="section-title">📊 任务进度</h3>
        <div class="timeline">
          <div
            v-for="(event, index) in taskTimeline"
            :key="index"
            class="timeline-item"
          >
            <div :class="['timeline-dot', { 'active': event.completed }]"></div>
            <div class="timeline-content">
              <h4 class="timeline-title">{{ event.title }}</h4>
              <p class="timeline-time">{{ event.time }}</p>
              <p v-if="event.description" class="timeline-desc">
                {{ event.description }}
              </p>
            </div>
          </div>
        </div>
      </AppleCard>

      <!-- 操作按钮区域 -->
      <div class="action-buttons">
        <AppleButton
          variant="secondary"
          @click="editTask"
        >
          编辑任务
        </AppleButton>
        <AppleButton
          variant="primary"
          @click="startTask"
          :disabled="task.status === 'in_progress'"
        >
          {{ task.status === 'in_progress' ? '任务进行中' : '开始任务' }}
        </AppleButton>
      </div>
    </div>
  </div>
</template>

<script>
import AppleCard from './apple/AppleCard.vue';
import AppleButton from './apple/AppleButton.vue';

export default {
  name: "FlightTaskDetail",
  components: {
    AppleCard,
    AppleButton
  },
  data() {
    return {
      task: {
        id: 'TASK-001',
        name: '任务1',
        status: 'pending',
        start: '北京',
        end: '上海',
        aircraft: '飞行器1',
        createdAt: new Date(),
        duration: '2小时30分钟',
        altitude: '500米',
        speed: '60公里/小时'
      },
      taskTimeline: [
        {
          title: '任务创建',
          time: '2025-10-15 09:00',
          description: '任务已创建并提交审批',
          completed: true
        },
        {
          title: '审批通过',
          time: '2025-10-15 09:30',
          description: '飞行计划已通过审批',
          completed: true
        },
        {
          title: '准备起飞',
          time: '2025-10-15 10:00',
          description: '正在进行飞行前检查',
          completed: false
        },
        {
          title: '任务执行',
          time: '预计 2025-10-15 10:30',
          description: '开始执行飞行任务',
          completed: false
        },
        {
          title: '任务完成',
          time: '预计 2025-10-15 13:00',
          description: '任务完成并返航',
          completed: false
        }
      ]
    };
  },
  mounted() {
    // const taskId = this.$route.params.id;
    // TODO: 根据taskId加载任务详情
  },
  methods: {
    goBack() {
      this.$router.go(-1);
    },
    getStatusText(status) {
      const statusMap = {
        'pending': '待执行',
        'in_progress': '进行中',
        'completed': '已完成',
        'cancelled': '已取消'
      };
      return statusMap[status] || status;
    },
    formatDate(date) {
      if (!date) return 'N/A';
      const d = new Date(date);
      return d.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
      });
    },
    editTask() {
      // TODO: 导航到编辑页面
      console.log('编辑任务');
    },
    startTask() {
      // TODO: 开始任务
      console.log('开始任务');
    }
  }
};
</script>

<style scoped>
.flight-task-detail {
  min-height: 100vh;
  background: var(--color-bg-secondary, #F5F5F7);
  padding-bottom: var(--space-8, 32px);
}

/* 顶部导航 */
.detail-header {
  background: var(--color-bg-primary, #FFFFFF);
  padding: var(--space-4, 16px) var(--space-6, 24px);
  border-bottom: 1px solid var(--color-border, #E5E5E7);
  position: sticky;
  top: 0;
  z-index: 100;
}

.back-button {
  display: inline-flex;
  align-items: center;
  gap: var(--space-2, 8px);
  background: none;
  border: none;
  color: var(--color-primary, #0071E3);
  font-size: var(--font-size-base, 16px);
  cursor: pointer;
  padding: var(--space-2, 8px) 0;
  margin-bottom: var(--space-3, 12px);
  transition: opacity var(--transition-smooth, 0.2s ease);
}

.back-button:hover {
  opacity: 0.7;
}

.back-icon {
  font-size: 20px;
}

.page-title {
  font-size: var(--font-size-2xl, 28px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin: 0;
}

/* 主容器 */
.detail-container {
  max-width: 900px;
  margin: 0 auto;
  padding: var(--space-6, 24px) var(--space-4, 16px);
  display: flex;
  flex-direction: column;
  gap: var(--space-5, 20px);
}

/* 状态卡片 */
.status-card {
  padding: var(--space-6, 24px);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-5, 20px);
}

.card-title {
  font-size: var(--font-size-xl, 24px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin: 0;
}

.status-badge {
  padding: var(--space-2, 8px) var(--space-4, 16px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
}

.status-pending {
  background: #FFF4E6;
  color: #E67E00;
}

.status-in_progress {
  background: #E6F4FF;
  color: #0071E3;
}

.status-completed {
  background: #E6F9F0;
  color: #00A86B;
}

.status-cancelled {
  background: #FEE6E6;
  color: #D32F2F;
}

.task-meta {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: var(--space-4, 16px);
}

.meta-item {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
}

.meta-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #6E6E73);
}

.meta-value {
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 500;
}

/* 信息卡片 */
.info-card {
  padding: var(--space-6, 24px);
}

.section-title {
  font-size: var(--font-size-lg, 20px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin: 0 0 var(--space-5, 20px) 0;
}

.info-grid {
  display: grid;
  grid-template-columns: 1fr auto 1fr;
  gap: var(--space-4, 16px);
  margin-bottom: var(--space-6, 24px);
}

.info-item {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
}

.info-icon {
  font-size: 32px;
  width: 48px;
  height: 48px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-md, 12px);
}

.info-content {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
}

.info-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #6E6E73);
}

.info-value {
  font-size: var(--font-size-lg, 18px);
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 600;
}

.info-divider {
  width: 1px;
  background: var(--color-border, #E5E5E7);
}

.info-detail {
  padding-top: var(--space-5, 20px);
  border-top: 1px solid var(--color-border, #E5E5E7);
  display: flex;
  flex-direction: column;
  gap: var(--space-4, 16px);
}

.detail-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.detail-label {
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #6E6E73);
}

.detail-value {
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 500;
}

/* 时间线卡片 */
.timeline-card {
  padding: var(--space-6, 24px);
}

.timeline {
  display: flex;
  flex-direction: column;
  gap: var(--space-5, 20px);
}

.timeline-item {
  display: flex;
  gap: var(--space-4, 16px);
  position: relative;
}

.timeline-item:not(:last-child)::before {
  content: '';
  position: absolute;
  left: 11px;
  top: 32px;
  bottom: -20px;
  width: 2px;
  background: var(--color-border, #E5E5E7);
}

.timeline-dot {
  width: 24px;
  height: 24px;
  border-radius: 50%;
  border: 2px solid var(--color-border, #E5E5E7);
  background: var(--color-bg-primary, #FFFFFF);
  flex-shrink: 0;
  margin-top: 4px;
  position: relative;
  z-index: 1;
}

.timeline-dot.active {
  border-color: var(--color-primary, #0071E3);
  background: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.timeline-content {
  flex: 1;
  padding-top: 2px;
}

.timeline-title {
  font-size: var(--font-size-base, 16px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  margin: 0 0 var(--space-1, 4px) 0;
}

.timeline-time {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #6E6E73);
  margin: 0 0 var(--space-2, 8px) 0;
}

.timeline-desc {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-tertiary, #86868B);
  margin: 0;
}

/* 操作按钮 */
.action-buttons {
  display: flex;
  gap: var(--space-3, 12px);
  margin-top: var(--space-4, 16px);
}

.action-buttons > * {
  flex: 1;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .page-title {
    font-size: var(--font-size-xl, 24px);
  }

  .detail-container {
    padding: var(--space-4, 16px);
  }

  .status-card,
  .info-card,
  .timeline-card {
    padding: var(--space-4, 16px);
  }

  .card-header {
    flex-direction: column;
    align-items: flex-start;
    gap: var(--space-3, 12px);
  }

  .task-meta {
    grid-template-columns: 1fr;
  }

  .info-grid {
    grid-template-columns: 1fr;
  }

  .info-divider {
    display: none;
  }

  .action-buttons {
    flex-direction: column;
  }
}

@media (max-width: 480px) {
  .detail-header {
    padding: var(--space-3, 12px) var(--space-4, 16px);
  }

  .page-title {
    font-size: var(--font-size-lg, 20px);
  }

  .card-title {
    font-size: var(--font-size-lg, 20px);
  }

  .section-title {
    font-size: var(--font-size-base, 16px);
  }
}

/* 深色模式支持 */
@media (prefers-color-scheme: dark) {
  .flight-task-detail {
    background: #000000;
  }

  .detail-header {
    background: #1C1C1E;
    border-bottom-color: #38383A;
  }

  .page-title,
  .card-title,
  .section-title,
  .meta-value,
  .info-value,
  .detail-value,
  .timeline-title {
    color: #F5F5F7;
  }

  .meta-label,
  .info-label,
  .detail-label,
  .timeline-time {
    color: #98989D;
  }

  .timeline-desc {
    color: #636366;
  }

  .info-icon {
    background: #2C2C2E;
  }

  .info-divider {
    background: #38383A;
  }

  .info-detail {
    border-top-color: #38383A;
  }

  .timeline-dot {
    background: #1C1C1E;
    border-color: #38383A;
  }

  .timeline-item:not(:last-child)::before {
    background: #38383A;
  }
}
</style>
