#!/bin/bash

echo "===================================="
echo "城市智能低空交通系统 - 后端服务启动"
echo "===================================="

# 检查可执行文件是否存在
if [ ! -f "build/Release/low_altitude_traffic_system_backend" ]; then
    echo "错误: 找不到可执行文件"
    echo "请先运行 ./build.sh 编译项目"
    exit 1
fi

# 检查配置文件是否存在
if [ ! -f "config/server.json" ]; then
    echo "错误: 找不到配置文件 config/server.json"
    exit 1
fi

# 创建日志目录
mkdir -p logs

echo "正在启动后端服务..."
echo "配置文件: config/server.json"
echo "日志目录: logs/"
echo
echo "服务将运行在 http://localhost:8081"
echo "按 Ctrl+C 停止服务"
echo

# 启动服务
cd build/Release
./low_altitude_traffic_system_backend ../../config/server.json

echo
echo "服务已停止"