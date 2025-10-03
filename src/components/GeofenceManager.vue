<template>
  <div class="geofence-manager">
    <el-card shadow="hover">
      <template #header>
        <div class="header">
          <span class="title">
            <el-icon><SetUp /></el-icon>
            地理围栏管理
          </span>
          <el-button
            type="primary"
            size="small"
            @click="createDialogVisible = true"
          >
            <el-icon><Plus /></el-icon>
            创建围栏
          </el-button>
        </div>
      </template>

      <!-- 围栏列表 -->
      <div class="geofence-list">
        <el-empty
          v-if="geofences.length === 0"
          description="暂无地理围栏"
          :image-size="80"
        />

        <div
          v-for="fence in geofences"
          :key="fence.id"
          class="geofence-item"
          :class="{ active: selectedGeofence?.id === fence.id }"
          @click="selectGeofence(fence)"
        >
          <div class="fence-header">
            <div class="fence-title">
              <el-icon :color="getTypeColor(fence.purpose)">
                <Location />
              </el-icon>
              <span class="name">{{ fence.name }}</span>
            </div>
            <div class="fence-actions">
              <el-switch
                v-model="fence.status"
                active-value="active"
                inactive-value="inactive"
                @change="toggleFenceStatus(fence)"
                size="small"
              />
              <el-button
                type="text"
                size="small"
                @click.stop="editGeofence(fence)"
              >
                <el-icon><Edit /></el-icon>
              </el-button>
              <el-button
                type="text"
                size="small"
                @click.stop="deleteGeofence(fence)"
              >
                <el-icon><Delete /></el-icon>
              </el-button>
            </div>
          </div>

          <div class="fence-info">
            <el-tag :type="getPurposeType(fence.purpose)" size="small">
              {{ getPurposeText(fence.purpose) }}
            </el-tag>
            <el-tag type="info" size="small">
              {{ getTypeText(fence.fence_type) }}
            </el-tag>
            <span class="description">{{ fence.description || '无描述' }}</span>
          </div>

          <div v-if="fence.max_altitude || fence.min_altitude" class="altitude-info">
            <span v-if="fence.min_altitude" class="altitude">
              最低: {{ fence.min_altitude }}m
            </span>
            <span v-if="fence.max_altitude" class="altitude">
              最高: {{ fence.max_altitude }}m
            </span>
          </div>
        </div>
      </div>
    </el-card>

    <!-- 创建/编辑围栏对话框 -->
    <el-dialog
      v-model="createDialogVisible"
      :title="editingFence ? '编辑围栏' : '创建围栏'"
      width="500px"
    >
      <el-form
        ref="fenceFormRef"
        :model="fenceForm"
        :rules="fenceRules"
        label-width="100px"
      >
        <el-form-item label="围栏名称" prop="name">
          <el-input
            v-model="fenceForm.name"
            placeholder="请输入围栏名称"
            maxlength="100"
          />
        </el-form-item>

        <el-form-item label="围栏描述" prop="description">
          <el-input
            v-model="fenceForm.description"
            type="textarea"
            :rows="3"
            placeholder="请输入围栏描述（可选）"
            maxlength="500"
          />
        </el-form-item>

        <el-form-item label="围栏类型" prop="fence_type">
          <el-radio-group v-model="fenceForm.fence_type">
            <el-radio label="polygon">多边形</el-radio>
            <el-radio label="circular">圆形</el-radio>
            <el-radio label="corridor">走廊</el-radio>
          </el-radio-group>
        </el-form-item>

        <el-form-item label="围栏用途" prop="purpose">
          <el-select v-model="fenceForm.purpose" placeholder="请选择围栏用途">
            <el-option label="警告区" value="warning" />
            <el-option label="限制区" value="restricted" />
            <el-option label="允许区" value="allowed" />
          </el-select>
        </el-form-item>

        <el-form-item label="最大高度" prop="max_altitude">
          <el-input-number
            v-model="fenceForm.max_altitude"
            :min="0"
            :max="10000"
            :step="10"
            placeholder="米"
          />
          <span class="unit">米</span>
        </el-form-item>

        <el-form-item label="最小高度" prop="min_altitude">
          <el-input-number
            v-model="fenceForm.min_altitude"
            :min="0"
            :max="10000"
            :step="10"
            placeholder="米"
          />
          <span class="unit">米</span>
        </el-form-item>

        <el-form-item label="绘制方式">
          <el-button
            type="primary"
            plain
            @click="startDrawing"
            :disabled="!fenceForm.fence_type"
          >
            <el-icon><EditPen /></el-icon>
            在地图上绘制
          </el-button>
          <span v-if="fenceForm.geometry" class="geometry-info">
            <el-icon color="#67C23A"><CircleCheck /></el-icon>
            已绘制
          </span>
        </el-form-item>
      </el-form>

      <template #footer>
        <el-button @click="createDialogVisible = false">取消</el-button>
        <el-button
          type="primary"
          @click="saveFence"
          :disabled="!fenceForm.geometry"
        >
          保存
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  SetUp,
  Plus,
  Location,
  Edit,
  Delete,
  EditPen,
  CircleCheck
} from '@element-plus/icons-vue'

const props = defineProps({
  mapInstance: {
    type: Object,
    default: null
  }
})

const emit = defineEmits(['geofence-created', 'geofence-updated', 'geofence-deleted', 'start-drawing'])

// 地理围栏列表
const geofences = ref([])

// 选中的围栏
const selectedGeofence = ref(null)

// 对话框可见性
const createDialogVisible = ref(false)

// 编辑中的围栏
const editingFence = ref(null)

// 表单引用
const fenceFormRef = ref(null)

// 围栏表单
const fenceForm = reactive({
  name: '',
  description: '',
  fence_type: 'polygon',
  purpose: 'warning',
  max_altitude: null,
  min_altitude: null,
  geometry: null
})

// 表单验证规则
const fenceRules = {
  name: [
    { required: true, message: '请输入围栏名称', trigger: 'blur' },
    { min: 2, max: 100, message: '长度在 2 到 100 个字符', trigger: 'blur' }
  ],
  fence_type: [
    { required: true, message: '请选择围栏类型', trigger: 'change' }
  ],
  purpose: [
    { required: true, message: '请选择围栏用途', trigger: 'change' }
  ]
}

// 加载地理围栏列表
const loadGeofences = async () => {
  try {
    // 这里应该调用API获取地理围栏列表
    // const response = await geofenceApi.getGeofences()
    // geofences.value = response.data

    // 暂时使用模拟数据
    geofences.value = [
      {
        id: 1,
        name: '北京市中心安全区',
        description: '北京市中心核心区域',
        fence_type: 'polygon',
        purpose: 'restricted',
        status: 'active',
        max_altitude: 500,
        min_altitude: 0,
        geometry: {
          points: [
            { lat: 39.904200, lng: 116.407396 },
            { lat: 39.914200, lng: 116.407396 },
            { lat: 39.914200, lng: 116.417396 },
            { lat: 39.904200, lng: 116.417396 }
          ]
        }
      }
    ]
  } catch (error) {
    console.error('加载地理围栏失败:', error)
    ElMessage.error('加载地理围栏失败')
  }
}

// 选择围栏
const selectGeofence = (fence) => {
  selectedGeofence.value = fence
  // 可以在地图上高亮显示选中的围栏
}

// 切换围栏状态
const toggleFenceStatus = async (fence) => {
  try {
    // 调用API更新围栏状态
    // await geofenceApi.updateStatus(fence.id, fence.status)
    ElMessage.success(`围栏已${fence.status === 'active' ? '激活' : '禁用'}`)
  } catch (error) {
    console.error('更新围栏状态失败:', error)
    ElMessage.error('更新围栏状态失败')
    // 恢复原状态
    fence.status = fence.status === 'active' ? 'inactive' : 'active'
  }
}

// 编辑围栏
const editGeofence = (fence) => {
  editingFence.value = fence
  Object.assign(fenceForm, {
    name: fence.name,
    description: fence.description,
    fence_type: fence.fence_type,
    purpose: fence.purpose,
    max_altitude: fence.max_altitude,
    min_altitude: fence.min_altitude,
    geometry: fence.geometry
  })
  createDialogVisible.value = true
}

// 删除围栏
const deleteGeofence = async (fence) => {
  try {
    await ElMessageBox.confirm(
      `确定要删除围栏 "${fence.name}" 吗？`,
      '删除确认',
      {
        confirmButtonText: '确定',
        cancelButtonText: '取消',
        type: 'warning'
      }
    )

    // 调用API删除围栏
    // await geofenceApi.delete(fence.id)

    const index = geofences.value.findIndex(f => f.id === fence.id)
    if (index > -1) {
      geofences.value.splice(index, 1)
    }

    emit('geofence-deleted', fence)
    ElMessage.success('围栏已删除')
  } catch (error) {
    if (error !== 'cancel') {
      console.error('删除围栏失败:', error)
      ElMessage.error('删除围栏失败')
    }
  }
}

// 开始绘制
const startDrawing = () => {
  emit('start-drawing', {
    type: fenceForm.fence_type,
    callback: (geometry) => {
      fenceForm.geometry = geometry
    }
  })
  createDialogVisible.value = false
}

// 保存围栏
const saveFence = async () => {
  try {
    await fenceFormRef.value.validate()

    const fenceData = {
      name: fenceForm.name,
      description: fenceForm.description,
      fence_type: fenceForm.fence_type,
      purpose: fenceForm.purpose,
      max_altitude: fenceForm.max_altitude,
      min_altitude: fenceForm.min_altitude,
      geometry: fenceForm.geometry
    }

    if (editingFence.value) {
      // 更新围栏
      // await geofenceApi.update(editingFence.value.id, fenceData)
      Object.assign(editingFence.value, fenceData)
      emit('geofence-updated', editingFence.value)
      ElMessage.success('围栏已更新')
    } else {
      // 创建围栏
      // const response = await geofenceApi.create(fenceData)
      const newFence = {
        id: Date.now(),
        status: 'active',
        ...fenceData
      }
      geofences.value.push(newFence)
      emit('geofence-created', newFence)
      ElMessage.success('围栏已创建')
    }

    resetForm()
    createDialogVisible.value = false
  } catch (error) {
    if (error !== 'cancel') {
      console.error('保存围栏失败:', error)
      ElMessage.error('保存围栏失败')
    }
  }
}

// 重置表单
const resetForm = () => {
  editingFence.value = null
  Object.assign(fenceForm, {
    name: '',
    description: '',
    fence_type: 'polygon',
    purpose: 'warning',
    max_altitude: null,
    min_altitude: null,
    geometry: null
  })
  fenceFormRef.value?.resetFields()
}

// 获取类型颜色
const getTypeColor = (purpose) => {
  const colorMap = {
    'warning': '#FFA500',
    'restricted': '#FF0000',
    'allowed': '#00FF00'
  }
  return colorMap[purpose] || '#FFA500'
}

// 获取用途类型
const getPurposeType = (purpose) => {
  const typeMap = {
    'warning': 'warning',
    'restricted': 'danger',
    'allowed': 'success'
  }
  return typeMap[purpose] || 'info'
}

// 获取用途文本
const getPurposeText = (purpose) => {
  const textMap = {
    'warning': '警告区',
    'restricted': '限制区',
    'allowed': '允许区'
  }
  return textMap[purpose] || purpose
}

// 获取类型文本
const getTypeText = (type) => {
  const textMap = {
    'polygon': '多边形',
    'circular': '圆形',
    'corridor': '走廊'
  }
  return textMap[type] || type
}

// 初始化
loadGeofences()

// 暴露方法
defineExpose({
  loadGeofences,
  setGeometry: (geometry) => {
    fenceForm.geometry = geometry
    createDialogVisible.value = true
  }
})
</script>

<style scoped lang="scss">
.geofence-manager {
  .header {
    display: flex;
    justify-content: space-between;
    align-items: center;

    .title {
      display: flex;
      align-items: center;
      gap: 8px;
      font-weight: bold;
    }
  }

  .geofence-list {
    max-height: 600px;
    overflow-y: auto;

    .geofence-item {
      padding: 12px;
      margin-bottom: 12px;
      border: 2px solid #e4e7ed;
      border-radius: 8px;
      cursor: pointer;
      transition: all 0.3s;

      &:hover {
        border-color: #409EFF;
        box-shadow: 0 2px 12px rgba(0, 0, 0, 0.1);
      }

      &.active {
        border-color: #409EFF;
        background-color: #ecf5ff;
      }

      .fence-header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 8px;

        .fence-title {
          display: flex;
          align-items: center;
          gap: 8px;

          .name {
            font-weight: 600;
            font-size: 14px;
          }
        }

        .fence-actions {
          display: flex;
          align-items: center;
          gap: 8px;
        }
      }

      .fence-info {
        display: flex;
        align-items: center;
        gap: 8px;
        margin-bottom: 8px;
        flex-wrap: wrap;

        .description {
          font-size: 12px;
          color: #909399;
        }
      }

      .altitude-info {
        display: flex;
        gap: 16px;
        font-size: 12px;
        color: #606266;

        .altitude {
          display: flex;
          align-items: center;
          gap: 4px;
        }
      }
    }
  }

  .unit {
    margin-left: 8px;
    color: #909399;
  }

  .geometry-info {
    margin-left: 12px;
    color: #67C23A;
    display: flex;
    align-items: center;
    gap: 4px;
  }
}
</style>
