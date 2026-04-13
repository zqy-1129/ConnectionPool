cmake_minimum_required(VERSION 3.16)
project(ConnectionPool LANGUAGES CXX)

# 1. 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 3. MySQL 配置（请根据你的实际安装路径修改）
set(MYSQL_ROOT "C:/Program Files/MySQL/MySQL Server 8.0")
set(MYSQL_INCLUDE_DIR "${MYSQL_ROOT}/include")
set(MYSQL_LIB_DIR "${MYSQL_ROOT}/lib")

include_directories(${MYSQL_INCLUDE_DIR})
link_directories(${MYSQL_LIB_DIR})

# 4. 收集所有源文件
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 5. 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 6. 链接 MySQL 库和 Windows 依赖库
target_link_libraries(${PROJECT_NAME}
    libmysql
    ws2_32
)

# 7. 预编译头（如果你用了 pch）
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/pch.h)