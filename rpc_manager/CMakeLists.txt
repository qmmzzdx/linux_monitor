add_library(client INTERFACE)

target_include_directories(client INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}    # rpc_manager 目录
    ${PROJECT_SOURCE_DIR}/proto    # proto 目录
)

target_link_libraries(client INTERFACE
    monitor_proto  # Protobuf 生成的库
)

add_library(server_lib INTERFACE)

target_include_directories(server_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/proto
)

target_link_libraries(server_lib INTERFACE
    monitor_proto
)

add_subdirectory(client)    # 客户端测试程序
add_subdirectory(server)    # 服务器程序