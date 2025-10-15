<template>
  <div class="notification-center">
    <!-- 通知图标按钮 -->
    <button
      class="notification-btn"
      :class="{ 'has-unread': unreadCount > 0 }"
      @click="toggleNotifications"
      type="button"
    >
      <span class="notification-icon">🔔</span>
      <span
        v-if="unreadCount > 0"
        class="notification-badge"
      >{{ unreadCount > 99 ? '99+' : unreadCount }}</span>
    </button>

    <!-- 通知弹出面板 -->
    <Teleport to="body">
      <Transition name="notification-fade">
        <div
          v-if="isVisible"
          class="notification-overlay"
          @click="closeNotifications"
        >
          <div
            class="notification-panel"
            @click.stop
          >
            <!-- 头部 -->
            <div class="notification-header">
              <h3 class="notification-title">通知中心</h3>
              <button
                class="clear-all-btn"
                @click="clearAll"
                type="button"
              >
                清空全部
              </button>
            </div>

            <!-- 通知列表 -->
            <div class="notifications-container">
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
                    <div class="notification-icon-wrapper">
                      <span
                        class="notification-type-icon"
                        :class="`type-${notification.type}`"
                      >
                        {{ getIconEmoji(notification.type) }}
                      </span>
                    </div>
                    <div class="notification-text">
                      <div class="notification-item-title">
                        {{ notification.title }}
                      </div>
                      <div class="notification-message">
                        {{ notification.message }}
                      </div>
                      <div class="notification-time">
                        {{ formatTime(notification.timestamp) }}
                      </div>
                    </div>
                    <button
                      v-if="!notification.read"
                      class="mark-read-btn"
                      @click.stop="markAsRead(notification.id)"
                      type="button"
                    >
                      <span class="read-indicator"></span>
                    </button>
                  </div>
                </div>
              </div>

              <div
                v-else
                class="no-notifications"
              >
                <span class="no-notifications-icon">📭</span>
                <p class="no-notifications-text">暂无通知</p>
              </div>
            </div>

            <!-- 底部操作 -->
            <div class="notification-footer">
              <AppleButton
                variant="primary"
                size="small"
                :disabled="unreadCount === 0"
                @click="markAllAsRead"
                style="width: 100%;"
              >
                标记全部为已读
              </AppleButton>
            </div>
          </div>
        </div>
      </Transition>
    </Teleport>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { useStore } from 'vuex'
import AppleButton from './apple/AppleButton.vue'

const store = useStore()
const isVisible = ref(false)

// 计算未读通知数量
const unreadCount = computed(() => store.getters.unreadNotifications)

// 获取所有通知
const notifications = computed(() => store.state.notifications)

// 根据通知类型获取 Emoji 图标
const getIconEmoji = (type) => {
  switch(type) {
    case 'success':
      return '✅'
    case 'error':
      return '❌'
    case 'warning':
      return '⚠️'
    case 'info':
      return 'ℹ️'
    default:
      return '🔔'
  }
}

// 切换通知面板
const toggleNotifications = () => {
  isVisible.value = !isVisible.value
}

// 关闭通知面板
const closeNotifications = () => {
  isVisible.value = false
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
  } else if (diff < 604800000) { // 7天内
    return `${Math.floor(diff / 86400000)}天前`
  } else {
    const year = date.getFullYear()
    const month = String(date.getMonth() + 1).padStart(2, '0')
    const day = String(date.getDate()).padStart(2, '0')
    const hours = String(date.getHours()).padStart(2, '0')
    const minutes = String(date.getMinutes()).padStart(2, '0')
    return `${year}-${month}-${day} ${hours}:${minutes}`
  }
}

// 监听 ESC 键关闭面板
const handleKeydown = (e) => {
  if (e.key === 'Escape' && isVisible.value) {
    isVisible.value = false
  }
}

onMounted(() => {
  window.addEventListener('keydown', handleKeydown)

  // 监听通知变化
  store.watch(
    () => store.state.notifications,
    (newNotifications) => {
      // 如果所有通知都已读,可以选择自动关闭面板
      if (unreadCount.value === 0 && isVisible.value && newNotifications.length === 0) {
        isVisible.value = false
      }
    },
    { deep: true }
  )
})

onUnmounted(() => {
  window.removeEventListener('keydown', handleKeydown)
})
</script>

<style scoped>
.notification-center {
  position: relative;
  display: inline-block;
}

/* 通知按钮 */
.notification-btn {
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  width: 40px;
  height: 40px;
  padding: 0;
  background: transparent;
  border: none;
  border-radius: var(--radius-full, 9999px);
  cursor: pointer;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.notification-btn:hover {
  background: rgba(0, 0, 0, 0.04);
  transform: scale(1.05);
}

.notification-btn:active {
  transform: scale(0.95);
}

.notification-icon {
  font-size: 20px;
  line-height: 1;
  filter: grayscale(0.3);
}

.notification-btn.has-unread .notification-icon {
  filter: grayscale(0);
  animation: bell-ring 0.5s ease-in-out;
}

@keyframes bell-ring {
  0%, 100% { transform: rotate(0deg); }
  10%, 30%, 50%, 70%, 90% { transform: rotate(-10deg); }
  20%, 40%, 60%, 80% { transform: rotate(10deg); }
}

.notification-badge {
  position: absolute;
  top: 4px;
  right: 4px;
  background: linear-gradient(135deg, #FF3B30 0%, #FF453A 100%);
  color: #FFFFFF;
  border-radius: var(--radius-full, 9999px);
  min-width: 18px;
  height: 18px;
  padding: 0 5px;
  font-size: 11px;
  font-weight: 600;
  display: flex;
  align-items: center;
  justify-content: center;
  line-height: 1;
  box-shadow: 0 2px 4px rgba(255, 59, 48, 0.3);
}

/* 遮罩层 */
.notification-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(8px);
  -webkit-backdrop-filter: blur(8px);
  z-index: 9999;
  display: flex;
  align-items: flex-start;
  justify-content: flex-end;
  padding: var(--space-6, 24px);
}

/* 通知面板 */
.notification-panel {
  width: 420px;
  max-width: 90vw;
  max-height: 80vh;
  background: #FFFFFF;
  border-radius: var(--radius-2xl, 20px);
  box-shadow:
    0 20px 60px rgba(0, 0, 0, 0.15),
    0 0 0 0.5px rgba(0, 0, 0, 0.05);
  display: flex;
  flex-direction: column;
  overflow: hidden;
  margin-top: var(--space-16, 64px);
}

/* 头部 */
.notification-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: var(--space-5, 20px) var(--space-6, 24px);
  border-bottom: 1px solid rgba(0, 0, 0, 0.06);
  background: #FAFAFA;
}

.notification-title {
  margin: 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text, #1D1D1F);
  letter-spacing: -0.02em;
}

.clear-all-btn {
  padding: var(--space-2, 8px) var(--space-3, 12px);
  background: transparent;
  border: none;
  border-radius: var(--radius-lg, 12px);
  color: #0071E3;
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.clear-all-btn:hover {
  background: rgba(0, 113, 227, 0.08);
}

.clear-all-btn:active {
  transform: scale(0.96);
}

/* 通知列表容器 */
.notifications-container {
  flex: 1;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

.notifications-list {
  flex: 1;
  overflow-y: auto;
  overscroll-behavior: contain;
}

/* 自定义滚动条 */
.notifications-list::-webkit-scrollbar {
  width: 6px;
}

.notifications-list::-webkit-scrollbar-track {
  background: transparent;
}

.notifications-list::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.15);
  border-radius: var(--radius-full, 9999px);
}

.notifications-list::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.25);
}

/* 通知项 */
.notification-item {
  position: relative;
  padding: var(--space-4, 16px) var(--space-6, 24px);
  border-bottom: 1px solid rgba(0, 0, 0, 0.06);
  cursor: pointer;
  transition: background-color 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.notification-item:hover {
  background: #F9F9F9;
}

.notification-item:active {
  background: #F5F5F7;
}

.notification-item.read {
  opacity: 0.6;
}

.notification-item.read:hover {
  opacity: 0.7;
}

.notification-content {
  display: flex;
  align-items: flex-start;
  gap: var(--space-3, 12px);
  position: relative;
}

/* 通知图标 */
.notification-icon-wrapper {
  flex-shrink: 0;
}

.notification-type-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 36px;
  height: 36px;
  border-radius: var(--radius-full, 9999px);
  font-size: 18px;
  line-height: 1;
}

.notification-type-icon.type-success {
  background: rgba(52, 199, 89, 0.1);
}

.notification-type-icon.type-error {
  background: rgba(255, 59, 48, 0.1);
}

.notification-type-icon.type-warning {
  background: rgba(255, 149, 0, 0.1);
}

.notification-type-icon.type-info {
  background: rgba(0, 113, 227, 0.1);
}

/* 通知文本 */
.notification-text {
  flex: 1;
  min-width: 0;
  padding-right: var(--space-6, 24px);
}

.notification-item-title {
  font-weight: 600;
  color: var(--color-text, #1D1D1F);
  margin-bottom: var(--space-1, 4px);
  font-size: var(--font-size-base, 15px);
  line-height: 1.4;
  letter-spacing: -0.01em;
}

.notification-message {
  color: #6E6E73;
  font-size: var(--font-size-sm, 14px);
  line-height: 1.5;
  margin-bottom: var(--space-1, 4px);
  word-wrap: break-word;
  overflow-wrap: break-word;
}

.notification-time {
  color: #86868B;
  font-size: 12px;
  margin-top: var(--space-1, 4px);
}

/* 标记已读按钮 */
.mark-read-btn {
  position: absolute;
  top: var(--space-4, 16px);
  right: var(--space-6, 24px);
  padding: 0;
  background: transparent;
  border: none;
  cursor: pointer;
  transition: transform 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.mark-read-btn:hover {
  transform: scale(1.1);
}

.mark-read-btn:active {
  transform: scale(0.9);
}

.read-indicator {
  display: block;
  width: 8px;
  height: 8px;
  background: #0071E3;
  border-radius: var(--radius-full, 9999px);
  box-shadow: 0 0 0 2px rgba(0, 113, 227, 0.2);
}

/* 无通知状态 */
.no-notifications {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: var(--space-16, 64px) var(--space-6, 24px);
  text-align: center;
  min-height: 240px;
}

.no-notifications-icon {
  font-size: 64px;
  line-height: 1;
  margin-bottom: var(--space-4, 16px);
  opacity: 0.4;
}

.no-notifications-text {
  margin: 0;
  color: #86868B;
  font-size: var(--font-size-base, 15px);
  font-weight: 500;
}

/* 底部操作 */
.notification-footer {
  padding: var(--space-4, 16px) var(--space-6, 24px);
  border-top: 1px solid rgba(0, 0, 0, 0.06);
  background: #FAFAFA;
}

/* 过渡动画 */
.notification-fade-enter-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.notification-fade-leave-active {
  transition: all 0.2s cubic-bezier(0.4, 0, 1, 1);
}

.notification-fade-enter-from,
.notification-fade-leave-to {
  opacity: 0;
}

.notification-fade-enter-from .notification-panel {
  transform: translateY(-20px) scale(0.95);
  opacity: 0;
}

.notification-fade-leave-to .notification-panel {
  transform: translateY(-10px) scale(0.98);
  opacity: 0;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .notification-overlay {
    padding: 0;
    align-items: stretch;
  }

  .notification-panel {
    width: 100%;
    max-width: 100%;
    max-height: 100vh;
    margin-top: 0;
    border-radius: 0;
  }

  .notification-header {
    padding: var(--space-4, 16px);
  }

  .notification-item {
    padding: var(--space-3, 12px) var(--space-4, 16px);
  }

  .notification-footer {
    padding: var(--space-3, 12px) var(--space-4, 16px);
  }
}

/* 暗色模式 */
@media (prefers-color-scheme: dark) {
  .notification-btn:hover {
    background: rgba(255, 255, 255, 0.08);
  }

  .notification-panel {
    background: #1C1C1E;
    box-shadow:
      0 20px 60px rgba(0, 0, 0, 0.5),
      0 0 0 0.5px rgba(255, 255, 255, 0.1);
  }

  .notification-header {
    background: #2C2C2E;
    border-bottom-color: rgba(255, 255, 255, 0.1);
  }

  .notification-title {
    color: #F5F5F7;
  }

  .clear-all-btn {
    color: #0A84FF;
  }

  .clear-all-btn:hover {
    background: rgba(10, 132, 255, 0.15);
  }

  .notification-item {
    border-bottom-color: rgba(255, 255, 255, 0.1);
  }

  .notification-item:hover {
    background: #2C2C2E;
  }

  .notification-item:active {
    background: #3A3A3C;
  }

  .notification-item-title {
    color: #F5F5F7;
  }

  .notification-message {
    color: #98989D;
  }

  .notification-time {
    color: #8E8E93;
  }

  .notification-type-icon.type-success {
    background: rgba(52, 199, 89, 0.2);
  }

  .notification-type-icon.type-error {
    background: rgba(255, 69, 58, 0.2);
  }

  .notification-type-icon.type-warning {
    background: rgba(255, 159, 10, 0.2);
  }

  .notification-type-icon.type-info {
    background: rgba(10, 132, 255, 0.2);
  }

  .read-indicator {
    background: #0A84FF;
    box-shadow: 0 0 0 2px rgba(10, 132, 255, 0.3);
  }

  .no-notifications-text {
    color: #8E8E93;
  }

  .notification-footer {
    background: #2C2C2E;
    border-top-color: rgba(255, 255, 255, 0.1);
  }

  .notifications-list::-webkit-scrollbar-thumb {
    background: rgba(255, 255, 255, 0.2);
  }

  .notifications-list::-webkit-scrollbar-thumb:hover {
    background: rgba(255, 255, 255, 0.3);
  }
}
</style>
