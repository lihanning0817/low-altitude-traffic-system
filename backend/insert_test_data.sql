-- 插入测试飞行任务（沈阳地区）
INSERT INTO low_altitude_traffic_system.flight_tasks
(task_id, user_id, task_name, description, task_type, priority, status,
 start_lat, start_lng, start_altitude, start_address,
 end_lat, end_lng, end_altitude, end_address,
 planned_speed, planned_altitude, estimated_duration, estimated_distance,
 scheduled_start_time, scheduled_end_time)
VALUES
('TASK-SY-001', 4, '沈阳故宫巡检任务', '文物保护日常巡检', 'inspection', 'medium', 'pending',
 41.796700, 123.451200, 120.00, '沈阳市沈河区沈阳路171号(沈阳故宫)',
 41.806700, 123.461200, 120.00, '沈阳故宫北侧',
 15.00, 120.00, 1200, 1500.00,
 DATE_ADD(NOW(), INTERVAL 2 HOUR), DATE_ADD(NOW(), INTERVAL 3 HOUR)),

('TASK-SY-002', 4, '浑南区物流配送', '紧急医疗物资配送', 'delivery', 'high', 'executing',
 41.714500, 123.458700, 100.00, '沈阳市浑南区浑南中路',
 41.724500, 123.468700, 100.00, '沈阳市浑南区奥体中心',
 20.00, 100.00, 600, 1200.00,
 DATE_SUB(NOW(), INTERVAL 10 MINUTE), DATE_ADD(NOW(), INTERVAL 20 MINUTE)),

('TASK-SY-003', 4, '沈阳北站周边监控', '交通枢纽安全监控', 'surveillance', 'high', 'pending',
 41.829800, 123.431200, 150.00, '沈阳市沈河区北站路102号(沈阳北站)',
 41.839800, 123.441200, 150.00, '沈阳北站东广场',
 12.00, 150.00, 1800, 1800.00,
 DATE_ADD(NOW(), INTERVAL 1 HOUR), DATE_ADD(NOW(), INTERVAL 2 HOUR));

-- 插入测试空域（沈阳地区）
INSERT INTO low_altitude_traffic_system.airspaces
(airspace_id, name, type, description,
 north_lat, south_lat, east_lng, west_lng,
 min_altitude, max_altitude, status, created_at, updated_at)
VALUES
('AS-SY-001', '沈阳故宫管控空域', 'restricted', '文物保护管控空域',
 41.806700, 41.786700, 123.461200, 123.441200,
 0.00, 200.00, 'active', NOW(), NOW()),

('AS-SY-002', '浑南新区商业空域', 'controlled', '商业配送专用空域',
 41.764500, 41.664500, 123.508700, 123.408700,
 50.00, 300.00, 'active', NOW(), NOW()),

('AS-SY-003', '北站交通枢纽空域', 'controlled', '交通枢纽监控空域',
 41.849800, 41.809800, 123.451200, 123.411200,
 100.00, 250.00, 'active', NOW(), NOW()),

('AS-SY-004', '桃仙机场禁飞区', 'restricted', '机场周边禁飞空域',
 41.739500, 41.539500, 123.583300, 123.383300,
 0.00, 500.00, 'active', NOW(), NOW());

-- 插入测试紧急降落点（沈阳地区）
INSERT INTO low_altitude_traffic_system.emergency_landing_points
(point_code, name, type, lat, lng, altitude, address,
 capacity, status, safety_rating, weather_protected, accessible_24h,
 contact_name, contact_phone, description, created_at, updated_at)
VALUES
('ELP-SY-001', '中国医科大学附属第一医院停机坪', 'hospital',
 41.790500, 123.417800, 45.00, '沈阳市和平区南京北街155号',
 2, 'active', 'A', 1, 1,
 '医院调度中心', '024-83282888', '医院楼顶专用停机坪，24小时待命', NOW(), NOW()),

('ELP-SY-002', '沈阳市人民医院紧急降落区', 'hospital',
 41.804300, 123.420100, 50.00, '沈阳市沈河区文艺路64号',
 1, 'active', 'A', 1, 1,
 '急诊科', '024-24016114', '急诊楼顶紧急降落区', NOW(), NOW()),

('ELP-SY-003', '北陵公园紧急降落区', 'park',
 41.835800, 123.433900, 60.00, '沈阳市皇姑区泰山路12号',
 3, 'active', 'B', 0, 1,
 '公园管理处', '024-86893218', '北陵公园广场开阔地带', NOW(), NOW()),

('ELP-SY-004', '浑南奥体中心应急降落点', 'field',
 41.714700, 123.463300, 55.00, '沈阳市浑南区浑南中路',
 4, 'active', 'A', 0, 1,
 '奥体中心安保', '024-31288888', '奥体中心外广场', NOW(), NOW()),

('ELP-SY-005', '沈阳K11购物中心楼顶停机坪', 'building_roof',
 41.795300, 123.466700, 80.00, '沈阳市沈河区青年大街1号',
 1, 'active', 'B', 1, 0,
 '物业管理', '024-31266666', '购物中心楼顶停机坪，营业时间开放', NOW(), NOW()),

('ELP-SY-006', '沈阳站广场应急降落区', 'field',
 41.793600, 123.446900, 45.00, '沈阳市和平区胜利南街2号',
 2, 'active', 'C', 0, 1,
 '车站安保', '024-12306', '沈阳站南广场开阔区域', NOW(), NOW());
