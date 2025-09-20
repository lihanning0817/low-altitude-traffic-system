#!/bin/bash

echo "🚀 开始初始化低空交通系统开发环境..."

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查函数
check_command() {
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ $1 成功${NC}"
    else
        echo -e "${RED}❌ $1 失败${NC}"
        exit 1
    fi
}

# 安装前端依赖
echo -e "${YELLOW}📦 安装前端依赖...${NC}"
if [ -f "package.json" ]; then
    npm install
    check_command "前端依赖安装"
else
    echo -e "${YELLOW}⚠️  未找到 package.json，跳过前端依赖安装${NC}"
fi

# 检查MySQL连接
echo -e "${YELLOW}🗄️  检查数据库连接...${NC}"
max_attempts=30
attempt=1

while [ $attempt -le $max_attempts ]; do
    if mysql -h mysql -u appuser -papppassword -e "SELECT 1;" > /dev/null 2>&1; then
        echo -e "${GREEN}✅ 数据库连接成功${NC}"
        break
    else
        echo -e "${YELLOW}⏳ 等待数据库启动... (尝试 $attempt/$max_attempts)${NC}"
        sleep 2
        ((attempt++))
    fi
done

if [ $attempt -gt $max_attempts ]; then
    echo -e "${RED}❌ 数据库连接失败${NC}"
else
    # 检查数据库表是否存在
    echo -e "${YELLOW}📋 检查数据库表...${NC}"
    mysql -h mysql -u appuser -papppassword low_altitude_traffic -e "SHOW TABLES;" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ 数据库表已存在${NC}"
    else
        echo -e "${YELLOW}⚠️  数据库表可能需要初始化${NC}"
    fi
fi

# 创建后端构建目录
echo -e "${YELLOW}🔨 准备后端构建环境...${NC}"
if [ -d "backend" ]; then
    mkdir -p backend/build
    echo -e "${GREEN}✅ 后端构建目录已创建${NC}"
fi

# 设置git安全目录
echo -e "${YELLOW}🔒 配置Git安全设置...${NC}"
git config --global --add safe.directory /workspace
check_command "Git安全设置"

# 显示环境信息
echo -e "${YELLOW}📋 开发环境信息:${NC}"
echo "Node.js版本: $(node --version)"
echo "npm版本: $(npm --version)"
echo "CMake版本: $(cmake --version | head -n1)"
echo "gcc版本: $(gcc --version | head -n1)"
echo "vcpkg根目录: $VCPKG_ROOT"

# 创建常用脚本
echo -e "${YELLOW}📝 创建开发脚本...${NC}"

# 前端开发脚本
cat > run-frontend.sh << 'EOF'
#!/bin/bash
echo "🎨 启动前端开发服务器..."
npm run serve
EOF

# 后端构建脚本
cat > build-backend.sh << 'EOF'
#!/bin/bash
echo "🔨 构建后端..."
cd backend/build
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..
make -j$(nproc)
EOF

# 设置脚本可执行权限
chmod +x run-frontend.sh build-backend.sh

echo -e "${GREEN}🎉 开发环境初始化完成！${NC}"
echo ""
echo -e "${YELLOW}💡 使用提示:${NC}"
echo "  • 启动前端: ./run-frontend.sh 或 npm run serve"
echo "  • 构建后端: ./build-backend.sh"
echo "  • 数据库地址: mysql:3306"
echo "  • 前端访问: http://localhost:5173"
echo "  • 后端API: http://localhost:8080"
echo ""
echo -e "${GREEN}开始愉快的开发吧！ 🚀${NC}"