<template>
  <div class="profile-settings">
    <div class="settings-container">
      <h2 class="settings-title">
        个人设置
      </h2>

      <!-- 用户信息卡片 -->
      <AppleCard class="user-info-card">
        <div class="card-header">
          <h3>
            <span class="section-icon">👤</span>
            基本信息
          </h3>
          <AppleButton
            variant="secondary"
            size="small"
            :loading="userLoading"
            @click="refreshUserInfo"
          >
            🔄 刷新
          </AppleButton>
        </div>

        <div
          v-if="user"
          class="user-info"
        >
          <div class="info-item">
            <label>用户名</label>
            <span class="info-value">{{ user.username }}</span>
          </div>
          <div class="info-item">
            <label>邮箱</label>
            <span class="info-value">{{ user.email }}</span>
          </div>
          <div class="info-item">
            <label>真实姓名</label>
            <span class="info-value">{{ user.full_name || '未设置' }}</span>
          </div>
          <div class="info-item">
            <label>角色</label>
            <span
              class="role-badge"
              :class="`role-${user.role}`"
            >
              {{ getRoleDisplayName(user.role) }}
            </span>
          </div>
          <div class="info-item">
            <label>状态</label>
            <span
              class="status-badge"
              :class="`status-${user.status}`"
            >
              <span class="status-dot" />
              {{ getStatusDisplayName(user.status) }}
            </span>
          </div>
          <div class="info-item">
            <label>注册时间</label>
            <span class="info-value">{{ formatDate(user.created_at) }}</span>
          </div>
          <div class="info-item">
            <label>最后更新</label>
            <span class="info-value">{{ formatDate(user.updated_at) }}</span>
          </div>
        </div>

        <div
          v-else
          class="no-user"
        >
          <span class="empty-icon">📭</span>
          <p>无法获取用户信息</p>
        </div>
      </AppleCard>

      <!-- 修改密码卡片 -->
      <AppleCard class="password-card">
        <div class="card-header">
          <h3>
            <span class="section-icon">🔒</span>
            修改密码
          </h3>
        </div>

        <form
          class="password-form"
          @submit.prevent="handleChangePassword"
        >
          <div class="form-group">
            <label class="form-label">当前密码</label>
            <AppleInput
              v-model="passwordData.old_password"
              type="password"
              placeholder="请输入当前密码"
              required
            />
          </div>

          <div class="form-group">
            <label class="form-label">新密码</label>
            <AppleInput
              v-model="passwordData.new_password"
              type="password"
              placeholder="至少8位，包含字母和数字"
              required
            />
            <p
              v-if="passwordError.new_password"
              class="form-error"
            >
              {{ passwordError.new_password }}
            </p>
          </div>

          <div class="form-group">
            <label class="form-label">确认新密码</label>
            <AppleInput
              v-model="passwordData.confirm_password"
              type="password"
              placeholder="请再次输入新密码"
              required
            />
            <p
              v-if="passwordError.confirm_password"
              class="form-error"
            >
              {{ passwordError.confirm_password }}
            </p>
          </div>

          <div class="form-actions">
            <AppleButton
              type="submit"
              variant="primary"
              :loading="passwordLoading"
            >
              修改密码
            </AppleButton>
            <AppleButton
              type="button"
              variant="secondary"
              @click="resetPasswordForm"
            >
              重置
            </AppleButton>
          </div>
        </form>
      </AppleCard>
    </div>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div
        v-if="showToast"
        :class="['toast-notification', toastType]"
      >
        <div class="toast-icon">
          {{ toastIcon }}
        </div>
        <div class="toast-message">
          {{ toastMessage }}
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { useStore } from 'vuex'
import AppleCard from './apple/AppleCard.vue'
import AppleButton from './apple/AppleButton.vue'
import AppleInput from './apple/AppleInput.vue'

const store = useStore()

// 获取当前用户信息
const user = computed(() => store.state.user)
const userLoading = ref(false)
const passwordLoading = ref(false)

// Toast 通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

const showToastNotification = (message, type = 'success') => {
  toastMessage.value = message
  toastType.value = type
  showToast.value = true

  if (toastTimer) {
    clearTimeout(toastTimer)
  }

  toastTimer = setTimeout(() => {
    showToast.value = false
  }, 3000)
}

// 密码修改表单数据
const passwordData = reactive({
  old_password: '',
  new_password: '',
  confirm_password: ''
})

// 密码错误信息
const passwordError = reactive({
  new_password: '',
  confirm_password: ''
})

// 验证密码
const validatePassword = () => {
  let isValid = true

  // 重置错误
  passwordError.new_password = ''
  passwordError.confirm_password = ''

  // 验证新密码
  if (passwordData.new_password.length < 8) {
    passwordError.new_password = '密码长度不能少于8位'
    isValid = false
  } else if (!/^(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d@$!%*#?&]/.test(passwordData.new_password)) {
    passwordError.new_password = '密码必须包含字母和数字'
    isValid = false
  }

  // 验证确认密码
  if (passwordData.confirm_password !== passwordData.new_password) {
    passwordError.confirm_password = '两次输入的密码不一致'
    isValid = false
  }

  return isValid
}

// 刷新用户信息
const refreshUserInfo = async () => {
  try {
    userLoading.value = true
    await store.dispatch('getCurrentUser')
    showToastNotification('用户信息已刷新', 'success')
  } catch (error) {
    console.error('刷新用户信息失败:', error)
    showToastNotification('刷新用户信息失败', 'error')
  } finally {
    userLoading.value = false
  }
}

// 修改密码
const handleChangePassword = async () => {
  if (!validatePassword()) {
    return
  }

  try {
    passwordLoading.value = true

    await store.dispatch('changePassword', {
      old_password: passwordData.old_password,
      new_password: passwordData.new_password
    })

    showToastNotification('密码修改成功', 'success')

    // 修改成功后重置表单
    resetPasswordForm()

  } catch (error) {
    console.error('修改密码失败:', error)
    showToastNotification(error.message || '修改密码失败', 'error')
  } finally {
    passwordLoading.value = false
  }
}

// 重置密码表单
const resetPasswordForm = () => {
  Object.assign(passwordData, {
    old_password: '',
    new_password: '',
    confirm_password: ''
  })
  Object.assign(passwordError, {
    new_password: '',
    confirm_password: ''
  })
}

// 获取角色显示名称
const getRoleDisplayName = (role) => {
  const roleNames = {
    'admin': '管理员',
    'operator': '操作员',
    'user': '普通用户',
    'viewer': '观察员'
  }
  return roleNames[role] || role
}

// 获取状态显示名称
const getStatusDisplayName = (status) => {
  const statusNames = {
    'active': '正常',
    'inactive': '未激活',
    'banned': '已封禁'
  }
  return statusNames[status] || status
}

// 格式化时间
const formatDate = (timestamp) => {
  if (!timestamp) return '未知'
  const date = new Date(timestamp * 1000)
  const year = date.getFullYear()
  const month = String(date.getMonth() + 1).padStart(2, '0')
  const day = String(date.getDate()).padStart(2, '0')
  const hours = String(date.getHours()).padStart(2, '0')
  const minutes = String(date.getMinutes()).padStart(2, '0')
  return `${year}-${month}-${day} ${hours}:${minutes}`
}

// 组件挂载时刷新用户信息
onMounted(() => {
  if (!user.value) {
    refreshUserInfo()
  }
})
</script>

<style scoped>
.profile-settings {
  padding: var(--space-8, 32px);
  background-color: #F5F5F7;
  min-height: 100vh;
}

.settings-container {
  max-width: 900px;
  margin: 0 auto;
}

.settings-title {
  font-size: var(--font-size-2xl, 28px);
  font-weight: 600;
  color: var(--color-text, #1D1D1F);
  margin-bottom: var(--space-8, 32px);
  letter-spacing: -0.02em;
}

.user-info-card,
.password-card {
  margin-bottom: var(--space-6, 24px);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-6, 24px);
  padding-bottom: var(--space-4, 16px);
  border-bottom: 1px solid rgba(0, 0, 0, 0.06);
}

.card-header h3 {
  margin: 0;
  font-size: var(--font-size-lg, 20px);
  font-weight: 600;
  color: var(--color-text, #1D1D1F);
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
}

.section-icon {
  font-size: 24px;
  line-height: 1;
}

/* 用户信息网格 */
.user-info {
  display: grid;
  gap: var(--space-4, 16px);
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
}

.info-item {
  display: flex;
  flex-direction: column;
  gap: var(--space-2, 8px);
  padding: var(--space-4, 16px);
  background: #FAFAFA;
  border-radius: var(--radius-lg, 12px);
  border: 1px solid rgba(0, 0, 0, 0.04);
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.info-item:hover {
  background: #F5F5F7;
  border-color: rgba(0, 0, 0, 0.08);
}

.info-item label {
  font-size: var(--font-size-sm, 13px);
  font-weight: 600;
  color: #6E6E73;
  text-transform: uppercase;
  letter-spacing: 0.05em;
}

.info-value {
  font-size: var(--font-size-base, 15px);
  color: var(--color-text, #1D1D1F);
  font-weight: 500;
}

/* 角色徽章 */
.role-badge {
  display: inline-flex;
  align-items: center;
  padding: var(--space-2, 8px) var(--space-3, 12px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 600;
  align-self: flex-start;
}

.role-badge.role-admin {
  background: rgba(255, 59, 48, 0.1);
  color: #FF3B30;
}

.role-badge.role-operator {
  background: rgba(255, 149, 0, 0.1);
  color: #FF9500;
}

.role-badge.role-user {
  background: rgba(52, 199, 89, 0.1);
  color: #34C759;
}

.role-badge.role-viewer {
  background: rgba(0, 113, 227, 0.1);
  color: #0071E3;
}

/* 状态徽章 */
.status-badge {
  display: inline-flex;
  align-items: center;
  gap: var(--space-2, 8px);
  padding: var(--space-2, 8px) var(--space-3, 12px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 600;
  align-self: flex-start;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: var(--radius-full, 9999px);
}

.status-badge.status-active {
  background: rgba(52, 199, 89, 0.1);
  color: #34C759;
}

.status-badge.status-active .status-dot {
  background: #34C759;
  animation: pulse 2s infinite;
}

.status-badge.status-inactive {
  background: rgba(255, 149, 0, 0.1);
  color: #FF9500;
}

.status-badge.status-inactive .status-dot {
  background: #FF9500;
}

.status-badge.status-banned {
  background: rgba(255, 59, 48, 0.1);
  color: #FF3B30;
}

.status-badge.status-banned .status-dot {
  background: #FF3B30;
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}

/* 无用户状态 */
.no-user {
  text-align: center;
  padding: var(--space-16, 64px) var(--space-6, 24px);
}

.empty-icon {
  font-size: 64px;
  line-height: 1;
  display: block;
  margin-bottom: var(--space-4, 16px);
  opacity: 0.4;
}

.no-user p {
  margin: 0;
  color: #86868B;
  font-size: var(--font-size-base, 15px);
}

/* 密码表单 */
.password-form {
  max-width: 600px;
}

.form-group {
  margin-bottom: var(--space-5, 20px);
}

.form-label {
  display: block;
  margin-bottom: var(--space-2, 8px);
  font-size: var(--font-size-base, 15px);
  font-weight: 600;
  color: var(--color-text, #1D1D1F);
}

.form-error {
  margin: var(--space-2, 8px) 0 0;
  font-size: var(--font-size-sm, 13px);
  color: #FF3B30;
  font-weight: 500;
}

.form-actions {
  display: flex;
  gap: var(--space-3, 12px);
  margin-top: var(--space-6, 24px);
}

/* 响应式设计 */
@media (max-width: 768px) {
  .profile-settings {
    padding: var(--space-4, 16px);
  }

  .settings-title {
    font-size: var(--font-size-xl, 24px);
  }

  .user-info {
    grid-template-columns: 1fr;
  }

  .card-header {
    flex-direction: column;
    align-items: flex-start;
    gap: var(--space-3, 12px);
  }

  .password-form {
    max-width: none;
  }

  .form-actions {
    flex-direction: column;
  }

  .form-actions button {
    width: 100%;
  }
}

/* 暗色模式 */
@media (prefers-color-scheme: dark) {
  .profile-settings {
    background-color: #000000;
  }

  .settings-title {
    color: #F5F5F7;
  }

  .card-header {
    border-bottom-color: rgba(255, 255, 255, 0.1);
  }

  .card-header h3 {
    color: #F5F5F7;
  }

  .info-item {
    background: #1C1C1E;
    border-color: rgba(255, 255, 255, 0.1);
  }

  .info-item:hover {
    background: #2C2C2E;
    border-color: rgba(255, 255, 255, 0.15);
  }

  .info-item label {
    color: #98989D;
  }

  .info-value {
    color: #F5F5F7;
  }

  .role-badge.role-admin {
    background: rgba(255, 69, 58, 0.2);
    color: #FF453A;
  }

  .role-badge.role-operator {
    background: rgba(255, 159, 10, 0.2);
    color: #FF9F0A;
  }

  .role-badge.role-user {
    background: rgba(48, 209, 88, 0.2);
    color: #30D158;
  }

  .role-badge.role-viewer {
    background: rgba(10, 132, 255, 0.2);
    color: #0A84FF;
  }

  .status-badge.status-active {
    background: rgba(48, 209, 88, 0.2);
    color: #30D158;
  }

  .status-badge.status-active .status-dot {
    background: #30D158;
  }

  .status-badge.status-inactive {
    background: rgba(255, 159, 10, 0.2);
    color: #FF9F0A;
  }

  .status-badge.status-inactive .status-dot {
    background: #FF9F0A;
  }

  .status-badge.status-banned {
    background: rgba(255, 69, 58, 0.2);
    color: #FF453A;
  }

  .status-badge.status-banned .status-dot {
    background: #FF453A;
  }

  .no-user p {
    color: #8E8E93;
  }

  .form-label {
    color: #F5F5F7;
  }

  .form-error {
    color: #FF453A;
  }
}

/* Toast 通知样式 */
.toast-notification {
  position: fixed;
  top: var(--space-8, 32px);
  right: var(--space-6, 24px);
  z-index: 10000;
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px) var(--space-5, 20px);
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-lg, 12px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  min-width: 280px;
  max-width: 400px;
}

.toast-notification.success {
  border-left: 4px solid #34C759;
}

.toast-notification.error {
  border-left: 4px solid #FF3B30;
}

.toast-notification.warning {
  border-left: 4px solid #FF9500;
}

.toast-notification.info {
  border-left: 4px solid #007AFF;
}

.toast-icon {
  font-size: 24px;
  line-height: 1;
  flex-shrink: 0;
}

.toast-message {
  font-size: var(--font-size-base, 15px);
  color: var(--color-text, #1D1D1F);
  font-weight: 500;
  flex: 1;
  line-height: 1.4;
}

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

@keyframes slideInRight {
  from {
    transform: translateX(400px);
    opacity: 0;
  }
  to {
    transform: translateX(0);
    opacity: 1;
  }
}

@keyframes slideOutRight {
  from {
    transform: translateX(0);
    opacity: 1;
  }
  to {
    transform: translateX(400px);
    opacity: 0;
  }
}

/* Toast 暗色模式 */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: #1C1C1E;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
  }

  .toast-message {
    color: #F5F5F7;
  }

  .toast-notification.success {
    border-left-color: #30D158;
  }

  .toast-notification.error {
    border-left-color: #FF453A;
  }

  .toast-notification.warning {
    border-left-color: #FF9F0A;
  }

  .toast-notification.info {
    border-left-color: #0A84FF;
  }
}
</style>
