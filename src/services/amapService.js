/**
 * 高德地图API服务
 * 提供地图初始化、地理围栏、路径规划等功能
 */

class AmapService {
  constructor() {
    this.map = null
    this.AMap = null
    this.isInitialized = false

    // 高德地图API密钥（需要替换为您自己的密钥）
    this.apiKey = '_e2c73fb2b0dbcb9c7f44b3f09c8ff3f1' // 请替换为您的高德地图API Key
    this.apiVersion = '2.0'
    this.plugins = [
      'AMap.Scale',
      'AMap.ToolBar',
      'AMap.ControlBar',
      'AMap.Geolocation',
      'AMap.Geocoder',
      'AMap.Driving',
      'AMap.Walking',
      'AMap.Riding',
      'AMap.DistrictSearch',
      'AMap.PlaceSearch',
      'AMap.MouseTool',
      'AMap.PolyEditor',
      'AMap.CircleEditor'
    ]

    // 地图图层管理
    this.layers = {
      markers: [],
      polylines: [],
      polygons: [],
      circles: [],
      geofences: [],
      noFlyZones: []
    }

    // 编辑器实例
    this.editors = {
      polylineEditor: null,
      polygonEditor: null,
      circleEditor: null
    }
  }

  /**
   * 加载高德地图API
   * @returns {Promise<AMap>}
   */
  async loadAmapScript() {
    if (window.AMap) {
      this.AMap = window.AMap
      this.isInitialized = true
      return this.AMap
    }

    return new Promise((resolve, reject) => {
      const script = document.createElement('script')
      script.type = 'text/javascript'
      script.src = `https://webapi.amap.com/maps?v=${this.apiVersion}&key=${this.apiKey}&plugin=${this.plugins.join(',')}`
      script.async = true

      script.onload = () => {
        if (window.AMap) {
          this.AMap = window.AMap
          this.isInitialized = true
          console.log('高德地图API加载成功')
          resolve(this.AMap)
        } else {
          reject(new Error('高德地图API加载失败'))
        }
      }

      script.onerror = () => {
        reject(new Error('高德地图API脚本加载失败'))
      }

      document.head.appendChild(script)
    })
  }

  /**
   * 初始化地图
   * @param {string} container - 地图容器ID或DOM元素
   * @param {Object} options - 地图配置选项
   * @returns {Promise<AMap.Map>}
   */
  async initMap(container, options = {}) {
    try {
      await this.loadAmapScript()

      const defaultOptions = {
        center: [116.397428, 39.90923], // 北京天安门
        zoom: 13,
        resizeEnable: true,
        mapStyle: 'amap://styles/normal',
        viewMode: '3D', // 使用3D视图
        pitch: 0,
        rotation: 0
      }

      const mapOptions = { ...defaultOptions, ...options }

      this.map = new this.AMap.Map(container, mapOptions)

      // 添加常用控件
      this.addControls()

      console.log('地图初始化成功', this.map)
      return this.map
    } catch (error) {
      console.error('地图初始化失败:', error)
      throw error
    }
  }

  /**
   * 添加地图控件
   */
  addControls() {
    if (!this.map) return

    // 添加缩放控件
    this.map.addControl(new this.AMap.Scale())

    // 添加工具条
    this.map.addControl(new this.AMap.ToolBar({
      position: 'RT'
    }))

    // 添加定位控件
    const geolocation = new this.AMap.Geolocation({
      enableHighAccuracy: true,
      timeout: 10000,
      position: 'RB'
    })
    this.map.addControl(geolocation)
  }

  /**
   * 添加标记点
   * @param {Object} options - 标记选项
   * @returns {AMap.Marker}
   */
  addMarker(options) {
    if (!this.map) {
      console.error('地图未初始化')
      return null
    }

    const markerOptions = {
      map: this.map,
      position: options.position || [116.397428, 39.90923],
      title: options.title || '',
      icon: options.icon || undefined,
      draggable: options.draggable || false,
      ...options
    }

    const marker = new this.AMap.Marker(markerOptions)
    this.layers.markers.push(marker)

    return marker
  }

  /**
   * 绘制路径
   * @param {Array} path - 路径坐标数组
   * @param {Object} options - 路径样式选项
   * @returns {AMap.Polyline}
   */
  drawPolyline(path, options = {}) {
    if (!this.map) {
      console.error('地图未初始化')
      return null
    }

    const polylineOptions = {
      map: this.map,
      path: path,
      strokeColor: options.strokeColor || '#3366FF',
      strokeOpacity: options.strokeOpacity || 0.8,
      strokeWeight: options.strokeWeight || 5,
      strokeStyle: options.strokeStyle || 'solid',
      showDir: options.showDir || true,
      ...options
    }

    const polyline = new this.AMap.Polyline(polylineOptions)
    this.layers.polylines.push(polyline)

    return polyline
  }

  /**
   * 绘制多边形（地理围栏）
   * @param {Array} path - 多边形顶点坐标数组
   * @param {Object} options - 多边形样式选项
   * @returns {AMap.Polygon}
   */
  drawPolygon(path, options = {}) {
    if (!this.map) {
      console.error('地图未初始化')
      return null
    }

    const polygonOptions = {
      map: this.map,
      path: path,
      strokeColor: options.strokeColor || '#FF6600',
      strokeOpacity: options.strokeOpacity || 0.8,
      strokeWeight: options.strokeWeight || 3,
      fillColor: options.fillColor || '#FF6600',
      fillOpacity: options.fillOpacity || 0.2,
      ...options
    }

    const polygon = new this.AMap.Polygon(polygonOptions)
    this.layers.polygons.push(polygon)

    return polygon
  }

  /**
   * 绘制圆形
   * @param {Object} center - 圆心坐标
   * @param {number} radius - 半径（米）
   * @param {Object} options - 圆形样式选项
   * @returns {AMap.Circle}
   */
  drawCircle(center, radius, options = {}) {
    if (!this.map) {
      console.error('地图未初始化')
      return null
    }

    const circleOptions = {
      map: this.map,
      center: center,
      radius: radius,
      strokeColor: options.strokeColor || '#FF6600',
      strokeOpacity: options.strokeOpacity || 0.8,
      strokeWeight: options.strokeWeight || 3,
      fillColor: options.fillColor || '#FF6600',
      fillOpacity: options.fillOpacity || 0.2,
      ...options
    }

    const circle = new this.AMap.Circle(circleOptions)
    this.layers.circles.push(circle)

    return circle
  }

  /**
   * 添加地理围栏
   * @param {Object} geofence - 地理围栏数据
   * @returns {AMap.Polygon|AMap.Circle}
   */
  addGeofence(geofence) {
    let shape = null

    const styleOptions = {
      strokeColor: this.getGeofenceColor(geofence.purpose),
      fillColor: this.getGeofenceColor(geofence.purpose)
    }

    if (geofence.fence_type === 'circular') {
      const center = [geofence.geometry.center.lng, geofence.geometry.center.lat]
      shape = this.drawCircle(center, geofence.geometry.radius, styleOptions)
    } else if (geofence.fence_type === 'polygon') {
      const path = geofence.geometry.points.map(p => [p.lng, p.lat])
      shape = this.drawPolygon(path, styleOptions)
    }

    if (shape) {
      shape.setExtData({ type: 'geofence', data: geofence })
      this.layers.geofences.push(shape)
    }

    return shape
  }

  /**
   * 添加禁飞区
   * @param {Object} zone - 禁飞区数据
   * @returns {AMap.Polygon}
   */
  addNoFlyZone(zone) {
    const path = zone.geometry.points.map(p => [p.lng, p.lat])

    const styleOptions = {
      strokeColor: '#FF0000',
      fillColor: '#FF0000',
      fillOpacity: 0.4
    }

    const polygon = this.drawPolygon(path, styleOptions)

    if (polygon) {
      polygon.setExtData({ type: 'no_fly_zone', data: zone })
      this.layers.noFlyZones.push(polygon)
    }

    return polygon
  }

  /**
   * 路径规划
   * @param {Array} startPoint - 起点坐标 [lng, lat]
   * @param {Array} endPoint - 终点坐标 [lng, lat]
   * @param {Object} options - 规划选项
   * @returns {Promise}
   */
  async planRoute(startPoint, endPoint, options = {}) {
    if (!this.AMap) {
      throw new Error('高德地图API未加载')
    }

    return new Promise((resolve, reject) => {
      const driving = new this.AMap.Driving({
        map: this.map,
        policy: options.policy || this.AMap.DrivingPolicy.LEAST_TIME
      })

      driving.search(startPoint, endPoint, (status, result) => {
        if (status === 'complete') {
          resolve(result)
        } else {
          reject(new Error('路径规划失败'))
        }
      })
    })
  }

  /**
   * 地理编码（地址转坐标）
   * @param {string} address - 地址
   * @returns {Promise}
   */
  async geocode(address) {
    if (!this.AMap) {
      throw new Error('高德地图API未加载')
    }

    return new Promise((resolve, reject) => {
      const geocoder = new this.AMap.Geocoder()

      geocoder.getLocation(address, (status, result) => {
        if (status === 'complete' && result.info === 'OK') {
          resolve(result.geocodes[0])
        } else {
          reject(new Error('地理编码失败'))
        }
      })
    })
  }

  /**
   * 逆地理编码（坐标转地址）
   * @param {Array} lnglat - 坐标 [lng, lat]
   * @returns {Promise}
   */
  async regeocode(lnglat) {
    if (!this.AMap) {
      throw new Error('高德地图API未加载')
    }

    return new Promise((resolve, reject) => {
      const geocoder = new this.AMap.Geocoder()

      geocoder.getAddress(lnglat, (status, result) => {
        if (status === 'complete' && result.info === 'OK') {
          resolve(result.regeocode)
        } else {
          reject(new Error('逆地理编码失败'))
        }
      })
    })
  }

  /**
   * 清除所有标记
   */
  clearMarkers() {
    this.layers.markers.forEach(marker => {
      this.map.remove(marker)
    })
    this.layers.markers = []
  }

  /**
   * 清除所有路径
   */
  clearPolylines() {
    this.layers.polylines.forEach(polyline => {
      this.map.remove(polyline)
    })
    this.layers.polylines = []
  }

  /**
   * 清除所有多边形
   */
  clearPolygons() {
    this.layers.polygons.forEach(polygon => {
      this.map.remove(polygon)
    })
    this.layers.polygons = []
  }

  /**
   * 清除所有图层
   */
  clearAll() {
    this.clearMarkers()
    this.clearPolylines()
    this.clearPolygons()

    this.layers.circles.forEach(circle => {
      this.map.remove(circle)
    })
    this.layers.circles = []
  }

  /**
   * 获取地理围栏颜色
   * @param {string} purpose - 围栏用途
   * @returns {string}
   */
  getGeofenceColor(purpose) {
    const colorMap = {
      'warning': '#FFA500',    // 橙色 - 警告
      'restricted': '#FF0000', // 红色 - 限制
      'allowed': '#00FF00'     // 绿色 - 允许
    }
    return colorMap[purpose] || '#FFA500'
  }

  /**
   * 销毁地图实例
   */
  destroy() {
    if (this.map) {
      this.clearAll()
      this.map.destroy()
      this.map = null
    }
  }
}

// 创建单例实例
const amapService = new AmapService()

export default amapService
