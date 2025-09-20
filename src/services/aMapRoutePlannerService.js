/**
 * 高德地图路径规划服务
 * 提供路径规划、路线渲染、导航等功能
 */

class AMapRoutePlannerService {
  constructor() {
    this.map = null
    this.driving = null
    this.walking = null
    this.transit = null
    this.routePolylines = []
    this.routeMarkers = []
    this.apiKey = '1872806f332dab32a1a3dc895b0ad542'
  }

  /**
   * 初始化路径规划服务
   * @param {Object} map - 高德地图实例
   */
  init(map) {
    this.map = map

    // 初始化不同类型的路径规划服务
    this.driving = new AMap.Driving({
      map: map,
      panel: null, // 不显示默认面板
      hideMarkers: true, // 隐藏默认标记
      showTraffic: false, // 不显示交通状况
      policy: AMap.DrivingPolicy.LEAST_TIME
    })

    this.walking = new AMap.Walking({
      map: map,
      panel: null,
      hideMarkers: true
    })

    this.transit = new AMap.Transfer({
      map: map,
      panel: null,
      hideMarkers: true
    })
  }

  /**
   * 规划驾车路线
   * @param {Array} startPoint - 起点坐标 [lng, lat]
   * @param {Array} endPoint - 终点坐标 [lng, lat]
   * @param {Object} options - 规划选项
   * @returns {Promise}
   */
  planDrivingRoute(startPoint, endPoint, options = {}) {
    return new Promise((resolve, reject) => {
      const {
        waypoints = [],
        policy = AMap.DrivingPolicy.LEAST_TIME,
        restrictions = []
      } = options

      // 设置规划策略
      this.driving.setPolicy(policy)

      // 设置避让选项
      if (restrictions.includes('avoidhighspeed')) {
        this.driving.setAvoidHighSpeed(true)
      }
      if (restrictions.includes('avoidroad')) {
        this.driving.setAvoidRoad(restrictions.avoidroad)
      }

      // 构建搜索参数
      const searchOptions = {}
      if (waypoints.length > 0) {
        searchOptions.waypoints = waypoints.map(wp => new AMap.LngLat(wp[0], wp[1]))
      }

      // 执行路径规划
      this.driving.search(
        new AMap.LngLat(startPoint[0], startPoint[1]),
        new AMap.LngLat(endPoint[0], endPoint[1]),
        searchOptions,
        (status, result) => {
          if (status === 'complete') {
            const routeData = this.parseRouteResult(result, 'driving')
            resolve(routeData)
          } else {
            reject(new Error(`路线规划失败: ${result}`))
          }
        }
      )
    })
  }

  /**
   * 规划步行路线
   * @param {Array} startPoint - 起点坐标
   * @param {Array} endPoint - 终点坐标
   * @returns {Promise}
   */
  planWalkingRoute(startPoint, endPoint) {
    return new Promise((resolve, reject) => {
      this.walking.search(
        new AMap.LngLat(startPoint[0], startPoint[1]),
        new AMap.LngLat(endPoint[0], endPoint[1]),
        (status, result) => {
          if (status === 'complete') {
            const routeData = this.parseRouteResult(result, 'walking')
            resolve(routeData)
          } else {
            reject(new Error(`步行路线规划失败: ${result}`))
          }
        }
      )
    })
  }

  /**
   * 规划公交路线
   * @param {Array} startPoint - 起点坐标
   * @param {Array} endPoint - 终点坐标
   * @param {Object} options - 选项
   * @returns {Promise}
   */
  planTransitRoute(startPoint, endPoint, options = {}) {
    return new Promise((resolve, reject) => {
      const { city = '北京' } = options

      this.transit.search(
        new AMap.LngLat(startPoint[0], startPoint[1]),
        new AMap.LngLat(endPoint[0], endPoint[1]),
        { city },
        (status, result) => {
          if (status === 'complete') {
            const routeData = this.parseRouteResult(result, 'transit')
            resolve(routeData)
          } else {
            reject(new Error(`公交路线规划失败: ${result}`))
          }
        }
      )
    })
  }

  /**
   * 解析路线规划结果
   * @param {Object} result - API返回结果
   * @param {string} type - 路线类型
   * @returns {Object} 格式化的路线数据
   */
  parseRouteResult(result, type) {
    const route = result.routes[0]

    if (!route) {
      throw new Error('未找到有效路线')
    }

    const routeData = {
      type,
      distance: route.distance, // 距离（米）
      time: route.time, // 时间（秒）
      path: [],
      steps: [],
      bounds: null
    }

    // 解析路径点
    if (type === 'driving' || type === 'walking') {
      route.steps.forEach(step => {
        const stepData = {
          instruction: step.instruction,
          distance: step.distance,
          time: step.time,
          path: step.path.map(point => [point.lng, point.lat])
        }
        routeData.steps.push(stepData)
        routeData.path.push(...stepData.path)
      })
    } else if (type === 'transit') {
      // 公交路线解析逻辑
      const segments = route.segments
      segments.forEach(segment => {
        if (segment.transit_mode === 'WALKING') {
          segment.walking.steps.forEach(step => {
            routeData.path.push(...step.path.map(point => [point.lng, point.lat]))
          })
        } else if (segment.transit_mode === 'BUS') {
          segment.bus.buslines.forEach(busline => {
            routeData.path.push(...busline.path.map(point => [point.lng, point.lat]))
          })
        }
      })
    }

    // 计算边界
    routeData.bounds = this.calculateBounds(routeData.path)

    return routeData
  }

  /**
   * 在地图上渲染路线
   * @param {Object} routeData - 路线数据
   * @param {Object} options - 渲染选项
   */
  renderRoute(routeData, options = {}) {
    const {
      strokeColor = '#00AA00',
      strokeWeight = 6,
      strokeOpacity = 0.8,
      strokeStyle = 'solid'
    } = options

    // 清除之前的路线
    this.clearRoute()

    // 创建路线多边形
    const polyline = new AMap.Polyline({
      path: routeData.path,
      strokeColor,
      strokeWeight,
      strokeOpacity,
      strokeStyle,
      lineJoin: 'round'
    })

    this.map.add(polyline)
    this.routePolylines.push(polyline)

    // 调整地图视野
    this.map.setFitView([polyline], false, [20, 20, 20, 20])

    return polyline
  }

  /**
   * 添加路线标记
   * @param {Array} startPoint - 起点
   * @param {Array} endPoint - 终点
   * @param {Array} waypoints - 途经点
   */
  addRouteMarkers(startPoint, endPoint, waypoints = []) {
    // 清除之前的标记
    this.clearMarkers()

    // 起点标记
    if (startPoint) {
      const startMarker = new AMap.Marker({
        position: startPoint,
        icon: new AMap.Icon({
          size: new AMap.Size(25, 34),
          image: 'https://webapi.amap.com/theme/v1.3/markers/n/mark_g.png'
        }),
        title: '起点',
        zIndex: 100
      })
      this.map.add(startMarker)
      this.routeMarkers.push(startMarker)
    }

    // 终点标记
    if (endPoint) {
      const endMarker = new AMap.Marker({
        position: endPoint,
        icon: new AMap.Icon({
          size: new AMap.Size(25, 34),
          image: 'https://webapi.amap.com/theme/v1.3/markers/n/mark_r.png'
        }),
        title: '终点',
        zIndex: 100
      })
      this.map.add(endMarker)
      this.routeMarkers.push(endMarker)
    }

    // 途经点标记
    waypoints.forEach((waypoint, index) => {
      const waypointMarker = new AMap.Marker({
        position: waypoint,
        icon: new AMap.Icon({
          size: new AMap.Size(25, 34),
          image: 'https://webapi.amap.com/theme/v1.3/markers/n/mark_b.png'
        }),
        title: `途经点 ${index + 1}`,
        zIndex: 100
      })
      this.map.add(waypointMarker)
      this.routeMarkers.push(waypointMarker)
    })
  }

  /**
   * 清除路线
   */
  clearRoute() {
    this.routePolylines.forEach(polyline => {
      this.map.remove(polyline)
    })
    this.routePolylines = []
  }

  /**
   * 清除标记
   */
  clearMarkers() {
    this.routeMarkers.forEach(marker => {
      this.map.remove(marker)
    })
    this.routeMarkers = []
  }

  /**
   * 清除所有路线和标记
   */
  clearAll() {
    this.clearRoute()
    this.clearMarkers()
  }

  /**
   * 计算路径边界
   * @param {Array} path - 路径点数组
   * @returns {Object} 边界对象
   */
  calculateBounds(path) {
    if (!path || path.length === 0) return null

    let minLng = path[0][0]
    let maxLng = path[0][0]
    let minLat = path[0][1]
    let maxLat = path[0][1]

    path.forEach(point => {
      const [lng, lat] = point
      minLng = Math.min(minLng, lng)
      maxLng = Math.max(maxLng, lng)
      minLat = Math.min(minLat, lat)
      maxLat = Math.max(maxLat, lat)
    })

    return {
      southwest: new AMap.LngLat(minLng, minLat),
      northeast: new AMap.LngLat(maxLng, maxLat)
    }
  }

  /**
   * 格式化距离
   * @param {number} distance - 距离（米）
   * @returns {string} 格式化后的距离
   */
  formatDistance(distance) {
    if (distance < 1000) {
      return `${Math.round(distance)}米`
    } else {
      return `${(distance / 1000).toFixed(1)}公里`
    }
  }

  /**
   * 格式化时间
   * @param {number} time - 时间（秒）
   * @returns {string} 格式化后的时间
   */
  formatTime(time) {
    const hours = Math.floor(time / 3600)
    const minutes = Math.floor((time % 3600) / 60)

    if (hours > 0) {
      return `${hours}小时${minutes}分钟`
    } else {
      return `${minutes}分钟`
    }
  }

  /**
   * 获取路线策略选项
   * @returns {Array} 策略选项
   */
  getPolicyOptions() {
    return [
      { label: '最快路线', value: AMap.DrivingPolicy.LEAST_TIME },
      { label: '最短路线', value: AMap.DrivingPolicy.LEAST_DISTANCE },
      { label: '避免拥堵', value: AMap.DrivingPolicy.LEAST_FEE },
      { label: '不走高速', value: AMap.DrivingPolicy.REAL_TRAFFIC }
    ]
  }

  /**
   * 销毁服务
   */
  destroy() {
    this.clearAll()

    if (this.driving) {
      this.driving.clear()
      this.driving = null
    }

    if (this.walking) {
      this.walking.clear()
      this.walking = null
    }

    if (this.transit) {
      this.transit.clear()
      this.transit = null
    }

    this.map = null
  }
}

export default new AMapRoutePlannerService()