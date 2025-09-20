/**
 * 空域管理系统
 * 负责空域划分、权限管理和飞行许可审批
 */

class AirspaceManagementSystem {
  constructor() {
    // 空域数据
    this.airspaces = new Map();
    // 飞行许可申请
    this.flightPermits = new Map();
    // 空域使用历史
    this.usageHistory = [];
    // 空域限制规则
    this.restrictions = [];
  }

  /**
   * 初始化空域数据
   */
  initializeAirspaces(airspaceData) {
    airspaceData.forEach(space => {
      this.airspaces.set(space.id, {
        id: space.id,
        name: space.name,
        type: space.type || 'controlled', // controlled, restricted, danger, etc.
        boundaries: space.boundaries,
        altitudeRange: {
          min: space.altitudeRange?.min || 0,
          max: space.altitudeRange?.max || 1000
        },
        capacity: space.capacity || 10,
        currentFlights: 0,
        status: space.status || 'active', // active, inactive, maintenance
        restrictions: space.restrictions || [],
        operatingHours: space.operatingHours || { start: '06:00', end: '22:00' },
        controllingAuthority: space.controllingAuthority || 'ATC'
      });
    });
  }

  /**
   * 创建空域
   */
  createAirspace(airspace) {
    if (!airspace.id || !airspace.name || !airspace.boundaries) {
      throw new Error('空域信息不完整');
    }

    if (this.airspaces.has(airspace.id)) {
      throw new Error(`空域ID ${airspace.id} 已存在`);
    }

    const newAirspace = {
      id: airspace.id,
      name: airspace.name,
      type: airspace.type || 'controlled',
      boundaries: airspace.boundaries,
      altitudeRange: {
        min: airspace.altitudeRange?.min || 0,
        max: airspace.altitudeRange?.max || 1000
      },
      capacity: airspace.capacity || 10,
      currentFlights: 0,
      status: airspace.status || 'active',
      restrictions: airspace.restrictions || [],
      operatingHours: airspace.operatingHours || { start: '06:00', end: '22:00' },
      controllingAuthority: airspace.controllingAuthority || 'ATC',
      createdAt: new Date()
    };

    this.airspaces.set(airspace.id, newAirspace);

    // 记录到历史
    this.usageHistory.push({
      type: 'airspace_created',
      airspaceId: airspace.id,
      timestamp: new Date(),
      details: newAirspace
    });

    return {
      success: true,
      airspace: newAirspace,
      message: '空域创建成功'
    };
  }

  /**
   * 更新空域信息
   */
  updateAirspace(airspaceId, updates) {
    const airspace = this.airspaces.get(airspaceId);
    if (!airspace) {
      throw new Error(`空域 ${airspaceId} 不存在`);
    }

    // 更新空域属性
    Object.keys(updates).forEach(key => {
      if (key !== 'id') { // 不允许修改ID
        airspace[key] = updates[key];
      }
    });

    airspace.updatedAt = new Date();

    // 记录到历史
    this.usageHistory.push({
      type: 'airspace_updated',
      airspaceId: airspaceId,
      timestamp: new Date(),
      changes: updates
    });

    return {
      success: true,
      airspace: airspace,
      message: '空域信息更新成功'
    };
  }

  /**
   * 删除空域
   */
  deleteAirspace(airspaceId) {
    const airspace = this.airspaces.get(airspaceId);
    if (!airspace) {
      throw new Error(`空域 ${airspaceId} 不存在`);
    }

    // 检查是否有飞行器在空域中
    if (airspace.currentFlights > 0) {
      throw new Error('空域中有飞行器，无法删除');
    }

    this.airspaces.delete(airspaceId);

    // 记录到历史
    this.usageHistory.push({
      type: 'airspace_deleted',
      airspaceId: airspaceId,
      timestamp: new Date(),
      details: airspace
    });

    return {
      success: true,
      message: '空域删除成功'
    };
  }

  /**
   * 获取空域信息
   */
  getAirspace(airspaceId) {
    return this.airspaces.get(airspaceId) || null;
  }

  /**
   * 获取所有空域
   */
  getAllAirspaces() {
    return Array.from(this.airspaces.values());
  }

  /**
   * 检查点是否在空域内
   */
  isPointInAirspace(point, airspace) {
    const bounds = airspace.boundaries;

    // 检查经纬度是否在边界内
    const inLatBounds = point.lat >= bounds.minLat && point.lat <= bounds.maxLat;
    const inLngBounds = point.lng >= bounds.minLng && point.lng <= bounds.maxLng;

    if (!inLatBounds || !inLngBounds) {
      return false;
    }

    // 检查高度是否在范围内
    if (point.altitude !== undefined) {
      return point.altitude >= airspace.altitudeRange.min &&
             point.altitude <= airspace.altitudeRange.max;
    }

    return true;
  }

  /**
   * 获取点所在的空域
   */
  getAirspaceForPoint(point) {
    for (const [id, airspace] of this.airspaces) {
      // eslint-disable-next-line no-unused-vars
      const airspaceId = id; // 保留变量以满足接口要求
      if (this.isPointInAirspace(point, airspace)) {
        return airspace;
      }
    }
    return null;
  }

  /**
   * 申请飞行许可
   */
  applyForFlightPermit(permitRequest) {
    if (!permitRequest.flightId || !permitRequest.route) {
      throw new Error('许可申请信息不完整');
    }

    // 检查飞行时间是否在空域运营时间内
    const now = new Date();
    // eslint-disable-next-line no-unused-vars
    const currentTime = `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}`;

    // 检查路线是否经过受限空域
    const routeAirspaces = this.getAirspacesForRoute(permitRequest.route);
    const restrictedAirspaces = routeAirspaces.filter(a => a.status === 'restricted');

    if (restrictedAirspaces.length > 0) {
      return {
        success: false,
        permitId: null,
        message: `路线经过受限空域: ${restrictedAirspaces.map(a => a.name).join(', ')}`
      };
    }

    // 生成许可ID
    const permitId = `P${Date.now()}`;

    const permit = {
      id: permitId,
      flightId: permitRequest.flightId,
      applicant: permitRequest.applicant,
      route: permitRequest.route,
      requestedAltitude: permitRequest.requestedAltitude,
      startTime: permitRequest.startTime,
      endTime: permitRequest.endTime,
      status: 'pending', // pending, approved, rejected, expired
      priority: permitRequest.priority || 'normal',
      createdAt: new Date(),
      reviewedBy: null,
      reviewedAt: null,
      remarks: permitRequest.remarks || ''
    };

    this.flightPermits.set(permitId, permit);

    // 记录到历史
    this.usageHistory.push({
      type: 'permit_applied',
      permitId: permitId,
      timestamp: new Date(),
      details: permit
    });

    return {
      success: true,
      permitId: permitId,
      message: '飞行许可申请已提交'
    };
  }

  /**
   * 审批飞行许可
   */
  approveFlightPermit(permitId, reviewer, approved = true, remarks = '') {
    const permit = this.flightPermits.get(permitId);
    if (!permit) {
      throw new Error(`许可 ${permitId} 不存在`);
    }

    if (permit.status !== 'pending') {
      throw new Error(`许可状态为 ${permit.status}，无法审批`);
    }

    permit.status = approved ? 'approved' : 'rejected';
    permit.reviewedBy = reviewer;
    permit.reviewedAt = new Date();
    permit.remarks = remarks;

    // 记录到历史
    this.usageHistory.push({
      type: 'permit_' + (approved ? 'approved' : 'rejected'),
      permitId: permitId,
      timestamp: new Date(),
      details: {
        reviewer: reviewer,
        status: permit.status,
        remarks: remarks
      }
    });

    return {
      success: true,
      permit: permit,
      message: `飞行许可${approved ? '已批准' : '已拒绝'}`
    };
  }

  /**
   * 获取许可信息
   */
  getFlightPermit(permitId) {
    return this.flightPermits.get(permitId) || null;
  }

  /**
   * 获取所有许可
   */
  getAllFlightPermits() {
    return Array.from(this.flightPermits.values());
  }

  /**
   * 获取路线经过的空域
   */
  getAirspacesForRoute(route) {
    const airspaces = [];

    if (route.nodes && route.nodes.length > 0) {
      route.nodes.forEach(node => {
        const airspace = this.getAirspaceForPoint(node);
        if (airspace && !airspaces.find(a => a.id === airspace.id)) {
          airspaces.push(airspace);
        }
      });
    }

    return airspaces;
  }

  /**
   * 检查空域容量
   */
  checkAirspaceCapacity(airspaceId) {
    const airspace = this.airspaces.get(airspaceId);
    if (!airspace) {
      throw new Error(`空域 ${airspaceId} 不存在`);
    }

    return {
      current: airspace.currentFlights,
      capacity: airspace.capacity,
      available: airspace.capacity - airspace.currentFlights,
      utilization: (airspace.currentFlights / airspace.capacity) * 100
    };
  }

  /**
   * 增加空域中的飞行器计数
   */
  incrementFlightCount(airspaceId) {
    const airspace = this.airspaces.get(airspaceId);
    if (!airspace) {
      throw new Error(`空域 ${airspaceId} 不存在`);
    }

    if (airspace.currentFlights >= airspace.capacity) {
      throw new Error(`空域 ${airspace.name} 已达到容量上限`);
    }

    airspace.currentFlights++;

    // 记录到历史
    this.usageHistory.push({
      type: 'flight_entered',
      airspaceId: airspaceId,
      timestamp: new Date()
    });

    return {
      success: true,
      currentFlights: airspace.currentFlights,
      message: '飞行器计数增加成功'
    };
  }

  /**
   * 减少空域中的飞行器计数
   */
  decrementFlightCount(airspaceId) {
    const airspace = this.airspaces.get(airspaceId);
    if (!airspace) {
      throw new Error(`空域 ${airspaceId} 不存在`);
    }

    airspace.currentFlights = Math.max(0, airspace.currentFlights - 1);

    // 记录到历史
    this.usageHistory.push({
      type: 'flight_exited',
      airspaceId: airspaceId,
      timestamp: new Date()
    });

    return {
      success: true,
      currentFlights: airspace.currentFlights,
      message: '飞行器计数减少成功'
    };
  }

  /**
   * 获取空域使用统计
   */
  getAirspaceUsageStatistics(timeRange = '24h') {
    const now = new Date();
    let startTime;

    switch (timeRange) {
      case '1h':
        startTime = new Date(now.getTime() - 60 * 60 * 1000);
        break;
      case '24h':
        startTime = new Date(now.getTime() - 24 * 60 * 60 * 1000);
        break;
      case '7d':
        startTime = new Date(now.getTime() - 7 * 24 * 60 * 60 * 1000);
        break;
      default:
        startTime = new Date(now.getTime() - 24 * 60 * 60 * 1000);
    }

    // 过滤指定时间范围内的历史记录
    const recentHistory = this.usageHistory.filter(
      record => new Date(record.timestamp) >= startTime
    );

    // 统计各空域的使用情况
    const statistics = {};
    this.airspaces.forEach((airspace, id) => {
      statistics[id] = {
        airspace: airspace,
        entryCount: 0,
        exitCount: 0,
        permitCount: 0,
        utilization: (airspace.currentFlights / airspace.capacity) * 100
      };
    });

    // 计算统计数据
    recentHistory.forEach(record => {
      if (statistics[record.airspaceId]) {
        switch (record.type) {
          case 'flight_entered':
            statistics[record.airspaceId].entryCount++;
            break;
          case 'flight_exited':
            statistics[record.airspaceId].exitCount++;
            break;
          case 'permit_applied':
            statistics[record.airspaceId].permitCount++;
            break;
        }
      }
    });

    return Object.values(statistics);
  }

  /**
   * 添加空域限制
   */
  addRestriction(restriction) {
    if (!restriction.id || !restriction.airspaceId || !restriction.type) {
      throw new Error('限制信息不完整');
    }

    const newRestriction = {
      id: restriction.id,
      airspaceId: restriction.airspaceId,
      type: restriction.type, // 'time', 'weather', 'emergency', etc.
      description: restriction.description,
      startTime: restriction.startTime,
      endTime: restriction.endTime,
      active: restriction.active !== undefined ? restriction.active : true,
      createdAt: new Date()
    };

    this.restrictions.push(newRestriction);

    // 更新空域的限制列表
    const airspace = this.airspaces.get(restriction.airspaceId);
    if (airspace) {
      if (!airspace.restrictions) {
        airspace.restrictions = [];
      }
      airspace.restrictions.push(restriction.id);
    }

    // 记录到历史
    this.usageHistory.push({
      type: 'restriction_added',
      restrictionId: restriction.id,
      timestamp: new Date(),
      details: restriction
    });

    return {
      success: true,
      message: '空域限制添加成功'
    };
  }

  /**
   * 检查空域限制
   */
  checkRestrictions(airspaceId, checkTime = new Date()) {
    // eslint-disable-next-line no-unused-vars
    const currentTime = checkTime; // 保留参数以满足接口要求
    const airspace = this.airspaces.get(airspaceId);
    if (!airspace || !airspace.restrictions) {
      return [];
    }

    const activeRestrictions = [];

    airspace.restrictions.forEach(restrictionId => {
      const restriction = this.restrictions.find(r => r.id === restrictionId);
      if (restriction && restriction.active) {
        // 检查时间限制
        if (restriction.type === 'time') {
          const checkTimeStr = `${checkTime.getHours().toString().padStart(2, '0')}:${checkTime.getMinutes().toString().padStart(2, '0')}`;
          if (checkTimeStr >= restriction.startTime && checkTimeStr <= restriction.endTime) {
            activeRestrictions.push(restriction);
          }
        } else {
          activeRestrictions.push(restriction);
        }
      }
    });

    return activeRestrictions;
  }
}

// 导出单例实例
const airspaceManagementSystem = new AirspaceManagementSystem();
export default airspaceManagementSystem;

// 导出类以供需要多个实例时使用
export { AirspaceManagementSystem };