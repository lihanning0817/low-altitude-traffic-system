#ifndef MOCK_DATA_H
#define MOCK_DATA_H

#include <nlohmann/json.hpp>
#include <ctime>

/**
 * @file MockData.h
 * @brief Mock数据配置文件
 *
 * 集中管理系统中使用的Mock数据,便于维护和测试
 * 实际项目中这些数据应该从数据库查询
 */

namespace config {

/**
 * @brief 获取Mock无人机数据
 * @return JSON数组,包含4个无人机的Mock数据
 *
 * 用于系统监控模块的无人机列表展示
 * 实际项目中应该从drones表查询
 */
inline nlohmann::json getMockDroneData() {
    return nlohmann::json::array({
        {
            {"id", 1},
            {"drone_id", "DJI-001"},
            {"name", "Phantom 4 Pro"},
            {"model", "Phantom 4 Pro"},
            {"status", "active"},
            {"battery", 85.5},
            {"location", {
                {"lat", 39.904200},
                {"lng", 116.407396}
            }},
            {"last_update", std::time(nullptr) - 300} // 5分钟前更新
        },
        {
            {"id", 2},
            {"drone_id", "DJI-002"},
            {"name", "Mavic 3"},
            {"model", "Mavic 3"},
            {"status", "active"},
            {"battery", 72.3},
            {"location", {
                {"lat", 39.914200},
                {"lng", 116.417396}
            }},
            {"last_update", std::time(nullptr) - 120} // 2分钟前更新
        },
        {
            {"id", 3},
            {"drone_id", "DJI-003"},
            {"name", "Mini 3 Pro"},
            {"model", "Mini 3 Pro"},
            {"status", "maintenance"},
            {"battery", 0.0},
            {"location", {
                {"lat", 39.924200},
                {"lng", 116.427396}
            }},
            {"last_update", std::time(nullptr) - 3600} // 1小时前更新
        },
        {
            {"id", 4},
            {"drone_id", "DJI-004"},
            {"name", "Air 2S"},
            {"model", "Air 2S"},
            {"status", "inactive"},
            {"battery", 45.8},
            {"location", {
                {"lat", 39.934200},
                {"lng", 116.437396}
            }},
            {"last_update", std::time(nullptr) - 1800} // 30分钟前更新
        }
    });
}

} // namespace config

#endif // MOCK_DATA_H
