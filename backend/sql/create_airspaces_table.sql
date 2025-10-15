-- ====================================================
-- 空域管理表 (airspaces)
-- 用于管理低空空域信息，包括限制区、管制区、警告区、禁飞区等
-- ====================================================

CREATE TABLE IF NOT EXISTS `airspaces` (
  `id` BIGINT NOT NULL AUTO_INCREMENT COMMENT '空域ID',
  `airspace_code` VARCHAR(50) NOT NULL UNIQUE COMMENT '空域编号（唯一标识）',
  `name` VARCHAR(100) NOT NULL COMMENT '空域名称',
  `type` VARCHAR(20) NOT NULL COMMENT '空域类型: restricted(限制区), controlled(管制区), warning(警告区), prohibited(禁飞区)',
  `status` VARCHAR(20) NOT NULL DEFAULT 'active' COMMENT '空域状态: active(激活), inactive(停用), maintenance(维护中)',
  `min_altitude` DECIMAL(10,2) NOT NULL DEFAULT 0.00 COMMENT '最低飞行高度（米）',
  `max_altitude` DECIMAL(10,2) NOT NULL DEFAULT 500.00 COMMENT '最高飞行高度（米）',
  `geometry` TEXT NOT NULL COMMENT '空域几何信息（GeoJSON格式，存储多边形边界坐标）',
  `description` TEXT COMMENT '空域描述信息',
  `effective_time` DATETIME COMMENT '空域生效时间',
  `expiry_time` DATETIME COMMENT '空域失效时间',
  `created_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`),
  INDEX `idx_airspace_code` (`airspace_code`),
  INDEX `idx_type` (`type`),
  INDEX `idx_status` (`status`),
  INDEX `idx_effective_time` (`effective_time`),
  INDEX `idx_expiry_time` (`expiry_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='空域管理表';

-- ====================================================
-- 插入示例数据
-- ====================================================

INSERT INTO `airspaces` (
  `airspace_code`,
  `name`,
  `type`,
  `status`,
  `min_altitude`,
  `max_altitude`,
  `geometry`,
  `description`,
  `effective_time`,
  `expiry_time`
) VALUES
(
  'AS20240101001',
  '中心城区禁飞区',
  'prohibited',
  'active',
  0.00,
  500.00,
  '{
    "type": "Polygon",
    "coordinates": [
      [
        [116.395, 39.905],
        [116.415, 39.905],
        [116.415, 39.925],
        [116.395, 39.925],
        [116.395, 39.905]
      ]
    ]
  }',
  '市中心核心区域，全天候禁止无人机飞行',
  '2024-01-01 00:00:00',
  '2025-12-31 23:59:59'
),
(
  'AS20240101002',
  '机场管制区',
  'controlled',
  'active',
  0.00,
  1000.00,
  '{
    "type": "Polygon",
    "coordinates": [
      [
        [116.580, 40.065],
        [116.620, 40.065],
        [116.620, 40.095],
        [116.580, 40.095],
        [116.580, 40.065]
      ]
    ]
  }',
  '首都国际机场周边管制区，需要特殊许可',
  '2024-01-01 00:00:00',
  NULL
),
(
  'AS20240101003',
  '商业飞行限制区',
  'restricted',
  'active',
  0.00,
  300.00,
  '{
    "type": "Polygon",
    "coordinates": [
      [
        [116.300, 39.850],
        [116.350, 39.850],
        [116.350, 39.900],
        [116.300, 39.900],
        [116.300, 39.850]
      ]
    ]
  }',
  '商业区域，限制飞行高度和时间',
  '2024-01-01 00:00:00',
  '2024-12-31 23:59:59'
),
(
  'AS20240101004',
  '公园飞行警告区',
  'warning',
  'active',
  0.00,
  150.00,
  '{
    "type": "Polygon",
    "coordinates": [
      [
        [116.380, 39.990],
        [116.400, 39.990],
        [116.400, 40.010],
        [116.380, 40.010],
        [116.380, 39.990]
      ]
    ]
  }',
  '奥林匹克公园区域，需注意游客安全',
  '2024-01-01 00:00:00',
  NULL
);

-- ====================================================
-- 查询示例
-- ====================================================

-- 1. 查询所有激活状态的空域
-- SELECT * FROM airspaces WHERE status = 'active';

-- 2. 查询禁飞区
-- SELECT * FROM airspaces WHERE type = 'prohibited';

-- 3. 查询指定高度范围的空域
-- SELECT * FROM airspaces WHERE min_altitude <= 200 AND max_altitude >= 200;

-- 4. 查询当前有效的空域
-- SELECT * FROM airspaces
-- WHERE status = 'active'
-- AND (effective_time IS NULL OR effective_time <= NOW())
-- AND (expiry_time IS NULL OR expiry_time >= NOW());
