# 高德地图 v2.0 兼容性修复说明

## 问题描述

原项目使用的高德地图 v1.4.15 中的 `map.lngLatToPixel()` 方法在 v2.0 版本中已被移除，导致双击地图时出现以下错误：

```
map.lngLatToPixel is not a function
```

## 修复方案

### 1. 更新API版本
**文件**: `public/index.html`

```html
<!-- 修改前 -->
<script src="https://webapi.amap.com/maps?v=1.4.15&key=1872806f332dab32a1a3dc895b0ad542&plugin=..."></script>

<!-- 修改后 -->
<script src="https://webapi.amap.com/maps?v=2.0&key=1872806f332dab32a1a3dc895b0ad542&plugin=..."></script>
```

### 2. 替换废弃方法
**文件**: `src/components/MapComponent.vue`

```javascript
// 修改前 (v1.4.15)
const showContextMenu = (e) => {
  contextMenu.lnglat = e.lnglat
  const pixel = map.lngLatToPixel(e.lnglat)  // ❌ 已废弃
  contextMenu.x = pixel.x
  contextMenu.y = pixel.y
  // ...
}

// 修改后 (v2.0)
const showContextMenu = (e) => {
  contextMenu.lnglat = e.lnglat
  const pixel = map.lngLatToContainer(e.lnglat)  // ✅ 推荐方法
  contextMenu.x = pixel.x
  contextMenu.y = pixel.y
  // ...
}
```

### 3. 更新动态加载配置
**文件**: `src/components/MapComponent.vue`

```javascript
// 修改前
script.src = `https://webapi.amap.com/maps?v=1.4.15&key=...&plugin=AMap.Driving`

// 修改后
script.src = `https://webapi.amap.com/maps?v=2.0&key=...&plugin=AMap.Driving`
```

## API 方法对比

| v1.4.15 (已废弃) | v2.0 (推荐) | 说明 |
|------------------|------------|------|
| `map.lngLatToPixel(lnglat)` | `map.lngLatToContainer(lnglat)` | 经纬度坐标转换为屏幕像素坐标 |
| `map.pixelToLngLat(pixel)` | `map.containerToLngLat(pixel)` | 屏幕像素坐标转换为经纬度坐标 |

## 测试验证

### 功能测试步骤
1. 访问 `http://localhost:8087/map`
2. 双击地图任意位置
3. 确认弹出右键菜单
4. 选择"设置为起点"/"设置为终点"
5. 点击"规划路线"按钮
6. 确认绿色路线正常渲染

### 预期结果
- ✅ 双击地图不再报错
- ✅ 右键菜单正常显示
- ✅ 坐标设置功能正常
- ✅ 路径规划功能正常
- ✅ 路线渲染正常

## 兼容性说明

### 高德地图 v2.0 主要变化
1. **坐标转换方法重命名**
   - `lngLatToPixel` → `lngLatToContainer`
   - `pixelToLngLat` → `containerToLngLat`

2. **API 加载方式**
   - 保持向后兼容
   - 推荐使用 v2.0 获得更好性能和新特性

3. **插件支持**
   - 所有核心插件（Driving, Walking, Transfer等）继续支持
   - UI组件库保持兼容

### 浏览器兼容性
- Chrome 70+
- Firefox 65+
- Safari 12+
- Edge 79+

## 注意事项

1. **API Key 使用**
   - 当前使用的测试 Key: `1872806f332dab32a1a3dc895b0ad542`
   - 生产环境建议申请正式版 API Key

2. **缓存清理**
   - 修复后建议清理浏览器缓存
   - 或使用硬刷新 (Ctrl+F5)

3. **错误监控**
   - 建议添加 JavaScript 错误监控
   - 及时发现 API 兼容性问题

## 修复验证

项目现已成功修复并运行在：
- **本地地址**: http://localhost:8087/
- **网络地址**: http://192.168.181.172:8087/

所有路径规划功能现在都可以正常使用！

## 相关文档

- [高德地图 JavaScript API v2.0 文档](https://lbs.amap.com/api/javascript-api/summary)
- [v1.4到v2.0升级指南](https://lbs.amap.com/api/javascript-api/guide/abc/upgrade)
- [坐标转换方法说明](https://lbs.amap.com/api/javascript-api/reference/map#m_AMap.Map.lngLatToContainer)