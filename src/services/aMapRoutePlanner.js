/**
 * 高德地图路径规划服务
 * 适配高德地图API实现基于城市主干道的无人机路径规划
 */

class AMapRoutePlanner {
  constructor() {
    // 路线规划策略
    this.planningStrategy = 'main_road'; // main_road, shortest, fastest
    // 缓存已规划的路线
    this.routeCache = new Map();
    // 缓存过期时间(毫秒)
    this.cacheExpiry = 30 * 60 * 1000; // 30分钟
  }

  /**
   * 使用高德地图API规划路线
   * @param {Object} startPoint - 起点坐标 {lng, lat}
   * @param {Object} endPoint - 终点坐标 {lng, lat}
   * @param {Object} options - 规划选项
   * @returns {Promise<Object>} 路线规划结果
   */
  async planRoute(startPoint, endPoint, options = {}) {
    // 检查缓存
    const cacheKey = this.getCacheKey(startPoint, endPoint, options);
    const cached = this.routeCache.get(cacheKey);

    if (cached && (Date.now() - cached.timestamp) < this.cacheExpiry) {
      return cached.data;
    }

    // 检查高德地图API是否可用
    if (!window.AMap || !window.AMap.plugin) {
      throw new Error('高德地图API未加载');
    }

    try {
      // 加载路线规划插件
      await this.loadAMapPlugin('AMap.Driving');

      // 创建路线规划对象
      const driving = new window.AMap.Driving({
        policy: window.AMap.DrivingPolicy.LEAST_DISTANCE, // 最短距离
        ferry: false, // 不乘轮渡
        province: options.province || '京', // 省份
        city: options.city || '北京市', // 城市
        ...options
      });

      // 转换坐标格式
      const startLngLat = new window.AMap.LngLat(startPoint.lng, startPoint.lat);
      const endLngLat = new window.AMap.LngLat(endPoint.lng, endPoint.lat);

      // 返回Promise包装的路线规划结果
      return new Promise((resolve, reject) => {
        driving.search(startLngLat, endLngLat, (status, result) => {
          if (status === 'complete' && result.routes && result.routes.length > 0) {
            // 处理路线结果
            const processedRoute = this.processAMapRoute(result.routes[0], startPoint, endPoint);

            // 缓存结果
            this.routeCache.set(cacheKey, {
              data: processedRoute,
              timestamp: Date.now()
            });

            resolve(processedRoute);
          } else {
            reject(new Error('路线规划失败: ' + (result && result.info) || '未知错误'));
          }
        });
      });
    } catch (error) {
      console.error('路线规划出错:', error);
      throw new Error('路线规划服务不可用: ' + error.message);
    }
  }

  /**
   * 加载高德地图插件
   * @param {string} pluginName - 插件名称
   * @returns {Promise<void>}
   */
  loadAMapPlugin(pluginName) {
    return new Promise((resolve, reject) => {
      if (!window.AMap) {
        reject(new Error('高德地图API未加载'));
        return;
      }

      window.AMap.plugin([pluginName], () => {
        resolve();
      }, (error) => {
        reject(new Error('插件加载失败: ' + error));
      });
    });
  }

  /**
   * 处理高德地图路线结果
   * @param {Object} amapRoute - 高德地图路线数据
   * @param {Object} startPoint - 起点
   * @param {Object} endPoint - 终点
   * @returns {Object} 处理后的路线数据
   */
  processAMapRoute(amapRoute, startPoint, endPoint) {
    const steps = amapRoute.steps || [];
    const routeNodes = [];
    let totalDistance = 0;
    let totalTime = 0;

    // 提取路线节点
    steps.forEach(step => {
      const path = step.path || [];
      path.forEach(point => {
        routeNodes.push({
          lng: point.lng,
          lat: point.lat,
          isMainRoad: true // 高德地图规划的路线默认为主干道
        });
      });

      totalDistance += step.distance || 0;
      totalTime += step.time || 0;
    });

    // 确保起点和终点在路线中
    if (routeNodes.length > 0) {
      // 添加起点（如果不在路线中）
      const firstNode = routeNodes[0];
      const startDistance = this.calculateDistance(startPoint, firstNode);
      if (startDistance > 0.001) { // 1米以上才添加
        routeNodes.unshift({
          lng: startPoint.lng,
          lat: startPoint.lat,
          isMainRoad: true,
          isStart: true
        });
      }

      // 添加终点（如果不在路线中）
      const lastNode = routeNodes[routeNodes.length - 1];
      const endDistance = this.calculateDistance(endPoint, lastNode);
      if (endDistance > 0.001) { // 1米以上才添加
        routeNodes.push({
          lng: endPoint.lng,
          lat: endPoint.lat,
          isMainRoad: true,
          isEnd: true
        });
      }
    }

    return {
      id: 'route_' + Date.now(),
      nodes: routeNodes,
      segments: this.createSegments(routeNodes),
      totalDistance: totalDistance / 1000, // 转换为公里
      totalTime: totalTime / 60, // 转换为分钟
      startPoint: startPoint,
      endPoint: endPoint,
      createTime: new Date().toISOString()
    };
  }

  /**
   * 创建路线段
   * @param {Array} nodes - 路线节点
   * @returns {Array} 路线段
   */
  createSegments(nodes) {
    const segments = [];
    for (let i = 0; i < nodes.length - 1; i++) {
      const from = nodes[i];
      const to = nodes[i + 1];
      const distance = this.calculateDistance(from, to);

      segments.push({
        from: from,
        to: to,
        distance: distance
      });
    }
    return segments;
  }

  /**
   * 计算两点间距离(公里)
   * @param {Object} point1 - 点1 {lng, lat}
   * @param {Object} point2 - 点2 {lng, lat}
   * @returns {number} 距离(公里)
   */
  calculateDistance(point1, point2) {
    if (!window.AMap || !window.AMap.GeometryUtil) {
      // 使用简化的计算方法
      const R = 6371; // 地球半径(公里)
      const dLat = this.toRad(point2.lat - point1.lat);
      const dLon = this.toRad(point2.lng - point1.lng);
      const a = Math.sin(dLat/2) * Math.sin(dLat/2) +
                Math.cos(this.toRad(point1.lat)) * Math.cos(this.toRad(point2.lat)) *
                Math.sin(dLon/2) * Math.sin(dLon/2);
      const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
      return R * c;
    }

    // 使用高德地图的几何工具
    const lngLat1 = new window.AMap.LngLat(point1.lng, point1.lat);
    const lngLat2 = new window.AMap.LngLat(point2.lng, point2.lat);
    return window.AMap.GeometryUtil.distance(lngLat1, lngLat2) / 1000; // 转换为公里
  }

  /**
   * 角度转弧度
   * @param {number} deg - 角度
   * @returns {number} 弧度
   */
  toRad(deg) {
    return deg * Math.PI / 180;
  }

  /**
   * 生成缓存键
   * @param {Object} startPoint - 起点
   * @param {Object} endPoint - 终点
   * @param {Object} options - 选项
   * @returns {string} 缓存键
   */
  getCacheKey(startPoint, endPoint, options) {
    return `${startPoint.lng.toFixed(6)},${startPoint.lat.toFixed(6)}_${endPoint.lng.toFixed(6)},${endPoint.lat.toFixed(6)}_${JSON.stringify(options)}`;
  }

  /**
   * 清除过期缓存
   */
  clearExpiredCache() {
    const now = Date.now();
    for (const [key, value] of this.routeCache) {
      if ((now - value.timestamp) >= this.cacheExpiry) {
        this.routeCache.delete(key);
      }
    }
  }

  /**
   * 获取缓存统计
   * @returns {Object} 缓存统计信息
   */
  getCacheStats() {
    return {
      size: this.routeCache.size,
      expiry: this.cacheExpiry
    };
  }

  /**
   * 清除所有缓存
   */
  clearCache() {
    this.routeCache.clear();
  }
}

// 导出单例实例
const aMapRoutePlanner = new AMapRoutePlanner();
export default aMapRoutePlanner;

// 导出类以供需要多个实例时使用
export { AMapRoutePlanner };