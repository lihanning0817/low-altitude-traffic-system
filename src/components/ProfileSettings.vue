<template>
  <div class="profile-settings">
    <div class="settings-container">
      <h2 class="settings-title">
        个人设置
      </h2>

      <!-- 用户信息卡片 -->
      <el-card
        v-loading="userLoading"
        class="user-info-card"
      >
        <template #header>
          <div class="card-header">
            <h3>基本信息</h3>
            <el-button
              type="primary"
              size="small"
              :loading="userLoading"
              @click="refreshUserInfo"
            >
              刷新
            </el-button>
          </div>
        </template>

        <div
          v-if="user"
          class="user-info"
        >
          <div class="info-item">
            <label>用户名：</label>
            <span>{{ user.username }}</span>
          </div>
          <div class="info-item">
            <label>邮箱：</label>
            <span>{{ user.email }}</span>
          </div>
          <div class="info-item">
            <label>真实姓名：</label>
            <span>{{ user.full_name || '未设置' }}</span>
          </div>
          <div class="info-item">
            <label>角色：</label>
            <el-tag :type="getRoleTagType(user.role)">
              {{ getRoleDisplayName(user.role) }}
            </el-tag>
          </div>
          <div class="info-item">
            <label>状态：</label>
            <el-tag :type="getStatusTagType(user.status)">
              {{ getStatusDisplayName(user.status) }}
            </el-tag>
          </div>
          <div class="info-item">
            <label>注册时间：</label>
            <span>{{ formatDate(user.created_at) }}</span>
          </div>
          <div class="info-item">
            <label>最后更新：</label>
            <span>{{ formatDate(user.updated_at) }}</span>
          </div>
        </div>

        <div
          v-else
          class="no-user"
        >
          <el-empty description="无法获取用户信息" />
        </div>
      </el-card>

      <!-- 修改密码卡片 -->
      <el-card class="password-card">
        <template #header>
          <h3>修改密码</h3>
        </template>

        <el-form
          ref="passwordForm"
          :model="passwordData"
          :rules="passwordRules"
          label-width="120px"
          class="password-form"
        >
          <el-form-item
            label="当前密码"
            prop="old_password"
          >
            <el-input
              v-model="passwordData.old_password"
              type="password"
              placeholder="请输入当前密码"
              show-password
              clearable
            />
          </el-form-item>

          <el-form-item
            label="新密码"
            prop="new_password"
          >
            <el-input
              v-model="passwordData.new_password"
              type="password"
              placeholder="请输入新密码（至少8位，包含字母和数字）"
              show-password
              clearable
            />
          </el-form-item>

          <el-form-item
            label="确认新密码"
            prop="confirm_password"
          >
            <el-input
              v-model="passwordData.confirm_password"
              type="password"
              placeholder="请再次输入新密码"
              show-password
              clearable
            />
          </el-form-item>

          <el-form-item>
            <el-button
              type="primary"
              :loading="passwordLoading"
              @click="handleChangePassword"
            >
              修改密码
            </el-button>
            <el-button @click="resetPasswordForm">
              重置
            </el-button>
          </el-form-item>
        </el-form>
      </el-card>
    </div>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { ElMessage } from 'element-plus'
import { useStore } from 'vuex'

const store = useStore()

// 获取当前用户信息
const user = computed(() => store.state.user)
const userLoading = ref(false)
const passwordLoading = ref(false)
const passwordForm = ref(null)

// 密码修改表单数据
const passwordData = reactive({
  old_password: '',
  new_password: '',
  confirm_password: ''
})

// 密码验证规则
const validateConfirmPassword = (rule, value, callback) => {
  if (value !== passwordData.new_password) {
    callback(new Error('两次输入的密码不一致'))
  } else {
    callback()
  }
}

const passwordRules = {
  old_password: [
    { required: true, message: '请输入当前密码', trigger: 'blur' }
  ],
  new_password: [
    { required: true, message: '请输入新密码', trigger: 'blur' },
    { min: 8, message: '密码长度不能少于8位', trigger: 'blur' },
    { pattern: /^(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d@$!%*#?&]/, message: '密码必须包含字母和数字', trigger: 'blur' }
  ],
  confirm_password: [
    { required: true, message: '请确认新密码', trigger: 'blur' },
    { validator: validateConfirmPassword, trigger: 'blur' }
  ]
}

// 刷新用户信息
const refreshUserInfo = async () => {
  try {
    userLoading.value = true
    await store.dispatch('getCurrentUser')
    ElMessage.success('用户信息已刷新')
  } catch (error) {
    console.error('刷新用户信息失败:', error)
    ElMessage.error('刷新用户信息失败')
  } finally {
    userLoading.value = false
  }
}

// 修改密码
const handleChangePassword = async () => {
  if (!passwordForm.value) return

  try {
    const valid = await passwordForm.value.validate()
    if (!valid) return

    passwordLoading.value = true

    await store.dispatch('changePassword', {
      old_password: passwordData.old_password,
      new_password: passwordData.new_password
    })

    // 修改成功后重置表单
    resetPasswordForm()

  } catch (error) {
    console.error('修改密码失败:', error)
  } finally {
    passwordLoading.value = false
  }
}

// 重置密码表单
const resetPasswordForm = () => {
  if (passwordForm.value) {
    passwordForm.value.resetFields()
  }
  Object.assign(passwordData, {
    old_password: '',
    new_password: '',
    confirm_password: ''
  })
}

// 获取角色标签类型
const getRoleTagType = (role) => {
  const roleTypes = {
    'admin': 'danger',
    'operator': 'warning',
    'user': 'success',
    'viewer': 'info'
  }
  return roleTypes[role] || 'info'
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

// 获取状态标签类型
const getStatusTagType = (status) => {
  const statusTypes = {
    'active': 'success',
    'inactive': 'warning',
    'banned': 'danger'
  }
  return statusTypes[status] || 'info'
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
  const date = new Date(timestamp * 1000) // 假设后端返回的是Unix时间戳
  return date.toLocaleString('zh-CN')
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
  padding: 24px;
  background-color: var(--bg-primary);
}

.settings-container {
  max-width: 800px;
  margin: 0 auto;
}

.settings-title {
  font-size: 24px;
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 24px;
  text-align: left;
}

.user-info-card,
.password-card {
  margin-bottom: 24px;
  border-radius: 12px;
  border: 1px solid var(--border-color);
  box-shadow: var(--shadow-sm);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-header h3 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
  color: var(--text-primary);
}

.user-info {
  display: grid;
  gap: 16px;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
}

.info-item {
  display: flex;
  align-items: center;
  padding: 12px 16px;
  background-color: var(--bg-secondary);
  border-radius: 8px;
  border: 1px solid var(--border-color);
}

.info-item label {
  font-weight: 600;
  color: var(--text-secondary);
  min-width: 100px;
  margin-right: 12px;
}

.info-item span {
  color: var(--text-primary);
  flex: 1;
}

.password-form {
  max-width: 500px;
}

.no-user {
  text-align: center;
  padding: 40px 0;
}

/* Element Plus 样式覆盖 */
:deep(.el-card__header) {
  background-color: var(--bg-tertiary);
  border-bottom: 1px solid var(--border-color);
}

:deep(.el-card__body) {
  background-color: var(--bg-secondary);
}

:deep(.el-input__wrapper) {
  border-radius: 8px;
  border: 1px solid var(--border-color);
  background-color: var(--bg-primary);
}

:deep(.el-input__wrapper:hover) {
  border-color: var(--primary-color);
}

:deep(.el-input__wrapper.is-focus) {
  border-color: var(--primary-color);
  box-shadow: 0 0 0 2px rgba(0, 113, 227, 0.1);
}

:deep(.el-form-item__label) {
  color: var(--text-secondary);
  font-weight: 500;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .profile-settings {
    padding: 16px;
  }

  .user-info {
    grid-template-columns: 1fr;
  }

  .info-item {
    flex-direction: column;
    align-items: flex-start;
    text-align: left;
  }

  .info-item label {
    min-width: auto;
    margin-right: 0;
    margin-bottom: 4px;
  }

  .card-header {
    flex-direction: column;
    align-items: flex-start;
    gap: 12px;
  }

  .password-form {
    max-width: none;
  }
}

/* 深色模式 */
[data-theme='dark'] .user-info-card,
[data-theme='dark'] .password-card {
  background-color: var(--bg-secondary);
  border-color: var(--border-color);
}

[data-theme='dark'] .info-item {
  background-color: var(--bg-tertiary);
  border-color: var(--border-color);
}

[data-theme='dark'] :deep(.el-card__header) {
  background-color: var(--bg-darker);
  border-bottom-color: var(--border-color);
}

[data-theme='dark'] :deep(.el-card__body) {
  background-color: var(--bg-secondary);
}
</style>
