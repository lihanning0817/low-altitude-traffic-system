<template>
  <div class="register-dashboard">
    <div class="register-container">
      <div class="register-form">
        <h2>注册智能低空交通系统账户</h2>
        <form @submit.prevent="handleRegister">
          <div class="form-group">
            <label for="username">用户名</label>
            <input
              id="username"
              v-model="username"
              type="text"
              required
              placeholder="请输入用户名"
            >
          </div>
          <div class="form-group">
            <label for="email">邮箱</label>
            <input
              id="email"
              v-model="email"
              type="email"
              required
              placeholder="请输入邮箱地址"
            >
          </div>
          <div class="form-group">
            <label for="password">密码</label>
            <input
              id="password"
              v-model="password"
              type="password"
              required
              placeholder="请输入密码"
            >
          </div>
          <div class="form-group">
            <label for="confirmPassword">确认密码</label>
            <input
              id="confirmPassword"
              v-model="confirmPassword"
              type="password"
              required
              placeholder="请再次输入密码"
            >
          </div>
          <div class="form-group">
            <label for="role">角色</label>
            <select
              id="role"
              v-model="role"
              required
            >
              <option value="">
                请选择角色
              </option>
              <option value="user">
                普通用户
              </option>
              <option value="admin">
                管理员
              </option>
            </select>
          </div>
          <button
            type="submit"
            class="register-btn"
          >
            注册
          </button>
          <div class="login-link">
            已有账号？<router-link to="/login">
              立即登录
            </router-link>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { useStore } from 'vuex'

const router = useRouter()
const store = useStore()

const username = ref('')
const email = ref('')
const password = ref('')
const confirmPassword = ref('')
const role = ref('')

const handleRegister = async () => {
  if (password.value !== confirmPassword.value) {
    alert('密码和确认密码不匹配')
    return
  }

  try {
    const response = await store.dispatch('register', {
      username: username.value,
      email: email.value,
      password: password.value,
      role: role.value
    })

    if (response.success) {
      router.push('/login')
    } else {
      alert('注册失败：' + response.message)
    }
  } catch (error) {
    console.error('注册错误:', error)
    alert('注册失败，请稍后再试')
  }
}
</script>

<style scoped>
.register-dashboard {
  min-height: 100vh;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 2rem;
}

.register-container {
  background: white;
  border-radius: 16px;
  box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
  padding: 3rem;
  width: 100%;
  max-width: 400px;
}

.register-form h2 {
  text-align: center;
  color: #333;
  margin-bottom: 2rem;
  font-size: 2rem;
  font-weight: 700;
}

.form-group {
  margin-bottom: 1.5rem;
}

.form-group label {
  display: block;
  margin-bottom: 0.5rem;
  color: #555;
  font-weight: 600;
}

.form-group input,
.form-group select {
  width: 100%;
  padding: 1rem;
  border: 1px solid #ddd;
  border-radius: 8px;
  font-size: 1rem;
  transition: border-color 0.3s ease;
}

.form-group input:focus,
.form-group select:focus {
  outline: none;
  border-color: #667eea;
  box-shadow: 0 0 0 2px rgba(102, 126, 234, 0.2);
}

.register-btn {
  width: 100%;
  padding: 1rem;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border: none;
  border-radius: 8px;
  font-size: 1.1rem;
  font-weight: 600;
  cursor: pointer;
  transition: transform 0.2s ease, box-shadow 0.2s ease;
}

.register-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 20px rgba(102, 126, 234, 0.3);
}

.login-link {
  text-align: center;
  margin-top: 1.5rem;
}

.login-link a {
  color: #667eea;
  text-decoration: none;
  font-weight: 600;
}

.login-link a:hover {
  text-decoration: underline;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .register-container {
    padding: 2rem;
    margin: 0 1rem;
  }
  
  .register-form h2 {
    font-size: 1.8rem;
  }
}
</style>
