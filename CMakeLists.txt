cmake_minimum_required(VERSION 3.5)

project(main)

# 将可执行文件输出到 bin 目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 自动收集 src 目录下的所有源文件
aux_source_directory(src SRC_LIST)

# 添加头文件搜索路径
include_directories(include)

# 生成可执行文件
add_executable(main ${SRC_LIST})