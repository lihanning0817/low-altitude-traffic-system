# 智能低空交通管理系统构建和部署指南

## 1. 配置高德地图API密钥

1. 编辑 `public/index.html` 文件
2. 找到以下行：
   ```html
   <script type="text/javascript" src="https://webapi.amap.com/maps?v=1.4.15&key=YOUR_AMAP_API_KEY"></script>
   ```
3. 将 `YOUR_AMAP_API_KEY` 替换为您提供的API密钥：
   ```html
   <script type="text/javascript" src="https://webapi.amap.com/maps?v=1.4.15&key=1872806f332dab32a1a3dc895b0ad542"></script>
   ```

## 2. 配置数据库

### 2.1 启动MySQL服务
打开命令提示符，执行以下命令：
```bash
net start mysql
```

### 2.2 创建数据库和用户
打开MySQL命令行：
```bash
mysql -u root -p
```

输入您提供的密码：Aabc123456+

执行以下SQL命令：
```sql
CREATE DATABASE low_altitude_traffic;
CREATE USER 'lat_user'@'localhost' IDENTIFIED BY 'lat_password';
GRANT ALL PRIVILEGES ON low_altitude_traffic.* TO 'lat_user'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

### 2.3 创建表结构
在项目根目录下执行：
```bash
mysql -u lat_user -p low_altitude_traffic < backend/database/schema.sql
```

### 2.4 配置数据库连接
编辑 `backend/config/database.cpp` 文件，修改以下内容：
```cpp
const std::string DB_HOST = "localhost";
const std::string DB_USER = "lat_user";
const std::string DB_PASSWORD = "lat_password";
const std::string DB_NAME = "low_altitude_traffic";
const int DB_PORT = 3306;
```

## 3. 构建后端服务

### 3.1 创建构建目录
```bash
cd backend
mkdir build
cd build
```

### 3.2 配置CMake
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
```

### 3.3 构建项目
```bash
cmake --build . --config Release
```

### 3.4 运行后端服务
```bash
./Release/low_altitude_traffic_system.exe
```

## 4. 构建前端应用

### 4.1 安装前端依赖
在项目根目录下执行：
```bash
npm install
```

### 4.2 配置API地址
编辑 `src/services/api.js` 文件，确保API基础URL正确：
```javascript
const API_BASE_URL = 'http://localhost:8081/api/v1';
```

### 4.3 开发环境运行
```bash
npm run serve
```

### 4.4 生产环境构建
```bash
npm run build
```

构建后的文件将位于 `dist` 目录中。

## 5. 验证系统运行

### 5.1 验证后端服务
访问 `http://localhost:8081/api/v1/health`，应该返回健康检查信息。

### 5.2 验证前端页面
访问 `http://localhost:8080`（开发环境）或部署后的URL，应该能看到登录页面。

### 5.3 验证数据库连接
检查后端服务日志，确认数据库连接成功。

## 6. 系统使用

1. 打开浏览器访问前端页面
2. 注册新用户或使用默认账户登录
3. 开始使用系统的各项功能：
   - 飞行任务管理
   - 地图监控
   - 路径规划
   - 空域管理
   - 气象监测
   - 应急响应
   - 数据分析

## 7. 故障排除

### 7.1 数据库连接失败
1. 检查MySQL服务是否启动
2. 验证数据库用户名和密码
3. 确认防火墙设置允许MySQL端口通信

### 7.2 前端无法连接后端API
1. 检查后端服务是否运行
2. 验证API地址配置是否正确
3. 检查防火墙设置

### 7.3 高德地图无法显示
1. 检查API密钥是否正确配置
2. 确认网络连接正常
3. 验证域名白名单设置

### 7.4 构建失败
1. 检查依赖库是否正确安装
2. 确认CMake版本符合要求
3. 验证编译器支持C++17标准

按照以上步骤，您应该能够成功构建和部署智能低空交通管理系统。
