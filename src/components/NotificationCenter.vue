<template>
  <div class="notification-center">
    <!-- 通知图标按钮 -->
    <el-button
      type="text"
      class="notification-btn"
      :class="{ 'has-unread': unreadCount > 0 }"
      @click="toggleNotifications"
    >
      <el-icon><Bell /></el-icon>
      <span
        v-if="unreadCount > 0"
        class="notification-badge"
      >{{ unreadCount }}</span>
    </el-button>

    <!-- 通知弹出面板 -->
    <el-popover
      ref="popoverRef"
      v-model:visible="isVisible"
      placement="bottom-end"
      width="360"
      trigger="click"
      :popper-class="'notification-popover'"
    >
      <template #header>
        <div class="notification-header">
          <h3>通知中心</h3>
          <el-button
            type="text"
            size="small"
            class="clear-all-btn"
            @click="clearAll"
          >
            清空全部
          </el-button>
        </div>
      </template>

      <div
        v-if="notifications.length > 0"
        class="notifications-list"
      >
        <div
          v-for="notification in notifications"
          :key="notification.id"
          class="notification-item"
          :class="{ 'read': notification.read }"
          @click="markAsRead(notification.id)"
        >
          <div class="notification-content">
            <div class="notification-icon">
              <el-icon :size="18">
                <component :is="getIconType(notification.type)" />
              </el-icon>
            </div>
            <div class="notification-text">
              <div class="notification-title">
                {{ notification.title }}
              </div>
              <div class="notification-message">
                {{ notification.message }}
              </div>
              <div class="notification-time">
                {{ formatTime(notification.timestamp) }}
              </div>
            </div>
          </div>
          <el-button
            type="text"
            size="small"
            class="mark-read-btn"
            @click.stop="markAsRead(notification.id)"
          >
            已读
          </el-button>
        </div>
      </div>

      <div
        v-else
        class="no-notifications"
      >
        <el-icon><Info /></el-icon>
        <p>暂无通知</p>
      </div>

      <template #footer>
        <div class="notification-footer">
          <el-button
            type="primary"
            size="small"
            :disabled="unreadCount === 0"
            @click="markAllAsRead"
          >
            标记全部为已读
          </el-button>
        </div>
      </template>
    </el-popover>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useStore } from 'vuex'
import { Bell, Info } from '@element-plus/icons-vue'

const store = useStore()
const isVisible = ref(false)

// 计算未读通知数量
const unreadCount = computed(() => store.getters.unreadNotifications)

// 获取所有通知
const notifications = computed(() => store.state.notifications)

// 根据通知类型获取图标
const getIconType = (type) => {
  switch(type) {
    case 'success':
      return 'Check'
    case 'error':
      return 'Close'
    case 'warning':
      return 'Warning'
    case 'info':
      return 'Info'
    default:
      return 'Bell'
  }
}

// 切换通知面板
const toggleNotifications = () => {
  isVisible.value = !isVisible.value
}

// 标记单个通知为已读
const markAsRead = (notificationId) => {
  store.dispatch('markNotificationRead', notificationId)
}

// 标记所有通知为已读
const markAllAsRead = () => {
  notifications.value.forEach(n => {
    if (!n.read) {
      store.dispatch('markNotificationRead', n.id)
    }
  })
}

// 清空所有通知
const clearAll = () => {
  store.dispatch('clearNotifications')
  isVisible.value = false
}

// 格式化时间
const formatTime = (timestamp) => {
  const date = new Date(timestamp)
  const now = new Date()
  const diff = now - date
  
  if (diff < 60000) { // 1分钟内
    return '刚刚'
  } else if (diff < 3600000) { // 1小时内
    return `${Math.floor(diff / 60000)}分钟前`
  } else if (diff < 86400000) { // 24小时内
    return `${Math.floor(diff / 3600000)}小时前`
  } else {
    return date.toLocaleDateString() + ' ' + date.toLocaleTimeString()
  }
}

// 监听通知变化，自动关闭弹窗（当没有未读通知时）
onMounted(() => {
  store.watch(
    () => store.state.notifications,
    () => {
      if (unreadCount.value === 0 && isVisible.value) {
        isVisible.value = false
      }
    },
    { deep: true }
  )
})
</script>

<style scoped>
.notification-center {
  position: relative;
  display: inline-block;
}

.notification-btn {
  padding: 8px;
  border-radius: 50%;
  transition: all 0.3s ease;
  color: var(--text-primary);
}

.notification-btn:hover {
  background-color: rgba(0, 0, 0, 0.05);
  transform: scale(1.1);
}

.notification-btn.has-unread {
  color: var(--danger-color);
}

.notification-badge {
  position: absolute;
  top: -4px;
  right: -4px;
  background-color: var(--danger-color);
  color: white;
  border-radius: 50%;
  width: 16px;
  height: 16px;
  font-size: 10px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: bold;
}

.notification-popover {
  border-radius: 12px;
  box-shadow: 0 12px 40px rgba(0, 0, 0, 0.15);
  border: none;
  overflow: hidden;
}

.notification-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 16px;
  border-bottom: 1px solid var(--border-color);
}

.notification-header h3 {
  margin: 0;
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
}

.clear-all-btn {
  color: var(--primary-color);
  padding: 0;
  font-size: 13px;
}

.notifications-list {
  max-height: 300px;
  overflow-y: auto;
  padding: 0;
}

.notification-item {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  padding: 12px 16px;
  border-bottom: 1px solid var(--border-color);
  cursor: pointer;
  transition: background-color 0.2s ease;
}

.notification-item:hover {
  background-color: var(--bg-secondary);
}

.notification-item.read {
  opacity: 0.7;
}

.notification-content {
  display: flex;
  align-items: flex-start;
  flex: 1;
  gap: 12px;
}

.notification-icon {
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: var(--primary-color);
  color: white;
  border-radius: 50%;
  flex-shrink: 0;
}

.notification-text {
  flex: 1;
  min-width: 0;
}

.notification-title {
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 4px;
  font-size: 14px;
  line-height: 1.4;
}

.notification-message {
  color: var(--text-secondary);
  font-size: 13px;
  line-height: 1.4;
  margin-bottom: 4px;
  word-wrap: break-word;
  overflow-wrap: break-word;
}

.notification-time {
  color: var(--text-tertiary);
  font-size: 12px;
  margin-top: 4px;
}

.mark-read-btn {
  color: var(--primary-color);
  padding: 0;
  font-size: 12px;
  margin-left: 12px;
}

.no-notifications {
  text-align: center;
  padding: 40px 16px;
  color: var(--text-tertiary);
}

.no-notifications i {
  font-size: 32px;
  margin-bottom: 12px;
  color: var(--border-color);
}

.no-notifications p {
  margin: 0;
  font-size: 14px;
}

.notification-footer {
  padding: 12px 16px;
  border-top: 1px solid var(--border-color);
  display: flex;
  justify-content: flex-end;
}

.notification-footer button {
  margin-left: 8px;
}
</style>
