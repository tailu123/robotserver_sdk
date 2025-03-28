# test_2 示例的 CMakeLists.txt
add_executable(test_2 test_2.cpp)
target_link_libraries(test_2 PRIVATE robotserver_sdk Threads::Threads)

# 添加动作命令测试程序
add_executable(test_2_ActionCommand test_2_ActionCommand.cpp)
target_link_libraries(test_2_ActionCommand PRIVATE robotserver_sdk Threads::Threads)

# 添加移动控制测试程序
add_executable(test_2_move test_2_move.cpp)
target_link_libraries(test_2_move PRIVATE robotserver_sdk Threads::Threads)
