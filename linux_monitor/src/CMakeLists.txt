set(SOURCES
    main.cpp
    monitor/cpu_softirq_monitor.cpp
    monitor/cpu_load_monitor.cpp
    monitor/cpu_stat_monitor.cpp
    monitor/mem_monitor.cpp
    monitor/net_monitor.cpp
    utils/read_file.cpp
)

add_executable(monitor ${SOURCES})

# 包含目录配置
target_include_directories(monitor PUBLIC
    ${PROJECT_SOURCE_DIR}/linux_monitor/include
    ${PROJECT_SOURCE_DIR}/rpc_manager
    # 如果需要，添加其他包含目录
)

target_link_libraries(monitor
    PUBLIC
    monitor_proto
    client
)
