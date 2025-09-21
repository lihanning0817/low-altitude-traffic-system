@echo off
echo ====================================
echo UserRepository功能测试 - 编译验证
echo ====================================

REM 检查vcpkg环境变量
if "%VCPKG_ROOT%"=="" (
    echo 错误: 请设置VCPKG_ROOT环境变量
    pause
    exit /b 1
)

echo 进入构建目录...
if not exist "build" mkdir build
cd build

echo 配置CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -A x64

if %ERRORLEVEL% neq 0 (
    echo CMake配置失败！
    pause
    exit /b 1
)

echo 编译项目...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo.
echo ====================================
echo 编译成功！可用的程序：
echo 1. 主服务程序: Release\low_altitude_traffic_system_backend.exe
echo 2. UserRepository测试: Release\test_user_repository.exe
echo ====================================
echo.

echo 提示：运行前请确保：
echo 1. MySQL服务已启动
echo 2. 已创建数据库：low_altitude_traffic_system
echo 3. 已导入数据库结构：database\schema.sql
echo 4. 已配置数据库连接：config\server.json
echo.

pause