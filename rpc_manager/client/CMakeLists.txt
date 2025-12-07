# 客户端测试程序
add_executable(client_test client_test.cpp)
target_link_libraries(client_test PRIVATE client)

# 设置输出目录
set_target_properties(client_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "rpc_client_test"
)