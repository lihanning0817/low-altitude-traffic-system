/**
 * Mock数据
 * Mock Data for Testing
 */

// Mock无人机数据
export const mockDrones = [
  {
    id: 1,
    drone_id: 'UAV-001',
    model: 'DJI Phantom 4',
    status: 'active',
    battery: 85,
    location: {
      lat: 41.796700,
      lng: 123.451200,
      altitude: 100
    },
    last_update: '2025-10-14 10:30:00'
  },
  {
    id: 2,
    drone_id: 'UAV-002',
    model: 'DJI Mavic 3',
    status: 'idle',
    battery: 15,
    location: {
      lat: 41.800000,
      lng: 123.460000,
      altitude: 0
    },
    last_update: '2025-10-14 10:25:00'
  },
  {
    id: 3,
    drone_id: 'UAV-003',
    model: 'DJI Inspire 2',
    status: 'maintenance',
    battery: 100,
    location: {
      lat: 41.810000,
      lng: 123.470000,
      altitude: 0
    },
    last_update: '2025-10-14 09:00:00'
  }
]

// Mock空域数据
export const mockAirspaces = [
  {
    id: 1,
    airspace_id: 'AS-001',
    name: '沈阳故宫文化遗产保护区',
    type: 'restricted',
    status: 'active',
    north_lat: 41.800000,
    south_lat: 41.790000,
    east_lng: 123.460000,
    west_lng: 123.440000,
    max_altitude: 150,
    min_altitude: 0,
    max_concurrent_flights: 3
  },
  {
    id: 2,
    airspace_id: 'AS-002',
    name: '北陵公园',
    type: 'controlled',
    status: 'active',
    north_lat: 41.850000,
    south_lat: 41.840000,
    east_lng: 123.430000,
    west_lng: 123.410000,
    max_altitude: 200,
    min_altitude: 0,
    max_concurrent_flights: 5
  }
]

// Mock飞行许可数据
export const mockPermits = [
  {
    id: 1,
    permit_code: 'FP-2025101400001',
    task_id: 7,
    airspace_id: 1,
    applicant_id: 4,
    status: 'pending',
    start_time: '2025-10-14 14:00:00',
    end_time: '2025-10-14 16:00:00',
    application_time: '2025-10-14 10:00:00',
    remarks: '沈阳故宫文物巡检飞行许可申请'
  },
  {
    id: 2,
    permit_code: 'FP-2025101400002',
    task_id: 8,
    airspace_id: 2,
    applicant_id: 4,
    status: 'approved',
    start_time: '2025-10-15 09:00:00',
    end_time: '2025-10-15 11:00:00',
    application_time: '2025-10-14 09:00:00',
    approval_time: '2025-10-14 09:30:00',
    remarks: '北陵公园环境监测'
  }
]

// Mock飞行任务数据
export const mockTasks = [
  {
    id: 7,
    task_id: 'TASK-SY-20251014-001',
    task_name: '沈阳故宫文物巡检',
    task_type: 'inspection',
    status: 'planned',
    priority: 'high',
    drone_id: 1,
    created_by: 4,
    created_at: '2025-10-14 09:00:00'
  },
  {
    id: 8,
    task_id: 'TASK-SY-20251014-002',
    task_name: '北陵公园环境监测',
    task_type: 'monitoring',
    status: 'in_progress',
    priority: 'normal',
    drone_id: 2,
    created_by: 4,
    created_at: '2025-10-14 08:00:00'
  }
]

// Mock应急事件数据
export const mockEmergencyEvents = [
  {
    id: 1,
    event_id: 'EMG-001',
    event_type: 'low_battery',
    severity: 'high',
    drone_id: 2,
    location: {
      lat: 41.800000,
      lng: 123.460000
    },
    status: 'active',
    description: '无人机电量低于20%',
    created_at: '2025-10-14 10:25:00'
  }
]

// Mock应急降落点数据
export const mockLandingPoints = [
  {
    id: 1,
    point_code: 'ELP-001',
    name: '沈阳故宫停机坪',
    location: {
      lat: 41.796700,
      lng: 123.451200
    },
    capacity: 5,
    status: 'available',
    surface_type: 'concrete'
  },
  {
    id: 2,
    point_code: 'ELP-002',
    name: '北陵公园停机坪',
    location: {
      lat: 41.845000,
      lng: 123.420000
    },
    capacity: 3,
    status: 'available',
    surface_type: 'grass'
  }
]

// Mock天气数据
export const mockWeather = {
  city: '沈阳',
  temperature: 15,
  weather: '晴',
  wind_speed: 3.5,
  wind_direction: '东南',
  humidity: 45,
  pressure: 1013,
  visibility: 10,
  update_time: '2025-10-14 10:00:00'
}

// Mock地图标记数据
export const mockMarkers = [
  {
    id: 1,
    type: 'drone',
    name: 'UAV-001',
    location: {
      lat: 41.796700,
      lng: 123.451200
    }
  },
  {
    id: 2,
    type: 'airspace',
    name: '沈阳故宫文化遗产保护区',
    location: {
      lat: 41.795000,
      lng: 123.450000
    }
  }
]

// Mock路线数据
export const mockRoute = {
  start: { lat: 41.796700, lng: 123.451200 },
  end: { lat: 41.806700, lng: 123.461200 },
  distance: 1.5,
  duration: 180,
  waypoints: [
    { lat: 41.799000, lng: 123.454000 },
    { lat: 41.802000, lng: 123.457000 }
  ]
}
