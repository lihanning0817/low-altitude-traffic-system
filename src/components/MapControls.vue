<template>
  <div class="central-controls">
    <el-button-group>
      <el-button
        type="success"
        @click="emit('refresh')"
      >
        <el-icon><Refresh /></el-icon>
        刷新
      </el-button>
      <el-button @click="emit('center')">
        <el-icon><Aim /></el-icon>
        居中
      </el-button>
      <el-button @click="emit('toggle-traffic')">
        <el-icon><View /></el-icon>
        {{ showTraffic ? '隐藏' : '显示' }}交通
      </el-button>
    </el-button-group>
  </div>
</template>

<script setup>
defineProps({
  showTraffic: {
    type: Boolean,
    required: true
  }
})

const emit = defineEmits(['refresh', 'center', 'toggle-traffic'])
</script>

<style scoped>
.central-controls {
  display: flex;
  gap: 12px;
}
</style>
