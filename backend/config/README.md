# 配置文件说明

## 快速开始

1. 复制模板文件：
   ```bash
   cp server.json.template server.json
   ```

2. 编辑 `server.json`，替换以下占位符为实际值：

### 必需配置项

| 配置项 | 说明 | 示例 |
|--------|------|------|
| `database.username` | MySQL数据库用户名 | `root` |
| `database.password` | MySQL数据库密码 | `your_secure_password` |
| `security.jwt_secret` | JWT签名密钥（至少32字符） | 使用随机生成工具生成 |
| `external_apis.amap.key` | 高德地图API密钥 | 从高德开放平台获取 |
| `weather.api_key` | 天气API密钥（目前使用高德） | 与amap.key相同 |

### JWT密钥生成方法

**Linux/Mac：**
```bash
openssl rand -base64 48
```

**Windows PowerShell：**
```powershell
$bytes = New-Object byte[] 48
[System.Security.Cryptography.RNGCryptoServiceProvider]::Create().GetBytes($bytes)
[Convert]::ToBase64String($bytes)
```

**在线工具：**
- https://generate-secret.vercel.app/64

### 高德地图API密钥获取

1. 访问：https://console.amap.com/
2. 注册/登录账号
3. 创建应用并获取Web服务API密钥
4. 将密钥填入 `external_apis.amap.key` 和 `weather.api_key`

## 配置文件结构

```json
{
  "server": {
    "host": "服务器监听地址",
    "port": "服务器端口",
    "threads": "工作线程数",
    "timeout": "请求超时时间（秒）"
  },
  "database": {
    "host": "数据库主机",
    "port": "数据库端口（MySQL X Protocol默认33060）",
    "database": "数据库名称",
    "username": "数据库用户名",
    "password": "数据库密码",
    "connection_pool_size": "连接池大小",
    "connection_timeout": "连接超时时间（秒）"
  },
  "security": {
    "jwt_secret": "JWT签名密钥",
    "jwt_issuer": "JWT签发者",
    "jwt_access_token_expiry": "访问令牌过期时间（秒）",
    "jwt_refresh_token_expiry": "刷新令牌过期时间（秒）",
    "jwt_algorithm": "签名算法",
    "password_min_length": "密码最小长度",
    "max_login_attempts": "最大登录失败次数",
    "lockout_duration": "账户锁定时长（秒）"
  },
  "flight": {
    "max_altitude": "最大飞行高度（米）",
    "min_altitude": "最小飞行高度（米）",
    "max_speed": "最大飞行速度（m/s）",
    "default_speed": "默认飞行速度（m/s）"
  },
  "logging": {
    "level": "日志级别（trace/debug/info/warn/error）",
    "file": "日志文件路径",
    "max_size": "单个日志文件最大大小",
    "max_files": "日志文件最大保留数量"
  },
  "external_apis": {
    "amap": {
      "key": "高德地图API密钥",
      "base_url": "高德地图API基础URL",
      "timeout": "API请求超时时间（毫秒）"
    }
  },
  "cors": {
    "enabled": "是否启用CORS",
    "origins": "允许的源地址（逗号分隔）",
    "methods": "允许的HTTP方法（逗号分隔）",
    "headers": "允许的请求头（逗号分隔）"
  }
}
```

## 安全注意事项

⚠️ **重要：切勿将 `server.json` 提交到版本控制系统！**

- `server.json` 已加入 `.gitignore`
- 仅提交模板文件 `server.json.template`
- 生产环境建议使用环境变量或密钥管理服务
- JWT密钥至少32字符，使用强随机性
- 定期轮换敏感凭证

## 环境特定配置

### 开发环境
```json
{
  "logging": {
    "level": "debug"
  },
  "cors": {
    "origins": "http://localhost:8080,http://localhost:8082"
  }
}
```

### 生产环境
```json
{
  "logging": {
    "level": "warn"
  },
  "cors": {
    "origins": "https://your-production-domain.com"
  }
}
```

## 故障排查

### 数据库连接失败
- 检查MySQL X Plugin是否启用（默认端口33060）
- 验证用户名和密码是否正确
- 确认数据库已创建：`CREATE DATABASE low_altitude_traffic_system;`

### API调用失败
- 验证高德地图API密钥是否有效
- 检查网络连接和防火墙设置
- 查看日志文件获取详细错误信息

### JWT认证问题
- 确保jwt_secret至少32字符
- 验证系统时钟同步（JWT依赖时间戳）
- 检查token过期时间设置是否合理
