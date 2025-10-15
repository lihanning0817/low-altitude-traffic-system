# 需要添加到 main.cpp 的路由注册代码

## 1. 引入头文件

在 main.cpp 文件顶部添加以下头文件引入：

```cpp
#include "controllers/AirspaceController.h"
#include "controllers/DroneController.h"
```

## 2. 创建控制器实例

在 main() 函数中创建数据库会话和JWT服务之后，添加以下控制器实例：

```cpp
// 创建空域管理控制器
auto airspaceController = std::make_shared<controllers::AirspaceController>(dbSession, jwtService);

// 创建无人机管理控制器
auto droneController = std::make_shared<controllers::DroneController>(dbSession, jwtService);
```

## 3. 注册 AirspaceController 路由

在路由注册部分添加以下代码：

```cpp
// ========== 空域管理路由 (AirspaceController) ==========

// GET /api/v1/airspaces - 获取空域列表
if (method == http::verb::get && target.starts_with("/api/v1/airspaces") &&
    target.find('/', 17) == std::string::npos) // 确保没有子路径
{
    return airspaceController->getAirspaces(req);
}

// GET /api/v1/airspaces/{id} - 获取空域详情
if (method == http::verb::get && target.starts_with("/api/v1/airspaces/"))
{
    std::string airspace_id = std::string(target.substr(18));
    if (!airspace_id.empty() && airspace_id.find('/') == std::string::npos)
    {
        return airspaceController->getAirspaceById(req, airspace_id);
    }
}

// POST /api/v1/airspaces - 创建空域（管理员）
if (method == http::verb::post && target == "/api/v1/airspaces")
{
    return airspaceController->createAirspace(req);
}

// PUT /api/v1/airspaces/{id} - 更新空域（管理员）
if (method == http::verb::put && target.starts_with("/api/v1/airspaces/"))
{
    std::string airspace_id = std::string(target.substr(18));
    if (!airspace_id.empty() && airspace_id.find('/') == std::string::npos)
    {
        return airspaceController->updateAirspace(req, airspace_id);
    }
}

// DELETE /api/v1/airspaces/{id} - 删除空域（管理员）
if (method == http::verb::delete_ && target.starts_with("/api/v1/airspaces/"))
{
    std::string airspace_id = std::string(target.substr(18));
    if (!airspace_id.empty() && airspace_id.find('/') == std::string::npos)
    {
        return airspaceController->deleteAirspace(req, airspace_id);
    }
}
```

## 4. 注册 DroneController 路由

```cpp
// ========== 无人机管理路由 (DroneController) ==========

// GET /api/v1/drones - 获取无人机列表
if (method == http::verb::get && target.starts_with("/api/v1/drones"))
{
    // 检查是否是详情查询（带ID）
    if (target.length() > 15 && target[14] == '/')
    {
        std::string drone_id = std::string(target.substr(15));
        if (!drone_id.empty() && drone_id.find('/') == std::string::npos && drone_id.find('?') == std::string::npos)
        {
            return droneController->getDroneById(req, drone_id);
        }
    }
    else
    {
        return droneController->getDrones(req);
    }
}

// GET /api/v1/drones/{id} - 获取无人机详情（已包含在上面）
```

## 5. 更新 FlightPermitController 路由

在现有的 FlightPermitController 路由中添加以下代码：

```cpp
// GET /api/v1/flight-permits/{id} - 获取飞行许可详情
if (method == http::verb::get && target.starts_with("/api/v1/flight-permits/"))
{
    std::string permit_id_str = std::string(target.substr(23));

    // 检查是否是特定ID（不是空字符串且不包含其他路径）
    if (!permit_id_str.empty() && permit_id_str.find('/') == std::string::npos &&
        permit_id_str.find('?') == std::string::npos)
    {
        return flightPermitController->getFlightPermitById(req, permit_id_str);
    }
}

// POST /api/v1/flight-permits/{id}/revoke - 撤销飞行许可
if (method == http::verb::post && target.find("/api/v1/flight-permits/") == 0 &&
    target.ends_with("/revoke"))
{
    size_t start = 23; // "/api/v1/flight-permits/".length()
    size_t end = target.find("/revoke");
    std::string permit_id = std::string(target.substr(start, end - start));
    return flightPermitController->revokeFlightPermit(req, permit_id);
}
```

## 6. 更新 EmergencyLandingController 路由

在现有的 EmergencyLandingController 路由中添加以下代码：

```cpp
// GET /api/v1/emergency-landing-points/{id} - 获取紧急降落点详情
if (method == http::verb::get && target.starts_with("/api/v1/emergency-landing-points/"))
{
    std::string point_id_str = std::string(target.substr(34));

    // 检查是否是特定ID
    if (!point_id_str.empty() && point_id_str.find('/') == std::string::npos &&
        point_id_str.find('?') == std::string::npos && point_id_str != "nearest")
    {
        return emergencyLandingController->getEmergencyLandingPointById(req, point_id_str);
    }
}

// DELETE /api/v1/emergency-landing-points/{id} - 删除紧急降落点（管理员）
if (method == http::verb::delete_ && target.starts_with("/api/v1/emergency-landing-points/"))
{
    std::string point_id = std::string(target.substr(34));
    if (!point_id.empty() && point_id.find('/') == std::string::npos)
    {
        return emergencyLandingController->deleteEmergencyLandingPoint(req, point_id);
    }
}
```

## 7. 更新 ConflictDetectionController 路由

在现有的 ConflictDetectionController 路由中添加以下代码：

```cpp
// POST /api/v1/conflict-detection/flights/batch - 批量检测飞行冲突
if (method == http::verb::post && target == "/api/v1/conflict-detection/flights/batch")
{
    return conflictDetectionController->detectMultipleFlightConflicts(req);
}

// GET /api/v1/conflict-detection/conflicts/statistics - 获取冲突统计
if (method == http::verb::get && target.starts_with("/api/v1/conflict-detection/conflicts/statistics"))
{
    return conflictDetectionController->getConflictStatistics(req);
}

// GET /api/v1/conflict-detection/conflicts/{id} - 获取冲突详情
if (method == http::verb::get && target.starts_with("/api/v1/conflict-detection/conflicts/"))
{
    std::string conflict_id_str = std::string(target.substr(39));

    // 检查是否是特定ID（不是statistics）
    if (!conflict_id_str.empty() && conflict_id_str != "statistics" &&
        conflict_id_str.find('/') == std::string::npos && conflict_id_str.find('?') == std::string::npos)
    {
        return conflictDetectionController->getConflictById(req, conflict_id_str);
    }
}
```

## 8. 数据库建表

在部署之前，请运行以下 SQL 脚本创建 airspaces 表：

```bash
mysql -u root -p low_altitude_traffic_system < backend/sql/create_airspaces_table.sql
```

或者在 MySQL 客户端中执行该文件的内容。

## 注意事项

1. **路由顺序很重要**：确保更具体的路由（如带参数的路由）在更通用的路由之前注册
2. **字符串比较**：使用 `starts_with()` 和 `ends_with()` 进行路径匹配
3. **参数提取**：使用 `substr()` 从URL中提取ID参数
4. **参数验证**：检查提取的ID不为空且不包含其他路径分隔符
5. **HTTP方法**：注意 DELETE 方法在 Boost.Beast 中是 `http::verb::delete_`（带下划线）

## 编译注意事项

确保在 CMakeLists.txt 或 Makefile 中包含新的源文件：
- `backend/src/controllers/AirspaceController.cpp`
- `backend/src/controllers/DroneController.cpp`

如果使用 CMake，通常会自动检测新文件，但可能需要重新运行 cmake 配置。
