# basic 示例目录的 CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 基本示例
add_executable(test test.cpp)
target_link_libraries(test PRIVATE robotserver_sdk Threads::Threads)
