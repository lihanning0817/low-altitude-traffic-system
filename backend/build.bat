@echo off
echo ====================================
echo 城市智能低空交通系统 - 后端构建脚本
echo ====================================

REM 检查vcpkg环境变量
if "%VCPKG_ROOT%"=="" (
    echo 错误: 请设置VCPKG_ROOT环境变量
    echo 例如: set VCPKG_ROOT=C:\vcpkg
    pause
    exit /b 1
)

echo 检查vcpkg安装...
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo 错误: 在 %VCPKG_ROOT% 中找不到vcpkg.exe
    echo 请确保vcpkg正确安装
    pause
    exit /b 1
)

echo.
echo 正在安装依赖库...
echo.

REM 安装依赖库
%VCPKG_ROOT%\vcpkg.exe install boost-beast:x64-windows
%VCPKG_ROOT%\vcpkg.exe install boost-json:x64-windows
%VCPKG_ROOT%\vcpkg.exe install boost-thread:x64-windows
%VCPKG_ROOT%\vcpkg.exe install boost-system:x64-windows
%VCPKG_ROOT%\vcpkg.exe install nlohmann-json:x64-windows
%VCPKG_ROOT%\vcpkg.exe install mysql-connector-cpp:x64-windows
%VCPKG_ROOT%\vcpkg.exe install openssl:x64-windows
%VCPKG_ROOT%\vcpkg.exe install spdlog:x64-windows
%VCPKG_ROOT%\vcpkg.exe install curl:x64-windows

echo.
echo 依赖库安装完成！
echo.

REM 创建构建目录
if not exist "build" mkdir build
cd build

echo 正在配置CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -A x64

if %ERRORLEVEL% neq 0 (
    echo CMake配置失败！
    pause
    exit /b 1
)

echo.
echo 正在编译项目...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo.
echo ====================================
echo 构建成功！
echo.
echo 可执行文件位置: build\Release\low_altitude_traffic_system_backend.exe
echo 配置文件位置: config\server.json
echo.
echo 运行命令:
echo cd build\Release
echo low_altitude_traffic_system_backend.exe ..\..\config\server.json
echo ====================================

pause