<template>
  <div class="airspace-management">
    <!-- 页面头部 -->
    <div class="page-header">
      <h1 class="page-title">🗺️ 空域管理</h1>
      <p class="page-subtitle">配置和管理城市低空飞行空域权限</p>
    </div>

    <!-- 空域统计卡片 -->
    <div class="stats-grid">
      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">📍</div>
          <div class="stat-info">
            <div class="stat-value">{{ airspaces.length }}</div>
            <div class="stat-label">总空域数</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">⚠️</div>
          <div class="stat-info">
            <div class="stat-value">{{ restrictedAirspaces }}</div>
            <div class="stat-label">受限制空域</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">✅</div>
          <div class="stat-info">
            <div class="stat-value">{{ permittedAirspaces }}</div>
            <div class="stat-label">开放空域</div>
          </div>
        </div>
      </AppleCard>

      <AppleCard class="stat-card">
        <div class="stat-content">
          <div class="stat-icon">🔒</div>
          <div class="stat-info">
            <div class="stat-value">{{ protectedAirspaces }}</div>
            <div class="stat-label">保护区域</div>
          </div>
        </div>
      </AppleCard>
    </div>

    <!-- 筛选和搜索区域 -->
    <AppleCard class="filter-card">
      <div class="filter-content">
        <div class="search-area">
          <input
            v-model="searchKeyword"
            type="text"
            class="apple-search"
            placeholder="🔍 搜索空域名称或ID..."
          />
        </div>

        <div class="filter-controls">
          <select v-model="typeFilter" class="apple-select">
            <option value="">全部类型</option>
            <option value="restricted">受限制</option>
            <option value="permitted">开放</option>
            <option value="protected">保护</option>
            <option value="controlled">控制</option>
            <option value="recreational">娱乐</option>
          </select>

          <AppleButton variant="secondary" @click="refreshAirspaces">
            🔄 刷新
          </AppleButton>

          <AppleButton @click="showCreateDialog = true">
            ➕ 创建空域
          </AppleButton>
        </div>
      </div>
    </AppleCard>

    <!-- 空域列表 -->
    <AppleCard class="airspace-list-card">
      <div class="card-header">
        <h2 class="section-title">空域列表</h2>
        <span class="count-badge">共 {{ filteredAirspaces.length }} 个空域</span>
      </div>

      <div v-if="loading" class="loading-state">
        <div class="loading-spinner"></div>
        <p>加载中...</p>
      </div>

      <div v-else class="airspace-list">
        <div
          v-for="airspace in filteredAirspaces"
          :key="airspace.id"
          class="airspace-item"
          @click="viewDetails(airspace)"
        >
          <div class="airspace-main">
            <div class="airspace-header">
              <h3 class="airspace-name">{{ airspace.name }}</h3>
              <span :class="['type-badge', `type-${airspace.type}`]">
                {{ getTypeName(airspace.type) }}
              </span>
            </div>

            <div class="airspace-meta">
              <div class="meta-item">
                <span class="meta-label">ID:</span>
                <span class="meta-value">{{ airspace.id }}</span>
              </div>
              <div class="meta-item">
                <span class="meta-label">高度:</span>
                <span class="meta-value">{{ airspace.altitude.min }}m - {{ airspace.altitude.max }}m</span>
              </div>
              <div class="meta-item">
                <span class="meta-label">容量:</span>
                <span class="meta-value">{{ airspace.capacity || '无限制' }}</span>
              </div>
              <div class="meta-item">
                <span class="meta-label">状态:</span>
                <span :class="['status-indicator', `status-${airspace.status}`]">
                  <span class="status-dot"></span>
                  {{ getStatusText(airspace.status) }}
                </span>
              </div>
            </div>
          </div>

          <div class="airspace-actions" @click.stop>
            <AppleButton variant="secondary" size="small" @click="viewDetails(airspace)">
              查看
            </AppleButton>
            <AppleButton variant="secondary" size="small" @click="applyPermit(airspace)">
              申请许可
            </AppleButton>
            <button class="delete-button" @click="deleteAirspace(airspace)">
              🗑️
            </button>
          </div>
        </div>

        <div v-if="filteredAirspaces.length === 0" class="empty-state">
          <div class="empty-icon">🗺️</div>
          <p class="empty-text">暂无空域数据</p>
          <AppleButton @click="showCreateDialog = true">创建第一个空域</AppleButton>
        </div>
      </div>
    </AppleCard>

    <!-- 空域详情模态框 -->
    <AppleModal
      v-if="showDetailsDialog"
      title="空域详情"
      @close="showDetailsDialog = false"
    >
      <div class="details-content">
        <div class="detail-section">
          <h3 class="detail-section-title">基本信息</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="detail-label">空域名称</span>
              <span class="detail-value">{{ selectedAirspace?.name }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">空域ID</span>
              <span class="detail-value">{{ selectedAirspace?.id }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">类型</span>
              <span class="detail-value">{{ getDetailedType(selectedAirspace?.type) }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">状态</span>
              <span class="detail-value">{{ getStatusText(selectedAirspace?.status) }}</span>
            </div>
          </div>
        </div>

        <div class="detail-section">
          <h3 class="detail-section-title">飞行参数</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <span class="detail-label">高度范围</span>
              <span class="detail-value">{{ selectedAirspace?.altitude.min }}m - {{ selectedAirspace?.altitude.max }}m</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">容量限制</span>
              <span class="detail-value">{{ selectedAirspace?.capacity || '无限制' }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">最后更新</span>
              <span class="detail-value">{{ formatTime(selectedAirspace?.lastUpdated) }}</span>
            </div>
          </div>
        </div>

        <div class="detail-section">
          <h3 class="detail-section-title">限制条件</h3>
          <div class="restrictions-list">
            <span
              v-for="(restriction, index) in selectedAirspace?.restrictions"
              :key="index"
              class="restriction-tag"
            >
              {{ restriction }}
            </span>
          </div>
        </div>

        <div class="detail-section">
          <h3 class="detail-section-title">描述</h3>
          <p class="description-text">{{ selectedAirspace?.description || '暂无描述' }}</p>
        </div>
      </div>

      <template #footer>
        <AppleButton variant="secondary" @click="showDetailsDialog = false">
          关闭
        </AppleButton>
        <AppleButton @click="editAirspace">
          编辑
        </AppleButton>
      </template>
    </AppleModal>

    <!-- 创建空域模态框 -->
    <AppleModal
      v-if="showCreateDialog"
      title="创建新空域"
      @close="closeCreateDialog"
    >
      <form class="create-form" @submit.prevent="createAirspace">
        <div class="form-group">
          <label class="form-label">空域名称</label>
          <input
            v-model="newAirspace.name"
            type="text"
            class="apple-input"
            placeholder="请输入空域名称"
            required
          />
        </div>

        <div class="form-group">
          <label class="form-label">空域类型</label>
          <select v-model="newAirspace.type" class="apple-select" required>
            <option value="restricted">受限制</option>
            <option value="permitted">开放</option>
            <option value="protected">保护</option>
            <option value="controlled">控制</option>
            <option value="recreational">娱乐</option>
          </select>
        </div>

        <div class="form-row">
          <div class="form-group">
            <label class="form-label">最低高度 (米)</label>
            <input
              v-model.number="newAirspace.altitude.min"
              type="number"
              class="apple-input"
              min="0"
              max="500"
              step="10"
              required
            />
          </div>

          <div class="form-group">
            <label class="form-label">最高高度 (米)</label>
            <input
              v-model.number="newAirspace.altitude.max"
              type="number"
              class="apple-input"
              min="10"
              max="500"
              step="10"
              required
            />
          </div>
        </div>

        <div class="form-group">
          <label class="form-label">容量限制 (0表示无限制)</label>
          <input
            v-model.number="newAirspace.capacity"
            type="number"
            class="apple-input"
            min="0"
            max="100"
          />
        </div>

        <div class="form-group">
          <label class="form-label">状态</label>
          <div class="radio-group">
            <label class="radio-label">
              <input v-model="newAirspace.status" type="radio" value="active" />
              <span>启用</span>
            </label>
            <label class="radio-label">
              <input v-model="newAirspace.status" type="radio" value="inactive" />
              <span>禁用</span>
            </label>
          </div>
        </div>

        <div class="form-group">
          <label class="form-label">描述</label>
          <textarea
            v-model="newAirspace.description"
            class="apple-textarea"
            rows="3"
            placeholder="请输入空域描述"
          ></textarea>
        </div>
      </form>

      <template #footer>
        <AppleButton variant="secondary" @click="closeCreateDialog">
          取消
        </AppleButton>
        <AppleButton @click="createAirspace">
          确定
        </AppleButton>
      </template>
    </AppleModal>

    <!-- 编辑空域模态框 -->
    <AppleModal
      v-if="showEditDialog"
      title="编辑空域"
      @close="showEditDialog = false"
    >
      <form class="create-form" @submit.prevent="saveEditedAirspace">
        <div class="form-group">
          <label class="form-label">空域名称</label>
          <input
            v-model="editedAirspace.name"
            type="text"
            class="apple-input"
            placeholder="请输入空域名称"
            required
          />
        </div>

        <div class="form-group">
          <label class="form-label">空域类型</label>
          <select v-model="editedAirspace.type" class="apple-select" disabled>
            <option value="restricted">受限制</option>
            <option value="permitted">开放</option>
            <option value="protected">保护</option>
            <option value="controlled">控制</option>
            <option value="recreational">娱乐</option>
          </select>
          <p class="form-hint">⚠️ 类型不可修改</p>
        </div>

        <div class="form-row">
          <div class="form-group">
            <label class="form-label">最低高度 (米)</label>
            <input
              v-model.number="editedAirspace.altitude.min"
              type="number"
              class="apple-input"
              min="0"
              max="500"
              step="10"
              required
            />
          </div>

          <div class="form-group">
            <label class="form-label">最高高度 (米)</label>
            <input
              v-model.number="editedAirspace.altitude.max"
              type="number"
              class="apple-input"
              min="10"
              max="500"
              step="10"
              required
            />
          </div>
        </div>

        <div class="form-group">
          <label class="form-label">容量限制</label>
          <input
            v-model.number="editedAirspace.capacity"
            type="number"
            class="apple-input"
            min="0"
            max="100"
          />
        </div>

        <div class="form-group">
          <label class="form-label">状态</label>
          <div class="radio-group">
            <label class="radio-label">
              <input v-model="editedAirspace.status" type="radio" value="active" />
              <span>启用</span>
            </label>
            <label class="radio-label">
              <input v-model="editedAirspace.status" type="radio" value="inactive" />
              <span>禁用</span>
            </label>
          </div>
        </div>

        <div class="form-group">
          <label class="form-label">描述</label>
          <textarea
            v-model="editedAirspace.description"
            class="apple-textarea"
            rows="3"
            placeholder="请输入空域描述"
          ></textarea>
        </div>
      </form>

      <template #footer>
        <AppleButton variant="secondary" @click="showEditDialog = false">
          取消
        </AppleButton>
        <AppleButton @click="saveEditedAirspace">
          保存
        </AppleButton>
      </template>
    </AppleModal>

    <!-- 申请飞行许可模态框 -->
    <AppleModal
      v-if="showPermitDialog"
      title="申请飞行许可"
      @close="showPermitDialog = false"
    >
      <form class="create-form" @submit.prevent="submitPermitApplication">
        <div class="form-group">
          <label class="form-label">空域名称</label>
          <input
            v-model="newPermit.airspaceName"
            type="text"
            class="apple-input"
            readonly
          />
        </div>

        <div class="form-group">
          <label class="form-label">申请人</label>
          <input
            v-model="newPermit.applicant"
            type="text"
            class="apple-input"
            readonly
          />
        </div>

        <div class="form-group">
          <label class="form-label">申请理由</label>
          <textarea
            v-model="newPermit.reason"
            class="apple-textarea"
            rows="3"
            placeholder="请说明申请飞行许可的理由"
            required
          ></textarea>
        </div>

        <div class="form-group">
          <label class="form-label">飞行高度 (米)</label>
          <input
            v-model.number="newPermit.altitude"
            type="number"
            class="apple-input"
            min="0"
            max="500"
            step="10"
            required
          />
        </div>

        <div class="form-group">
          <label class="form-label">预计飞行时长 (分钟)</label>
          <input
            v-model.number="newPermit.duration"
            type="number"
            class="apple-input"
            min="1"
            max="120"
            step="5"
            required
          />
        </div>
      </form>

      <template #footer>
        <AppleButton variant="secondary" @click="showPermitDialog = false">
          取消
        </AppleButton>
        <AppleButton @click="submitPermitApplication">
          提交申请
        </AppleButton>
      </template>
    </AppleModal>

    <!-- Toast 通知 -->
    <Transition name="toast">
      <div v-if="showToast" :class="['toast-notification', toastType]">
        <div class="toast-icon">{{ toastIcon }}</div>
        <div class="toast-message">{{ toastMessage }}</div>
      </div>
    </Transition>

    <!-- 确认对话框 -->
    <Transition name="modal">
      <div v-if="showConfirmDialog" class="modal-overlay" @click="handleConfirmNo">
        <div class="modal-container confirm-dialog" @click.stop>
          <h3 class="modal-title">{{ confirmDialogTitle }}</h3>
          <p class="modal-message">{{ confirmDialogMessage }}</p>
          <div class="modal-actions">
            <AppleButton variant="secondary" @click="handleConfirmNo">取消</AppleButton>
            <AppleButton @click="handleConfirmYes">确定</AppleButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed, reactive } from 'vue'
import AppleCard from '@/components/apple/AppleCard.vue'
import AppleButton from '@/components/apple/AppleButton.vue'
import AppleModal from '@/components/apple/AppleModal.vue'
import { useStore } from 'vuex'

const store = useStore()
const userName = computed(() => store.getters.userName || '当前用户')

// 响应式数据
const loading = ref(false)
const searchKeyword = ref('')
const typeFilter = ref('')
const showDetailsDialog = ref(false)
const showCreateDialog = ref(false)
const showEditDialog = ref(false)
const showPermitDialog = ref(false)

const selectedAirspace = ref(null)

// Toast通知状态
const showToast = ref(false)
const toastMessage = ref('')
const toastType = ref('success')
let toastTimer = null

// 确认对话框状态
const showConfirmDialog = ref(false)
const confirmDialogTitle = ref('')
const confirmDialogMessage = ref('')
const confirmDialogCallback = ref(null)
const newAirspace = reactive({
  name: '',
  type: 'permitted',
  altitude: { min: 0, max: 100 },
  capacity: 0,
  status: 'active',
  restrictions: [],
  description: ''
})

const editedAirspace = reactive({
  id: '',
  name: '',
  type: 'permitted',
  altitude: { min: 0, max: 100 },
  capacity: 0,
  status: 'active',
  restrictions: [],
  description: ''
})

const newPermit = reactive({
  airspaceName: '',
  applicant: '',
  reason: '',
  altitude: 100,
  duration: 30
})

// 模拟数据
const airspaces = ref([
  {
    id: 'A1',
    name: '城区核心空域',
    type: 'restricted',
    altitude: { min: 0, max: 120 },
    capacity: 10,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['禁止商业飞行', '仅限授权无人机'],
    description: '城市中心区域，包含政府机构、主要交通枢纽等重要设施'
  },
  {
    id: 'B2',
    name: '交通枢纽空域',
    type: 'controlled',
    altitude: { min: 0, max: 150 },
    capacity: 20,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['需提前申请', '限速80km/h'],
    description: '火车站、机场等交通枢纽周边空域，交通流量大'
  },
  {
    id: 'C3',
    name: '学校周边空域',
    type: 'protected',
    altitude: { min: 0, max: 60 },
    capacity: 5,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['禁止所有飞行', '紧急情况除外'],
    description: '中小学及幼儿园周边，学生安全优先'
  },
  {
    id: 'D4',
    name: '工业区空域',
    type: 'permitted',
    altitude: { min: 0, max: 100 },
    capacity: 15,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['允许商业飞行', '需佩戴识别标签'],
    description: '工业园区、物流中心等商业活动区域'
  },
  {
    id: 'E5',
    name: '公园空域',
    type: 'recreational',
    altitude: { min: 0, max: 80 },
    capacity: 12,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['允许娱乐飞行', '限高80米'],
    description: '城市公园、绿地等公共休闲区域'
  },
  {
    id: 'F6',
    name: '医院周边空域',
    type: 'protected',
    altitude: { min: 0, max: 100 },
    capacity: 8,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['禁止商业飞行', '紧急情况除外'],
    description: '医疗机构周边，保障医疗救援通道畅通'
  },
  {
    id: 'G7',
    name: '电力设施空域',
    type: 'restricted',
    altitude: { min: 0, max: 150 },
    capacity: 5,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['禁止所有飞行', '紧急情况除外'],
    description: '变电站、输电线路等关键基础设施上方'
  },
  {
    id: 'H8',
    name: '商业区空域',
    type: 'permitted',
    altitude: { min: 0, max: 120 },
    capacity: 18,
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z',
    restrictions: ['允许商业飞行', '需提前申请'],
    description: '购物中心、写字楼等商业密集区域'
  }
])

// 计算属性
const filteredAirspaces = computed(() => {
  let result = airspaces.value

  if (searchKeyword.value) {
    result = result.filter(airspace =>
      airspace.name.toLowerCase().includes(searchKeyword.value.toLowerCase()) ||
      airspace.id.toLowerCase().includes(searchKeyword.value.toLowerCase())
    )
  }

  if (typeFilter.value) {
    result = result.filter(airspace => airspace.type === typeFilter.value)
  }

  return result
})

const restrictedAirspaces = computed(() =>
  airspaces.value.filter(a => a.type === 'restricted').length
)
const permittedAirspaces = computed(() =>
  airspaces.value.filter(a => a.type === 'permitted').length
)
const protectedAirspaces = computed(() =>
  airspaces.value.filter(a => a.type === 'protected').length
)

// Toast通知图标
const toastIcon = computed(() => {
  switch (toastType.value) {
    case 'success': return '✅'
    case 'error': return '❌'
    case 'warning': return '⚠️'
    case 'info': return 'ℹ️'
    default: return '✅'
  }
})

// Toast通知方法
const showToastNotification = (message, type = 'success') => {
  toastMessage.value = message
  toastType.value = type
  showToast.value = true

  if (toastTimer) {
    clearTimeout(toastTimer)
  }

  toastTimer = setTimeout(() => {
    showToast.value = false
  }, 3000)
}

// 确认对话框方法
const showConfirm = (title, message) => {
  return new Promise((resolve) => {
    confirmDialogTitle.value = title
    confirmDialogMessage.value = message
    showConfirmDialog.value = true
    confirmDialogCallback.value = resolve
  })
}

const handleConfirmYes = () => {
  showConfirmDialog.value = false
  if (confirmDialogCallback.value) {
    confirmDialogCallback.value(true)
  }
}

const handleConfirmNo = () => {
  showConfirmDialog.value = false
  if (confirmDialogCallback.value) {
    confirmDialogCallback.value(false)
  }
}

// 方法
const refreshAirspaces = () => {
  loading.value = true
  setTimeout(() => {
    loading.value = false
    showToastNotification('空域列表已刷新', 'success')
  }, 1000)
}

const viewDetails = (airspace) => {
  selectedAirspace.value = { ...airspace }
  showDetailsDialog.value = true
}

const editAirspace = () => {
  Object.assign(editedAirspace, selectedAirspace.value)
  showEditDialog.value = true
  showDetailsDialog.value = false
}

const saveEditedAirspace = () => {
  if (editedAirspace.altitude.min >= editedAirspace.altitude.max) {
    showToastNotification('最低高度不能大于或等于最高高度', 'warning')
    return
  }

  const index = airspaces.value.findIndex(a => a.id === editedAirspace.id)
  if (index !== -1) {
    airspaces.value[index] = { ...editedAirspace }
    showToastNotification('空域信息已更新', 'success')
    showEditDialog.value = false
  }
}

const deleteAirspace = async (airspace) => {
  const confirmed = await showConfirm('确认删除', `确定要删除空域 "${airspace.name}" 吗？`)

  if (confirmed) {
    const index = airspaces.value.findIndex(a => a.id === airspace.id)
    if (index > -1) {
      airspaces.value.splice(index, 1)
      showToastNotification('空域删除成功', 'success')
    }
  }
}

const createAirspace = () => {
  if (!newAirspace.name) {
    showToastNotification('请输入空域名称', 'warning')
    return
  }

  if (newAirspace.altitude.min >= newAirspace.altitude.max) {
    showToastNotification('最低高度不能大于或等于最高高度', 'warning')
    return
  }

  const newId = `A${String(airspaces.value.length + 1).padStart(2, '0')}`

  const airspace = {
    id: newId,
    name: newAirspace.name,
    type: newAirspace.type,
    altitude: { ...newAirspace.altitude },
    capacity: newAirspace.capacity,
    status: newAirspace.status,
    restrictions: [],
    description: newAirspace.description,
    lastUpdated: new Date().toISOString()
  }

  airspaces.value.push(airspace)
  showCreateDialog.value = false
  resetCreateForm()
  showToastNotification('空域创建成功', 'success')
}

const closeCreateDialog = () => {
  showCreateDialog.value = false
  resetCreateForm()
}

const resetCreateForm = () => {
  newAirspace.name = ''
  newAirspace.type = 'permitted'
  newAirspace.altitude = { min: 0, max: 100 }
  newAirspace.capacity = 0
  newAirspace.status = 'active'
  newAirspace.description = ''
}

const applyPermit = (airspace) => {
  newPermit.airspaceName = airspace.name
  newPermit.applicant = userName.value
  showPermitDialog.value = true
}

const submitPermitApplication = () => {
  if (!newPermit.reason) {
    showToastNotification('请输入申请理由', 'warning')
    return
  }

  showToastNotification('飞行许可申请已提交，等待审批', 'success')
  showPermitDialog.value = false
  resetPermitForm()
}

const resetPermitForm = () => {
  newPermit.airspaceName = ''
  newPermit.applicant = ''
  newPermit.reason = ''
  newPermit.altitude = 100
  newPermit.duration = 30
}

const getTypeName = (type) => {
  const nameMap = {
    'restricted': '受限制',
    'permitted': '开放',
    'protected': '保护',
    'controlled': '控制',
    'recreational': '娱乐'
  }
  return nameMap[type] || type
}

const getDetailedType = (type) => {
  const nameMap = {
    'restricted': '受限制空域 (Restricted)',
    'permitted': '开放空域 (Permitted)',
    'protected': '保护空域 (Protected)',
    'controlled': '控制空域 (Controlled)',
    'recreational': '娱乐空域 (Recreational)'
  }
  return nameMap[type] || type
}

const getStatusText = (status) => {
  const textMap = {
    'active': '启用',
    'inactive': '禁用'
  }
  return textMap[status] || status
}

const formatTime = (timestamp) => {
  const date = new Date(timestamp)
  return date.toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit'
  }).replace(/\//g, '-')
}
</script>

<style scoped>
.airspace-management {
  padding: var(--space-6, 24px);
  max-width: 1400px;
  margin: 0 auto;
}

/* 页面头部 */
.page-header {
  margin-bottom: var(--space-8, 32px);
}

.page-title {
  margin: 0 0 var(--space-2, 8px) 0;
  font-size: var(--font-size-3xl, 32px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  letter-spacing: -0.02em;
}

.page-subtitle {
  margin: 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
}

/* 统计卡片 */
.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: var(--space-4, 16px);
  margin-bottom: var(--space-6, 24px);
}

.stat-card {
  padding: var(--space-5, 20px);
  transition: all var(--transition-smooth, 0.3s ease);
}

.stat-card:hover {
  transform: translateY(-2px);
}

.stat-content {
  display: flex;
  align-items: center;
  gap: var(--space-4, 16px);
}

.stat-icon {
  font-size: 40px;
  line-height: 1;
}

.stat-info {
  flex: 1;
}

.stat-value {
  font-size: var(--font-size-2xl, 28px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  line-height: 1.2;
  margin-bottom: var(--space-1, 4px);
}

.stat-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 筛选区域 */
.filter-card {
  padding: var(--space-5, 20px);
  margin-bottom: var(--space-6, 24px);
}

.filter-content {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  flex-wrap: wrap;
}

.search-area {
  flex: 1;
  min-width: 300px;
}

.apple-search {
  width: 100%;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  transition: all var(--transition-input, 0.2s ease);
  background: var(--color-bg-primary, #FFFFFF);
}

.apple-search:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.filter-controls {
  display: flex;
  gap: var(--space-3, 12px);
  align-items: center;
}

.apple-select {
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  background: var(--color-bg-primary, #FFFFFF);
  cursor: pointer;
  transition: all var(--transition-input, 0.2s ease);
  appearance: none;
  background-image: url("data:image/svg+xml,%3Csvg width='12' height='8' viewBox='0 0 12 8' fill='none' xmlns='http://www.w3.org/2000/svg'%3E%3Cpath d='M1 1.5L6 6.5L11 1.5' stroke='%2386868B' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/%3E%3C/svg%3E");
  background-repeat: no-repeat;
  background-position: right 12px center;
  padding-right: 36px;
}

.apple-select:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

/* 空域列表 */
.airspace-list-card {
  padding: var(--space-6, 24px);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: var(--space-5, 20px);
}

.section-title {
  margin: 0;
  font-size: var(--font-size-xl, 20px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.count-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

/* 加载状态 */
.loading-state {
  text-align: center;
  padding: var(--space-10, 40px);
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 3px solid var(--color-bg-secondary, #F5F5F7);
  border-top-color: var(--color-primary, #0071E3);
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin: 0 auto var(--space-4, 16px);
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

/* 空域列表项 */
.airspace-list {
  display: flex;
  flex-direction: column;
  gap: var(--space-3, 12px);
}

.airspace-item {
  padding: var(--space-5, 20px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: var(--space-4, 16px);
  cursor: pointer;
  transition: all var(--transition-smooth, 0.3s ease);
  background: var(--color-bg-primary, #FFFFFF);
}

.airspace-item:hover {
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08);
  transform: translateY(-1px);
}

.airspace-main {
  flex: 1;
  min-width: 0;
}

.airspace-header {
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  margin-bottom: var(--space-3, 12px);
  flex-wrap: wrap;
}

.airspace-name {
  margin: 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.type-badge {
  padding: var(--space-1, 4px) var(--space-3, 12px);
  border-radius: var(--radius-full, 9999px);
  font-size: var(--font-size-xs, 12px);
  font-weight: 500;
}

.type-restricted {
  background: #FEE;
  color: #C00;
}

.type-permitted {
  background: #EFE;
  color: #0A0;
}

.type-protected {
  background: #FFE;
  color: #C60;
}

.type-controlled {
  background: #EEF;
  color: #06C;
}

.type-recreational {
  background: #F5F5F7;
  color: #86868B;
}

.airspace-meta {
  display: flex;
  gap: var(--space-6, 24px);
  flex-wrap: wrap;
}

.meta-item {
  display: flex;
  gap: var(--space-2, 8px);
  font-size: var(--font-size-sm, 14px);
}

.meta-label {
  color: var(--color-text-secondary, #86868B);
}

.meta-value {
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 500;
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

.status-active .status-dot {
  background: #34C759;
}

.status-inactive .status-dot {
  background: #FF9500;
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}

.airspace-actions {
  display: flex;
  gap: var(--space-2, 8px);
  align-items: center;
  flex-shrink: 0;
}

.delete-button {
  padding: var(--space-2, 8px);
  background: none;
  border: none;
  font-size: 18px;
  cursor: pointer;
  opacity: 0.6;
  transition: all var(--transition-smooth, 0.3s ease);
  border-radius: var(--radius-md, 8px);
}

.delete-button:hover {
  opacity: 1;
  background: #FEE;
}

/* 空状态 */
.empty-state {
  text-align: center;
  padding: var(--space-12, 48px) var(--space-6, 24px);
}

.empty-icon {
  font-size: 64px;
  margin-bottom: var(--space-4, 16px);
  opacity: 0.5;
}

.empty-text {
  margin: 0 0 var(--space-6, 24px) 0;
  font-size: var(--font-size-lg, 18px);
  color: var(--color-text-secondary, #86868B);
}

/* 详情内容 */
.details-content {
  padding: var(--space-6, 24px) 0;
}

.detail-section {
  margin-bottom: var(--space-6, 24px);
}

.detail-section:last-child {
  margin-bottom: 0;
}

.detail-section-title {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-lg, 18px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
  padding-bottom: var(--space-2, 8px);
  border-bottom: 1px solid var(--color-border, #D2D2D7);
}

.detail-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: var(--space-4, 16px);
}

.detail-item {
  display: flex;
  flex-direction: column;
  gap: var(--space-1, 4px);
}

.detail-label {
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-secondary, #86868B);
}

.detail-value {
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
  font-weight: 500;
}

.restrictions-list {
  display: flex;
  flex-wrap: wrap;
  gap: var(--space-2, 8px);
}

.restriction-tag {
  padding: var(--space-2, 8px) var(--space-3, 12px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-md, 8px);
  font-size: var(--font-size-sm, 14px);
  color: var(--color-text-primary, #1D1D1F);
}

.description-text {
  margin: 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
  line-height: 1.6;
  padding: var(--space-4, 16px);
  background: var(--color-bg-secondary, #F5F5F7);
  border-radius: var(--radius-md, 8px);
  border-left: 3px solid var(--color-primary, #0071E3);
}

/* 表单样式 */
.create-form {
  padding: var(--space-6, 24px) 0;
}

.form-group {
  margin-bottom: var(--space-5, 20px);
}

.form-label {
  display: block;
  margin-bottom: var(--space-2, 8px);
  font-size: var(--font-size-sm, 14px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.apple-input,
.apple-textarea {
  width: 100%;
  padding: var(--space-3, 12px) var(--space-4, 16px);
  border: 1px solid var(--color-border, #D2D2D7);
  border-radius: var(--radius-lg, 12px);
  font-size: var(--font-size-base, 16px);
  transition: all var(--transition-input, 0.2s ease);
  background: var(--color-bg-primary, #FFFFFF);
  font-family: inherit;
}

.apple-input:focus,
.apple-textarea:focus {
  outline: none;
  border-color: var(--color-primary, #0071E3);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.1);
}

.apple-textarea {
  resize: vertical;
  min-height: 80px;
}

.form-row {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: var(--space-4, 16px);
}

.radio-group {
  display: flex;
  gap: var(--space-4, 16px);
}

.radio-label {
  display: flex;
  align-items: center;
  gap: var(--space-2, 8px);
  cursor: pointer;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-primary, #1D1D1F);
}

.radio-label input[type="radio"] {
  width: 18px;
  height: 18px;
  cursor: pointer;
}

.form-hint {
  margin: var(--space-2, 8px) 0 0 0;
  font-size: var(--font-size-xs, 12px);
  color: var(--color-text-secondary, #86868B);
}

/* 响应式设计 */
@media (max-width: 768px) {
  .airspace-management {
    padding: var(--space-4, 16px);
  }

  .page-title {
    font-size: var(--font-size-2xl, 28px);
  }

  .stats-grid {
    grid-template-columns: repeat(2, 1fr);
  }

  .filter-content {
    flex-direction: column;
    align-items: stretch;
  }

  .search-area {
    min-width: auto;
  }

  .filter-controls {
    flex-direction: column;
  }

  .airspace-item {
    flex-direction: column;
    align-items: stretch;
  }

  .airspace-actions {
    justify-content: flex-end;
  }

  .detail-grid {
    grid-template-columns: 1fr;
  }

  .form-row {
    grid-template-columns: 1fr;
  }
}

@media (max-width: 480px) {
  .stats-grid {
    grid-template-columns: 1fr;
  }

  .airspace-meta {
    flex-direction: column;
    gap: var(--space-2, 8px);
  }
}

/* 深色模式 */
@media (prefers-color-scheme: dark) {
  .page-title {
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .page-subtitle,
  .stat-label,
  .meta-label,
  .detail-label {
    color: var(--color-text-secondary-dark, #A1A1A6);
  }

  .stat-value,
  .airspace-name,
  .section-title,
  .meta-value,
  .detail-value {
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .airspace-item {
    background: var(--color-bg-secondary-dark, #1C1C1E);
    border-color: var(--color-border-dark, #38383A);
  }

  .apple-search,
  .apple-select,
  .apple-input,
  .apple-textarea {
    background: var(--color-bg-secondary-dark, #1C1C1E);
    border-color: var(--color-border-dark, #38383A);
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .count-badge,
  .restriction-tag {
    background: var(--color-bg-tertiary-dark, #2C2C2E);
    color: var(--color-text-primary-dark, #F5F5F7);
  }

  .description-text {
    background: var(--color-bg-tertiary-dark, #2C2C2E);
    color: var(--color-text-secondary-dark, #A1A1A6);
  }
}

/* Toast 通知 */
.toast-notification {
  position: fixed;
  top: var(--space-8, 32px);
  right: var(--space-6, 24px);
  z-index: 10000;
  display: flex;
  align-items: center;
  gap: var(--space-3, 12px);
  padding: var(--space-4, 16px) var(--space-5, 20px);
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-lg, 12px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
  backdrop-filter: blur(20px);
  -webkit-backdrop-filter: blur(20px);
  min-width: 280px;
  max-width: 400px;
}

.toast-icon {
  font-size: 24px;
  line-height: 1;
}

.toast-message {
  flex: 1;
  font-size: var(--font-size-base, 16px);
  font-weight: 500;
  color: var(--color-text-primary, #1D1D1F);
}

.toast-notification.success {
  border-left: 4px solid #34C759;
}

.toast-notification.error {
  border-left: 4px solid #FF3B30;
}

.toast-notification.warning {
  border-left: 4px solid #FF9500;
}

.toast-notification.info {
  border-left: 4px solid #007AFF;
}

/* Toast 动画 */
.toast-enter-active {
  animation: slideInRight 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

.toast-leave-active {
  animation: slideOutRight 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

@keyframes slideInRight {
  from {
    transform: translateX(100%);
    opacity: 0;
  }
  to {
    transform: translateX(0);
    opacity: 1;
  }
}

@keyframes slideOutRight {
  from {
    transform: translateX(0);
    opacity: 1;
  }
  to {
    transform: translateX(100%);
    opacity: 0;
  }
}

/* 模态对话框覆盖层 */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  width: 100vw;
  height: 100vh;
  background: rgba(0, 0, 0, 0.5);
  backdrop-filter: blur(10px);
  -webkit-backdrop-filter: blur(10px);
  z-index: 9998;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: var(--space-4, 16px);
}

.modal-container.confirm-dialog {
  background: var(--color-bg-primary, #FFFFFF);
  border-radius: var(--radius-lg, 12px);
  padding: var(--space-6, 24px);
  max-width: 400px;
  width: 100%;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
}

.modal-container .modal-title {
  margin: 0 0 var(--space-4, 16px) 0;
  font-size: var(--font-size-xl, 24px);
  font-weight: 600;
  color: var(--color-text-primary, #1D1D1F);
}

.modal-container .modal-message {
  margin: 0 0 var(--space-5, 20px) 0;
  font-size: var(--font-size-base, 16px);
  color: var(--color-text-secondary, #86868B);
  line-height: 1.5;
}

.modal-container .modal-actions {
  display: flex;
  gap: var(--space-3, 12px);
  justify-content: flex-end;
}

/* 模态框动画 */
.modal-enter-active, .modal-leave-active {
  transition: opacity 0.3s ease;
}

.modal-enter-active .modal-container,
.modal-leave-active .modal-container {
  transition: transform 0.3s cubic-bezier(0.4, 0.0, 0.2, 1);
}

.modal-enter-from, .modal-leave-to {
  opacity: 0;
}

.modal-enter-from .modal-container {
  transform: scale(0.95) translateY(20px);
}

.modal-leave-to .modal-container {
  transform: scale(0.95) translateY(-20px);
}

/* 深色模式下的Toast和Modal */
@media (prefers-color-scheme: dark) {
  .toast-notification {
    background: rgba(28, 28, 30, 0.95);
  }

  .toast-message {
    color: #F5F5F7;
  }

  .modal-container.confirm-dialog {
    background: #1C1C1E;
  }

  .modal-container .modal-title {
    color: #F5F5F7;
  }

  .modal-container .modal-message {
    color: #98989D;
  }
}
</style>
