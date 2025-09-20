<template>
  <div class="api-test">
    <h2>前后端API连接测试</h2>

    <div class="test-section">
      <h3>后端服务状态</h3>
      <el-button @click="testHealth" type="primary" :loading="loading.health">
        测试健康检查
      </el-button>
      <div v-if="results.health" class="result">
        <pre>{{ JSON.stringify(results.health, null, 2) }}</pre>
      </div>
    </div>

    <div class="test-section">
      <h3>系统信息</h3>
      <el-button @click="testSystemInfo" type="info" :loading="loading.info">
        获取系统信息
      </el-button>
      <div v-if="results.info" class="result">
        <pre>{{ JSON.stringify(results.info, null, 2) }}</pre>
      </div>
    </div>

    <div class="test-section">
      <h3>无人机数据</h3>
      <el-button @click="testDrones" type="success" :loading="loading.drones">
        获取无人机列表
      </el-button>
      <div v-if="results.drones" class="result">
        <pre>{{ JSON.stringify(results.drones, null, 2) }}</pre>
      </div>
    </div>

    <div class="test-section">
      <h3>任务数据</h3>
      <el-button @click="testTasks" type="warning" :loading="loading.tasks">
        获取任务列表
      </el-button>
      <div v-if="results.tasks" class="result">
        <pre>{{ JSON.stringify(results.tasks, null, 2) }}</pre>
      </div>
    </div>

    <div class="test-section">
      <h3>用户数据</h3>
      <el-button @click="testUsers" type="danger" :loading="loading.users">
        获取用户列表
      </el-button>
      <div v-if="results.users" class="result">
        <pre>{{ JSON.stringify(results.users, null, 2) }}</pre>
      </div>
    </div>

    <div class="test-section">
      <h3>综合测试</h3>
      <el-button @click="runAllTests" type="primary" :loading="allTesting">
        运行所有测试
      </el-button>
      <div v-if="testSummary" class="summary">
        <el-alert
          :type="testSummary.success ? 'success' : 'error'"
          :title="testSummary.message"
          show-icon
        />
      </div>
    </div>

    <div v-if="errors.length > 0" class="errors">
      <h3>错误信息</h3>
      <div v-for="(error, index) in errors" :key="index" class="error">
        {{ error }}
      </div>
    </div>
  </div>
</template>

<script>
import apiService from '@/services/api'

export default {
  name: 'ApiTest',
  data() {
    return {
      loading: {
        health: false,
        info: false,
        drones: false,
        tasks: false,
        users: false
      },
      results: {
        health: null,
        info: null,
        drones: null,
        tasks: null,
        users: null
      },
      errors: [],
      allTesting: false,
      testSummary: null
    }
  },
  methods: {
    async testHealth() {
      this.loading.health = true
      this.clearErrors()
      try {
        const result = await apiService.getHealth()
        this.results.health = result
        this.$message.success('健康检查成功')
      } catch (error) {
        this.handleError('健康检查', error)
      } finally {
        this.loading.health = false
      }
    },

    async testSystemInfo() {
      this.loading.info = true
      this.clearErrors()
      try {
        const result = await apiService.getSystemInfo()
        this.results.info = result
        this.$message.success('获取系统信息成功')
      } catch (error) {
        this.handleError('系统信息', error)
      } finally {
        this.loading.info = false
      }
    },

    async testDrones() {
      this.loading.drones = true
      this.clearErrors()
      try {
        const result = await apiService.getDrones()
        this.results.drones = result
        this.$message.success('获取无人机数据成功')
      } catch (error) {
        this.handleError('无人机数据', error)
      } finally {
        this.loading.drones = false
      }
    },

    async testTasks() {
      this.loading.tasks = true
      this.clearErrors()
      try {
        const result = await apiService.getTasks()
        this.results.tasks = result
        this.$message.success('获取任务数据成功')
      } catch (error) {
        this.handleError('任务数据', error)
      } finally {
        this.loading.tasks = false
      }
    },

    async testUsers() {
      this.loading.users = true
      this.clearErrors()
      try {
        const result = await apiService.getUsers()
        this.results.users = result
        this.$message.success('获取用户数据成功')
      } catch (error) {
        this.handleError('用户数据', error)
      } finally {
        this.loading.users = false
      }
    },

    async runAllTests() {
      this.allTesting = true
      this.clearAll()

      let successCount = 0
      let totalCount = 0

      const tests = [
        { name: '健康检查', method: this.testHealth },
        { name: '系统信息', method: this.testSystemInfo },
        { name: '无人机数据', method: this.testDrones },
        { name: '任务数据', method: this.testTasks },
        { name: '用户数据', method: this.testUsers }
      ]

      for (const test of tests) {
        totalCount++
        try {
          await test.method()
          successCount++
        } catch (error) {
          console.error(`${test.name}测试失败:`, error)
        }
      }

      this.testSummary = {
        success: successCount === totalCount,
        message: `测试完成: ${successCount}/${totalCount} 个API测试通过`
      }

      if (successCount === totalCount) {
        this.$message.success('所有API测试通过！前后端连接正常')
      } else {
        this.$message.warning(`部分测试失败: ${successCount}/${totalCount} 通过`)
      }

      this.allTesting = false
    },

    handleError(testName, error) {
      const errorMsg = `${testName}失败: ${error.message}`
      this.errors.push(errorMsg)
      this.$message.error(errorMsg)
    },

    clearErrors() {
      this.errors = []
    },

    clearAll() {
      this.results = {
        health: null,
        info: null,
        drones: null,
        tasks: null,
        users: null
      }
      this.errors = []
      this.testSummary = null
    }
  }
}
</script>

<style scoped>
.api-test {
  padding: 20px;
  max-width: 1000px;
  margin: 0 auto;
}

.test-section {
  margin-bottom: 30px;
  padding: 20px;
  border: 1px solid #ddd;
  border-radius: 8px;
  background-color: #fafafa;
}

.test-section h3 {
  margin-top: 0;
  color: #409eff;
  border-bottom: 2px solid #e4e7ed;
  padding-bottom: 10px;
}

.result {
  margin-top: 15px;
  padding: 15px;
  background-color: #f5f5f5;
  border: 1px solid #e4e7ed;
  border-radius: 4px;
  font-family: 'Courier New', monospace;
  max-height: 300px;
  overflow-y: auto;
  font-size: 12px;
}

.result pre {
  margin: 0;
  white-space: pre-wrap;
  word-break: break-all;
}

.summary {
  margin-top: 20px;
}

.errors {
  margin-top: 30px;
  padding: 20px;
  background-color: #fef0f0;
  border: 1px solid #fbc4c4;
  border-radius: 8px;
}

.errors h3 {
  color: #f56c6c;
  margin-top: 0;
}

.error {
  color: #f56c6c;
  margin-bottom: 10px;
  padding: 8px;
  background-color: #ffffff;
  border-left: 4px solid #f56c6c;
  border-radius: 4px;
}
</style>