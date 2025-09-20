<template>
  <div class="admin-dashboard">
    <h1>管理后台</h1>
    <div class="dashboard-content">
      <div class="card-grid">
        <div class="card">
          <h3>无人机管理</h3>
          <p>新增/编辑/删除无人机，查看状态</p>
          <button @click="goToDrones">
            进入管理
          </button>
        </div>
        <div class="card">
          <h3>任务管理</h3>
          <p>分配无人机，修改任务状态</p>
          <button @click="goToTasks">
            进入管理
          </button>
        </div>
        <div class="card">
          <h3>数据大屏</h3>
          <p>任务统计、收入、无人机利用率</p>
          <button @click="goToAnalytics">
            查看数据
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'AdminDashboard',
  methods: {
    goToDrones() {
      this.$router.push('/admin/drones')
    },
    goToTasks() {
      this.$router.push('/admin/tasks')
    },
    goToAnalytics() {
      this.$router.push('/admin/analytics')
    }
  }
}
</script>

<style scoped>
.admin-dashboard {
  max-width: 1200px;
  margin: 0 auto;
  padding: 2rem;
}

h1 {
  text-align: center;
  color: var(--text-primary);
  margin-bottom: 2rem;
  font-weight: 600;
}

.dashboard-content {
  background: var(--card-bg);
  border-radius: var(--radius-lg);
  padding: 2rem;
  box-shadow: var(--card-shadow);
  border: 1px solid var(--card-border);
}

.card-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 2rem;
}

.card {
  background: var(--card-bg);
  border: 1px solid var(--card-border);
  border-radius: var(--radius-lg);
  padding: 1.5rem;
  text-align: center;
  transition: all var(--transition-normal);
  box-shadow: var(--shadow-sm);
}

.card:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-lg);
  border-color: var(--primary-color);
}

.card h3 {
  color: var(--primary-color);
  margin-bottom: 1rem;
  font-weight: 600;
}

.card p {
  color: var(--text-secondary);
  margin-bottom: 1.5rem;
  line-height: 1.5;
}

.card button {
  padding: 0.75rem 1.5rem;
  background-color: var(--btn-primary-bg);
  color: var(--btn-primary-text);
  border: none;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all var(--transition-fast);
  font-weight: 500;
  font-size: var(--font-size-sm);
}

.card button:hover {
  background-color: var(--btn-primary-hover);
  transform: translateY(-1px);
  box-shadow: var(--shadow-md);
}
</style>