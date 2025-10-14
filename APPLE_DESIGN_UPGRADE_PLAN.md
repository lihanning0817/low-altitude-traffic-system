# Apple 设计系统升级计划

## 📊 项目概览

**项目目标**: 将低空交通管理系统的所有前端页面升级为 Apple.com 风格的极简设计系统

**设计理念**:
- 纯白色和浅灰色背景 (#FFFFFF, #F5F5F7)
- 蓝色 (#0071E3) 仅用于 CTA 按钮,不作为主色调
- 简洁的卡片布局,充足的留白
- SF Pro Display 字体风格
- 8px 网格间距系统
- 流畅的过渡动画

---

## ✅ 已完成页面 (9/34)

### 核心组件 (4个)
1. ✅ **AppleButton.vue** - Apple 风格按钮组件
2. ✅ **AppleCard.vue** - Apple 风格卡片容器
3. ✅ **AppleInput.vue** - Apple 风格输入框
4. ✅ **AppleModal.vue** - Apple 风格模态框

### 功能页面 (8个)
5. ✅ **LoginPage.vue** - 登录页 (已改造)
6. ✅ **RegisterPage.vue** - 注册页 (已改造)
7. ✅ **OfficialWebsite.vue** - 官网首页 (已改造)
8. ✅ **FlightTaskList.vue** - 飞行任务列表 (已改造)
9. ✅ **FlightPermitManagement.vue** - 飞行许可管理 (新创建)
10. ✅ **EmergencyLandingManagement.vue** - 应急降落点管理 (新创建)
11. ✅ **FlightConflictMonitor.vue** - 飞行冲突监控 (新创建)
12. ✅ **DroneManagement.vue** - 无人机管理 (新创建)

---

## 🔄 待改造页面 (25/34)

### 优先级 P0 - 核心业务页面 (6个)

#### 1. FlightTaskDetail.vue (飞行任务详情) 🔴
**优先级**: P0 - 高优先级
**预估工作量**: 3-4小时
**要替换的组件**: el-descriptions, el-timeline, el-tabs, el-button, el-dialog

#### 2. RoutePlanner.vue (路线规划) 🔴  
**优先级**: P0 - 高优先级
**预估工作量**: 4-5小时
**要替换的组件**: el-form, el-input, el-button, el-select, el-slider

#### 3. AirspaceManagement.vue (空域管理) 🔴
**优先级**: P0 - 高优先级  
**预估工作量**: 3-4小时
**要替换的组件**: el-table(保留+样式覆盖), el-input, el-button, el-dialog, el-select

#### 4. WeatherIntegration.vue (天气集成) 🔴
**优先级**: P0 - 高优先级
**预估工作量**: 3小时
**要替换的组件**: el-card, el-input, el-button, el-icon

#### 5. EmergencyResponse.vue (应急响应) 🔴
**优先级**: P0 - 高优先级
**预估工作量**: 4小时  
**要替换的组件**: el-alert, el-timeline, el-table(保留), el-button, el-dialog

#### 6. NotificationCenter.vue (通知中心) 🔴
**优先级**: P0 - 高优先级
**预估工作量**: 2-3小时
**要替换的组件**: el-badge, el-button, el-popover, el-tabs

---

### 优先级 P1 - 布局和设置页面 (4个)

#### 7. AppLayout.vue (主应用布局) 🟡
**优先级**: P1 - 中优先级
**预估工作量**: 3-4小时
**要替换的组件**: el-menu, el-dropdown, el-avatar, el-breadcrumb

#### 8. AuthLayout.vue (认证布局) 🟡
**优先级**: P1 - 中优先级  
**预估工作量**: 1-2小时
**要替换的组件**: el-card

#### 9. ProfileSettings.vue (个人设置) 🟡
**优先级**: P1 - 中优先级
**预估工作量**: 2-3小时
**要替换的组件**: el-form, el-input, el-button, el-upload, el-switch

#### 10. SystemSettings.vue (系统设置) 🟡
**优先级**: P1 - 中优先级
**预估工作量**: 2-3小时
**要替换的组件**: el-form, el-input, el-button, el-switch, el-table(保留)

---

### 优先级 P2 - 分析和地图组件 (11个)

#### 11. AnalyticsDashboard.vue (数据分析) 🟢
**优先级**: P2 - 低优先级
**预估工作量**: 4-5小时
**要替换的组件**: el-date-picker, el-select, el-button, el-table(保留)

#### 12-16. 地图组件 (5个) 🟢
- MapComponent.vue
- MapToolbar.vue  
- MapHeader.vue
- MapControls.vue
- MapActions.vue
**合计工作量**: 4-5小时

#### 17-22. 工具组件 (6个) 🟢
- SmartCard.vue (删除,已被AppleCard替代)
- LoadingSpinner.vue
- IconLibrary.vue
- ThemeToggle.vue
- BackgroundPattern.vue
- ApiTest.vue
**合计工作量**: 2-3小时

---

## 📋 改造标准流程

1. **读取原文件** - 分析现有代码
2. **替换组件**:
   - el-button → AppleButton
   - el-input → AppleInput
   - el-dialog → AppleModal
   - el-card/SmartCard → AppleCard
   - el-form → 原生form + AppleInput
   - el-select → 自定义select + Apple样式
   - el-table → 保留但用:deep()覆盖样式
3. **更新样式**: 使用CSS变量 (var(--space-*), var(--color-*))
4. **保持业务逻辑** - 不修改JS/TS代码
5. **添加响应式** - 移动端适配 (768px, 480px断点)
6. **深色模式** - 添加@media (prefers-color-scheme: dark)支持

---

## 🎨 设计系统参考

### CSS 变量清单

**间距**: --space-1 (4px) ~ --space-12 (48px)  
**颜色**: --color-bg-primary (#FFFFFF), --color-text-primary (#1D1D1F)
**字体**: --font-size-xs (12px) ~ --font-size-3xl (32px)
**圆角**: --radius-sm (8px) ~ --radius-full (9999px)
**阴影**: --shadow-sm ~ --shadow-xl
**过渡**: --transition-smooth, --transition-input

---

## 🚫 禁止事项

1. ❌ 不要使用蓝紫色渐变作为主色调
2. ❌ 不要使用大面积的品牌色背景  
3. ❌ 不要修改业务逻辑代码
4. ❌ 不要删除现有功能

## ✅ 推荐做法

1. ✅ 使用纯白色和浅灰色背景
2. ✅ 留白充足,内边距至少16px
3. ✅ 卡片阴影轻盈
4. ✅ 使用Emoji图标 (📊🚁✈️📍🔔)
5. ✅ 支持深色模式
6. ✅ 移动端优先

---

## 📈 进度追踪

**总体进度**: 9/34 完成 (26.5%)

- P0 (高优先级): 1/6 完成 (16.7%)
- P1 (中优先级): 0/4 完成 (0%)  
- P2 (低优先级): 0/11 完成 (0%)
- 核心组件: 4/4 完成 (100%)
- 新功能页面: 4/4 完成 (100%)

**预估剩余时间**: 36-47小时

---

## 🎯 下一步行动 (明天任务建议)

1. FlightTaskDetail.vue - 任务详情页
2. RoutePlanner.vue - 路线规划
3. AirspaceManagement.vue - 空域管理
4. WeatherIntegration.vue - 天气集成
5. EmergencyResponse.vue - 应急响应
6. NotificationCenter.vue - 通知中心

---

## 📝 参考资源

- **设计系统**: src/styles/design-system.css
- **组件库**: src/components/apple/
- **最佳实践**:
  - FlightTaskList.vue (表格+卡片布局)
  - DroneManagement.vue (网格布局)
  - FlightConflictMonitor.vue (统计卡片)

---

**最后更新**: 2025-10-14  
**文档版本**: v1.0  
**创建者**: Claude Code Assistant

祝改造顺利! 🚀
