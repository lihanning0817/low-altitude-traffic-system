/**
 * 应急事件相关常量
 * 集中管理所有枚举值、文本映射和颜色配置
 */

// ========== 事件类型枚举 ==========
export const EmergencyType = {
  EQUIPMENT_FAILURE: 'equipment_failure',
  WEATHER_EMERGENCY: 'weather_emergency',
  COLLISION_RISK: 'collision_risk',
  SIGNAL_LOSS: 'signal_loss',
  BATTERY_LOW: 'battery_low',
  GEOFENCE_VIOLATION: 'geofence_violation',
  MANUAL_EMERGENCY: 'manual_emergency',
  OTHER: 'other'
}

// 事件类型文本映射
export const EmergencyTypeText = {
  [EmergencyType.EQUIPMENT_FAILURE]: '设备故障',
  [EmergencyType.WEATHER_EMERGENCY]: '恶劣天气',
  [EmergencyType.COLLISION_RISK]: '碰撞风险',
  [EmergencyType.SIGNAL_LOSS]: '信号丢失',
  [EmergencyType.BATTERY_LOW]: '电量低',
  [EmergencyType.GEOFENCE_VIOLATION]: '电子围栏违规',
  [EmergencyType.MANUAL_EMERGENCY]: '手动紧急',
  [EmergencyType.OTHER]: '其他'
}

// 事件类型颜色映射
export const EmergencyTypeColor = {
  [EmergencyType.EQUIPMENT_FAILURE]: '#F56C6C',
  [EmergencyType.WEATHER_EMERGENCY]: '#E6A23C',
  [EmergencyType.COLLISION_RISK]: '#F56C6C',
  [EmergencyType.SIGNAL_LOSS]: '#909399',
  [EmergencyType.BATTERY_LOW]: '#E6A23C',
  [EmergencyType.GEOFENCE_VIOLATION]: '#F56C6C',
  [EmergencyType.MANUAL_EMERGENCY]: '#409EFF',
  [EmergencyType.OTHER]: '#909399'
}

// 事件类型选项（用于下拉框）
export const EmergencyTypeOptions = [
  { label: EmergencyTypeText[EmergencyType.EQUIPMENT_FAILURE], value: EmergencyType.EQUIPMENT_FAILURE },
  { label: EmergencyTypeText[EmergencyType.WEATHER_EMERGENCY], value: EmergencyType.WEATHER_EMERGENCY },
  { label: EmergencyTypeText[EmergencyType.COLLISION_RISK], value: EmergencyType.COLLISION_RISK },
  { label: EmergencyTypeText[EmergencyType.SIGNAL_LOSS], value: EmergencyType.SIGNAL_LOSS },
  { label: EmergencyTypeText[EmergencyType.BATTERY_LOW], value: EmergencyType.BATTERY_LOW },
  { label: EmergencyTypeText[EmergencyType.GEOFENCE_VIOLATION], value: EmergencyType.GEOFENCE_VIOLATION },
  { label: EmergencyTypeText[EmergencyType.MANUAL_EMERGENCY], value: EmergencyType.MANUAL_EMERGENCY },
  { label: EmergencyTypeText[EmergencyType.OTHER], value: EmergencyType.OTHER }
]

// ========== 严重程度枚举 ==========
export const EmergencySeverity = {
  LOW: 'low',
  MEDIUM: 'medium',
  HIGH: 'high',
  CRITICAL: 'critical'
}

// 严重程度文本映射
export const EmergencySeverityText = {
  [EmergencySeverity.LOW]: '低',
  [EmergencySeverity.MEDIUM]: '中',
  [EmergencySeverity.HIGH]: '高',
  [EmergencySeverity.CRITICAL]: '严重'
}

// 严重程度颜色映射
export const EmergencySeverityColor = {
  [EmergencySeverity.LOW]: '#67C23A',
  [EmergencySeverity.MEDIUM]: '#E6A23C',
  [EmergencySeverity.HIGH]: '#F56C6C',
  [EmergencySeverity.CRITICAL]: '#F56C6C'
}

// 严重程度标签类型（Element Plus）
export const EmergencySeverityTagType = {
  [EmergencySeverity.LOW]: 'success',
  [EmergencySeverity.MEDIUM]: 'warning',
  [EmergencySeverity.HIGH]: 'danger',
  [EmergencySeverity.CRITICAL]: 'danger'
}

// 严重程度选项（用于下拉框）
export const EmergencySeverityOptions = [
  { label: EmergencySeverityText[EmergencySeverity.LOW], value: EmergencySeverity.LOW },
  { label: EmergencySeverityText[EmergencySeverity.MEDIUM], value: EmergencySeverity.MEDIUM },
  { label: EmergencySeverityText[EmergencySeverity.HIGH], value: EmergencySeverity.HIGH },
  { label: EmergencySeverityText[EmergencySeverity.CRITICAL], value: EmergencySeverity.CRITICAL }
]

// ========== 事件状态枚举 ==========
export const EmergencyStatus = {
  ACTIVE: 'active',
  RESPONDING: 'responding',
  RESOLVED: 'resolved',
  CANCELLED: 'cancelled'
}

// 事件状态文本映射
export const EmergencyStatusText = {
  [EmergencyStatus.ACTIVE]: '激活中',
  [EmergencyStatus.RESPONDING]: '响应中',
  [EmergencyStatus.RESOLVED]: '已解决',
  [EmergencyStatus.CANCELLED]: '已取消'
}

// 事件状态颜色映射
export const EmergencyStatusColor = {
  [EmergencyStatus.ACTIVE]: '#F56C6C',
  [EmergencyStatus.RESPONDING]: '#E6A23C',
  [EmergencyStatus.RESOLVED]: '#67C23A',
  [EmergencyStatus.CANCELLED]: '#909399'
}

// 事件状态标签类型（Element Plus）
export const EmergencyStatusTagType = {
  [EmergencyStatus.ACTIVE]: 'danger',
  [EmergencyStatus.RESPONDING]: 'warning',
  [EmergencyStatus.RESOLVED]: 'success',
  [EmergencyStatus.CANCELLED]: 'info'
}

// 事件状态选项（用于下拉框）
export const EmergencyStatusOptions = [
  { label: EmergencyStatusText[EmergencyStatus.ACTIVE], value: EmergencyStatus.ACTIVE },
  { label: EmergencyStatusText[EmergencyStatus.RESPONDING], value: EmergencyStatus.RESPONDING },
  { label: EmergencyStatusText[EmergencyStatus.RESOLVED], value: EmergencyStatus.RESOLVED },
  { label: EmergencyStatusText[EmergencyStatus.CANCELLED], value: EmergencyStatus.CANCELLED }
]

// ========== 工具函数 ==========

/**
 * 获取事件类型文本
 * @param {string} type - 事件类型
 * @returns {string} 文本
 */
export function getEventTypeText(type) {
  return EmergencyTypeText[type] || type
}

/**
 * 获取事件类型颜色
 * @param {string} type - 事件类型
 * @returns {string} 颜色代码
 */
export function getEventTypeColor(type) {
  return EmergencyTypeColor[type] || '#909399'
}

/**
 * 获取严重程度文本
 * @param {string} severity - 严重程度
 * @returns {string} 文本
 */
export function getSeverityText(severity) {
  return EmergencySeverityText[severity] || severity
}

/**
 * 获取严重程度颜色
 * @param {string} severity - 严重程度
 * @returns {string} 颜色代码
 */
export function getSeverityColor(severity) {
  return EmergencySeverityColor[severity] || '#909399'
}

/**
 * 获取严重程度标签类型
 * @param {string} severity - 严重程度
 * @returns {string} Element Plus标签类型
 */
export function getSeverityTagType(severity) {
  return EmergencySeverityTagType[severity] || 'info'
}

/**
 * 获取事件状态文本
 * @param {string} status - 事件状态
 * @returns {string} 文本
 */
export function getStatusText(status) {
  return EmergencyStatusText[status] || status
}

/**
 * 获取事件状态颜色
 * @param {string} status - 事件状态
 * @returns {string} 颜色代码
 */
export function getStatusColor(status) {
  return EmergencyStatusColor[status] || '#909399'
}

/**
 * 获取事件状态标签类型
 * @param {string} status - 事件状态
 * @returns {string} Element Plus标签类型
 */
export function getStatusTagType(status) {
  return EmergencyStatusTagType[status] || 'info'
}
