<template>
  <div class="register-container">
    <div class="register-background">
      <div class="background-overlay" />
    </div>

    <div class="register-content">
      <div class="register-card">
        <div class="logo-section">
          <div class="logo-icon">
            <el-icon size="40">
              <UserFilled />
            </el-icon>
          </div>
          <h1>创建新账户</h1>
          <p>加入智能低空交通管理系统</p>
        </div>

        <el-form
          ref="registerForm"
          :model="registerData"
          :rules="rules"
          class="register-form"
          @submit.prevent="handleRegister"
        >
          <el-form-item prop="username">
            <el-input
              v-model="registerData.username"
              placeholder="请输入用户名"
              size="large"
              :prefix-icon="User"
              clearable
            />
          </el-form-item>

          <el-form-item prop="email">
            <el-input
              v-model="registerData.email"
              placeholder="请输入邮箱地址"
              size="large"
              :prefix-icon="Message"
              clearable
            />
          </el-form-item>

          <el-form-item prop="password">
            <el-input
              v-model="registerData.password"
              type="password"
              placeholder="请输入密码"
              size="large"
              :prefix-icon="Lock"
              show-password
            />
          </el-form-item>

          <el-form-item prop="confirmPassword">
            <el-input
              v-model="registerData.confirmPassword"
              type="password"
              placeholder="请确认密码"
              size="large"
              :prefix-icon="Lock"
              show-password
            />
          </el-form-item>

          <el-form-item>
            <el-button
              type="primary"
              size="large"
              class="register-button"
              :loading="loading"
              native-type="submit"
              @click="handleRegister"
            >
              <span v-if="!loading">立即注册</span>
              <span v-else>注册中...</span>
            </el-button>
          </el-form-item>
        </el-form>

        <div class="login-link">
          <span>已有账户？</span>
          <el-button
            type="text"
            @click="$router.push('/login')"
          >
            立即登录
          </el-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import { User, Lock, Message, UserFilled } from '@element-plus/icons-vue'
import { useRouter } from 'vue-router'
import { useStore } from 'vuex'

const router = useRouter()
const store = useStore()

const loading = ref(false)
const registerForm = ref(null)

const registerData = reactive({
  username: '',
  email: '',
  password: '',
  confirmPassword: ''
})

const validateConfirmPassword = (rule, value, callback) => {
  if (value !== registerData.password) {
    callback(new Error('两次输入的密码不一致'))
  } else {
    callback()
  }
}

const rules = {
  username: [
    { required: true, message: '请输入用户名', trigger: 'blur' },
    { min: 3, max: 20, message: '用户名长度在3到20个字符', trigger: 'blur' },
    { pattern: /^[a-zA-Z0-9_]+$/, message: '用户名只能包含字母、数字和下划线', trigger: 'blur' }
  ],
  email: [
    { required: true, message: '请输入邮箱地址', trigger: 'blur' },
    { type: 'email', message: '请输入正确的邮箱格式', trigger: 'blur' }
  ],
  password: [
    { required: true, message: '请输入密码', trigger: 'blur' },
    { min: 6, message: '密码长度不能少于6位', trigger: 'blur' },
    { pattern: /^(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d@$!%*#?&]/, message: '密码必须包含字母和数字', trigger: 'blur' }
  ],
  confirmPassword: [
    { required: true, message: '请确认密码', trigger: 'blur' },
    { validator: validateConfirmPassword, trigger: 'blur' }
  ]
}

const handleRegister = async () => {
  if (!registerForm.value) return

  try {
    const valid = await registerForm.value.validate()
    if (!valid) return

    loading.value = true

    // 模拟注册请求
    await new Promise(resolve => setTimeout(resolve, 1500))

    // 这里应该调用实际的注册API
    store.dispatch('register', {
      username: registerData.username,
      email: registerData.email,
      password: registerData.password
    })

    ElMessage.success('注册成功，即将跳转到登录页面')

    // 延迟跳转到登录页面
    setTimeout(() => {
      router.push('/login')
    }, 1000)

  } catch (error) {
    ElMessage.error('注册失败，请稍后重试')
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.register-container {
  position: relative;
  height: 100vh;
  overflow: hidden;
  display: flex;
  align-items: center;
  justify-content: center;
}

.register-background {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(135deg, var(--secondary-color) 0%, var(--primary-color) 100%);
  z-index: 1;
}

.register-background::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-image:
    radial-gradient(circle at 75% 25%, rgba(255,255,255,0.1) 0%, transparent 50%),
    radial-gradient(circle at 25% 75%, rgba(255,255,255,0.1) 0%, transparent 50%);
  animation: float 6s ease-in-out infinite reverse;
}

@keyframes float {
  0%, 100% { transform: translateY(0px); }
  50% { transform: translateY(-20px); }
}

.background-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.3);
}

.register-content {
  position: relative;
  z-index: 2;
  width: 100%;
  max-width: 420px;
  padding: 20px;
}

.register-card {
  background: var(--bg-secondary);
  backdrop-filter: blur(10px);
  border-radius: var(--radius-xl);
  padding: 40px;
  box-shadow: var(--shadow-lg);
  border: 1px solid var(--border-color);
  transition: all var(--transition-normal);
}

.register-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 25px 50px rgba(0, 0, 0, 0.15);
}

.logo-section {
  text-align: center;
  margin-bottom: 30px;
}

.logo-icon {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 80px;
  height: 80px;
  background: linear-gradient(135deg, #764ba2 0%, #667eea 100%);
  border-radius: 50%;
  margin-bottom: 20px;
  color: white;
  box-shadow: 0 10px 30px rgba(118, 75, 162, 0.3);
}

.logo-section h1 {
  margin: 0 0 10px 0;
  font-size: var(--font-size-xl);
  font-weight: 600;
  color: var(--text-primary);
  background: linear-gradient(135deg, var(--secondary-color) 0%, var(--primary-color) 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.logo-section p {
  margin: 0;
  font-size: var(--font-size-sm);
  color: var(--text-secondary);
  font-weight: 300;
}

.register-form {
  margin-bottom: 20px;
}

.register-form .el-form-item {
  margin-bottom: 20px;
}

.register-button {
  width: 100%;
  height: 50px;
  font-size: var(--font-size-lg);
  font-weight: 600;
  background: linear-gradient(135deg, var(--secondary-color) 0%, var(--primary-color) 100%);
  border: none;
  border-radius: var(--radius-lg);
  transition: all var(--transition-normal);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.register-button:hover {
  transform: translateY(-2px);
  box-shadow: 0 15px 30px rgba(118, 75, 162, 0.3);
}

.register-button:active {
  transform: translateY(0);
}

.register-button:hover {
  transform: translateY(-2px);
  box-shadow: 0 15px 30px rgba(118, 75, 162, 0.3);
}

.register-button:active {
  transform: translateY(0);
}

.login-link {
  text-align: center;
  font-size: 14px;
  color: #7f8c8d;
}

.login-link .el-button {
  padding: 0;
  margin-left: 5px;
  color: #764ba2;
  font-weight: 600;
}

/* Element Plus 样式覆盖 */
:deep(.el-input__wrapper) {
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-color);
  transition: all var(--transition-normal);
  background-color: var(--bg-tertiary);
}

:deep(.el-input__wrapper:hover) {
  box-shadow: var(--shadow-md);
  border-color: var(--primary-color);
}

:deep(.el-input__wrapper.is-focus) {
  box-shadow: var(--shadow-lg);
  border-color: var(--primary-color);
  background-color: var(--bg-secondary);
}

:deep(.el-input__inner) {
  height: 48px;
  font-size: var(--font-size-base);
  color: var(--text-primary);
}

:deep(.el-input__prefix) {
  color: var(--text-tertiary);
}

:deep(.el-input__suffix) {
  color: var(--text-tertiary);
}

/* 深色模式样式 */
[data-theme='dark'] .register-background {
  background: linear-gradient(135deg, var(--secondary-color) 0%, var(--primary-dark) 100%);
}

[data-theme='dark'] .register-card {
  background: var(--bg-secondary);
  border: 1px solid var(--border-color);
}

[data-theme='dark'] .logo-section h1 {
  color: var(--text-primary);
  background: linear-gradient(135deg, var(--secondary-color) 0%, var(--primary-color) 100%);
}

[data-theme='dark'] .logo-section p {
  color: var(--text-secondary);
}

[data-theme='dark'] .login-link {
  color: var(--text-secondary);
}

[data-theme='dark'] .login-link .el-button {
  color: var(--primary-light);
}

[data-theme='dark'] .login-link .el-button:hover {
  color: var(--primary-color);
}

[data-theme='dark'] :deep(.el-input__wrapper) {
  background-color: var(--bg-tertiary);
  border-color: var(--border-color);
}

[data-theme='dark'] :deep(.el-input__wrapper:hover) {
  border-color: var(--primary-light);
}

[data-theme='dark'] :deep(.el-input__wrapper.is-focus) {
  border-color: var(--primary-light);
  background-color: var(--bg-darker);
}

[data-theme='dark'] :deep(.el-input__inner) {
  color: var(--text-primary);
  background-color: var(--bg-tertiary);
}

[data-theme='dark'] :deep(.el-input__prefix) {
  color: var(--text-tertiary);
}

[data-theme='dark'] :deep(.el-input__suffix) {
  color: var(--text-tertiary);
}
</style>
