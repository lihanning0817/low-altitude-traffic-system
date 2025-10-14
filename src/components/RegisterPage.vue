<template>
  <!-- 使用公共的 AuthLayout 组件 -->
  <AuthLayout>
    <!-- 自定义标题 -->
    <template #title>
      <h1 class="auth-title">
        创建您的智能低空交通系统账号
      </h1>
    </template>

    <!-- 注册表单 -->
    <template #form>
      <form
        class="register-form"
        @submit.prevent="handleRegister"
      >
        <AppleInput
          v-model="registerData.username"
          type="text"
          placeholder="用户名"
          required
          class="form-field"
        />

        <AppleInput
          v-model="registerData.email"
          type="email"
          placeholder="邮箱地址"
          required
          class="form-field"
        />

        <AppleInput
          v-model="registerData.full_name"
          type="text"
          placeholder="真实姓名"
          required
          class="form-field"
        />

        <AppleInput
          v-model="registerData.password"
          type="password"
          placeholder="密码"
          required
          class="form-field"
        />

        <AppleInput
          v-model="registerData.confirmPassword"
          type="password"
          placeholder="确认密码"
          required
          class="form-field"
        />

        <div class="form-field">
          <select
            v-model="registerData.role"
            class="form-select"
          >
            <option value="">
              选择角色(可选)
            </option>
            <option value="admin">
              管理员
            </option>
            <option value="operator">
              操作员
            </option>
            <option value="viewer">
              观察员
            </option>
          </select>
        </div>

        <AppleButton
          type="submit"
          variant="primary"
          size="large"
          :loading="loading"
          class="register-button"
        >
          {{ loading ? '注册中...' : '立即注册' }}
        </AppleButton>
      </form>
    </template>

    <!-- 底部登录链接 -->
    <template #bottomLink>
      <span>已有账号?</span>
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
import { AppleButton, AppleInput } from '@/components/apple'

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

// 注册处理函数
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
      ElMessage.success('注册成功!正在自动登录...')

      try {
        // 自动登录
        const loginResponse = await authApi.login({
          username: registerData.username,
          password: registerData.password
        })

        if (loginResponse.success) {
          // 保存用户信息到store
          const user = loginResponse.data.user
          await store.dispatch('setUser', user)

          ElMessage.success('登录成功!即将跳转到仪表盘')

          // 延迟跳转到仪表盘
          setTimeout(() => {
            router.replace('/dashboard')
          }, 1000)
        } else {
          // 自动登录失败,跳转到登录页
          ElMessage.warning('注册成功,但自动登录失败,请手动登录')
          setTimeout(() => {
            router.push('/login')
          }, 1500)
        }
      } catch (loginError) {
        console.error('自动登录失败:', loginError)
        ElMessage.warning('注册成功,但自动登录失败,请手动登录')
        setTimeout(() => {
          router.push('/login')
        }, 1500)
      }
    } else {
      ElMessage.error(response.message || '注册失败')
    }

  } catch (error) {
    console.error('注册错误:', error)
    const errorMessage = error.message || error.error_code || '注册失败,请检查网络连接'
    ElMessage.error(errorMessage)
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

.register-form {
  margin-bottom: var(--space-6);
}

.form-field {
  margin-bottom: var(--space-4);
}

/* 下拉选择框样式 */
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

.register-button {
  width: 100%;
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

  .form-select {
    background: #2c2c2e;
    border: 1px solid #48484a;
    color: #f2f2f7;
  }

  .form-select:focus {
    border-color: #0a84ff;
    box-shadow:
      0 0 0 3px rgba(10, 132, 255, 0.1),
      0 2px 8px rgba(0, 0, 0, 0.2);
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
