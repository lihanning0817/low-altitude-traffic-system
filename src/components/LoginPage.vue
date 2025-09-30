<template>
  <!-- 使用公共的 AuthLayout 组件 -->
  <AuthLayout>
    <!-- 自定义标题 -->
    <template #title>
      <h1 class="auth-title">智能低空交通系统</h1>
    </template>

    <!-- 登录表单 -->
    <template #form>
      <form class="login-form" @submit.prevent="handleLogin">
        <div class="form-group">
          <input
            v-model="loginData.username"
            type="text"
            placeholder="用户名"
            class="form-input"
            required
          />
        </div>

        <div class="form-group">
          <input
            v-model="loginData.password"
            type="password"
            placeholder="密码"
            class="form-input"
            required
          />
        </div>

        <div class="form-group">
          <select
            v-model="loginData.role"
            class="form-select"
            required
          >
            <option value="">请选择身份</option>
            <option value="admin">管理员</option>
            <option value="user">普通用户</option>
          </select>
        </div>

        <button
          type="submit"
          class="login-button"
          :disabled="loading"
        >
          <div class="button-content">
            <!-- Loading 图标 -->
            <div v-if="loading" class="loading-spinner">
              <svg
                class="spinner-icon"
                viewBox="0 0 24 24"
                fill="none"
                xmlns="http://www.w3.org/2000/svg"
              >
                <circle
                  cx="12"
                  cy="12"
                  r="10"
                  stroke="currentColor"
                  stroke-width="2"
                  stroke-linecap="round"
                  stroke-dasharray="31.416"
                  stroke-dashoffset="31.416"
                />
              </svg>
            </div>
            <!-- 按钮文字 -->
            <span class="button-text">{{ loading ? '登录中...' : '登录' }}</span>
          </div>
        </button>
      </form>
    </template>

    <!-- 测试账号信息 -->
    <template #extra>
      <div class="test-accounts">
        <p class="test-title">测试账号</p>
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
  </AuthLayout>
</template>

<script setup>
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import { useRouter, useRoute } from 'vue-router'
import { useStore } from 'vuex'
import authApi from '@/services/authApi'
import AuthLayout from './AuthLayout.vue'

const router = useRouter()
const route = useRoute()
const store = useStore()

const loading = ref(false)
const loginForm = ref(null)

const loginData = reactive({
  username: '',
  password: '',
  role: ''
})

// 角色选择现在是必填的

const rules = {
  username: [
    { required: true, message: '请输入用户名', trigger: 'blur' },
    { min: 3, max: 20, message: '用户名长度在3到20个字符', trigger: 'blur' }
  ],
  password: [
    { required: true, message: '请输入密码', trigger: 'blur' },
    { min: 6, message: '密码长度不能少于6位', trigger: 'blur' }
  ],
  role: [
    { required: true, message: '请选择角色', trigger: 'change' }
  ]
}

const handleLogin = async () => {
  // Basic validation
  if (!loginData.username || !loginData.password || !loginData.role) {
    ElMessage.error('请填写用户名、密码和身份')
    return
  }

  if (loginData.username.length < 3 || loginData.username.length > 20) {
    ElMessage.error('用户名长度在3到20个字符')
    return
  }

  if (loginData.password.length < 6) {
    ElMessage.error('密码长度不能少于6位')
    return
  }

  if (!['admin', 'user'].includes(loginData.role)) {
    ElMessage.error('请选择有效的身份')
    return
  }

  try {
    loading.value = true

    // 所有用户（包括testuser）都通过真实后端API登录

    // 调用真实的后端API（包括admin账户）
    const response = await authApi.login({
      username: loginData.username,
      password: loginData.password,
      role: loginData.role
    })

    console.log('登录响应:', response)

    if (response && response.success) {
      // 保存用户信息到store
      const user = response.data.user
      await store.dispatch('setUser', user)

      ElMessage.success('登录成功！')

      // 登录成功后重定向
      const redirect = route.query.redirect || '/dashboard'
      console.log('准备跳转到:', redirect)

      // 使用setTimeout确保状态更新完成后再跳转
      setTimeout(() => {
        router.push(redirect)
      }, 100)
    } else {
      ElMessage.error(response?.message || '登录失败')
    }

  } catch (error) {
    console.error('登录错误:', error)

    // 特别处理身份错误
    if (error.error_code === 'ROLE_MISMATCH') {
      ElMessage.error('身份错误')
      return
    }

    // 检查是否是成功登录但被异常捕获的情况
    if (error.success !== false) {
      // 如果有错误消息但不是明确的失败，尝试显示错误
      const errorMessage = error.message || error.error_code || '登录过程中出现错误'
      ElMessage.error(errorMessage)
    }
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
/* Apple-style form styling - 与注册页面完全一致 */
.auth-title {
  font-size: 24px;
  font-weight: 600;
  color: #1d1d1f;
  margin: 0;
  letter-spacing: -0.5px;
  line-height: 1.2;
}

.login-form {
  margin-bottom: 24px;
}

.form-group {
  margin-bottom: 16px;
}

.form-input,
.form-select {
  width: 100%;
  height: 44px;
  padding: 0 16px;
  font-size: 16px;
  font-weight: 400;
  color: #1d1d1f;
  background: #ffffff;
  border: 1px solid #d2d2d7;
  border-radius: 8px;
  outline: none;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  -webkit-appearance: none;
  box-sizing: border-box;
}

.form-input::placeholder {
  color: #86868b;
  font-weight: 400;
}

.form-input:focus,
.form-select:focus {
  border-color: #0071e3;
  box-shadow:
    0 0 0 3px rgba(0, 113, 227, 0.1),
    0 2px 8px rgba(0, 0, 0, 0.04);
  transform: translateY(-1px);
}

.form-select {
  background-image: url("data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' fill='none' viewBox='0 0 20 20'%3e%3cpath stroke='%2386868b' stroke-linecap='round' stroke-linejoin='round' stroke-width='1.5' d='m6 8 4 4 4-4'/%3e%3c/svg%3e");
  background-position: right 12px center;
  background-repeat: no-repeat;
  background-size: 16px;
  padding-right: 40px;
}

.form-select option {
  color: #1d1d1f;
  background: #ffffff;
  padding: 8px;
}

.login-button {
  width: 100%;
  height: 44px;
  background: #0071e3;
  border: none;
  border-radius: 8px;
  color: white;
  font-size: 16px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  outline: none;
  letter-spacing: -0.1px;
  display: flex;
  align-items: center;
  justify-content: center;
}

/* 按钮内容容器 */
.button-content {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  position: relative;
}

/* Loading 动画容器 */
.loading-spinner {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 16px;
  height: 16px;
}

/* Loading 图标 */
.spinner-icon {
  width: 100%;
  height: 100%;
  color: white;
  animation: spin 1s linear infinite;
}

/* 旋转动画 */
@keyframes spin {
  from {
    transform: rotate(0deg);
    stroke-dashoffset: 31.416;
  }
  50% {
    stroke-dashoffset: 15.708;
  }
  to {
    transform: rotate(360deg);
    stroke-dashoffset: 31.416;
  }
}

/* 按钮文字 */
.button-text {
  transition: opacity 0.2s ease;
}

.login-button:hover:not(:disabled) {
  background: #0056b3;
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(0, 113, 227, 0.2);
}

.login-button:active:not(:disabled) {
  transform: scale(0.98);
  transition: transform 0.1s;
}

.login-button:disabled {
  opacity: 0.6;
  cursor: not-allowed;
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
</style>
