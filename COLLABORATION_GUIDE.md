# 开发协作流程 - 命令行清单

## 🚀 新成员快速上手指南

### 第一步：获取代码
```bash
# 克隆项目（选择其一）
git clone https://github.com/Zguoxu/low-altitude-traffic-system.git
# 或
git clone https://gitee.com/ACE_001/low_altitude_traffic_system.git

# 进入项目目录
cd low-altitude-traffic-system
```

### 第二步：环境准备
```bash
# 安装前端依赖
npm install

# 检查Node.js和npm版本
node --version
npm --version
```

### 第三步：数据库初始化
```bash
# 1. 确保MySQL服务启动
net start mysql
# 或
systemctl start mysql

# 2. 登录MySQL创建数据库
mysql -u root -p
```

```sql
-- 在MySQL命令行中执行
CREATE DATABASE low_altitude_traffic_system
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 退出MySQL
EXIT;
```

```bash
# 3. 导入初始化脚本
mysql -u root -p low_altitude_traffic_system < database/init.sql

# 4. 验证数据库初始化
mysql -u root -p -e "USE low_altitude_traffic_system; SHOW TABLES;"
```

### 第四步：启动服务
```bash
# 终端1：启动前端开发服务器
npm run serve

# 终端2：构建并启动后端服务
cd backend
cmake -B build -S .
cmake --build build --config Release
./Release/low_altitude_traffic_system_backend.exe config/server.json
```

## 📋 日常开发工作流

### 开始新功能开发
```bash
# 1. 确保在主分支且是最新代码
git checkout master
git pull origin master

# 2. 创建并切换到功能分支
git checkout -b feature/你的功能名称

# 3. 开始编码...
```

### 开发过程中的常用命令
```bash
# 查看当前状态
git status

# 查看修改内容
git diff

# 添加文件到暂存区
git add .
# 或添加指定文件
git add src/components/NewComponent.vue

# 提交更改
git commit -m "feat: 添加新组件功能"

# 推送到远程分支
git push origin feature/你的功能名称
```

### 完成开发后的合并流程
```bash
# 1. 确保功能分支代码最新
git add .
git commit -m "feat: 完成功能开发"
git push origin feature/你的功能名称

# 2. 切换到主分支并拉取最新代码
git checkout master
git pull origin master

# 3. 合并功能分支（如果没有冲突）
git merge feature/你的功能名称
git push origin master

# 4. 删除功能分支
git branch -d feature/你的功能名称
git push origin --delete feature/你的功能名称
```

## 🔄 团队协作同步

### 每天开始工作前
```bash
# 拉取团队最新代码
git checkout master
git pull origin master

# 如果在功能分支上工作，同步最新代码
git checkout feature/你的分支名
git merge master
```

### 解决合并冲突
```bash
# 当出现冲突时
git status  # 查看冲突文件

# 手动编辑冲突文件，解决冲突后
git add 冲突文件名
git commit -m "fix: 解决合并冲突"
```

## 🗃️ 数据库管理

### 数据库结构更新流程
```bash
# 1. 修改database/init.sql文件
# 2. 在本地测试新的数据库结构
mysql -u root -p

# 在MySQL中执行
DROP DATABASE IF EXISTS test_low_altitude_traffic_system;
CREATE DATABASE test_low_altitude_traffic_system
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE test_low_altitude_traffic_system;
SOURCE database/init.sql;

# 3. 验证表结构
SHOW TABLES;
DESCRIBE users;

# 4. 提交数据库脚本更改
git add database/init.sql
git commit -m "feat: 更新数据库表结构"
git push origin 你的分支名
```

### 重置本地数据库
```bash
# ⚠️ 注意：这会删除所有本地数据！
mysql -u root -p -e "DROP DATABASE IF EXISTS low_altitude_traffic_system;"
mysql -u root -p -e "CREATE DATABASE low_altitude_traffic_system CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;"
mysql -u root -p low_altitude_traffic_system < database/init.sql
```

## 🛠️ 常见问题解决

### Git相关问题
```bash
# 撤销未提交的更改
git checkout -- 文件名
git reset --hard HEAD  # 撤销所有更改（危险操作）

# 修改最后一次提交信息
git commit --amend -m "新的提交信息"

# 查看提交历史
git log --oneline
git log --graph --oneline --all

# 强制同步远程分支（会丢失本地更改）
git fetch origin
git reset --hard origin/master
```

### 数据库连接问题
```bash
# 检查MySQL服务状态
net start | findstr mysql  # Windows
systemctl status mysql     # Linux

# 测试数据库连接
mysql -u root -p -e "SELECT 1;"

# 查看MySQL错误日志
# Windows: C:\ProgramData\MySQL\MySQL Server 8.0\Data\你的主机名.err
# Linux: /var/log/mysql/error.log
```

### 构建问题
```bash
# 清理构建缓存
cd backend
rm -rf build/  # Linux/Mac
rmdir /s build  # Windows

# 重新构建
cmake -B build -S .
cmake --build build --config Release

# 检查依赖
vcpkg list  # 查看已安装的包
```

## 📝 提交规范

### 提交信息格式
```
类型(范围): 简短描述

详细描述（可选）

相关Issue: #123
```

### 提交类型说明
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更新
- `style`: 代码格式调整（不影响功能）
- `refactor`: 代码重构
- `perf`: 性能优化
- `test`: 测试相关
- `build`: 构建系统或依赖更新
- `ci`: CI配置更新
- `chore`: 其他不修改源码的更新

### 提交示例
```bash
git commit -m "feat(auth): 添加JWT令牌刷新机制"
git commit -m "fix(database): 修复用户表外键约束问题"
git commit -m "docs: 更新API文档和使用说明"
```

## 🚨 重要提醒

### ❌ 绝对不要做的事
1. **直接在master分支开发**
2. **提交数据库文件（*.db, *.sqlite）**
3. **提交配置文件（config.json, .env）**
4. **强制推送到共享分支（git push -f origin master）**
5. **提交node_modules目录**

### ✅ 必须遵守的规则
1. **始终使用功能分支开发**
2. **只提交SQL脚本文件共享数据库结构**
3. **提交前先拉取最新代码**
4. **使用规范的提交信息格式**
5. **代码审查后再合并到master**

### 🆘 紧急情况处理
```bash
# 误提交了敏感文件，需要从历史中完全删除
git filter-branch --force --index-filter \
'git rm --cached --ignore-unmatch 敏感文件路径' \
--prune-empty --tag-name-filter cat -- --all

# 推送更改（需要团队成员重新克隆）
git push origin --force --all
```

---

## 🎯 快速参考

### 一天的开发流程
```bash
# 1. 同步代码
git checkout master && git pull origin master

# 2. 创建功能分支
git checkout -b feature/today-work

# 3. 开发... 编码... 测试...

# 4. 提交更改
git add . && git commit -m "feat: 完成今日开发任务"

# 5. 推送分支
git push origin feature/today-work

# 6. 创建Pull Request并等待审查
```

记住：**协作的关键是沟通和规范！** 🤝