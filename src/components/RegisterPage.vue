<template>
  <!-- 使用公共的 AuthLayout 组件，统一白色留白风格 -->
  <AuthLayout>
    <!-- 自定义标题 -->
    <template #title>
      <h1 class="auth-title">创建您的智能低空交通系统账号</h1>
    </template>

    <!-- 注册表单 -->
    <template #form>
      <form class="register-form" @submit.prevent="handleRegister">
        <!-- 用户名输入框 -->
        <div class="form-group">
          <input
            v-model="registerData.username"
            type="text"
            placeholder="用户名"
            class="form-input"
            required
          />
        </div>

        <!-- 邮箱输入框 -->
        <div class="form-group">
          <input
            v-model="registerData.email"
            type="email"
            placeholder="邮箱地址"
            class="form-input"
            required
          />
        </div>

        <!-- 真实姓名输入框 -->
        <div class="form-group">
          <input
            v-model="registerData.full_name"
            type="text"
            placeholder="真实姓名"
            class="form-input"
            required
          />
        </div>

        <!-- 密码输入框 -->
        <div class="form-group">
          <input
            v-model="registerData.password"
            type="password"
            placeholder="密码"
            class="form-input"
            required
          />
        </div>

        <!-- 确认密码输入框 -->
        <div class="form-group">
          <input
            v-model="registerData.confirmPassword"
            type="password"
            placeholder="确认密码"
            class="form-input"
            required
          />
        </div>

        <!-- 角色选择下拉框 -->
        <div class="form-group">
          <select
            v-model="registerData.role"
            class="form-select"
          >
            <option value="">选择角色（可选）</option>
            <option value="admin">管理员</option>
            <option value="operator">操作员</option>
            <option value="viewer">观察员</option>
          </select>
        </div>

        <!-- 注册按钮 -->
        <button
          type="submit"
          class="register-button"
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
            <span class="button-text">{{ loading ? '注册中...' : '立即注册' }}</span>
          </div>
        </button>
      </form>
    </template>

    <!-- 底部登录链接 -->
    <template #bottomLink>
      <span>已有账号？</span>
      <button
        type="button"
        class="login-link-button"
        @click="$router.push('/login')"
      >
        立即登录
      </button>
    </template>
  </AuthLayout>
</template>

<script setup>
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import { useRouter } from 'vue-router'
import { useStore } from 'vuex'
import authApi from '@/services/authApi'
import AuthLayout from './AuthLayout.vue'

const router = useRouter()
const store = useStore()

const loading = ref(false)

// 注册表单数据
const registerData = reactive({
  username: '',
  email: '',
  password: '',
  confirmPassword: '',
  full_name: '',
  role: ''
})

// 注册处理函数 - 与登录页面保持一致的验证风格
const handleRegister = async () => {
  // 基础表单验证
  if (!registerData.username || !registerData.email || !registerData.password || !registerData.confirmPassword || !registerData.full_name) {
    ElMessage.error('请填写所有必填字段')
    return
  }

  if (registerData.username.length < 3 || registerData.username.length > 20) {
    ElMessage.error('用户名长度在3到20个字符')
    return
  }

  if (!/^[a-zA-Z0-9_]+$/.test(registerData.username)) {
    ElMessage.error('用户名只能包含字母、数字和下划线')
    return
  }

  if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(registerData.email)) {
    ElMessage.error('请输入正确的邮箱格式')
    return
  }

  if (registerData.full_name.length < 2 || registerData.full_name.length > 50) {
    ElMessage.error('姓名长度在2到50个字符')
    return
  }

  if (registerData.password.length < 6) {
    ElMessage.error('密码长度不能少于6位')
    return
  }

  if (!/^(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d@$!%*#?&]/.test(registerData.password)) {
    ElMessage.error('密码必须包含字母和数字')
    return
  }

  if (registerData.password !== registerData.confirmPassword) {
    ElMessage.error('两次输入的密码不一致')
    return
  }

  try {
    loading.value = true

    // 调用真实的注册API
    const response = await authApi.register({
      username: registerData.username,
      email: registerData.email,
      password: registerData.password,
      full_name: registerData.full_name,
      role: registerData.role || 'viewer' // 默认角色为观察员
    })

    if (response.success) {
      ElMessage.success('注册成功！即将跳转到登录页面')

      // 延迟跳转到登录页面
      setTimeout(() => {
        router.push('/login')
      }, 1500)
    } else {
      ElMessage.error(response.message || '注册失败')
    }

  } catch (error) {
    console.error('注册错误:', error)
    const errorMessage = error.message || error.error_code || '注册失败，请检查网络连接'
    ElMessage.error(errorMessage)
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
/* Apple-style form styling - 与登录页面保持完全一致 */
.auth-title {
  font-size: 24px;
  font-weight: 600;
  color: #1d1d1f;
  margin: 0;
  letter-spacing: -0.5px;
  line-height: 1.2;
}

.register-form {
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

.register-button {
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

.register-button:hover:not(:disabled) {
  background: #0056b3;
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(0, 113, 227, 0.2);
}

.register-button:active:not(:disabled) {
  transform: scale(0.98);
  transition: transform 0.1s;
}

.register-button:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.login-link-button {
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

.login-link-button:hover {
  color: #0056b3;
  text-decoration: underline;
}

/* Responsive design */
@media (max-width: 480px) {
  .form-input,
  .form-select,
  .register-button {
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

  .register-button {
    background: #0a84ff;
  }

  .register-button:hover:not(:disabled) {
    background: #0056cc;
  }

  .login-link-button {
    color: #0a84ff;
  }

  .login-link-button:hover {
    color: #0056cc;
  }

  .form-select option {
    color: #f2f2f7;
    background: #2c2c2e;
  }
}
</style>