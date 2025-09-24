cmake_minimum_required(VERSION 3.16)
project(low_altitude_traffic_system_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置CMake策略
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# 设置vcpkg工具链
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# 查找必要的包
find_package(Threads REQUIRED)

# 使用CONFIG模式查找Boost包
find_package(Boost CONFIG REQUIRED COMPONENTS system thread json)

# 设置OpenSSL路径（vcpkg安装）
set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.lib")

# 设置ZLIB路径
set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows")

# 设置MySQL连接器路径（使用系统安装的MySQL连接器）
set(MYSQL_CONNECTOR_ROOT "C:/Program Files/MySQL/mysql-connector-c++-9.4.0-winx64")
set(MYSQL_CONNECTOR_INCLUDE_DIR "${MYSQL_CONNECTOR_ROOT}/include")
set(MYSQL_CONNECTOR_LIB_DIR "${MYSQL_CONNECTOR_ROOT}/lib64/vs14")

# 查找其他依赖包
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(zstd CONFIG REQUIRED)
# 不使用find_package，手动设置MySQL连接器
find_package(spdlog CONFIG REQUIRED)
find_package(CURL REQUIRED)

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${MYSQL_CONNECTOR_INCLUDE_DIR}
)

# 收集所有源文件
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# 移除main.cpp和测试文件，单独处理
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/test_user_repository.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/test_jwt_service.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/test_auth_controller.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/test_jwt_standalone.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/test_database_connection.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/test_mysql_basic.cpp")

# 创建静态库（包含除main.cpp外的所有源码）
add_library(${PROJECT_NAME}_lib STATIC ${SOURCES})

# 库的链接依赖
target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
    Boost::system
    Boost::thread
    Boost::json
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    "${MYSQL_CONNECTOR_LIB_DIR}/mysqlcppconnx.lib"
    spdlog::spdlog
    CURL::libcurl
    Threads::Threads
)

# 添加Boost Beast头文件支持（Beast是header-only库）
target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
    ${Boost_INCLUDE_DIRS}
)

# 设置库的编译选项
target_compile_definitions(${PROJECT_NAME}_lib PUBLIC
    BOOST_BEAST_USE_STD_STRING_VIEW
    BOOST_ASIO_HAS_STD_STRING_VIEW
)

# 创建主程序可执行文件
add_executable(${PROJECT_NAME} src/main.cpp)

# 链接主程序
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_NAME}_lib
)

# 测试程序已移除，仅保留主程序构建

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME}_lib PRIVATE /W4 /utf-8)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
    target_compile_definitions(${PROJECT_NAME}_lib PRIVATE
        _WIN32_WINNT=0x0601
        _SILENCE_CXX20_CISO646_REMOVED_WARNING
        NOMINMAX
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WIN32_WINNT=0x0601
        _SILENCE_CXX20_CISO646_REMOVED_WARNING
        NOMINMAX
    )
else()
    target_compile_options(${PROJECT_NAME}_lib PRIVATE -Wall -Wextra)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

# 可选：单元测试
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
    enable_testing()

    file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

    target_link_libraries(${PROJECT_NAME}_tests
        PRIVATE
        ${PROJECT_NAME}_lib
        GTest::gtest
        GTest::gtest_main
    )

    add_test(NAME AllTests COMMAND ${PROJECT_NAME}_tests)
endif()