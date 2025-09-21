# JWT认证系统实现指南

## 概述

本文档描述了低空交通系统后端的JWT认证系统实现，包括编译、测试和使用说明。

## 系统架构

### 核心组件

1. **JwtService** (`src/auth/JwtService.h/cpp`)
   - JWT Token的生成、验证和解析
   - 支持Access Token和Refresh Token
   - 使用HMAC SHA256签名算法
   - 集成配置管理系统

2. **AuthController** (`src/controllers/AuthController.h/cpp`)
   - 用户认证相关的HTTP API控制器
   - 提供登录、注册、Token刷新等接口
   - 集成用户仓储和JWT服务

3. **HttpResponse** (`src/utils/HttpResponse.h/cpp`)
   - 统一的HTTP响应格式工具
   - 标准化的成功/错误响应格式
   - CORS和安全头设置

4. **UserRepository** (`src/repositories/UserRepository.h/cpp`)
   - 用户数据访问层
   - 数据库操作封装

## 编译和构建

### 前置条件

1. **开发环境**
   - Visual Studio 2019或更高版本
   - CMake 3.16或更高版本
   - vcpkg包管理器

2. **依赖包**
   ```bash
   # 使用vcpkg安装必要的依赖
   vcpkg install boost-beast:x64-windows
   vcpkg install boost-json:x64-windows
   vcpkg install nlohmann-json:x64-windows
   vcpkg install openssl:x64-windows
   vcpkg install mysql-connector-cpp:x64-windows
   vcpkg install spdlog:x64-windows
   vcpkg install curl:x64-windows
   ```

### 构建步骤

1. **配置构建环境**
   ```bash
   # 在backend目录下
   mkdir build
   cd build

   # 配置CMake（确保设置了VCPKG_ROOT环境变量）
   cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
   ```

2. **编译项目**
   ```bash
   # 编译主程序和测试程序
   cmake --build . --config Release

   # 或者使用Debug模式进行开发
   cmake --build . --config Debug
   ```

3. **生成的可执行文件**
   - `low_altitude_traffic_system_backend.exe` - 主服务器程序
   - `test_jwt_service.exe` - JWT服务测试程序
   - `test_auth_controller.exe` - AuthController测试程序
   - `test_user_repository.exe` - 用户仓储测试程序

## 配置说明

### 服务器配置文件 (config/server.json)

```json
{
  "security": {
    "jwt_secret": "your-super-secure-jwt-secret-key-256-bits-change-in-production-2024",
    "jwt_issuer": "low-altitude-traffic-system",
    "jwt_access_token_expiry": 900,      // 15分钟
    "jwt_refresh_token_expiry": 604800,  // 7天
    "jwt_algorithm": "HS256",
    "password_min_length": 8,
    "max_login_attempts": 5,
    "lockout_duration": 1800            // 30分钟
  }
}
```

**重要说明：**
- 生产环境中必须更改`jwt_secret`为强随机密钥（至少256位）
- 建议通过环境变量设置敏感配置

## 测试指南

### 1. JWT服务测试

```bash
# 运行JWT服务测试
./test_jwt_service.exe
```

**测试内容：**
- ✅ 生成Access Token
- ✅ 生成Refresh Token
- ✅ 验证有效Token
- ✅ 解析Token信息
- ✅ 提取用户ID
- ✅ 提取JWT ID
- ✅ 获取Token剩余时间
- ✅ 验证无效Token
- ✅ 验证篡改的Token
- ✅ 生成自定义过期时间Token
- ✅ Token即将过期检查
- ✅ 不同角色用户Token

### 2. AuthController测试

```bash
# 运行AuthController测试（需要数据库连接）
./test_auth_controller.exe
```

**测试内容：**
- ✅ 用户注册功能
- ✅ 重复注册检测
- ✅ 用户登录功能
- ✅ 错误密码登录拒绝
- ✅ 获取当前用户信息
- ✅ Token刷新
- ✅ 修改密码
- ✅ 新密码登录
- ✅ 用户登出
- ✅ 参数验证

### 3. 用户仓储测试

```bash
# 运行用户仓储测试
./test_user_repository.exe
```

## API接口文档

### 认证相关接口

#### 1. 用户注册
```http
POST /api/auth/register
Content-Type: application/json

{
  "username": "testuser",
  "email": "test@example.com",
  "password": "SecurePass123",
  "full_name": "Test User"
}
```

**响应格式：**
```json
{
  "success": true,
  "message": "注册成功",
  "data": {
    "user": {
      "id": 1,
      "username": "testuser",
      "email": "test@example.com",
      "full_name": "Test User",
      "role": "USER",
      "status": "ACTIVE"
    }
  },
  "timestamp": 1635724800
}
```

#### 2. 用户登录
```http
POST /api/auth/login
Content-Type: application/json

{
  "username": "testuser",  // 可以是用户名或邮箱
  "password": "SecurePass123"
}
```

**响应格式：**
```json
{
  "success": true,
  "message": "登录成功",
  "data": {
    "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
    "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
    "user": {
      "id": 1,
      "username": "testuser",
      "email": "test@example.com",
      "full_name": "Test User",
      "role": "USER",
      "status": "ACTIVE"
    }
  },
  "timestamp": 1635724800
}
```

#### 3. Token刷新
```http
POST /api/auth/refresh
Content-Type: application/json

{
  "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
}
```

#### 4. 获取当前用户信息
```http
GET /api/auth/me
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

#### 5. 修改密码
```http
PUT /api/auth/password
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
Content-Type: application/json

{
  "old_password": "OldPass123",
  "new_password": "NewPass456"
}
```

#### 6. 用户登出
```http
POST /api/auth/logout
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

## 安全特性

### 1. 密码安全
- 使用bcrypt进行密码哈希
- 密码强度验证（最少8位，包含字母和数字）
- 登录失败次数限制（5次失败后锁定30分钟）

### 2. JWT安全
- HMAC SHA256签名算法
- 访问令牌短期有效（15分钟）
- 刷新令牌长期有效（7天）
- JWT ID用于令牌追踪和撤销

### 3. HTTP安全
- CORS头设置
- 安全头配置（X-Content-Type-Options, X-Frame-Options等）
- Bearer Token认证

### 4. 数据验证
- 输入参数严格验证
- 邮箱格式验证
- 用户名格式验证
- SQL注入防护（通过参数化查询）

## 错误处理

### 常见错误代码

| 错误代码 | HTTP状态 | 描述 |
|---------|---------|------|
| VALIDATION_ERROR | 400 | 请求参数验证失败 |
| INVALID_JSON | 400 | JSON格式错误 |
| WEAK_PASSWORD | 400 | 密码强度不足 |
| UNAUTHORIZED | 401 | 认证失败 |
| FORBIDDEN | 403 | 权限不足 |
| USERNAME_EXISTS | 409 | 用户名已存在 |
| EMAIL_EXISTS | 409 | 邮箱已注册 |
| ACCOUNT_LOCKED | 423 | 账户被锁定 |
| INTERNAL_ERROR | 500 | 服务器内部错误 |

### 错误响应格式
```json
{
  "success": false,
  "message": "用户名或密码错误",
  "error_code": "UNAUTHORIZED",
  "timestamp": 1635724800
}
```

## 性能优化建议

### 1. 数据库优化
- 为用户名和邮箱建立唯一索引
- 使用连接池管理数据库连接
- 实现适当的数据库缓存策略

### 2. 缓存策略
- 使用Redis缓存用户会话信息
- 实现JWT黑名单缓存
- 缓存用户权限信息

### 3. 安全增强
- 实现JWT令牌撤销机制
- 添加访问频率限制
- 集成WAF和DDoS防护

## 部署说明

### 1. 生产环境配置
```json
{
  "security": {
    "jwt_secret": "${JWT_SECRET}",  // 从环境变量读取
    "jwt_issuer": "low-altitude-traffic-system-prod",
    "jwt_access_token_expiry": 900,
    "jwt_refresh_token_expiry": 604800
  }
}
```

### 2. 环境变量设置
```bash
# 设置JWT密钥（生产环境）
set JWT_SECRET=your-production-secret-key-256-bits-minimum

# 设置数据库连接
set DB_HOST=localhost
set DB_PORT=3306
set DB_NAME=low_altitude_traffic_system
set DB_USER=app_user
set DB_PASS=secure_password
```

### 3. 服务启动
```bash
# 启动主服务器
./low_altitude_traffic_system_backend.exe

# 或使用服务管理器
sc create LowAltitudeTrafficSystem binPath="path\to\backend.exe"
sc start LowAltitudeTrafficSystem
```

## 故障排除

### 常见问题

1. **编译错误：找不到依赖包**
   - 确保vcpkg正确安装所有依赖
   - 检查CMAKE_TOOLCHAIN_FILE设置

2. **数据库连接失败**
   - 检查数据库服务器状态
   - 验证连接配置信息
   - 确保数据库用户权限正确

3. **JWT验证失败**
   - 检查密钥配置是否正确
   - 验证时间同步
   - 确认Token格式完整性

4. **内存泄漏**
   - 使用智能指针管理资源
   - 正确关闭数据库连接
   - 定期检查日志文件大小

## 开发和扩展

### 添加新的认证方法
1. 继承AuthController基类
2. 实现具体的认证逻辑
3. 注册到路由系统

### 自定义权限验证
1. 扩展UserRole枚举
2. 修改JWT Payload结构
3. 更新权限检查逻辑

---

**注意：** 本系统处于开发阶段，部分功能（如JWT黑名单、Redis缓存）需要在后续迭代中完善。生产使用前请务必进行充分的安全审计和性能测试。