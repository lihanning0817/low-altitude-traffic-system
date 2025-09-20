# 智能低空交通管理系统部署指南

## 1. 系统要求

### 1.1 硬件要求
- CPU: 2核或以上
- 内存: 4GB或以上
- 硬盘: 20GB可用空间
- 网络: 稳定的互联网连接

### 1.2 软件要求
- 操作系统: Windows 10/11, Linux (Ubuntu 18.04+), macOS 10.15+
- Node.js: 14.x 或更高版本
- npm: 6.x 或更高版本
- MySQL: 8.0 或更高版本
- CMake: 3.10 或更高版本
- C++编译器: 支持C++17标准

## 2. 环境准备

### 2.1 安装Node.js和npm
访问 [Node.js官网](https://nodejs.org/) 下载并安装Node.js，npm会随Node.js一起安装。

验证安装:
```bash
node --version
npm --version
```

### 2.2 安装MySQL
#### Windows:
1. 访问 [MySQL官网](https://dev.mysql.com/downloads/mysql/) 下载MySQL Installer
2. 运行安装程序，选择"Server only"安装
3. 配置root密码

#### Linux (Ubuntu):
```bash
sudo apt update
sudo apt install mysql-server
sudo mysql_secure_installation
```

#### macOS:
```bash
brew install mysql
brew services start mysql
```

### 2.3 安装CMake
#### Windows:
访问 [CMake官网](https://cmake.org/download/) 下载Windows installer并安装

#### Linux:
```bash
sudo apt install cmake
```

#### macOS:
```bash
brew install cmake
```

### 2.4 安装C++编译器
#### Windows:
安装 Visual Studio 2019 或更高版本，包含C++开发工具

#### Linux:
```bash
sudo apt install build-essential
```

#### macOS:
安装 Xcode 命令行工具:
```bash
xcode-select --install
```

### 2.5 安装依赖库
#### Windows:
使用 vcpkg 安装依赖:
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install boost-mysql websocketpp openssl unofficial-libmysql
```

#### Linux:
```bash
sudo apt install libmysqlclient-dev libboost-all-dev libwebsocketpp-dev libssl-dev
```

#### macOS:
```bash
brew install mysql boost websocketpp openssl
```

## 3. 数据库配置

### 3.1 启动MySQL服务
#### Windows:
```bash
net start mysql
```

#### Linux:
```bash
sudo systemctl start mysql
```

#### macOS:
```bash
brew services start mysql
```

### 3.2 创建数据库
```bash
mysql -u root -p
```

在MySQL命令行中执行:
```sql
CREATE DATABASE low_altitude_traffic;
CREATE USER 'lat_user'@'localhost' IDENTIFIED BY 'lat_password';
GRANT ALL PRIVILEGES ON low_altitude_traffic.* TO 'lat_user'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

### 3.3 创建表结构
```bash
mysql -u lat_user -p low_altitude_traffic < backend/database/schema.sql
```

## 4. 后端部署

### 4.1 配置数据库连接
编辑 `backend/config/database.cpp` 文件，修改数据库连接信息:
```cpp
const std::string DB_HOST = "localhost";
const std::string DB_USER = "lat_user";
const std::string DB_PASSWORD = "lat_password";
const std::string DB_NAME = "low_altitude_traffic";
const int DB_PORT = 3306;
```

### 4.2 构建项目
```bash
cd backend
mkdir build
cd build
cmake ..
make
```

### 4.3 运行后端服务
```bash
./low_altitude_traffic_system
```

默认情况下，后端服务将在端口8081上运行。

## 5. 前端部署

### 5.1 安装前端依赖
```bash
npm install
```

### 5.2 配置API地址
编辑 `src/services/api.js` 文件，确保API基础URL正确:
```javascript
const API_BASE_URL = 'http://localhost:8081/api/v1';
```

### 5.3 配置高德地图API密钥
编辑 `public/index.html` 文件，替换高德地图API密钥:
```html
<script type="text/javascript" src="https://webapi.amap.com/maps?v=1.4.15&key=YOUR_AMAP_API_KEY"></script>
```

您需要在[高德地图开放平台](https://console.amap.com/)申请API密钥。

### 5.4 开发环境运行
```bash
npm run serve
```

### 5.5 生产环境构建
```bash
npm run build
```

构建后的文件将位于 `dist` 目录中，可以部署到任何Web服务器上。

## 6. Docker部署（推荐）

### 6.1 安装Docker
访问 [Docker官网](https://www.docker.com/products/docker-desktop) 下载并安装Docker Desktop。

### 6.2 构建并运行
```bash
docker-compose up -d
```

这将自动构建并启动以下服务:
- MySQL数据库
- C++后端服务
- Nginx Web服务器（提供前端页面）

## 7. 系统验证

### 7.1 验证后端服务
访问 `http://localhost:8081/api/v1/health`，应该返回健康检查信息。

### 7.2 验证前端页面
访问 `http://localhost`，应该能看到登录页面。

### 7.3 验证数据库连接
检查后端服务日志，确认数据库连接成功。

## 8. 常见问题解决

### 8.1 数据库连接失败
1. 检查MySQL服务是否启动
2. 验证数据库用户名和密码
3. 确认防火墙设置允许MySQL端口通信

### 8.2 前端无法连接后端API
1. 检查后端服务是否运行
2. 验证API地址配置是否正确
3. 检查防火墙设置

### 8.3 高德地图无法显示
1. 检查API密钥是否正确配置
2. 确认网络连接正常
3. 验证域名白名单设置

### 8.4 构建失败
1. 检查依赖库是否正确安装
2. 确认CMake版本符合要求
3. 验证编译器支持C++17标准

## 9. 系统维护

### 9.1 日志查看
- 后端日志: 查看终端输出或配置的日志文件
- 前端日志: 浏览器开发者工具控制台

### 9.2 数据备份
定期备份MySQL数据库:
```bash
mysqldump -u lat_user -p low_altitude_traffic > backup.sql
```

### 9.3 系统升级
1. 备份当前系统和数据
2. 拉取最新代码
3. 重新构建和部署
4. 执行数据库迁移脚本（如果有）

## 10. 性能优化建议

### 10.1 数据库优化
- 定期执行数据库优化和碎片整理
- 为常用查询字段添加索引
- 监控慢查询日志

### 10.2 后端优化
- 调整线程池大小
- 优化数据库查询
- 使用连接池管理数据库连接

### 10.3 前端优化
- 启用gzip压缩
- 使用CDN加速静态资源
- 实施缓存策略

通过以上步骤，您应该能够成功部署和运行智能低空交通管理系统。如果在部署过程中遇到任何问题，请参考常见问题解决部分或联系技术支持。
