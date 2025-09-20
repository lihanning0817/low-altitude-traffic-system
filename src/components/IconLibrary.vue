<template>
  <div class="icon-library">
    <!-- 这是一个图标库组件，用于集中管理所有图标 -->
    <!-- 实际使用中，图标会通过Element Plus的图标组件动态导入 -->
    <!-- 此组件主要用于文档和开发参考 -->
    <div class="icon-category">
      <h3>系统图标库</h3>
      <p>本项目使用的全部图标集合</p>
    </div>
    
    <div class="icons-grid">
      <div
        v-for="icon in icons"
        :key="icon.name"
        class="icon-item"
      >
        <div class="icon-preview">
          <component :is="icon.component" />
        </div>
        <div class="icon-name">
          {{ icon.name }}
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import {
  // 主要图标
  House, DataLine, Map, Path, Location, Sun, Warning, Chart,
  User, Moon, Sun as SunIcon, Bell, Close, Message, 
  // 交通相关
  Pointer, Crop, Refresh, Aim, View, FullScreen, Setting,
  Plus, Minus, Grid, LocationInformation, Position,
  VideoCamera, Radar, Files, CircleCheck, Clock, Info,
  // 其他常用图标
  Search, Edit, Delete, Download, Upload, Export, Import,
  Check, X, Question, Help, Lock, Unlock, Shield, Key,
  Camera, Microphone, Speaker, Volume, Battery, Wifi, Signal,
  Calendar, Time, Link, Code, Settings, Tools, Document
} from '@element-plus/icons-vue'

const icons = [
  // 导航菜单图标
  { name: 'House', component: House },
  { name: 'DataLine', component: DataLine },
  { name: 'Map', component: Map },
  { name: 'Path', component: Path },
  { name: 'Location', component: Location },
  { name: 'Sun', component: Sun },
  { name: 'Warning', component: Warning },
  { name: 'Chart', component: Chart },
  
  // 用户界面图标
  { name: 'User', component: User },
  { name: 'Moon', component: Moon },
  { name: 'Sun', component: SunIcon },
  { name: 'Bell', component: Bell },
  { name: 'Close', component: Close },
  { name: 'Message', component: Message },
  
  // 地图和交通图标
  { name: 'Pointer', component: Pointer },
  { name: 'Crop', component: Crop },
  { name: 'Refresh', component: Refresh },
  { name: 'Aim', component: Aim },
  { name: 'View', component: View },
  { name: 'FullScreen', component: FullScreen },
  { name: 'Setting', component: Setting },
  { name: 'Plus', component: Plus },
  { name: 'Minus', component: Minus },
  { name: 'Grid', component: Grid },
  { name: 'LocationInformation', component: LocationInformation },
  { name: 'Position', component: Position },
  { name: 'VideoCamera', component: VideoCamera },
  { name: 'Radar', component: Radar },
  { name: 'Files', component: Files },
  { name: 'CircleCheck', component: CircleCheck },
  { name: 'Clock', component: Clock },
  { name: 'Info', component: Info },
  
  // 通用UI图标
  { name: 'Search', component: Search },
  { name: 'Edit', component: Edit },
  { name: 'Delete', component: Delete },
  { name: 'Download', component: Download },
  { name: 'Upload', component: Upload },
  { name: 'Export', component: Export },
  { name: 'Import', component: Import },
  { name: 'Check', component: Check },
  { name: 'X', component: X },
  { name: 'Question', component: Question },
  { name: 'Help', component: Help },
  { name: 'Lock', component: Lock },
  { name: 'Unlock', component: Unlock },
  { name: 'Shield', component: Shield },
  { name: 'Key', component: Key },
  { name: 'Camera', component: Camera },
  { name: 'Microphone', component: Microphone },
  { name: 'Speaker', component: Speaker },
  { name: 'Volume', component: Volume },
  { name: 'Battery', component: Battery },
  { name: 'Wifi', component: Wifi },
  { name: 'Signal', component: Signal },
  { name: 'Calendar', component: Calendar },
  { name: 'Time', component: Time },
  { name: 'Link', component: Link },
  { name: 'Code', component: Code },
  { name: 'Settings', component: Settings },
  { name: 'Tools', component: Tools },
  { name: 'Document', component: Document }
]
</script>

<style scoped>
.icon-library {
  padding: 2rem;
}

.icon-category {
  text-align: center;
  margin-bottom: 2rem;
}

.icon-category h3 {
  color: #2c3e50;
  margin-bottom: 8px;
}

.icon-category p {
  color: #7f8c8d;
  font-size: 16px;
}

.icons-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
  gap: 1.5rem;
}

.icon-item {
  text-align: center;
  padding: 1rem;
  border-radius: 8px;
  background: #f5f7fa;
  transition: all 0.3s ease;
}

.icon-item:hover {
  background: #e8eaec;
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

.icon-preview {
  width: 48px;
  height: 48px;
  margin: 0 auto 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #667eea;
  border-radius: 8px;
  color: white;
}

.icon-name {
  font-size: 12px;
  color: #2c3e50;
  font-weight: 500;
  word-wrap: break-word;
  line-height: 1.4;
}
</style>
