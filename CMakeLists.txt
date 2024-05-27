project(threadpool)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

# 添加可执行文件
add_executable(test main.cpp ThreadPool.cpp TaskQueue.cpp) 

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)