<template>
  <div class="airspace-management">
    <!-- 页面头部 -->
    <div class="page-header">
      <div class="header-content">
        <div class="title-section">
          <h2>空域管理</h2>
          <p>配置和管理城市低空飞行空域权限</p>
        </div>
        <div class="action-section">
          <el-button
            type="primary"
            size="large"
            @click="showCreateDialog = true"
          >
            <el-icon><Plus /></el-icon>
            创建空域
          </el-button>
        </div>
      </div>
    </div>

    <!-- 空域统计卡片 -->
    <div class="stats-section">
      <div class="stats-grid">
        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Location />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ airspaces.length }}</h3>
            <p>总空域数</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Warning />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ restrictedAirspaces.length }}</h3>
            <p>受限制空域</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <CircleCheck />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ permittedAirspaces.length }}</h3>
            <p>开放空域</p>
          </div>
        </SmartCard>

        <SmartCard
          hover-effect
          bordered
          class="stat-card"
        >
          <div
            class="stat-icon"
            style="background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);"
          >
            <el-icon
              size="24"
              color="#fff"
            >
              <Lock />
            </el-icon>
          </div>
          <div class="stat-content">
            <h3>{{ protectedAirspaces.length }}</h3>
            <p>保护区域</p>
          </div>
        </SmartCard>
      </div>
    </div>
    <!-- 筛选和搜索区域 -->
    <SmartCard
      hover-effect
      class="filter-card"
    >
      <div class="filter-content">
        <div class="search-area">
          <el-input
            :value="searchKeyword"
            placeholder="搜索空域名称或ID..."
            size="large"
            clearable
            class="search-input"
            @input="debouncedSearch"
          >
            <template #prefix>
              <el-icon><Search /></el-icon>
            </template>
          </el-input>
        </div>

        <div class="filter-controls">
          <el-select
            v-model="typeFilter"
            placeholder="按类型筛选"
            size="large"
            clearable
            class="filter-select"
          >
            <el-option
              label="全部类型"
              value=""
            />
            <el-option
              label="受限制"
              value="restricted"
            />
            <el-option
              label="开放"
              value="permitted"
            />
            <el-option
              label="保护"
              value="protected"
            />
            <el-option
              label="控制"
              value="controlled"
            />
            <el-option
              label="娱乐"
              value="recreational"
            />
          </el-select>

          <el-button
            type="primary"
            size="large"
            @click="refreshAirspaces"
          >
            <el-icon><Refresh /></el-icon>
            刷新
          </el-button>
        </div>
      </div>
    </SmartCard>

    <!-- 空域列表 -->
    <SmartCard
      hover-effect
      class="airspace-table-card"
    >
      <template #header>
        <div class="card-header">
          <span class="card-title">空域列表</span>
          <el-tag type="info">
            共 {{ filteredAirspaces.length }} 个空域
          </el-tag>
        </div>
      </template>

      <el-table
        v-loading="loading"
        :data="filteredAirspaces"
        class="airspace-table"
        style="width: 100%"
        @row-click="handleRowClick"
      >
        <el-table-column
          prop="name"
          label="空域名称"
          min-width="200"
        >
          <template #default="{ row }">
            <div class="airspace-name">
              <span class="name">{{ row.name }}</span>
              <span class="id">ID: {{ row.id }}</span>
            </div>
          </template>
        </el-table-column>

        <el-table-column
          prop="type"
          label="类型"
          width="120"
          align="center"
        >
          <template #default="{ row }">
            <el-tag
              :type="getTypeColor(row.type)"
              size="small"
              effect="dark"
            >
              {{ getTypeName(row.type) }}
            </el-tag>
          </template>
        </el-table-column>

        <el-table-column
          prop="altitude.min"
          label="高度范围"
          width="180"
          align="center"
        >
          <template #default="{ row }">
            <div class="altitude-range">
              {{ row.altitude.min }}m - {{ row.altitude.max }}m
            </div>
          </template>
        </el-table-column>

        <el-table-column
          prop="status"
          label="状态"
          width="120"
          align="center"
        >
          <template #default="{ row }">
            <div class="status-indicator">
              <div
                class="status-dot"
                :class="getStatusClass(row.status)"
              />
              <span>{{ getStatusText(row.status) }}</span>
            </div>
          </template>
        </el-table-column>

        <el-table-column
          prop="capacity"
          label="容量"
          width="100"
          align="center"
        >
          <template #default="{ row }">
            <div class="capacity-value">
              {{ row.capacity || '无限制' }}
            </div>
          </template>
        </el-table-column>

        <el-table-column
          prop="lastUpdated"
          label="最后更新"
          width="180"
          sortable
        >
          <template #default="{ row }">
            <div class="time-info">
              <el-icon
                size="14"
                color="#909399"
              >
                <Calendar />
              </el-icon>
              <span>{{ formatTime(row.lastUpdated) }}</span>
            </div>
          </template>
        </el-table-column>

        <el-table-column
          label="操作"
          width="200"
          align="center"
          fixed="right"
        >
          <template #default="{ row }">
            <div class="action-buttons">
              <el-button
                type="primary"
                size="small"
                @click.stop="viewDetails(row)"
              >
                <el-icon><View /></el-icon>
                查看
              </el-button>
              <el-button
                type="success"
                size="small"
                @click.stop="applyPermit(row)"
              >
                <el-icon><Promotion /></el-icon>
                申请许可
              </el-button>
              <el-button
                type="danger"
                size="small"
                @click.stop="deleteAirspace(row)"
              >
                <el-icon><Delete /></el-icon>
              </el-button>
            </div>
          </template>
        </el-table-column>
      </el-table>
    </SmartCard>

    <!-- 空域详情对话框 -->
    <el-dialog
      v-model="showDetailsDialog"
      title="空域详情"
      width="800px"
      :close-on-click-modal="false"
    >
      <div class="details-content">
        <div class="detail-item">
          <span class="label">空域名称:</span>
          <span class="value">{{ selectedAirspace?.name }}</span>
        </div>
        
        <div class="detail-item">
          <span class="label">空域ID:</span>
          <span class="value">{{ selectedAirspace?.id }}</span>
        </div>
        
        <div class="detail-item">
          <span class="label">类型:</span>
          <span class="value">{{ getDetailedType(selectedAirspace?.type) }}</span>
        </div>
        
        <div class="detail-item">
          <span class="label">高度范围:</span>
          <span class="value">{{ selectedAirspace?.altitude.min }}m - {{ selectedAirspace?.altitude.max }}m</span>
        </div>
        
        <div class="detail-item">
          <span class="label">容量:</span>
          <span class="value">{{ selectedAirspace?.capacity || '无限制' }}</span>
        </div>
        
        <div class="detail-item">
          <span class="label">状态:</span>
          <span class="value">{{ getStatusText(selectedAirspace?.status) }}</span>
        </div>
        
        <div class="detail-item">
          <span class="label">最后更新:</span>
          <span class="value">{{ formatTime(selectedAirspace?.lastUpdated) }}</span>
        </div>
        
        <div class="detail-item">
          <span class="label">限制条件:</span>
          <div class="restrictions-list">
            <el-tag
              v-for="(restriction, index) in selectedAirspace?.restrictions"
              :key="index"
              type="warning"
              size="small"
              style="margin-right: 8px; margin-bottom: 8px;"
            >
              {{ restriction }}
            </el-tag>
          </div>
        </div>
        
        <div class="detail-item">
          <span class="label">地理边界:</span>
          <div class="boundary-map">
            <div class="map-preview">
              <!-- 这里可以添加一个简化的边界可视化 -->
              <div
                class="boundary-polygon"
                :style="{ 
                  background: getBoundaryColor(selectedAirspace?.type),
                  opacity: 0.2,
                  border: `2px dashed ${getBoundaryColor(selectedAirspace?.type)}`
                }"
              >
                <div class="polygon-label">
                  {{ selectedAirspace?.name }}
                </div>
              </div>
            </div>
            <p class="map-description">
              在地图上查看精确边界
            </p>
          </div>
        </div>
        
        <div class="detail-item">
          <span class="label">描述:</span>
          <div class="description-text">
            {{ selectedAirspace?.description || '暂无描述' }}
          </div>
        </div>
      </div>
      
      <template #footer>
        <el-button @click="showDetailsDialog = false">
          关闭
        </el-button>
        <el-button
          type="primary"
          @click="editAirspace"
        >
          编辑
        </el-button>
      </template>
    </el-dialog>

    <!-- 创建空域对话框 -->
    <el-dialog
      v-model="showCreateDialog"
      title="创建新空域"
      width="600px"
      @close="resetCreateForm"
    >
      <el-form
        :model="newAirspace"
        label-width="120px"
      >
        <el-form-item label="空域名称">
          <el-input
            v-model="newAirspace.name"
            placeholder="请输入空域名称"
          />
        </el-form-item>
        
        <el-form-item label="空域类型">
          <el-select
            v-model="newAirspace.type"
            placeholder="请选择空域类型"
          >
            <el-option
              label="受限制"
              value="restricted"
            />
            <el-option
              label="开放"
              value="permitted"
            />
            <el-option
              label="保护"
              value="protected"
            />
            <el-option
              label="控制"
              value="controlled"
            />
            <el-option
              label="娱乐"
              value="recreational"
            />
          </el-select>
        </el-form-item>
        
        <el-form-item label="最低高度(m)">
          <el-input-number
            v-model="newAirspace.altitude.min"
            :min="0"
            :max="500"
            :step="10"
            controls-position="right"
            style="width: 100%"
          />
        </el-form-item>
        
        <el-form-item label="最高高度(m)">
          <el-input-number
            v-model="newAirspace.altitude.max"
            :min="10"
            :max="500"
            :step="10"
            controls-position="right"
            style="width: 100%"
          />
        </el-form-item>
        
        <el-form-item label="容量限制">
          <el-input-number
            v-model="newAirspace.capacity"
            :min="0"
            :max="100"
            :step="1"
            controls-position="right"
            style="width: 100%"
            placeholder="0表示无限制"
          />
        </el-form-item>
        
        <el-form-item label="状态">
          <el-radio-group v-model="newAirspace.status">
            <el-radio label="active">
              启用
            </el-radio>
            <el-radio label="inactive">
              禁用
            </el-radio>
          </el-radio-group>
        </el-form-item>
        
        <el-form-item label="限制条件">
          <el-select
            v-model="selectedRestrictions"
            multiple
            placeholder="选择限制条件"
            style="width: 100%"
          >
            <el-option
              label="禁止商业飞行"
              value="禁止商业飞行"
            />
            <el-option
              label="需提前申请"
              value="需提前申请"
            />
            <el-option
              label="仅限授权无人机"
              value="仅限授权无人机"
            />
            <el-option
              label="限速80km/h"
              value="限速80km/h"
            />
            <el-option
              label="禁止夜间飞行"
              value="禁止夜间飞行"
            />
            <el-option
              label="禁止在人口密集区飞行"
              value="禁止在人口密集区飞行"
            />
            <el-option
              label="必须佩戴识别标签"
              value="必须佩戴识别标签"
            />
            <el-option
              label="紧急情况除外"
              value="紧急情况除外"
            />
          </el-select>
        </el-form-item>
        
        <el-form-item label="描述">
          <el-input
            v-model="newAirspace.description"
            type="textarea"
            :rows="3"
            placeholder="请输入空域描述"
          />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <el-button @click="showCreateDialog = false">
          取消
        </el-button>
        <el-button
          type="primary"
          @click="createAirspace"
        >
          确定
        </el-button>
      </template>
    </el-dialog>

    <!-- 编辑空域对话框 -->
    <el-dialog
      v-model="showEditDialog"
      title="编辑空域"
      width="600px"
      @close="resetEditForm"
    >
      <el-form
        :model="editedAirspace"
        label-width="120px"
      >
        <el-form-item label="空域名称">
          <el-input
            v-model="editedAirspace.name"
            placeholder="请输入空域名称"
          />
        </el-form-item>
        
        <el-form-item label="空域类型">
          <el-select
            v-model="editedAirspace.type"
            placeholder="请选择空域类型"
            disabled
          >
            <el-option
              label="受限制"
              value="restricted"
            />
            <el-option
              label="开放"
              value="permitted"
            />
            <el-option
              label="保护"
              value="protected"
            />
            <el-option
              label="控制"
              value="controlled"
            />
            <el-option
              label="娱乐"
              value="recreational"
            />
          </el-select>
          <el-alert
            title="类型不可修改"
            type="warning"
            show-icon
            style="margin-top: 10px;"
          />
        </el-form-item>
        
        <el-form-item label="最低高度(m)">
          <el-input-number
            v-model="editedAirspace.altitude.min"
            :min="0"
            :max="500"
            :step="10"
            controls-position="right"
            style="width: 100%"
          />
        </el-form-item>
        
        <el-form-item label="最高高度(m)">
          <el-input-number
            v-model="editedAirspace.altitude.max"
            :min="10"
            :max="500"
            :step="10"
            controls-position="right"
            style="width: 100%"
          />
        </el-form-item>
        
        <el-form-item label="容量限制">
          <el-input-number
            v-model="editedAirspace.capacity"
            :min="0"
            :max="100"
            :step="1"
            controls-position="right"
            style="width: 100%"
            placeholder="0表示无限制"
          />
        </el-form-item>
        
        <el-form-item label="状态">
          <el-radio-group v-model="editedAirspace.status">
            <el-radio label="active">
              启用
            </el-radio>
            <el-radio label="inactive">
              禁用
            </el-radio>
          </el-radio-group>
        </el-form-item>
        
        <el-form-item label="限制条件">
          <el-select
            v-model="editedRestrictions"
            multiple
            placeholder="选择限制条件"
            style="width: 100%"
          >
            <el-option
              label="禁止商业飞行"
              value="禁止商业飞行"
            />
            <el-option
              label="需提前申请"
              value="需提前申请"
            />
            <el-option
              label="仅限授权无人机"
              value="仅限授权无人机"
            />
            <el-option
              label="限速80km/h"
              value="限速80km/h"
            />
            <el-option
              label="禁止夜间飞行"
              value="禁止夜间飞行"
            />
            <el-option
              label="禁止在人口密集区飞行"
              value="禁止在人口密集区飞行"
            />
            <el-option
              label="必须佩戴识别标签"
              value="必须佩戴识别标签"
            />
            <el-option
              label="紧急情况除外"
              value="紧急情况除外"
            />
          </el-select>
        </el-form-item>
        
        <el-form-item label="描述">
          <el-input
            v-model="editedAirspace.description"
            type="textarea"
            :rows="3"
            placeholder="请输入空域描述"
          />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <el-button @click="showEditDialog = false">
          取消
        </el-button>
        <el-button
          type="primary"
          @click="saveEditedAirspace"
        >
          保存
        </el-button>
      </template>
    </el-dialog>

    <!-- 申请飞行许可对话框 -->
    <el-dialog
      v-model="showPermitDialog"
      title="申请飞行许可"
      width="600px"
      @close="resetPermitForm"
    >
      <el-form
        :model="newPermit"
        label-width="120px"
      >
        <el-form-item label="空域名称">
          <el-input
            v-model="newPermit.airspaceName"
            readonly
          />
        </el-form-item>
        
        <el-form-item label="申请人">
          <el-input
            v-model="newPermit.applicant"
            :placeholder="userName"
            readonly
          />
        </el-form-item>
        
        <el-form-item label="申请理由">
          <el-input
            v-model="newPermit.reason"
            type="textarea"
            :rows="3"
            placeholder="请说明申请飞行许可的理由"
          />
        </el-form-item>
        
        <el-form-item label="计划飞行时间">
          <el-date-picker
            v-model="newPermit.flightTime"
            type="datetimerange"
            range-separator="至"
            start-placeholder="开始时间"
            end-placeholder="结束时间"
            value-format="YYYY-MM-DD HH:mm:ss"
            style="width: 100%"
          />
        </el-form-item>
        
        <el-form-item label="飞行高度">
          <el-input-number
            v-model="newPermit.altitude"
            :min="0"
            :max="500"
            :step="10"
            controls-position="right"
            style="width: 100%"
          />
        </el-form-item>
        
        <el-form-item label="预计飞行时长">
          <el-input-number
            v-model="newPermit.duration"
            :min="1"
            :max="120"
            :step="5"
            controls-position="right"
            style="width: 100%"
            unit="分钟"
          />
        </el-form-item>
      </el-form>
      
      <template #footer>
        <el-button @click="showPermitDialog = false">
          取消
        </el-button>
        <el-button
          type="primary"
          @click="submitPermitApplication"
        >
          提交申请
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, computed, reactive } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import {
  Plus, Location, Warning, CircleCheck, Search, Refresh,
  View, Promotion, Delete, Calendar, Lock
} from '@element-plus/icons-vue'
import SmartCard from '@/components/SmartCard.vue'
import { useStore } from 'vuex'

const store = useStore()
const userName = computed(() => store.getters.userName)

// 防抖函数
const debounce = (func, wait) => {
  let timeout
  return function executedFunction(...args) {
    const later = () => {
      clearTimeout(timeout)
      func(...args)
    }
    clearTimeout(timeout)
    timeout = setTimeout(later, wait)
  }
}

// 响应式数据
const loading = ref(false)
const searchKeyword = ref('')
const typeFilter = ref('')
const showDetailsDialog = ref(false)
const showCreateDialog = ref(false)
const showEditDialog = ref(false)
const showPermitDialog = ref(false)

const selectedAirspace = ref(null)
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

const editedRestrictions = ref([])
const selectedRestrictions = ref([])

const newPermit = reactive({
  airspaceName: '',
  applicant: '',
  reason: '',
  flightTime: [],
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

const restrictedAirspaces = computed(() => airspaces.value.filter(a => a.type === 'restricted').length)
const permittedAirspaces = computed(() => airspaces.value.filter(a => a.type === 'permitted').length)
const protectedAirspaces = computed(() => airspaces.value.filter(a => a.type === 'protected').length)

// 方法
const debouncedSearch = debounce((value) => {
  searchKeyword.value = value
}, 300)

const refreshAirspaces = () => {
  loading.value = true
  // 模拟API调用
  setTimeout(() => {
    loading.value = false
    ElMessage.success('空域列表已刷新')
  }, 1000)
}

const handleRowClick = (row) => {
  viewDetails(row)
}

const viewDetails = (airspace) => {
  selectedAirspace.value = { ...airspace }
  showDetailsDialog.value = true
}

const editAirspace = () => {
  editedAirspace.value = { ...selectedAirspace.value }
  editedRestrictions.value = [...selectedAirspace.value.restrictions]
  showEditDialog.value = true
  showDetailsDialog.value = false
}

const saveEditedAirspace = () => {
  const index = airspaces.value.findIndex(a => a.id === editedAirspace.value.id)
  if (index !== -1) {
    airspaces.value.splice(index, 1, { ...editedAirspace.value, restrictions: editedRestrictions.value })
    ElMessage.success('空域信息已更新')
    showEditDialog.value = false
  }
}

const deleteAirspace = async (airspace) => {
  try {
    await ElMessageBox.confirm(`确定要删除空域 "${airspace.name}" 吗？`, '确认删除', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'error'
    })

    const index = airspaces.value.findIndex(a => a.id === airspace.id)
    if (index > -1) {
      airspaces.value.splice(index, 1)
      ElMessage.success('空域删除成功')
    }
  } catch {
    // 用户取消操作
  }
}

const createAirspace = () => {
  if (!newAirspace.name) {
    ElMessage.warning('请输入空域名称')
    return
  }

  if (newAirspace.altitude.min >= newAirspace.altitude.max) {
    ElMessage.warning('最低高度不能大于或等于最高高度')
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
    restrictions: [...selectedRestrictions.value],
    description: newAirspace.description,
    lastUpdated: new Date().toISOString()
  }

  airspaces.value.push(airspace)
  showCreateDialog.value = false
  resetCreateForm()
  ElMessage.success('空域创建成功')
}

const resetCreateForm = () => {
  newAirspace.name = ''
  newAirspace.type = 'permitted'
  newAirspace.altitude = { min: 0, max: 100 }
  newAirspace.capacity = 0
  newAirspace.status = 'active'
  selectedRestrictions.value = []
  newAirspace.description = ''
}

const applyPermit = (airspace) => {
  newPermit.airspaceName = airspace.name
  newPermit.applicant = userName.value
  showPermitDialog.value = true
}

const submitPermitApplication = () => {
  if (!newPermit.reason) {
    ElMessage.warning('请输入申请理由')
    return
  }

  if (!newPermit.flightTime || newPermit.flightTime.length !== 2) {
    ElMessage.warning('请选择飞行时间')
    return
  }

  // 模拟提交申请
  ElMessage.success('飞行许可申请已提交，等待审批')
  showPermitDialog.value = false
  resetPermitForm()
}

const resetPermitForm = () => {
  newPermit.airspaceName = ''
  newPermit.applicant = ''
  newPermit.reason = ''
  newPermit.flightTime = []
  newPermit.altitude = 100
  newPermit.duration = 30
}

const getTypeColor = (type) => {
  const colorMap = {
    'restricted': 'danger',
    'permitted': 'success',
    'protected': 'warning',
    'controlled': 'info',
    'recreational': 'primary'
  }
  return colorMap[type] || 'info'
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
    'restricted': '受限制空域（Restricted）',
    'permitted': '开放空域（Permitted）',
    'protected': '保护空域（Protected）',
    'controlled': '控制空域（Controlled）',
    'recreational': '娱乐空域（Recreational）'
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

const getStatusClass = (status) => {
  const classMap = {
    'active': 'status-active',
    'inactive': 'status-inactive'
  }
  return classMap[status] || ''
}

const getBoundaryColor = (type) => {
  const colorMap = {
    'restricted': '#f56c6c',
    'permitted': '#67c23a',
    'protected': '#e6a23c',
    'controlled': '#409eff',
    'recreational': '#909399'
  }
  return colorMap[type] || '#909399'
}

const formatTime = (timestamp) => {
  const date = new Date(timestamp)
  return date.toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  }).replace(/\//g, '-')
}
</script>

<style scoped>
.airspace-management {
  min-height: 100%;
}

/* 页面头部 */
.page-header {
  margin-bottom: 24px;
}

.header-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 24px 0;
}

.title-section h2 {
  margin: 0 0 8px 0;
  font-size: 28px;
  font-weight: 600;
  color: #2c3e50;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.title-section p {
  margin: 0;
  color: #7f8c8d;
  font-size: 16px;
}

/* 统计卡片 */
.stats-section {
  margin-bottom: 24px;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 20px;
}

.stat-card {
  padding: 24px;
  display: flex;
  align-items: center;
  gap: 16px;
  transition: all 0.3s ease;
}

.stat-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 8px 30px rgba(0, 0, 0, 0.12);
}

.stat-icon {
  width: 60px;
  height: 60px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.stat-content h3 {
  margin: 0 0 4px 0;
  font-size: 28px;
  font-weight: 700;
  color: #2c3e50;
}

.stat-content p {
  margin: 0;
  font-size: 14px;
  color: #7f8c8d;
  font-weight: 500;
}

/* 筛选区域 */
.filter-card {
  margin-bottom: 24px;
}

.filter-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 20px;
}

.search-area {
  flex: 1;
  max-width: 400px;
}

.search-input {
  border-radius: 12px;
}

.filter-controls {
  display: flex;
  gap: 12px;
}

.filter-select {
  width: 150px;
}

/* 空域列表 */
.airspace-table-card {
  margin-bottom: 24px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-title {
  font-size: 18px;
  font-weight: 600;
  color: #2c3e50;
}

.airspace-table {
  border-radius: 12px;
}

.airspace-name .name {
  display: block;
  font-weight: 600;
  color: #2c3e50;
  margin-bottom: 4px;
}

.airspace-name .id {
  display: block;
  font-size: 12px;
  color: #909399;
}

.altitude-range {
  font-weight: 500;
  color: #2c3e50;
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: 8px;
  justify-content: center;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

.status-active {
  background: #67c23a;
}

.status-inactive {
  background: #e6a23c;
}

@keyframes pulse {
  0% { opacity: 1; }
  50% { opacity: 0.5; }
  100% { opacity: 1; }
}

.capacity-value {
  font-weight: 500;
  color: #2c3e50;
}

.time-info {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 13px;
  color: #909399;
}

.action-buttons {
  display: flex;
  gap: 8px;
  justify-content: center;
}

/* 空域详情对话框 */
.details-content {
  padding: 20px 0;
  line-height: 1.8;
}

.detail-item {
  margin-bottom: 20px;
}

.label {
  font-weight: 600;
  color: #2c3e50;
  display: inline-block;
  width: 120px;
}

.value {
  color: #606266;
  font-size: 15px;
}

.restrictions-list {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-top: 8px;
}

.boundary-map {
  margin-top: 10px;
  padding: 15px;
  border: 1px dashed #e8eaec;
  border-radius: 8px;
  background-color: #fafbfc;
}

.map-preview {
  width: 100%;
  height: 150px;
  border-radius: 8px;
  position: relative;
  overflow: hidden;
  margin-bottom: 10px;
}

.boundary-polygon {
  width: 100%;
  height: 100%;
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #606266;
  font-size: 14px;
  font-weight: 500;
}

.polygon-label {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  text-align: center;
  white-space: nowrap;
  padding: 5px 10px;
  background-color: rgba(255, 255, 255, 0.8);
  border-radius: 4px;
}

.map-description {
  font-size: 13px;
  color: #909399;
  text-align: center;
}

.description-text {
  color: #606266;
  font-size: 15px;
  line-height: 1.6;
  padding: 10px;
  background-color: #f8f9fa;
  border-radius: 8px;
  border-left: 4px solid #667eea;
}

/* 创建/编辑对话框 */
.el-dialog__body {
  padding: 20px;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .header-content {
    flex-direction: column;
    gap: 16px;
    align-items: flex-start;
  }

  .filter-content {
    flex-direction: column;
    gap: 16px;
  }

  .search-area,
  .filter-controls {
    width: 100%;
  }

  .filter-controls {
    justify-content: space-between;
  }

  .stats-grid {
    grid-template-columns: 1fr;
  }

  .action-buttons {
    flex-direction: column;
  }

  .details-content {
    padding: 15px 0;
  }

  .detail-item {
    margin-bottom: 15px;
  }

  .label {
    width: 100px;
  }

  .value {
    font-size: 14px;
  }
}

/* Element Plus 样式覆盖 */
:deep(.el-card__body) {
  padding: 24px;
}

:deep(.el-table__row:hover) {
  background-color: #f8f9fa;
  cursor: pointer;
}

:deep(.el-table__header) {
  background-color: #fafbfc;
}

:deep(.el-input__wrapper) {
  border-radius: 12px;
  transition: all 0.3s ease;
}

:deep(.el-input__wrapper:hover) {
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.1);
}

:deep(.el-button) {
  border-radius: 8px;
  font-weight: 500;
}

:deep(.el-date-editor) {
  border-radius: 12px;
}

:deep(.el-select) {
  border-radius: 12px;
}

:deep(.el-radio-group) {
  display: flex;
  gap: 20px;
}

:deep(.el-radio) {
  margin-right: 20px;
}

:deep(.el-alert) {
  margin-bottom: 15px;
}

:deep(.el-message-box__content) {
  padding: 20px;
  }
  </style>
