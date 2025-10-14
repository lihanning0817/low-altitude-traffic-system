// 🔒 BUG #5修复: 任务状态常量定义
// 统一管理任务状态,避免硬编码中文

/**
 * 任务状态枚举 - 使用英文常量作为后端API值
 */
export const TASK_STATUS = {
  PENDING: 'pending',        // 待执行
  IN_PROGRESS: 'in_progress', // 执行中
  COMPLETED: 'completed',    // 已完成
  CANCELLED: 'cancelled'     // 已取消
}

/**
 * 任务状态中文标签映射
 * 用于前端显示
 */
export const TASK_STATUS_LABELS = {
  [TASK_STATUS.PENDING]: '待执行',
  [TASK_STATUS.IN_PROGRESS]: '执行中',
  [TASK_STATUS.COMPLETED]: '已完成',
  [TASK_STATUS.CANCELLED]: '已取消'
}

/**
 * 任务状态颜色映射
 * 用于UI显示不同状态的颜色标识
 */
export const TASK_STATUS_COLORS = {
  [TASK_STATUS.PENDING]: '#909399',      // 灰色
  [TASK_STATUS.IN_PROGRESS]: '#409EFF',  // 蓝色
  [TASK_STATUS.COMPLETED]: '#67C23A',    // 绿色
  [TASK_STATUS.CANCELLED]: '#F56C6C'     // 红色
}

/**
 * 任务状态Element Plus标签类型映射
 */
export const TASK_STATUS_TAG_TYPES = {
  [TASK_STATUS.PENDING]: 'info',
  [TASK_STATUS.IN_PROGRESS]: 'primary',
  [TASK_STATUS.COMPLETED]: 'success',
  [TASK_STATUS.CANCELLED]: 'danger'
}

/**
 * 获取任务状态的中文标签
 * @param {string} status - 任务状态值
 * @returns {string} 中文标签
 */
export function getTaskStatusLabel(status) {
  return TASK_STATUS_LABELS[status] || status
}

/**
 * 获取任务状态的颜色
 * @param {string} status - 任务状态值
 * @returns {string} 颜色值
 */
export function getTaskStatusColor(status) {
  return TASK_STATUS_COLORS[status] || '#909399'
}

/**
 * 获取任务状态的标签类型
 * @param {string} status - 任务状态值
 * @returns {string} Element Plus标签类型
 */
export function getTaskStatusTagType(status) {
  return TASK_STATUS_TAG_TYPES[status] || 'info'
}

/**
 * 任务状态选项列表
 * 用于下拉框等组件
 */
export const TASK_STATUS_OPTIONS = [
  { value: TASK_STATUS.PENDING, label: TASK_STATUS_LABELS[TASK_STATUS.PENDING] },
  { value: TASK_STATUS.IN_PROGRESS, label: TASK_STATUS_LABELS[TASK_STATUS.IN_PROGRESS] },
  { value: TASK_STATUS.COMPLETED, label: TASK_STATUS_LABELS[TASK_STATUS.COMPLETED] },
  { value: TASK_STATUS.CANCELLED, label: TASK_STATUS_LABELS[TASK_STATUS.CANCELLED] }
]

/**
 * 判断任务状态是否有效
 * @param {string} status - 任务状态值
 * @returns {boolean} 是否有效
 */
export function isValidTaskStatus(status) {
  return Object.values(TASK_STATUS).includes(status)
}
