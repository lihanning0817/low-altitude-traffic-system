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
set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.lib")

# 设置ZLIB路径
set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows")

# 查找其他依赖包
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(mysql-concpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CURL REQUIRED)

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
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
    mysql::concpp
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

# 创建UserRepository测试程序
add_executable(test_user_repository src/test_user_repository.cpp)

# 链接UserRepository测试程序
target_link_libraries(test_user_repository
    PRIVATE
    ${PROJECT_NAME}_lib
)

# 创建JWT服务测试程序
add_executable(test_jwt_service src/test_jwt_service.cpp)

# 链接JWT服务测试程序
target_link_libraries(test_jwt_service
    PRIVATE
    ${PROJECT_NAME}_lib
)

# 创建AuthController测试程序
add_executable(test_auth_controller src/test_auth_controller.cpp)

# 链接AuthController测试程序
target_link_libraries(test_auth_controller
    PRIVATE
    ${PROJECT_NAME}_lib
)

# 创建独立JWT测试程序（无数据库依赖）
add_executable(test_jwt_standalone src/test_jwt_standalone.cpp)

# 链接独立JWT测试程序（只需要基本依赖，无数据库）
target_link_libraries(test_jwt_standalone
    PRIVATE
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog
)

# 创建数据库连接测试程序
add_executable(test_database_connection src/test_database_connection.cpp)

# 链接数据库连接测试程序
target_link_libraries(test_database_connection
    PRIVATE
    ${PROJECT_NAME}_lib
)

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