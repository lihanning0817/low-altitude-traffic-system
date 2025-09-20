/**
 * 高德地图交通管理服务
 * 集成高德地图API实现实时交通监控和管理
 */

class AMapTrafficManagement {
  constructor() {
    // 活跃飞行器
    this.activeFlights = new Map();
    // 交通事件
    this.trafficEvents = [];
    // 实时交通数据
    this.realTimeTraffic = null;
    // 交通图层
    this.trafficLayer = null;
  }

  /**
   * 初始化交通管理服务
   * @param {Object} map - 高德地图实例
   */
  init(map) {
    if (!map || !window.AMap) {
      throw new Error('高德地图实例未提供或API未加载');
    }

    this.map = map;

    // 初始化交通图层
    this.initTrafficLayer();

    // 初始化实时交通数据获取
    this.startRealTimeTrafficUpdate();
  }

  /**
   * 初始化交通图层
   */
  initTrafficLayer() {
    if (!this.map || !window.AMap) return;

    // 创建交通路况图层
    this.trafficLayer = new window.AMap.TileLayer.Traffic({
      zIndex: 10,
      opacity: 0.8
    });

    // 添加到地图
    this.map.add(this.trafficLayer);
  }

  /**
   * 开始实时交通数据更新
   */
  startRealTimeTrafficUpdate() {
    // 每30秒更新一次交通数据
    this.trafficUpdateInterval = setInterval(() => {
      this.updateRealTimeTraffic();
    }, 30000);
  }

  /**
   * 更新实时交通数据
   */
  async updateRealTimeTraffic() {
    if (!this.map || !window.AMap) return;

    try {
      // 获取当前地图视野范围
      const bounds = this.map.getBounds();

      // 这里应该调用实际的API获取交通数据
      // 暂时使用模拟数据
      this.realTimeTraffic = this.generateMockTrafficData(bounds);

      // 触发交通数据更新事件
      this.onTrafficDataUpdate(this.realTimeTraffic);
    } catch (error) {
      console.error('更新实时交通数据失败:', error);
    }
  }

  /**
   * 生成模拟交通数据
   * @param {Object} bounds - 地图边界
   * @returns {Object} 交通数据
   */
  generateMockTrafficData(bounds) {
    // 生成模拟的交通拥堵区域
    const congestionAreas = [];
    const roadCount = Math.floor(Math.random() * 5) + 3;

    for (let i = 0; i < roadCount; i++) {
      // 随机生成拥堵路段
      const southWest = bounds.getSouthWest();
      const northEast = bounds.getNorthEast();

      const lng = southWest.lng + Math.random() * (northEast.lng - southWest.lng);
      const lat = southWest.lat + Math.random() * (northEast.lat - southWest.lat);

      congestionAreas.push({
        position: [lng, lat],
        level: Math.floor(Math.random() * 4) + 1, // 1-4级拥堵
        speed: Math.floor(Math.random() * 30) + 10, // 10-40 km/h
        description: ['畅通', '缓慢', '拥堵', '严重拥堵'][Math.floor(Math.random() * 4)]
      });
    }

    return {
      timestamp: new Date().toISOString(),
      congestionAreas: congestionAreas,
      overallStatus: 'normal'
    };
  }

  /**
   * 注册飞行器
   * @param {Object} flight - 飞行器信息
   */
  registerFlight(flight) {
    if (!flight.id || !flight.position) {
      throw new Error('飞行器信息不完整');
    }

    // 创建飞行器标记
    const marker = this.createFlightMarker(flight);

    this.activeFlights.set(flight.id, {
      ...flight,
      marker: marker,
      registrationTime: new Date()
    });

    return {
      success: true,
      flightId: flight.id,
      message: '飞行器注册成功'
    };
  }

  /**
   * 创建飞行器标记
   * @param {Object} flight - 飞行器信息
   * @returns {Object} 高德地图标记
   */
  createFlightMarker(flight) {
    if (!this.map || !window.AMap) return null;

    const marker = new window.AMap.Marker({
      position: new window.AMap.LngLat(flight.position[0], flight.position[1]),
      title: flight.name || flight.id,
      icon: this.getFlightIcon(flight.type),
      offset: new window.AMap.Pixel(-16, -16)
    });

    // 添加点击事件
    marker.on('click', () => {
      this.onFlightMarkerClick(flight);
    });

    // 添加到地图
    this.map.add(marker);

    return marker;
  }

  /**
   * 获取飞行器图标
   * @param {string} type - 飞行器类型
   * @returns {Object} 图标配置
   */
  getFlightIcon(type) {
    // 实际项目中应该使用真实的图标
    return new window.AMap.Icon({
      size: new window.AMap.Size(32, 32),
      image: type === 'drone' ? '/icons/drone.png' : '/icons/default-uav.png',
      imageSize: new window.AMap.Size(32, 32)
    });
  }

  /**
   * 更新飞行器位置
   * @param {string} flightId - 飞行器ID
   * @param {Array} position - 新位置 [lng, lat]
   */
  updateFlightPosition(flightId, position) {
    const flight = this.activeFlights.get(flightId);
    if (!flight) {
      throw new Error(`飞行器 ${flightId} 未注册`);
    }

    if (flight.marker && window.AMap) {
      flight.marker.setPosition(new window.AMap.LngLat(position[0], position[1]));
    }

    flight.position = position;
    flight.lastUpdate = new Date();

    return {
      success: true,
      message: '位置更新成功'
    };
  }

  /**
   * 移除飞行器
   * @param {string} flightId - 飞行器ID
   */
  removeFlight(flightId) {
    const flight = this.activeFlights.get(flightId);
    if (!flight) return false;

    // 从地图移除标记
    if (flight.marker && this.map) {
      this.map.remove(flight.marker);
    }

    // 从活跃飞行器列表移除
    this.activeFlights.delete(flightId);

    return true;
  }

  /**
   * 检测飞行冲突
   * @returns {Array} 冲突列表
   */
  detectFlightConflicts() {
    const conflicts = [];
    const flights = Array.from(this.activeFlights.values());

    // 检查每对飞行器之间的距离
    for (let i = 0; i < flights.length; i++) {
      for (let j = i + 1; j < flights.length; j++) {
        const flight1 = flights[i];
        const flight2 = flights[j];

        const distance = this.calculateDistance(flight1.position, flight2.position);

        // 如果距离小于安全距离(50米)，则认为有冲突
        if (distance < 0.05) { // 50米 ≈ 0.05公里
          conflicts.push({
            flight1: flight1.id,
            flight2: flight2.id,
            distance: distance,
            position: this.getMidpoint(flight1.position, flight2.position),
            severity: distance < 0.02 ? 'high' : 'medium', // 20米内为高风险
            timestamp: new Date()
          });
        }
      }
    }

    return conflicts;
  }

  /**
   * 计算两点间距离(公里)
   * @param {Array} pos1 - 位置1 [lng, lat]
   * @param {Array} pos2 - 位置2 [lng, lat]
   * @returns {number} 距离(公里)
   */
  calculateDistance(pos1, pos2) {
    if (!window.AMap || !window.AMap.GeometryUtil) {
      // 使用简化的计算方法
      const R = 6371; // 地球半径(公里)
      const dLat = this.toRad(pos2[1] - pos1[1]);
      const dLon = this.toRad(pos2[0] - pos1[0]);
      const a = Math.sin(dLat/2) * Math.sin(dLat/2) +
                Math.cos(this.toRad(pos1[1])) * Math.cos(this.toRad(pos2[1])) *
                Math.sin(dLon/2) * Math.sin(dLon/2);
      const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
      return R * c;
    }

    // 使用高德地图的几何工具
    const lngLat1 = new window.AMap.LngLat(pos1[0], pos1[1]);
    const lngLat2 = new window.AMap.LngLat(pos2[0], pos2[1]);
    return window.AMap.GeometryUtil.distance(lngLat1, lngLat2) / 1000; // 转换为公里
  }

  /**
   * 获取两点中点
   * @param {Array} pos1 - 位置1 [lng, lat]
   * @param {Array} pos2 - 位置2 [lng, lat]
   * @returns {Array} 中点 [lng, lat]
   */
  getMidpoint(pos1, pos2) {
    return [
      (pos1[0] + pos2[0]) / 2,
      (pos1[1] + pos2[1]) / 2
    ];
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
   * 获取活跃飞行器列表
   * @returns {Array} 飞行器列表
   */
  getActiveFlights() {
    return Array.from(this.activeFlights.values()).map(flight => ({
      id: flight.id,
      name: flight.name,
      position: flight.position,
      type: flight.type,
      status: flight.status,
      lastUpdate: flight.lastUpdate
    }));
  }

  /**
   * 获取实时交通数据
   * @returns {Object} 交通数据
   */
  getRealTimeTraffic() {
    return this.realTimeTraffic;
  }

  /**
   * 飞行器标记点击事件处理
   * @param {Object} flight - 飞行器信息
   */
  onFlightMarkerClick(flight) {
    // 这里可以显示飞行器详细信息
    console.log('飞行器点击:', flight);
    // 实际项目中可以弹出信息窗口显示详细信息
  }

  /**
   * 交通数据更新事件处理
   * @param {Object} trafficData - 交通数据
   */
  onTrafficDataUpdate(trafficData) {
    // 这里可以更新UI显示交通状况
    console.log('交通数据更新:', trafficData);
  }

  /**
   * 清理资源
   */
  destroy() {
    // 清除定时器
    if (this.trafficUpdateInterval) {
      clearInterval(this.trafficUpdateInterval);
    }

    // 移除所有飞行器标记
    for (const flight of this.activeFlights.values()) {
      if (flight.marker && this.map) {
        this.map.remove(flight.marker);
      }
    }

    // 清空飞行器列表
    this.activeFlights.clear();

    // 移除交通图层
    if (this.trafficLayer && this.map) {
      this.map.remove(this.trafficLayer);
    }
  }
}

// 导出单例实例
const aMapTrafficManagement = new AMapTrafficManagement();
export default aMapTrafficManagement;

// 导出类以供需要多个实例时使用
export { AMapTrafficManagement };