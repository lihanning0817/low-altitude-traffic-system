#!/bin/bash

echo "===================================="
echo "城市智能低空交通系统 - 后端构建脚本"
echo "===================================="

# 检查vcpkg环境变量
if [ -z "$VCPKG_ROOT" ]; then
    echo "错误: 请设置VCPKG_ROOT环境变量"
    echo "例如: export VCPKG_ROOT=/path/to/vcpkg"
    exit 1
fi

echo "检查vcpkg安装..."
if [ ! -f "$VCPKG_ROOT/vcpkg" ]; then
    echo "错误: 在 $VCPKG_ROOT 中找不到vcpkg"
    echo "请确保vcpkg正确安装"
    exit 1
fi

echo
echo "正在安装依赖库..."
echo

# 安装依赖库
$VCPKG_ROOT/vcpkg install boost-beast
$VCPKG_ROOT/vcpkg install boost-json
$VCPKG_ROOT/vcpkg install boost-thread
$VCPKG_ROOT/vcpkg install boost-system
$VCPKG_ROOT/vcpkg install nlohmann-json
$VCPKG_ROOT/vcpkg install mysql-connector-cpp
$VCPKG_ROOT/vcpkg install openssl
$VCPKG_ROOT/vcpkg install spdlog
$VCPKG_ROOT/vcpkg install curl

echo
echo "依赖库安装完成！"
echo

# 创建构建目录
mkdir -p build
cd build

echo "正在配置CMake..."
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

if [ $? -ne 0 ]; then
    echo "CMake配置失败！"
    exit 1
fi

echo
echo "正在编译项目..."
cmake --build . --config Release

if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi

echo
echo "===================================="
echo "构建成功！"
echo
echo "可执行文件位置: build/Release/low_altitude_traffic_system_backend"
echo "配置文件位置: config/server.json"
echo
echo "运行命令:"
echo "cd build/Release"
echo "./low_altitude_traffic_system_backend ../../config/server.json"
echo "===================================="