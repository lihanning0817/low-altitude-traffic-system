# 认证 API 接口测试指南

本文档提供了 AuthController 中所有认证接口的 curl 测试示例。

## 前提条件

1. 确保后端服务正在运行
2. 假设服务运行在 `http://localhost:8080`
3. 数据库已正确配置并连接

## API 接口测试

### 1. 用户注册 (POST /api/v1/auth/register)

**接口描述：** 创建新用户账户

**curl 示例：**
```bash
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "email": "testuser@example.com",
    "password": "Test123456",
    "full_name": "测试用户"
  }'
```

**期待响应：**
```json
{
  "success": true,
  "message": "注册成功",
  "data": {
    "user": {
      "id": 1,
      "username": "testuser",
      "email": "testuser@example.com",
      "full_name": "测试用户",
      "role": "user",
      "status": "active",
      "created_at": 1640995200,
      "updated_at": 1640995200
    }
  }
}
```

### 2. 用户登录 (POST /api/v1/auth/login)

**接口描述：** 用户登录获取访问令牌

**curl 示例：**
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "Test123456"
  }'
```

**使用邮箱登录：**
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser@example.com",
    "password": "Test123456"
  }'
```

**期待响应：**
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
      "email": "testuser@example.com",
      "full_name": "测试用户",
      "role": "user",
      "status": "active",
      "created_at": 1640995200,
      "updated_at": 1640995200
    }
  }
}
```

### 3. 获取当前用户信息 (GET /api/v1/auth/me)

**接口描述：** 根据 JWT Token 获取当前用户信息

**curl 示例：**
```bash
curl -X GET http://localhost:8080/api/v1/auth/me \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN_HERE"
```

**期待响应：**
```json
{
  "success": true,
  "message": "获取用户信息成功",
  "data": {
    "id": 1,
    "username": "testuser",
    "email": "testuser@example.com",
    "full_name": "测试用户",
    "role": "user",
    "status": "active",
    "created_at": 1640995200,
    "updated_at": 1640995200
  }
}
```

### 4. 修改密码 (PUT /api/v1/auth/password)

**接口描述：** 修改当前用户的密码

**curl 示例：**
```bash
curl -X PUT http://localhost:8080/api/v1/auth/password \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN_HERE" \
  -d '{
    "old_password": "Test123456",
    "new_password": "NewTest123456"
  }'
```

**期待响应：**
```json
{
  "success": true,
  "message": "密码修改成功",
  "data": {}
}
```

### 5. Token 刷新 (POST /api/v1/auth/refresh)

**接口描述：** 使用 refresh token 获取新的 access token

**curl 示例：**
```bash
curl -X POST http://localhost:8080/api/v1/auth/refresh \
  -H "Content-Type: application/json" \
  -d '{
    "refresh_token": "YOUR_REFRESH_TOKEN_HERE"
  }'
```

**期待响应：**
```json
{
  "success": true,
  "message": "Token刷新成功",
  "data": {
    "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
    "user": {
      "id": 1,
      "username": "testuser",
      "email": "testuser@example.com",
      "full_name": "测试用户",
      "role": "user",
      "status": "active",
      "created_at": 1640995200,
      "updated_at": 1640995200
    }
  }
}
```

### 6. 用户登出 (POST /api/v1/auth/logout)

**接口描述：** 用户登出，使当前 token 失效

**curl 示例：**
```bash
curl -X POST http://localhost:8080/api/v1/auth/logout \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN_HERE"
```

**期待响应：**
```json
{
  "success": true,
  "message": "登出成功",
  "data": {}
}
```

## 完整测试流程示例

下面是一个完整的测试流程，展示如何串联使用这些接口：

```bash
#!/bin/bash

# 1. 注册新用户
echo "=== 注册用户 ==="
REGISTER_RESPONSE=$(curl -s -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser001",
    "email": "testuser001@example.com",
    "password": "Test123456",
    "full_name": "测试用户001"
  }')
echo $REGISTER_RESPONSE

# 2. 用户登录
echo -e "\n=== 用户登录 ==="
LOGIN_RESPONSE=$(curl -s -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser001",
    "password": "Test123456"
  }')
echo $LOGIN_RESPONSE

# 提取 access_token
ACCESS_TOKEN=$(echo $LOGIN_RESPONSE | grep -o '"access_token":"[^"]*"' | cut -d'"' -f4)
REFRESH_TOKEN=$(echo $LOGIN_RESPONSE | grep -o '"refresh_token":"[^"]*"' | cut -d'"' -f4)

# 3. 获取用户信息
echo -e "\n=== 获取用户信息 ==="
USER_INFO_RESPONSE=$(curl -s -X GET http://localhost:8080/api/v1/auth/me \
  -H "Authorization: Bearer $ACCESS_TOKEN")
echo $USER_INFO_RESPONSE

# 4. 修改密码
echo -e "\n=== 修改密码 ==="
CHANGE_PASSWORD_RESPONSE=$(curl -s -X PUT http://localhost:8080/api/v1/auth/password \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $ACCESS_TOKEN" \
  -d '{
    "old_password": "Test123456",
    "new_password": "NewTest123456"
  }')
echo $CHANGE_PASSWORD_RESPONSE

# 5. 使用新密码重新登录
echo -e "\n=== 使用新密码登录 ==="
NEW_LOGIN_RESPONSE=$(curl -s -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser001",
    "password": "NewTest123456"
  }')
echo $NEW_LOGIN_RESPONSE

# 6. Token 刷新
echo -e "\n=== Token 刷新 ==="
REFRESH_RESPONSE=$(curl -s -X POST http://localhost:8080/api/v1/auth/refresh \
  -H "Content-Type: application/json" \
  -d "{
    \"refresh_token\": \"$REFRESH_TOKEN\"
  }")
echo $REFRESH_RESPONSE

# 7. 登出
echo -e "\n=== 用户登出 ==="
LOGOUT_RESPONSE=$(curl -s -X POST http://localhost:8080/api/v1/auth/logout \
  -H "Authorization: Bearer $ACCESS_TOKEN")
echo $LOGOUT_RESPONSE
```

## 错误响应示例

### 验证错误 (400)
```json
{
  "success": false,
  "message": "用户名长度不能少于3位",
  "error_code": "VALIDATION_ERROR"
}
```

### 认证失败 (401)
```json
{
  "success": false,
  "message": "用户名或密码错误",
  "error_code": "UNAUTHORIZED"
}
```

### 账户被锁定 (423)
```json
{
  "success": false,
  "message": "账户已被锁定，请稍后重试",
  "error_code": "ACCOUNT_LOCKED"
}
```

### 资源冲突 (409)
```json
{
  "success": false,
  "message": "用户名已存在",
  "error_code": "USERNAME_EXISTS"
}
```

## 注意事项

1. **Token 有效期：** Access token 通常有较短的有效期（如15分钟），refresh token 有较长的有效期（如7天）
2. **密码要求：** 密码必须至少8位，包含字母和数字
3. **用户名要求：** 用户名长度3-20位，只能包含字母、数字和下划线
4. **登录锁定：** 连续5次登录失败会锁定账户30分钟
5. **邮箱格式：** 必须是有效的邮箱格式

## 开发提示

- 在实际测试中，将 `YOUR_ACCESS_TOKEN_HERE` 替换为从登录接口返回的真实 token
- 建议使用环境变量或配置文件管理 API 基础URL
- 可以使用 Postman 或类似工具进行更方便的 API 测试
- 测试时注意检查响应状态码和返回的 JSON 结构