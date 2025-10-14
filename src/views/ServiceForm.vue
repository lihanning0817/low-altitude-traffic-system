<template>
  <div class="service-form-container">
    <h1>{{ serviceTitle }}预约</h1>
    <form @submit.prevent="submitServiceForm">
      <div class="form-group">
        <label for="startPoint">起点</label>
        <input
          id="startPoint"
          v-model="serviceForm.startPoint"
          type="text"
          placeholder="请输入起点位置"
          required
        >
      </div>
      <div class="form-group">
        <label for="endPoint">终点</label>
        <input
          id="endPoint"
          v-model="serviceForm.endPoint"
          type="text"
          placeholder="请输入终点位置"
          required
        >
      </div>
      <div class="form-group">
        <label for="requirements">特殊需求</label>
        <textarea
          id="requirements"
          v-model="serviceForm.requirements"
          placeholder="请输入特殊需求或备注信息"
          rows="4"
        />
      </div>
      <button
        type="submit"
        class="submit-button"
      >
        提交预约
      </button>
    </form>
  </div>
</template>

<script>
export default {
  name: 'ServiceForm',
  data() {
    return {
      serviceType: this.$route.params.serviceType,
      serviceForm: {
        startPoint: '',
        endPoint: '',
        requirements: ''
      }
    }
  },
  computed: {
    serviceTitle() {
      const titles = {
        logistics: '物流配送',
        agriculture: '农业植保',
        inspection: '巡检服务',
        rescue: '应急救援'
      }
      return titles[this.serviceType] || '服务'
    }
  },
  methods: {
    submitServiceForm() {
      // 这里应该调用后端API提交服务预约
      // 模拟提交成功，跳转到任务追踪页
      this.$router.push('/task-tracking')
    }
  }
}
</script>

<style scoped>
.service-form-container {
  max-width: 600px;
  margin: 0 auto;
  padding: 2rem;
}

h1 {
  text-align: center;
  color: #333;
  margin-bottom: 2rem;
}

.form-group {
  margin-bottom: 1.5rem;
}

label {
  display: block;
  margin-bottom: 0.5rem;
  color: #555;
  font-weight: 500;
}

input,
textarea {
  width: 100%;
  padding: 0.75rem;
  border: 1px solid #ddd;
  border-radius: 4px;
  font-size: 1rem;
  transition: border-color 0.3s;
}

input:focus,
textarea:focus {
  outline: none;
  border-color: #409eff;
  box-shadow: 0 0 0 2px rgba(64, 158, 255, 0.2);
}

.submit-button {
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

.submit-button:hover {
  background-color: #66b1ff;
}
</style>