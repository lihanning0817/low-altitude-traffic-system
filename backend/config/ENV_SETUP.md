# 环境变量配置指南

本文档说明如何为低空交通系统后端配置环境变量和敏感信息。

## 快速开始

### 1. 复制配置模板

```bash
cd backend/config
cp server.json.template server.json
```

### 2. 编辑配置文件

使用文本编辑器打开 `server.json`,替换以下占位符:

- `YOUR_DB_USERNAME` → 数据库用户名 (例如: `root`)
- `YOUR_DB_PASSWORD` → 数据库密码
- `YOUR_JWT_SECRET_AT_LEAST_32_CHARS` → JWT密钥 (至少32字符)
- `YOUR_AMAP_API_KEY` → 高德地图API密钥
- `YOUR_WEATHER_API_KEY` → OpenWeather API密钥

## 配置项说明

### 数据库配置 (`database`)

```json
{
  "host": "localhost",        // 数据库主机地址
  "port": 33060,             // MySQL X Protocol端口 (标准端口为3306)
  "database": "low_altitude_traffic_system",  // 数据库名称
  "username": "YOUR_DB_USERNAME",   // 数据库用户名
  "password": "YOUR_DB_PASSWORD",   // 数据库密码
  "connection_pool_size": 10,
  "connection_timeout": 10
}
```

**注意**:
- 如果使用标准MySQL连接,端口应设置为 `3306`
- 如果使用MySQL X Protocol,端口应设置为 `33060`

### 安全配置 (`security`)

```json
{
  "jwt_secret": "YOUR_JWT_SECRET_AT_LEAST_32_CHARS",  // JWT密钥
  "jwt_issuer": "low-altitude-traffic-system",
  "jwt_access_token_expiry": 900,      // Access Token过期时间(秒) - 15分钟
  "jwt_refresh_token_expiry": 604800,  // Refresh Token过期时间(秒) - 7天
  "jwt_algorithm": "HS256",
  "password_min_length": 8,
  "max_login_attempts": 5,
  "lockout_duration": 1800             // 锁定时长(秒) - 30分钟
}
```

**JWT密钥生成方法**:

**Windows (PowerShell)**:
```powershell
# 生成32字节随机密钥
$bytes = New-Object Byte[] 32
[Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes)
[Convert]::ToBase64String($bytes)
```

**Linux/Mac**:
```bash
# 生成32字节随机密钥
openssl rand -base64 32
```

### 外部API配置 (`external_apis`)

#### 高德地图API (`amap`)

```json
{
  "key": "YOUR_AMAP_API_KEY",
  "base_url": "https://restapi.amap.com",
  "timeout": 5000
}
```

**获取高德地图API密钥**:
1. 访问 [高德开放平台](https://lbs.amap.com/)
2. 注册并登录
3. 进入"应用管理" → "我的应用" → "创建新应用"
4. 添加Key,选择"Web服务"类型
5. 复制生成的Key

#### OpenWeather API (`weather`)

```json
{
  "key": "YOUR_WEATHER_API_KEY",
  "base_url": "http://api.openweathermap.org",
  "timeout": 5000
}
```

**获取OpenWeather API密钥**:
1. 访问 [OpenWeather](https://openweathermap.org/api)
2. 注册免费账户
3. 在"API keys"页面获取密钥

## 环境变量方式 (可选,推荐生产环境)

### Windows (PowerShell)

```powershell
# 设置环境变量
$env:DB_PASSWORD="your_database_password"
$env:JWT_SECRET="your_jwt_secret_minimum_32_characters"
$env:AMAP_API_KEY="your_amap_api_key"
$env:WEATHER_API_KEY="your_openweather_api_key"

# 验证环境变量
echo $env:DB_PASSWORD
```

### Linux/Mac

```bash
# 设置环境变量
export DB_PASSWORD="your_database_password"
export JWT_SECRET="your_jwt_secret_minimum_32_characters"
export AMAP_API_KEY="your_amap_api_key"
export WEATHER_API_KEY="your_openweather_api_key"

# 验证环境变量
echo $DB_PASSWORD
```

### 永久设置 (Windows)

```powershell
# 使用setx命令设置系统环境变量 (需要管理员权限)
setx JWT_SECRET "your_jwt_secret_minimum_32_characters" /M
setx DB_PASSWORD "your_database_password" /M
setx AMAP_API_KEY "your_amap_api_key" /M
setx WEATHER_API_KEY "your_openweather_api_key" /M
```

**注意**: 使用 `setx` 后需要重新打开命令提示符才能生效。

## 安全最佳实践

### ✅ 推荐做法

1. **不要提交敏感配置到Git**
   - `server.json` 已在 `.gitignore` 中
   - 只提交 `server.json.template` 模板文件

2. **使用强密码和密钥**
   - JWT密钥至少32个字符
   - 数据库密码至少16个字符
   - 使用随机生成的密钥

3. **定期轮换密钥**
   - JWT密钥: 建议每3-6个月更换
   - 数据库密码: 建议每6个月更换
   - API密钥: 根据服务商建议

4. **生产环境使用密钥管理服务**
   - AWS Secrets Manager
   - Azure Key Vault
   - HashiCorp Vault
   - Google Cloud Secret Manager

### ❌ 禁止做法

1. **不要硬编码敏感信息到代码中**
2. **不要在命令行历史中暴露密码**
3. **不要通过邮件或即时通讯工具分享密钥**
4. **不要使用默认或弱密码**

## 故障排查

### 问题1: 无法连接数据库

**错误信息**: `ERROR 2003 (HY000): Can't connect to MySQL server`

**解决方案**:
1. 检查MySQL服务是否启动
   ```bash
   # Windows
   net start MySQL
   # 或
   net start MySQL80
   ```

2. 检查端口配置是否正确
   - 标准连接: `3306`
   - X Protocol: `33060`

3. 检查数据库用户名和密码是否正确

### 问题2: JWT Token验证失败

**错误信息**: `Token无效或已过期`

**解决方案**:
1. 确认 `jwt_secret` 已正确配置
2. 确认密钥至少32个字符
3. 前后端使用相同的JWT密钥

### 问题3: API请求失败

**错误信息**: `CURL request failed` 或 `HTTP error: 401`

**解决方案**:
1. 检查API密钥是否有效
2. 确认API配额是否用完
3. 检查网络连接

## 配置验证清单

启动服务前,请确认以下配置已完成:

- [ ] 已复制 `server.json.template` 为 `server.json`
- [ ] 已设置数据库用户名和密码
- [ ] 已生成并设置JWT密钥 (至少32字符)
- [ ] 已配置高德地图API密钥
- [ ] 已配置OpenWeather API密钥
- [ ] MySQL服务已启动
- [ ] 数据库已创建: `low_altitude_traffic_system`
- [ ] 数据库表结构已初始化
- [ ] `server.json` 未提交到Git

## 支持与反馈

如遇到配置问题,请:
1. 检查本文档的故障排查章节
2. 查看项目README文档
3. 在GitHub Issues中提问

---

**安全提醒**: 请妥善保管所有密钥和密码,不要分享给未授权人员!
