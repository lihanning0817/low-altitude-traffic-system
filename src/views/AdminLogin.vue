<template>
  <div class="admin-login-container">
    <div class="admin-login-box">
      <h2>管理后台登录</h2>
      <form @submit.prevent="handleAdminLogin">
        <div class="form-group">
          <label for="username">用户名</label>
          <input
            id="username"
            v-model="loginForm.username"
            type="text"
            placeholder="请输入用户名"
            required
          >
        </div>
        <div class="form-group">
          <label for="password">密码</label>
          <input
            id="password"
            v-model="loginForm.password"
            type="password"
            placeholder="请输入密码"
            required
          >
        </div>
        <button
          type="submit"
          class="admin-login-button"
          :disabled="loading"
        >
          <span v-if="loading">登录中...</span>
          <span v-else>登录</span>
        </button>
      </form>
    </div>
  </div>
</template>

<script>
import { ElMessage } from 'element-plus'
import authApi from '@/services/authApi'

export default {
  name: 'AdminLogin',
  data() {
    return {
      loginForm: {
        username: '',
        password: ''
      },
      loading: false
    }
  },
  methods: {
    async handleAdminLogin() {
      // 基础验证
      if (!this.loginForm.username || !this.loginForm.password) {
        ElMessage.error('请填写用户名和密码')
        return
      }

      if (this.loginForm.username.length < 3 || this.loginForm.username.length > 20) {
        ElMessage.error('用户名长度在3到20个字符')
        return
      }

      if (this.loginForm.password.length < 6) {
        ElMessage.error('密码长度不能少于6位')
        return
      }

      try {
        this.loading = true

        // 调用后端API进行登录验证
        const response = await authApi.login({
          username: this.loginForm.username,
          password: this.loginForm.password
        })

        if (response.success) {
          const user = response.data.user

          // 验证是否为管理员权限
          if (user.role !== 'admin') {
            ElMessage.error('此页面仅限管理员访问，请使用管理员账号登录')
            return
          }

          // 保存用户信息到store
          await this.$store.dispatch('setUser', user)

          ElMessage.success('管理员登录成功！')

          // 跳转到管理后台
          const redirect = this.$route.query.redirect || '/admin/dashboard'
          this.$router.push(redirect)
        } else {
          ElMessage.error(response.message || '登录失败')
        }

      } catch (error) {
        console.error('管理员登录错误:', error)
        // 错误信息已由 authApi 拦截器统一处理
      } finally {
        this.loading = false
      }
    }
  }
}
</script>

<style scoped>
.admin-login-container {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
  background-color: #f0f2f5;
}

.admin-login-box {
  background: white;
  padding: 2rem;
  border-radius: 8px;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
  width: 100%;
  max-width: 400px;
}

h2 {
  text-align: center;
  margin-bottom: 1.5rem;
  color: #333;
}

.form-group {
  margin-bottom: 1rem;
}

label {
  display: block;
  margin-bottom: 0.5rem;
  color: #555;
}

input {
  width: 100%;
  padding: 0.75rem;
  border: 1px solid #ddd;
  border-radius: 4px;
  font-size: 1rem;
}

input:focus {
  outline: none;
  border-color: #409eff;
  box-shadow: 0 0 0 2px rgba(64, 158, 255, 0.2);
}

.admin-login-button {
  width: 100%;
  padding: 0.75rem;
  background-color: #409eff;
  color: white;
  border: none;
  border-radius: 4px;
  font-size: 1rem;
  cursor: pointer;
  transition: background-color 0.3s;
}

.admin-login-button:hover {
  background-color: #66b1ff;
}

.admin-login-button:disabled {
  background-color: #c0c4cc;
  cursor: not-allowed;
  opacity: 0.6;
}

.admin-login-button:disabled:hover {
  background-color: #c0c4cc;
}
</style>