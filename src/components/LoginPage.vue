<template>
  <div class="login-container">
    <div class="login-content">
      <div class="login-card">
        <div class="logo-section">
          <div class="logo-icon">
            <img :src="`/logo.png?v=${Date.now()}`" alt="智能低空交通系统" class="logo-image" />
          </div>
        </div>

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
              <option value="" disabled>选择角色</option>
              <option value="admin">管理员</option>
              <option value="operator">操作员</option>
              <option value="viewer">观察员</option>
            </select>
          </div>

          <button
            type="submit"
            class="login-button"
            :disabled="loading"
          >
            <span v-if="!loading">登录</span>
            <span v-else>登录中...</span>
          </button>
        </form>

        <div class="test-accounts">
          <p class="test-title">测试账号</p>
          <div class="test-account-list">
            <div class="test-account">
              <strong>管理员:</strong> admin / 123456 / 管理员
            </div>
            <div class="test-account">
              <strong>操作员:</strong> operator / 123456 / 操作员
            </div>
            <div class="test-account">
              <strong>观察员:</strong> viewer / 123456 / 观察员
            </div>
          </div>
        </div>

        <div class="register-link">
          <span>还没有账号？</span>
          <button
            type="button"
            class="register-button"
            @click="$router.push('/register')"
          >
            立即注册
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import { useRouter, useRoute } from 'vue-router'
import { useStore } from 'vuex'

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
    ElMessage.error('请填写完整的登录信息')
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

  try {
    loading.value = true

    // 测试账号快速登录
    const testAccounts = {
      'admin': { password: '123456', role: 'admin', username: 'admin' },
      'operator': { password: '123456', role: 'operator', username: 'operator' },
      'viewer': { password: '123456', role: 'viewer', username: 'viewer' }
    }

    // 检查是否是测试账号
    const testAccount = testAccounts[loginData.username]
    if (testAccount && testAccount.password === loginData.password && testAccount.role === loginData.role) {
      // 使用测试账号登录
      const mockUser = {
        id: Date.now(),
        username: testAccount.username,
        role: testAccount.role,
        name: testAccount.role === 'admin' ? '系统管理员' : testAccount.role === 'operator' ? '系统操作员' : '系统观察员',
        avatar: null
      }

      store.commit('SET_USER', mockUser)
      ElMessage.success(`欢迎回来，${mockUser.name}！`)

      // 登录成功后重定向
      const redirect = route.query.redirect || '/dashboard'
      router.push(redirect)
      return
    }

    // 如果不是测试账号，调用真实API
    await store.dispatch('login', loginData)

    // 登录成功后重定向
    const redirect = route.query.redirect || '/dashboard'
    router.push(redirect)

  } catch (error) {
    ElMessage.error(error.message || '登录失败，请检查用户名和密码')
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
/* Apple-style minimal login page */
.login-container {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(180deg, #f5f7fa 0%, #ffffff 100%);
  font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Display', 'Segoe UI', Roboto, sans-serif;
  padding: 20px;
  position: relative;
  z-index: 1;
}

.login-content {
  width: 100%;
  max-width: 380px;
}

.login-card {
  background: rgba(255, 255, 255, 0.9);
  backdrop-filter: blur(20px);
  border-radius: 16px;
  padding: 48px 40px 40px;
  box-shadow:
    0 1px 3px rgba(0, 0, 0, 0.02),
    0 8px 24px rgba(0, 0, 0, 0.04);
  border: 1px solid rgba(255, 255, 255, 0.8);
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.login-card:hover {
  transform: translateY(-2px);
  box-shadow:
    0 4px 12px rgba(0, 0, 0, 0.03),
    0 16px 48px rgba(0, 0, 0, 0.06);
}

.logo-section {
  text-align: center;
  margin-bottom: 40px;
}

.logo-icon {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 64px;
  height: 64px;
  margin-bottom: 20px;
  transition: all 0.3s ease;
}

.logo-icon:hover {
  transform: scale(1.05);
}

.logo-image {
  width: 100%;
  height: 100%;
  object-fit: contain;
  border-radius: 12px;
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

.register-link {
  text-align: center;
  font-size: 14px;
  color: #86868b;
  margin-top: 24px;
}

.register-button {
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

.register-button:hover {
  color: #0056b3;
  text-decoration: underline;
}

/* Responsive design */
@media (max-width: 480px) {
  .login-container {
    padding: 16px;
  }

  .login-card {
    padding: 40px 24px 32px;
  }


  .form-input,
  .form-select,
  .login-button {
    height: 48px;
    font-size: 16px; /* Prevent zoom on iOS */
  }
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .login-container {
    background: linear-gradient(180deg, #1c1c1e 0%, #000000 100%);
  }

  .login-card {
    background: rgba(44, 44, 46, 0.9);
    border: 1px solid rgba(84, 84, 88, 0.6);
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

  .register-link {
    color: #98989d;
  }

  .register-button {
    color: #0a84ff;
  }

  .register-button:hover {
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
}
</style>
