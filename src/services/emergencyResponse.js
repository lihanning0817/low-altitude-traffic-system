/**
 * 应急响应系统
 * 负责处理紧急情况、事故响应和安全着陆
 */

class EmergencyResponseSystem {
  constructor() {
    // 应急事件记录
    this.emergencyEvents = new Map();
    // 紧急降落点
    this.emergencyLandingPoints = [];
    // 应急联系人
    this.emergencyContacts = [];
    // 应急预案
    this.emergencyProtocols = new Map();
    // 事故报告
    this.incidentReports = [];
  }

  /**
   * 初始化紧急降落点
   */
  initializeEmergencyLandingPoints(landingPoints) {
    this.emergencyLandingPoints = landingPoints.map(point => ({
      id: point.id || `elp_${Date.now()}_${Math.floor(Math.random() * 1000)}`,
      name: point.name,
      location: {
        lat: point.lat,
        lng: point.lng,
        altitude: point.altitude || 0
      },
      type: point.type || 'generic', // 'helipad', 'park', 'roof', 'field'
      capacity: point.capacity || 1,
      occupied: point.occupied || false,
      status: point.status || 'available', // 'available', 'occupied', 'unavailable'
      equipment: point.equipment || [],
      contact: point.contact || null,
      lastInspection: point.lastInspection || new Date(),
      restrictions: point.restrictions || []
    }));
  }

  /**
   * 报告紧急事件
   */
  reportEmergency(emergencyData) {
    if (!emergencyData.flightId) {
      throw new Error('必须提供飞行器ID');
    }

    const eventId = `em_${Date.now()}_${Math.floor(Math.random() * 1000)}`;
    const eventTime = new Date();

    const emergencyEvent = {
      id: eventId,
      flightId: emergencyData.flightId,
      reporter: emergencyData.reporter || 'system',
      eventType: emergencyData.eventType || 'unknown', // 'mechanical_failure', 'weather', 'medical', 'security'
      severity: emergencyData.severity || 'medium', // 'low', 'medium', 'high', 'critical'
      location: emergencyData.location || null,
      altitude: emergencyData.altitude || null,
      status: 'reported', // 'reported', 'responding', 'resolved', 'closed'
      reportedAt: eventTime,
      acknowledgedAt: null,
      resolvedAt: null,
      responseTeam: null,
      actionsTaken: [],
      notes: emergencyData.notes || ''
    };

    this.emergencyEvents.set(eventId, emergencyEvent);

    // 触发应急响应
    this.triggerEmergencyResponse(emergencyEvent);

    // 记录日志
    console.warn(`紧急事件报告: ${eventId}`, emergencyEvent);

    return {
      success: true,
      eventId: eventId,
      message: '紧急事件已报告',
      emergencyEvent: emergencyEvent
    };
  }

  /**
   * 触发应急响应
   */
  triggerEmergencyResponse(emergencyEvent) {
    // 根据事件类型和严重程度选择应急预案
    const protocol = this.selectEmergencyProtocol(emergencyEvent);

    if (protocol) {
      // 执行应急措施
      this.executeEmergencyActions(emergencyEvent, protocol);

      // 通知相关人员
      this.notifyEmergencyContacts(emergencyEvent, protocol);
    }

    // 更新事件状态
    emergencyEvent.status = 'responding';
    emergencyEvent.acknowledgedAt = new Date();
  }

  /**
   * 选择应急预案
   */
  selectEmergencyProtocol(emergencyEvent) {
    // 根据事件类型和严重程度查找匹配的预案
    for (const [protocolId, protocol] of this.emergencyProtocols) {
      // eslint-disable-next-line no-unused-vars
      const id = protocolId; // 保留变量以满足接口要求
      if (protocol.eventType === emergencyEvent.eventType &&
          protocol.minSeverity <= this.getSeverityLevel(emergencyEvent.severity)) {
        return protocol;
      }
    }

    // 如果没有找到特定预案，使用通用预案
    return this.emergencyProtocols.get('generic') || null;
  }

  /**
   * 获取严重程度等级
   */
  getSeverityLevel(severity) {
    const levels = {
      'low': 1,
      'medium': 2,
      'high': 3,
      'critical': 4
    };
    return levels[severity] || 1;
  }

  /**
   * 执行应急措施
   */
  executeEmergencyActions(emergencyEvent, protocol) {
    const actions = [];

    // 1. 立即通知飞行器
    actions.push({
      action: 'notify_flight',
      target: emergencyEvent.flightId,
      timestamp: new Date(),
      status: 'completed'
    });

    // 2. 根据预案执行特定操作
    if (protocol.actions) {
      protocol.actions.forEach(action => {
        actions.push({
          action: action.type,
          target: action.target,
          parameters: action.parameters,
          timestamp: new Date(),
          status: 'initiated'
        });
      });
    }

    // 3. 寻找最近的紧急降落点
    if (emergencyEvent.location) {
      const nearestLandingPoint = this.findNearestLandingPoint(emergencyEvent.location);
      if (nearestLandingPoint) {
        actions.push({
          action: 'redirect_to_landing_point',
          target: emergencyEvent.flightId,
          parameters: { landingPoint: nearestLandingPoint },
          timestamp: new Date(),
          status: 'initiated'
        });
      }
    }

    // 记录执行的操作
    emergencyEvent.actionsTaken.push(...actions);
  }

  /**
   * 通知应急联系人
   */
  notifyEmergencyContacts(emergencyEvent, protocol) {
    // 根据预案确定通知哪些联系人
    const contactsToNotify = protocol.contacts || this.emergencyContacts;

    contactsToNotify.forEach(contact => {
      // 实际项目中这里会发送通知(短信、邮件、APP推送等)
      console.info(`通知应急联系人: ${contact.name}`, {
        eventId: emergencyEvent.id,
        contact: contact,
        event: emergencyEvent
      });
    });
  }

  /**
   * 查找最近的紧急降落点
   */
  findNearestLandingPoint(currentLocation) {
    if (!currentLocation || this.emergencyLandingPoints.length === 0) {
      return null;
    }

    let nearestPoint = null;
    let minDistance = Infinity;

    this.emergencyLandingPoints.forEach(point => {
      // 检查降落点是否可用
      if (point.status !== 'available' || point.occupied) {
        return;
      }

      // 计算距离
      const distance = this.calculateDistance(currentLocation, point.location);

      if (distance < minDistance) {
        minDistance = distance;
        nearestPoint = point;
      }
    });

    return nearestPoint;
  }

  /**
   * 计算两点间距离(米)
   */
  calculateDistance(point1, point2) {
    // 使用Haversine公式计算地理距离
    const R = 6371e3; // 地球半径(米)
    const φ1 = point1.lat * Math.PI/180;
    const φ2 = point2.lat * Math.PI/180;
    const Δφ = (point2.lat-point1.lat) * Math.PI/180;
    const Δλ = (point2.lng-point1.lng) * Math.PI/180;

    const a = Math.sin(Δφ/2) * Math.sin(Δφ/2) +
              Math.cos(φ1) * Math.cos(φ2) *
              Math.sin(Δλ/2) * Math.sin(Δλ/2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));

    return R * c;
  }

  /**
   * 更新紧急事件状态
   */
  updateEmergencyStatus(eventId, status, notes = '') {
    const event = this.emergencyEvents.get(eventId);
    if (!event) {
      throw new Error(`紧急事件 ${eventId} 不存在`);
    }

    event.status = status;
    if (status === 'resolved') {
      event.resolvedAt = new Date();
    }
    if (notes) {
      event.notes += `\n[${new Date().toISOString()}] ${notes}`;
    }

    return {
      success: true,
      message: `紧急事件状态已更新为: ${status}`
    };
  }

  /**
   * 获取紧急事件信息
   */
  getEmergencyEvent(eventId) {
    return this.emergencyEvents.get(eventId) || null;
  }

  /**
   * 获取所有紧急事件
   */
  getAllEmergencyEvents(filter = {}) {
    let events = Array.from(this.emergencyEvents.values());

    // 应用过滤器
    if (filter.status) {
      events = events.filter(e => e.status === filter.status);
    }
    if (filter.eventType) {
      events = events.filter(e => e.eventType === filter.eventType);
    }
    if (filter.severity) {
      events = events.filter(e => e.severity === filter.severity);
    }

    // 按报告时间排序(最新的在前)
    events.sort((a, b) => b.reportedAt - a.reportedAt);

    return events;
  }

  /**
   * 添加应急预案
   */
  addEmergencyProtocol(protocol) {
    if (!protocol.id || !protocol.eventType) {
      throw new Error('预案信息不完整');
    }

    this.emergencyProtocols.set(protocol.id, {
      id: protocol.id,
      name: protocol.name,
      eventType: protocol.eventType,
      minSeverity: protocol.minSeverity || 1,
      description: protocol.description || '',
      actions: protocol.actions || [],
      contacts: protocol.contacts || [],
      createdAt: new Date()
    });

    return {
      success: true,
      message: '应急预案添加成功'
    };
  }

  /**
   * 获取应急预案
   */
  getEmergencyProtocol(protocolId) {
    // eslint-disable-next-line no-unused-vars
    const id = protocolId; // 保留参数以满足接口要求
    return this.emergencyProtocols.get(protocolId) || null;
  }

  /**
   * 获取所有应急预案
   */
  getAllEmergencyProtocols() {
    return Array.from(this.emergencyProtocols.values());
  }

  /**
   * 添加应急联系人
   */
  addEmergencyContact(contact) {
    if (!contact.name || !contact.phone) {
      throw new Error('联系人信息不完整');
    }

    const newContact = {
      id: contact.id || `ec_${Date.now()}_${Math.floor(Math.random() * 1000)}`,
      name: contact.name,
      phone: contact.phone,
      email: contact.email || '',
      role: contact.role || 'general',
      organization: contact.organization || '',
      available: contact.available !== undefined ? contact.available : true,
      createdAt: new Date()
    };

    this.emergencyContacts.push(newContact);

    return {
      success: true,
      contact: newContact,
      message: '应急联系人添加成功'
    };
  }

  /**
   * 获取应急联系人
   */
  getEmergencyContacts(filter = {}) {
    let contacts = [...this.emergencyContacts];

    if (filter.role) {
      contacts = contacts.filter(c => c.role === filter.role);
    }
    if (filter.available !== undefined) {
      contacts = contacts.filter(c => c.available === filter.available);
    }

    return contacts;
  }

  /**
   * 提交事故报告
   */
  submitIncidentReport(report) {
    if (!report.eventId || !report.summary) {
      throw new Error('事故报告信息不完整');
    }

    const incidentReport = {
      id: `ir_${Date.now()}_${Math.floor(Math.random() * 1000)}`,
      eventId: report.eventId,
      reporter: report.reporter || 'system',
      summary: report.summary,
      details: report.details || '',
      findings: report.findings || [],
      recommendations: report.recommendations || [],
      submittedAt: new Date(),
      attachments: report.attachments || []
    };

    this.incidentReports.push(incidentReport);

    // 更新关联的紧急事件
    const event = this.emergencyEvents.get(report.eventId);
    if (event) {
      event.incidentReportId = incidentReport.id;
    }

    return {
      success: true,
      reportId: incidentReport.id,
      message: '事故报告提交成功'
    };
  }

  /**
   * 获取事故报告
   */
  getIncidentReport(reportId) {
    return this.incidentReports.find(r => r.id === reportId) || null;
  }

  /**
   * 获取所有事故报告
   */
  getAllIncidentReports() {
    // 按提交时间排序(最新的在前)
    return this.incidentReports.sort((a, b) => b.submittedAt - a.submittedAt);
  }

  /**
   * 启动紧急着陆程序
   */
  initiateEmergencyLanding(flightId, landingPoint = null) {
    const emergencyData = {
      flightId: flightId,
      eventType: 'emergency_landing',
      severity: 'high',
      notes: '启动紧急着陆程序'
    };

    // 如果指定了降落点，检查其可用性
    if (landingPoint) {
      const point = this.emergencyLandingPoints.find(p => p.id === landingPoint.id);
      if (point) {
        if (point.status !== 'available' || point.occupied) {
          throw new Error('指定的紧急降落点不可用');
        }
        // 标记为占用
        point.occupied = true;
      }
    }

    // 报告紧急事件
    const result = this.reportEmergency(emergencyData);

    return {
      success: true,
      eventId: result.eventId,
      message: '紧急着陆程序已启动'
    };
  }

  /**
   * 释放紧急降落点
   */
  releaseLandingPoint(landingPointId) {
    const point = this.emergencyLandingPoints.find(p => p.id === landingPointId);
    if (point) {
      point.occupied = false;
      point.status = 'available';
      return {
        success: true,
        message: '紧急降落点已释放'
      };
    }
    return {
      success: false,
      message: '未找到指定的紧急降落点'
    };
  }

  /**
   * 获取紧急降落点状态
   */
  getLandingPointStatus() {
    return this.emergencyLandingPoints.map(point => ({
      id: point.id,
      name: point.name,
      status: point.status,
      occupied: point.occupied,
      available: point.status === 'available' && !point.occupied
    }));
  }
}

// 导出单例实例
const emergencyResponseSystem = new EmergencyResponseSystem();
export default emergencyResponseSystem;

// 导出类以供需要多个实例时使用
export { EmergencyResponseSystem };