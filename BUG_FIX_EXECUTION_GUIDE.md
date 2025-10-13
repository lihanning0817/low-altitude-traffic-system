# Bug修复执行指南

> **目标**: 将项目完善到可申请软著、零Bug的程度
> **创建时间**: 2025-10-12
> **预估总时间**: 40-50小时

---

## 📋 修复进度追踪

### 总体进度
- [ ] **Critical级别** (3个) - 0/3 完成
- [ ] **High级别** (7个) - 0/7 完成
- [ ] **Medium级别** (9个) - 0/9 完成
- [ ] **Low级别** (4个) - 0/4 完成

---

## 🔴 Critical级别修复（必须优先完成）

### ✅ Bug #1: emergency_events表结构不匹配
**状态**: ⏸️ 待执行
**优先级**: P0
**预估时间**: 2-3小时

**修复步骤**:
```bash
# Step 1: 备份当前数据库
cd D:\low-altitude-traffic-system
mysqldump -u root -p low_altitude_traffic_system > database/backup_$(date +%Y%m%d).sql

# Step 2: 应用迁移脚本
mysql -u root -p low_altitude_traffic_system < database/migration_emergency_events_fix.sql

# Step 3: 验证表结构
mysql -u root -p -e "DESCRIBE low_altitude_traffic_system.emergency_events"

# Step 4: 测试API
curl -X POST http://localhost:8081/api/v1/emergency/events \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -d '{
    "event_code": "EMG-20251012-0001",
    "task_id": 1,
    "type": "equipment_failure",
    "severity": "high",
    "status": "active",
    "title": "测试事件",
    "location": {"lat": 39.9042, "lon": 116.4074}
  }'
```

**验收标准**:
- [x] 表结构匹配后端模型
- [ ] API测试通过
- [ ] 前端应急响应功能正常

---

### ✅ Bug #2: WeatherService CURL资源泄漏
**状态**: 🚧 进行中
**优先级**: P0
**预估时间**: 1-2小时

**已完成**:
- ✅ 创建了 `CurlHandle.h` RAII类

**待完成步骤**:

1. **修改 WeatherService.cpp**
```cpp
// 在文件顶部添加
#include "../utils/CurlHandle.h"

// 修改 makeHttpRequest 方法
std::string WeatherService::makeHttpRequest(const std::string& url) {
    std::string response;

    try {
        // 使用RAII管理CURL资源
        utils::CurlHandle curl;

        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 0L);

        CURLcode res = curl.perform();

        if (res != CURLE_OK) {
            throw std::runtime_error("CURL request failed: " +
                                    utils::CurlHandle::getErrorString(res));
        }

        long http_code = 0;
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code != 200) {
            throw std::runtime_error("HTTP error: " + std::to_string(http_code));
        }

        return response;
        // curl 在作用域结束时自动清理

    } catch (const std::exception& e) {
        // 异常情况下curl也会自动清理
        throw;
    }
}
```

2. **修改 RouteService.cpp**（同样的修改）

3. **编译测试**
```bash
cd backend
cmake --build build --config Release
```

4. **运行内存泄漏检测**（如果有工具）

**验收标准**:
- [ ] 代码编译通过
- [ ] API正常工作
- [ ] 长时间运行无内存泄漏

---

### ✅ Bug #3: SystemMonitorController std::stoi异常
**状态**: ⏸️ 待执行
**优先级**: P0
**预估时间**: 2小时

**修复步骤**:

1. **创建安全参数解析工具**

文件: `backend/src/utils/ParamParser.h`
```cpp
#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace utils {

/**
 * @brief 安全的参数解析工具类
 */
class ParamParser {
public:
    /**
     * @brief 安全解析整数参数
     * @param params 参数映射
     * @param key 参数键
     * @param defaultValue 默认值
     * @param minValue 最小值
     * @param maxValue 最大值
     * @return int 解析后的值（已约束在min-max范围内）
     */
    static int parseIntParam(
        const std::unordered_map<std::string, std::string>& params,
        const std::string& key,
        int defaultValue,
        int minValue = INT_MIN,
        int maxValue = INT_MAX
    ) {
        // 参数不存在，返回默认值
        if (params.find(key) == params.end()) {
            return defaultValue;
        }

        try {
            int value = std::stoi(params.at(key));

            // 约束到合法范围
            value = std::clamp(value, minValue, maxValue);

            return value;

        } catch (const std::invalid_argument& e) {
            spdlog::warn("[ParamParser] Invalid integer parameter '{}': '{}', using default: {}",
                        key, params.at(key), defaultValue);
            return defaultValue;

        } catch (const std::out_of_range& e) {
            spdlog::warn("[ParamParser] Out of range parameter '{}': '{}', using default: {}",
                        key, params.at(key), defaultValue);
            return defaultValue;
        }
    }

    /**
     * @brief 安全解析浮点参数
     */
    static double parseDoubleParam(
        const std::unordered_map<std::string, std::string>& params,
        const std::string& key,
        double defaultValue,
        double minValue = -DBL_MAX,
        double maxValue = DBL_MAX
    ) {
        if (params.find(key) == params.end()) {
            return defaultValue;
        }

        try {
            double value = std::stod(params.at(key));
            value = std::clamp(value, minValue, maxValue);
            return value;

        } catch (const std::exception& e) {
            spdlog::warn("[ParamParser] Invalid double parameter '{}': '{}', using default: {}",
                        key, params.at(key), defaultValue);
            return defaultValue;
        }
    }

    /**
     * @brief 安全解析布尔参数
     */
    static bool parseBoolParam(
        const std::unordered_map<std::string, std::string>& params,
        const std::string& key,
        bool defaultValue
    ) {
        if (params.find(key) == params.end()) {
            return defaultValue;
        }

        std::string value = params.at(key);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (value == "true" || value == "1" || value == "yes") {
            return true;
        } else if (value == "false" || value == "0" || value == "no") {
            return false;
        }

        return defaultValue;
    }
};

} // namespace utils
```

2. **修改所有使用 std::stoi 的控制器**

在 `SystemMonitorController.cpp`、`EmergencyController.cpp` 等文件中:
```cpp
#include "../utils/ParamParser.h"

// 替换所有 std::stoi 调用
// 旧代码:
// int page = params.count("page") ? std::stoi(params["page"]) : 1;

// 新代码:
int page = utils::ParamParser::parseIntParam(params, "page", 1, 1, 1000);
int page_size = utils::ParamParser::parseIntParam(params, "page_size", 20, 1, 100);
```

3. **添加单元测试**（可选但推荐）

4. **测试恶意输入**
```bash
# 测试非数字输入
curl "http://localhost:8081/api/v1/users?page=abc&page_size=xyz"

# 测试超大数字
curl "http://localhost:8081/api/v1/users?page=99999999999&page_size=-100"

# 应该返回正常结果而不是崩溃
```

**验收标准**:
- [ ] 所有std::stoi调用已替换
- [ ] 恶意输入不会导致崩溃
- [ ] 日志中记录警告信息

---

## 🟠 High级别修复

### Bug #4: JSON解析异常处理不细化
**位置**: `EmergencyController.cpp:88`
**修复**: 添加 `nlohmann::json::parse_error` 专门捕获

### Bug #5: Vue watch无限循环风险
**位置**: `WeatherIntegration.vue:1682`
**修复**: 添加防抖和条件检查

### Bug #6: 地图菜单坐标计算错误
**位置**: `MapComponent.vue:438`
**修复**: 使用地图容器的getBoundingClientRect

### Bug #7: SSL验证被禁用
**位置**: `WeatherService.cpp:37`
**修复**: 从配置文件读取SSL选项

### Bug #8: 分页total值错误
**位置**: `EmergencyResponse.vue:542`
**修复**: 后端返回真实总数

### Bug #9: 非线程安全的gmtime
**位置**: `SystemMonitorController.cpp:68`
**修复**: 使用 gmtime_r 或 gmtime_s

### Bug #10: HTTP请求未取消
**位置**: `WeatherIntegration.vue:1265`
**修复**: 使用 AbortController

---

## 🟡 Medium级别修复

*(详细修复步骤见PROJECT_HANDOVER_DOCUMENT.md)*

---

## 🔵 Low级别修复

### Bug #20: 魔法数字
**修复**: 定义常量
```cpp
static constexpr long DEFAULT_TIMEOUT = 30L;
static constexpr long DEFAULT_CONNECT_TIMEOUT = 10L;
```

### Bug #21: 枚举值硬编码
**修复**: 创建 `src/constants/emergencyConstants.js`

### Bug #22: console.log未清理
**修复**: 使用环境变量控制或Vite插件移除

### Bug #23: 内存对齐
**修复**: 按成员大小排序

---

## 📝 配置安全加固

### 1. 创建配置模板

**文件**: `backend/config/server.json.template`
```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8081,
    "threads": 4
  },
  "database": {
    "host": "localhost",
    "port": 33060,
    "database": "low_altitude_traffic_system",
    "username": "root",
    "password": "${DB_PASSWORD}",
    "connection_pool_size": 10
  },
  "security": {
    "jwt_secret": "${JWT_SECRET}",
    "jwt_issuer": "low-altitude-traffic-system",
    "jwt_access_token_expiry": 900,
    "jwt_refresh_token_expiry": 604800
  },
  "external_apis": {
    "amap": {
      "key": "${AMAP_API_KEY}"
    },
    "weather": {
      "key": "${WEATHER_API_KEY}"
    }
  }
}
```

### 2. 更新.gitignore
```
# 配置文件
backend/config/server.json
*.local.json

# 日志
logs/
*.log

# 构建产物
backend/build/
backend/Release/
backend/Debug/
node_modules/
dist/
```

### 3. 创建环境变量设置指南

**文件**: `backend/config/ENV_SETUP.md`
```markdown
# 环境变量配置指南

## Windows (PowerShell)
```powershell
$env:DB_PASSWORD="your_db_password"
$env:JWT_SECRET="your_jwt_secret_key"
$env:AMAP_API_KEY="your_amap_key"
$env:WEATHER_API_KEY="your_weather_key"
```

## Linux/Mac
```bash
export DB_PASSWORD="your_db_password"
export JWT_SECRET="your_jwt_secret_key"
export AMAP_API_KEY="your_amap_key"
export WEATHER_API_KEY="your_weather_key"
```

## 部署建议
生产环境建议使用密钥管理服务（如AWS Secrets Manager、Azure Key Vault等）
```

---

## 🗄️ 数据库优化

### 应用索引优化
```bash
mysql -u root -p low_altitude_traffic_system < database/optimize_indexes.sql
```

### 验证索引
```sql
SHOW INDEX FROM flight_tasks;
SHOW INDEX FROM users;
SHOW INDEX FROM emergency_events;
```

---

## 📚 软著申请文档准备

### 需要完善的文档

1. **软件设计说明书** (必需)
2. **用户手册** (必需)
3. **源代码** (部分)

### 文档模板

#### 1. 软件设计说明书目录

```
1. 引言
   1.1 编写目的
   1.2 背景
   1.3 定义
   1.4 参考资料

2. 总体设计
   2.1 需求规定
   2.2 运行环境
   2.3 基本设计概念和处理流程
   2.4 结构

3. 程序设计说明
   3.1 后端模块设计
   3.2 前端模块设计
   3.3 数据库设计
   3.4 接口设计

4. 运行设计
   4.1 运行模块组合
   4.2 运行控制
   4.3 运行时间

5. 出错处理设计
   5.1 出错输出信息
   5.2 补救措施
   5.3 系统维护设计
```

#### 2. 用户手册目录

```
1. 引言
   1.1 编写目的
   1.2 背景
   1.3 定义
   1.4 参考资料

2. 软件概述
   2.1 目标
   2.2 功能
   2.3 性能

3. 运行环境
   3.1 硬件设备
   3.2 支持软件
   3.3 数据库

4. 使用说明
   4.1 安装和初始化
   4.2 输入
   4.3 输出
   4.4 出错处理和恢复
   4.5 终端操作
```

---

## ✅ 验收检查清单

### 代码质量
- [ ] 所有Critical Bug已修复
- [ ] 所有High Bug已修复
- [ ] 所有Medium Bug已修复
- [ ] 所有Low问题已清理
- [ ] 代码编译无警告
- [ ] 无内存泄漏

### 功能测试
- [ ] 用户认证流程正常
- [ ] 飞行任务CRUD正常
- [ ] 应急事件管理正常
- [ ] 天气集成正常
- [ ] 地图功能正常
- [ ] 系统监控正常

### 安全性
- [ ] 敏感配置已移除
- [ ] 使用环境变量
- [ ] SQL注入防护有效
- [ ] XSS防护有效
- [ ] API限流已实施

### 性能
- [ ] 数据库索引已优化
- [ ] API响应时间<200ms
- [ ] 前端加载时间<3s
- [ ] 无明显内存泄漏

### 文档
- [ ] README.md完整
- [ ] API文档完整
- [ ] 软件设计说明书完成
- [ ] 用户手册完成
- [ ] 部署文档完成

---

## 🎯 后续工作

### 完成Bug修复后

1. **添加单元测试**
   - 后端Google Test
   - 前端Vitest
   - 目标覆盖率: 60%+

2. **CI/CD流程**
   - GitHub Actions配置
   - 自动化测试
   - 自动化部署

3. **性能优化**
   - 添加Redis缓存
   - CDN配置
   - 图片压缩

4. **安全加固**
   - HTTPS配置
   - API限流
   - 日志审计

---

**最后更新**: 2025-10-12
**负责人**: Claude (AI Assistant)
**项目状态**: 🚧 进行中
