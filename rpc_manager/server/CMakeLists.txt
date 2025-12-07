# 服务器可执行文件
add_executable(server server_main.cpp)
target_link_libraries(server PRIVATE server_lib)

# 设置输出目录
set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)