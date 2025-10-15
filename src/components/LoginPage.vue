<template>
  <!-- 使用公共的 AuthLayout 组件 -->
  <AuthLayout>
    <!-- 自定义标题 -->
    <template #title>
      <h1 class="auth-title">
        智能低空交通系统
      </h1>
    </template>

    <!-- 登录表单 -->
    <template #form>
      <form
        class="login-form"
        @submit.prevent="handleLogin"
      >
        <AppleInput
          v-model="loginData.username"
          type="text"
          placeholder="用户名"
          required
          class="form-field"
        />

        <AppleInput
          v-model="loginData.password"
          type="password"
          placeholder="密码"
          required
          class="form-field"
        />

        <div class="form-field">
          <select
            v-model="loginData.role"
            class="form-select"
          >
            <option value="admin">
              管理员
            </option>
            <option value="user">
              普通用户
            </option>
          </select>
        </div>

        <AppleButton
          type="submit"
          variant="primary"
          size="large"
          :loading="loading"
          class="login-button"
        >
          {{ loading ? '登录中...' : '登录' }}
        </AppleButton>
      </form>
    </template>

    <!-- 测试账号信息 -->
    <template #extra>
      <div class="test-accounts">
        <p class="test-title">
          测试账号
        </p>
        <div class="test-account-list">
          <div class="test-account">
            <strong>管理员:</strong> admin / admin123
          </div>
          <div class="test-account">
            <strong>普通用户:</strong> testuser / test1234
          </div>
        </div>
      </div>
    </template>

    <!-- 底部注册链接 -->
    <template #bottomLink>
      <span>还没有账号？</span>
      <button
        type="button"
        class="register-link-button"
        @click="$router.push('/register')"
      >
        立即注册
      </button>
    </template>

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
  </AuthLayout>
</template>

<script setup>
import { ref, reactive, computed } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { useStore } from 'vuex'
import authApi from '@/services/authApi'
import AuthLayout from './AuthLayout.vue'
import { AppleButton, AppleInput } from '@/components/apple'

const router = useRouter()
const route = useRoute()
const store = useStore()

const loading = ref(false)
// const loginForm = ref(null)

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

const loginData = reactive({
  username: '',
  password: '',
  role: 'admin'  // 默认为 admin，后端要求必填
})

// 角色选择现在是必填的

// const rules = {
//   username: [
//     { required: true, message: '请输入用户名', trigger: 'blur' },
//     { min: 3, max: 20, message: '用户名长度在3到20个字符', trigger: 'blur' }
//   ],
//   password: [
//     { required: true, message: '请输入密码', trigger: 'blur' },
//     { min: 6, message: '密码长度不能少于6位', trigger: 'blur' }
//   ],
//   role: [
//     { required: true, message: '请选择角色', trigger: 'change' }
//   ]
// }

const handleLogin = async () => {
  // Basic validation
  if (!loginData.username || !loginData.password) {
    showToastNotification('请填写用户名和密码', 'error')
    return
  }

  if (loginData.username.length < 3 || loginData.username.length > 20) {
    showToastNotification('用户名长度在3到20个字符', 'error')
    return
  }

  if (loginData.password.length < 6) {
    showToastNotification('密码长度不能少于6位', 'error')
    return
  }

  // role是可选的，如果提供了则验证有效性
  if (loginData.role && !['admin', 'user'].includes(loginData.role)) {
    showToastNotification('请选择有效的身份', 'error')
    return
  }

  try {
    loading.value = true

    // 所有用户（包括testuser）都通过真实后端API登录

    // 调用真实的后端API（包括admin账户）
    // 只在role不为空时才包含role字段，让后端自动识别
    const payload = {
      username: loginData.username,
      password: loginData.password
    }

    // 如果用户手动选择了角色，则包含在请求中
    if (loginData.role) {
      payload.role = loginData.role
    }

    const response = await authApi.login(payload)

    console.log('登录响应:', response)

    if (response && response.success) {
      // 保存用户信息到store
      const user = response.data.user
      await store.dispatch('setUser', user)

      showToastNotification('登录成功！', 'success')

      // 登录成功后重定向
      const redirect = route.query.redirect || '/dashboard'
      console.log('准备跳转到:', redirect)

      // 🔒 BUG #9: 使用replace代替push,避免后退时重定向循环
      // 使用setTimeout确保状态更新完成后再跳转
      setTimeout(() => {
        router.replace(redirect)
      }, 100)
    } else {
      showToastNotification(response?.message || '登录失败', 'error')
    }

  } catch (error) {
    console.error('登录错误:', error)

    // 特别处理身份错误
    if (error.error_code === 'ROLE_MISMATCH') {
      showToastNotification('身份错误', 'error')
      return
    }

    // 检查是否是成功登录但被异常捕获的情况
    if (error.success !== false) {
      // 如果有错误消息但不是明确的失败，尝试显示错误
      const errorMessage = error.message || error.error_code || '登录过程中出现错误'
      showToastNotification(errorMessage, 'error')
    }
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
/* 使用 Apple 设计系统 */
.auth-title {
  font-size: var(--font-size-2xl);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0;
  letter-spacing: var(--letter-spacing-tight);
  line-height: var(--line-height-compact);
}

.login-form {
  margin-bottom: var(--space-6);
}

.form-field {
  margin-bottom: var(--space-4);
}

/* 下拉选择框样式（使用设计系统） */
.form-select {
  width: 100%;
  height: 44px;
  padding: 0 var(--space-4);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-base);
  font-weight: 400;
  color: var(--color-text-primary);
  background: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  outline: none;
  transition: var(--transition-input);
  -webkit-appearance: none;
  box-sizing: border-box;
  background-image: url("data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' fill='none' viewBox='0 0 20 20'%3e%3cpath stroke='%2386868b' stroke-linecap='round' stroke-linejoin='round' stroke-width='1.5' d='m6 8 4 4 4-4'/%3e%3c/svg%3e");
  background-position: right 12px center;
  background-repeat: no-repeat;
  background-size: 16px;
  padding-right: 40px;
}

.form-select:hover {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.form-select:focus {
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
  background-color: var(--color-bg-primary);
}

.form-select option {
  color: var(--color-text-primary);
  background: var(--color-bg-primary);
  padding: var(--space-2);
}

.login-button {
  width: 100%;
}

/* 测试账号样式 */
.test-accounts {
  background: rgba(0, 113, 227, 0.05);
  border: 1px solid rgba(0, 113, 227, 0.1);
  border-radius: 8px;
  padding: 16px;
  margin-bottom: 24px;
}

.test-title {
  font-size: 14px;
  font-weight: 600;
  color: #0071e3;
  margin: 0 0 12px 0;
  text-align: center;
}

.test-account-list {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.test-account {
  font-size: 12px;
  color: #1d1d1f;
  text-align: center;
  line-height: 1.4;
}

.test-account strong {
  color: #0071e3;
}

.register-link-button {
  background: none;
  border: none;
  color: #0071e3;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  margin-left: 4px;
  transition: color 0.2s ease;
  outline: none;
  text-decoration: none;
}

.register-link-button:hover {
  color: #0056b3;
  text-decoration: underline;
}

/* Responsive design */
@media (max-width: 480px) {
  .form-input,
  .form-select,
  .login-button {
    height: 48px;
    font-size: 16px; /* Prevent zoom on iOS */
  }

  .auth-title {
    font-size: 22px;
  }
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .auth-title {
    color: #f2f2f7;
  }

  .form-input,
  .form-select {
    background: #2c2c2e;
    border: 1px solid #48484a;
    color: #f2f2f7;
  }

  .form-input::placeholder {
    color: #98989d;
  }

  .form-input:focus,
  .form-select:focus {
    border-color: #0a84ff;
    box-shadow:
      0 0 0 3px rgba(10, 132, 255, 0.1),
      0 2px 8px rgba(0, 0, 0, 0.2);
  }

  .login-button {
    background: #0a84ff;
  }

  .login-button:hover:not(:disabled) {
    background: #0056cc;
  }

  .register-link-button {
    color: #0a84ff;
  }

  .register-link-button:hover {
    color: #0056cc;
  }

  .test-accounts {
    background: rgba(10, 132, 255, 0.1);
    border: 1px solid rgba(10, 132, 255, 0.2);
  }

  .test-title {
    color: #0a84ff;
  }

  .test-account {
    color: #f2f2f7;
  }

  .test-account strong {
    color: #0a84ff;
  }

  .form-select option {
    color: #f2f2f7;
    background: #2c2c2e;
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
