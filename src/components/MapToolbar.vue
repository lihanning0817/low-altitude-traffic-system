<template>
  <div class="toolbar">
    <el-button-group>
      <el-button
        type="primary"
        :class="{ active: currentTool === 'select' }"
        @click="handleToolChange('select')"
      >
        <el-icon><Pointer /></el-icon>
        选择
      </el-button>
      <el-button
        :class="{ active: currentTool === 'measure' }"
        @click="handleToolChange('measure')"
      >
        <el-icon><Grid /></el-icon>
        测距
      </el-button>
      <el-button
        :class="{ active: currentTool === 'area' }"
        @click="handleToolChange('area')"
      >
        <el-icon><Crop /></el-icon>
        测面
      </el-button>
    </el-button-group>
  </div>
</template>

<script setup>
defineProps({
  currentTool: {
    type: String,
    required: true
  }
})

const emit = defineEmits(['tool-change'])

const handleToolChange = (tool) => {
  emit('tool-change', tool)
}
</script>

<style scoped>
.toolbar {
  display: flex;
  gap: 12px;
}

.toolbar .el-button.active {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border-color: transparent;
  color: white;
}
</style>
