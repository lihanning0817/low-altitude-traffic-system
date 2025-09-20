<template>
  <div class="action-controls">
    <el-button
      type="warning"
      @click="emit('fullscreen')"
    >
      <el-icon><FullScreen /></el-icon>
      全屏
    </el-button>
    <el-button
      type="info"
      @click="emit('settings')"
    >
      <el-icon><Setting /></el-icon>
      设置
    </el-button>
  </div>
</template>

<script setup>
const emit = defineEmits(['fullscreen', 'settings'])
</script>

<style scoped>
.action-controls {
  display: flex;
  gap: 12px;
}
</style>
