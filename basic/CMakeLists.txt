# 找到当前目录下所有 .cpp 文件
file(GLOB BASIC_SOURCES "*.cpp")

# 创建静态库 basic
add_library(basic STATIC ${BASIC_SOURCES})

# 添加头文件搜索路径
target_include_directories(basic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)
