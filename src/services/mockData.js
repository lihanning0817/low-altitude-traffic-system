// 模拟数据服务
export const mockFlightTasks = [
  {
    id: 'T001',
    name: '城区巡航任务',
    description: '对城区主要交通干道进行巡航监控',
    status: '进行中',
    priority: '高',
    progress: 65,
    createTime: '2024-01-15 09:30:00',
    droneId: 'UAV001',
    route: [
      { lat: 39.90923, lng: 116.397428 },
      { lat: 39.91923, lng: 116.407428 },
      { lat: 39.90923, lng: 116.417428 },
      { lat: 39.89923, lng: 116.407428 }
    ],
    airspace: 'A1',
    weatherConditions: '晴朗',
    duration: '4h 30m'
  },
  {
    id: 'T002',
    name: '应急响应任务',
    description: '响应交通事故紧急情况',
    status: '已完成',
    priority: '高',
    progress: 100,
    createTime: '2024-01-14 14:20:00',
    droneId: 'UAV002',
    route: [
      { lat: 39.90923, lng: 116.397428 },
      { lat: 39.91923, lng: 116.387428 }
    ],
    airspace: 'B2',
    weatherConditions: '多云',
    duration: '1h 15m'
  },
  {
    id: 'T003',
    name: '定点监控任务',
    description: '对重点区域进行定点监控',
    status: '待执行',
    priority: '中',
    progress: 0,
    createTime: '2024-01-16 08:00:00',
    droneId: 'UAV003',
    route: [
      { lat: 39.90923, lng: 116.397428 },
      { lat: 39.91423, lng: 116.399428 }
    ],
    airspace: 'C3',
    weatherConditions: '晴朗',
    duration: '2h 00m'
  },
  {
    id: 'T004',
    name: '数据收集任务',
    description: '收集交通流量数据',
    status: '进行中',
    priority: '中',
    progress: 30,
    createTime: '2024-01-15 16:45:00',
    droneId: 'UAV004',
    route: [
      { lat: 39.90923, lng: 116.397428 },
      { lat: 39.89923, lng: 116.387428 },
      { lat: 39.88923, lng: 116.397428 }
    ],
    airspace: 'D4',
    weatherConditions: '阴天',
    duration: '6h 00m'
  },
  {
    id: 'T005',
    name: '设备检测任务',
    description: '检测无人机设备状态',
    status: '已完成',
    priority: '低',
    progress: 100,
    createTime: '2024-01-13 10:15:00',
    droneId: 'UAV005',
    route: [
      { lat: 39.90923, lng: 116.397428 },
      { lat: 39.91423, lng: 116.399428 },
      { lat: 39.90923, lng: 116.407428 }
    ],
    airspace: 'E5',
    weatherConditions: '晴朗',
    duration: '1h 30m'
  },
  {
    id: 'T006',
    name: '夜间巡逻任务',
    description: '夜间城市安全巡逻',
    status: '待执行',
    priority: '高',
    progress: 0,
    createTime: '2024-01-17 18:00:00',
    droneId: 'UAV006',
    route: [
      { lat: 39.90923, lng: 116.397428 },
      { lat: 39.90423, lng: 116.399428 },
      { lat: 39.90923, lng: 116.407428 },
      { lat: 39.91423, lng: 116.405428 }
    ],
    airspace: 'F6',
    weatherConditions: '晴朗',
    duration: '5h 00m'
  }
]

export const mockDevices = [
  {
    id: 'UAV001',
    name: '巡逻无人机-01',
    type: 'drone',
    status: '巡航中',
    signal: 85,
    position: [116.397428, 39.90923],
    battery: 78,
    lastUpdate: '2024-01-16T10:30:00Z',
    firmwareVersion: 'v1.2.3'
  },
  {
    id: 'UAV002',
    name: '巡逻无人机-02',
    type: 'drone',
    status: '待命',
    signal: 92,
    position: [116.407428, 39.91923],
    battery: 95,
    lastUpdate: '2024-01-16T10:25:00Z',
    firmwareVersion: 'v1.2.3'
  },
  {
    id: 'UAV003',
    name: '巡逻无人机-03',
    type: 'drone',
    status: '充电中',
    signal: 88,
    position: [116.387428, 39.89923],
    battery: 25,
    lastUpdate: '2024-01-16T10:20:00Z',
    firmwareVersion: 'v1.2.3'
  },
  {
    id: 'CAM001',
    name: '监控摄像头-01',
    type: 'camera',
    status: '正常',
    signal: 95,
    position: [116.399428, 39.90423],
    lastUpdate: '2024-01-16T10:35:00Z',
    firmwareVersion: 'v2.1.0'
  },
  {
    id: 'RAD001',
    name: '雷达设备-01',
    type: 'radar',
    status: '运行中',
    signal: 82,
    position: [116.401428, 39.91423],
    lastUpdate: '2024-01-16T10:28:00Z',
    firmwareVersion: 'v1.5.2'
  },
  {
    id: 'UAV004',
    name: '巡逻无人机-04',
    type: 'drone',
    status: '飞行中',
    signal: 78,
    position: [116.395428, 39.90523],
    battery: 65,
    lastUpdate: '2024-01-16T10:32:00Z',
    firmwareVersion: 'v1.2.3'
  }
]

export const mockAirspaces = [
  {
    id: 'A1',
    name: '城区核心空域',
    type: 'restricted',
    boundary: [
      { lat: 39.90423, lng: 116.392428 },
      { lat: 39.90423, lng: 116.402428 },
      { lat: 39.91423, lng: 116.402428 },
      { lat: 39.91423, lng: 116.392428 }
    ],
    altitude: { min: 0, max: 120 },
    restrictions: ['禁止商业飞行', '仅限授权无人机'],
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z'
  },
  {
    id: 'B2',
    name: '交通枢纽空域',
    type: 'controlled',
    boundary: [
      { lat: 39.90923, lng: 116.382428 },
      { lat: 39.90923, lng: 116.392428 },
      { lat: 39.91923, lng: 116.392428 },
      { lat: 39.91923, lng: 116.382428 }
    ],
    altitude: { min: 0, max: 150 },
    restrictions: ['需提前申请', '限速80km/h'],
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z'
  },
  {
    id: 'C3',
    name: '学校周边空域',
    type: 'protected',
    boundary: [
      { lat: 39.91423, lng: 116.397428 },
      { lat: 39.91423, lng: 116.407428 },
      { lat: 39.92423, lng: 116.407428 },
      { lat: 39.92423, lng: 116.397428 }
    ],
    altitude: { min: 0, max: 60 },
    restrictions: ['禁止所有飞行', '紧急情况除外'],
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z'
  },
  {
    id: 'D4',
    name: '工业区空域',
    type: 'permitted',
    boundary: [
      { lat: 39.89423, lng: 116.387428 },
      { lat: 39.89423, lng: 116.397428 },
      { lat: 39.90423, lng: 116.397428 },
      { lat: 39.90423, lng: 116.387428 }
    ],
    altitude: { min: 0, max: 100 },
    restrictions: ['允许商业飞行', '需佩戴识别标签'],
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z'
  },
  {
    id: 'E5',
    name: '公园空域',
    type: 'recreational',
    boundary: [
      { lat: 39.90923, lng: 116.402428 },
      { lat: 39.90923, lng: 116.412428 },
      { lat: 39.91923, lng: 116.412428 },
      { lat: 39.91923, lng: 116.402428 }
    ],
    altitude: { min: 0, max: 80 },
    restrictions: ['允许娱乐飞行', '限高80米'],
    status: 'active',
    lastUpdated: '2024-01-15T08:00:00Z'
  }
]

export const mockWeatherData = {
  current: {
    temperature: 18,
    humidity: 65,
    windSpeed: 4.5,
    windDirection: '北风',
    pressure: 1013,
    condition: '晴朗',
    visibility: 10,
    uvIndex: 6
  },
  forecast: [
    {
      date: '2024-01-16',
      high: 20,
      low: 8,
      condition: '晴朗',
      precipitation: 0,
      windSpeed: 4.5
    },
    {
      date: '2024-01-17',
      high: 19,
      low: 7,
      condition: '多云',
      precipitation: 5,
      windSpeed: 5.2
    },
    {
      date: '2024-01-18',
      high: 17,
      low: 6,
      condition: '小雨',
      precipitation: 80,
      windSpeed: 6.8
    },
    {
      date: '2024-01-19',
      high: 16,
      low: 5,
      condition: '阴天',
      precipitation: 10,
      windSpeed: 4.2
    },
    {
      date: '2024-01-20',
      high: 18,
      low: 7,
      condition: '晴朗',
      precipitation: 0,
      windSpeed: 4.8
    }
  ]
}

export const mockEmergencyEvents = [
  {
    id: 'E001',
    title: '道路交通事故',
    description: '京港澳高速与二环交汇处发生三车追尾事故',
    location: { lat: 39.90923, lng: 116.397428 },
    severity: 'high',
    status: 'reported',
    reportedBy: '交通监控系统',
    timestamp: '2024-01-15T14:20:00Z',
    responseTeam: '应急救援队',
    estimatedResponseTime: '8分钟'
  },
  {
    id: 'E002',
    title: '无人机异常降落',
    description: 'UAV003在工业区空域出现异常降落',
    location: { lat: 39.89923, lng: 116.387428 },
    severity: 'medium',
    status: 'responding',
    reportedBy: '无人机自动报告系统',
    timestamp: '2024-01-15T13:45:00Z',
    responseTeam: '技术维护组',
    estimatedResponseTime: '15分钟'
  },
  {
    id: 'E003',
    title: '气象预警',
    description: '预计未来2小时将有强风天气，风速超过15m/s',
    location: { lat: 39.90923, lng: 116.397428 },
    severity: 'high',
    status: 'alerted',
    reportedBy: '气象监测系统',
    timestamp: '2024-01-15T12:30:00Z',
    responseTeam: '气象预警中心',
    estimatedResponseTime: '立即'
  }
]

export const mockFlightConflicts = [
  {
    id: 'C001',
    type: 'collision',
    description: 'UAV001与UAV004在高度120米处存在潜在碰撞风险',
    aircraft1: { id: 'UAV001', position: [116.397428, 39.90923], altitude: 120 },
    aircraft2: { id: 'UAV004', position: [116.399428, 39.90723], altitude: 120 },
    distance: 15.5,
    timeToCollision: '32秒',
    severity: 'high',
    status: 'active',
    resolved: false,
    timestamp: '2024-01-16T10:35:00Z'
  },
  {
    id: 'C002',
    type: 'airspace',
    description: 'UAV002进入受限制空域A1',
    aircraft1: { id: 'UAV002', position: [116.395428, 39.90723], altitude: 80 },
    aircraft2: { id: 'A1', name: '城区核心空域' },
    distance: 0,
    timeToCollision: 'N/A',
    severity: 'medium',
    status: 'active',
    resolved: false,
    timestamp: '2024-01-16T10:28:00Z'
  },
  {
    id: 'C003',
    type: 'altitude',
    description: 'UAV005与地面建筑高度差小于安全距离',
    aircraft1: { id: 'UAV005', position: [116.401428, 39.91423], altitude: 45 },
    aircraft2: { id: 'Building', name: '金融大厦', height: 50 },
    distance: 5,
    timeToCollision: 'N/A',
    severity: 'medium',
    status: 'active',
    resolved: false,
    timestamp: '2024-01-16T10:20:00Z'
  }
]

export const mockRoutes = [
  {
    id: 'R001',
    name: '机场到市中心航线',
    start: { lat: 39.875428, lng: 116.397428 },
    end: { lat: 39.90923, lng: 116.407428 },
    waypoints: [
      { lat: 39.885428, lng: 116.397428 },
      { lat: 39.895428, lng: 116.402428 },
      { lat: 39.900428, lng: 116.405428 }
    ],
    distance: 12.5,
    estimatedDuration: '18分钟',
    altitude: 100,
    weatherConditions: '晴朗',
    trafficDensity: '低',
    createdAt: '2024-01-15T09:00:00Z',
    createdBy: 'admin'
  },
  {
    id: 'R002',
    name: '东城至西城航线',
    start: { lat: 39.91423, lng: 116.392428 },
    end: { lat: 39.90423, lng: 116.412428 },
    waypoints: [
      { lat: 39.91023, lng: 116.398428 },
      { lat: 39.90723, lng: 116.404428 }
    ],
    distance: 8.2,
    estimatedDuration: '12分钟',
    altitude: 80,
    weatherConditions: '多云',
    trafficDensity: '中',
    createdAt: '2024-01-15T09:15:00Z',
    createdBy: 'admin'
  }
]

export const mockEmergencyLandingPoints = [
  {
    id: 'ELP001',
    name: '奥林匹克公园应急降落点',
    location: { lat: 39.999428, lng: 116.397428 },
    capacity: 5,
    facilities: ['照明', '通信', '医疗站'],
    status: 'active',
    lastInspected: '2024-01-10T10:00:00Z'
  },
  {
    id: 'ELP002',
    name: '朝阳医院应急降落点',
    location: { lat: 39.92423, lng: 116.457428 },
    capacity: 3,
    facilities: ['医疗站', '通信', '消防'],
    status: 'active',
    lastInspected: '2024-01-12T14:00:00Z'
  },
  {
    id: 'ELP003',
    name: '北京西站应急降落点',
    location: { lat: 39.88423, lng: 116.327428 },
    capacity: 4,
    facilities: ['照明', '通信', '安保'],
    status: 'active',
    lastInspected: '2024-01-11T09:00:00Z'
  }
]
