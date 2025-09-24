# 低空交通管理系统

一个基于Vue.js前端和C++后端的低空交通管理系统，用于无人机飞行任务的规划、监控和管理。

## 项目概述

本系统提供了完整的低空交通管理解决方案，包括：
- 用户身份认证与权限管理
- 飞行任务的创建、审批和监控
- 实时飞行数据记录与分析
- 系统监控和管理工具

## 技术栈

### 前端
- **Vue.js 3** - 主要前端框架
- **Vue Router** - 路由管理
- **Vuex** - 状态管理
- **Axios** - HTTP客户端

### 后端
- **C++** - 核心后端语言
- **Crow** - HTTP服务器框架
- **MySQL** - 主数据库
- **JWT** - 身份认证

### 构建工具
- **CMake** - C++项目构建
- **vcpkg** - C++包管理器

## 协作开发指南

### 1. 克隆仓库并拉取代码

```bash
# 克隆项目到本地
git clone https://github.com/Zguoxu/low-altitude-traffic-system.git

# 或者使用Gitee镜像（国内用户推荐）
git clone https://gitee.com/ACE_001/low_altitude_traffic_system.git

# 进入项目目录
cd low-altitude-traffic-system

# 拉取最新代码
git pull origin master
```

### 2. 环境准备

#### 前端环境
```bash
# 安装Node.js依赖
npm install

# 启动开发服务器
npm run serve
```

#### 后端环境
```bash
# 进入后端目录
cd backend

# 使用CMake构建项目
cmake -B build -S .
cmake --build build --config Release

# 运行后端服务
./Release/low_altitude_traffic_system_backend.exe config/server.json
```

### 3. 数据库初始化

**重要：不要上传本地数据库文件，只使用SQL脚本来共享数据库结构！**

#### 初始化本地数据库

1. 确保已安装MySQL服务器
2. 创建数据库：
```sql
CREATE DATABASE low_altitude_traffic_system
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
```

3. 使用初始化脚本：
```bash
# 方法1: 使用MySQL命令行
mysql -u root -p low_altitude_traffic_system < database/init.sql

# 方法2: 使用MySQL客户端工具
# 将database/init.sql文件内容复制到MySQL客户端中执行
```

4. 验证表结构：
```sql
USE low_altitude_traffic_system;
SHOW TABLES;
DESCRIBE users;
DESCRIBE flight_tasks;
```

### 4. Git协作流程

#### 开始工作前
```bash
# 1. 切换到主分支
git checkout master

# 2. 拉取最新代码
git pull origin master

# 3. 创建新的功能分支
git checkout -b feature/your-feature-name
```

#### 开发过程中
```bash
# 1. 查看修改状态
git status

# 2. 添加修改的文件
git add .
# 或者添加特定文件
git add src/components/YourComponent.vue

# 3. 提交更改
git commit -m "feat: 添加新功能描述"
```

#### 完成开发后
```bash
# 1. 推送分支到远程
git push origin feature/your-feature-name

# 2. 在GitHub/Gitee上创建Pull Request/Merge Request
# 3. 代码审核通过后合并到主分支

# 4. 合并后清理本地分支
git checkout master
git pull origin master
git branch -d feature/your-feature-name
```

### 5. 重要注意事项

#### ⚠️ 禁止上传的文件
- 数据库文件（*.db, *.sqlite, *.mdb）
- 配置文件（config.json, .env）
- 构建产物（node_modules/, dist/, build/）
- 个人IDE设置（.idea/, *.suo）

#### ✅ 应该上传的文件
- 源代码（*.js, *.vue, *.cpp, *.h）
- 数据库结构脚本（*.sql）
- 项目配置模板（config.template.json）
- 文档文件（*.md）

#### 🔧 配置文件管理
1. 复制配置模板：
```bash
cp config/server.template.json config/server.json
```

2. 根据本地环境修改配置
3. 配置文件会被.gitignore忽略，不会意外提交

### 6. 常用Git命令速查

```bash
# 查看状态
git status

# 查看提交历史
git log --oneline

# 查看远程仓库
git remote -v

# 同步远程分支
git fetch origin

# 合并远程更改
git merge origin/master

# 强制拉取（谨慎使用）
git reset --hard origin/master

# 查看差异
git diff

# 撤销未提交的更改
git checkout -- filename

# 撤销最后一次提交（保留更改）
git reset --soft HEAD~1
```

## 项目结构

```
low-altitude-traffic-system/
├── src/                    # 前端源码
│   ├── components/        # Vue组件
│   ├── views/            # 页面组件
│   ├── services/         # API服务
│   └── router/           # 路由配置
├── backend/               # 后端源码
│   ├── src/              # C++源码
│   ├── config/           # 配置文件
│   └── CMakeLists.txt    # 构建配置
├── database/             # 数据库脚本
│   ├── init.sql         # 初始化脚本
│   └── schema.sql       # 表结构定义
├── .gitignore           # Git忽略规则
└── README.md            # 项目说明
```

## 开发规范

### 提交消息格式
使用约定式提交格式：
- `feat: 新功能`
- `fix: 修复bug`
- `docs: 文档更新`
- `style: 代码格式调整`
- `refactor: 代码重构`
- `test: 测试相关`
- `chore: 构建/工具相关`

### 分支命名规范
- `feature/功能名称` - 新功能开发
- `bugfix/问题描述` - 问题修复
- `hotfix/紧急修复` - 紧急修复
- `docs/文档更新` - 文档相关

## 联系方式

如有问题或建议，请通过以下方式联系：
- 创建GitHub Issue
- 发起Pull Request
- 项目内部讨论

---

**记住：永远不要直接在master分支上开发，始终使用功能分支！**