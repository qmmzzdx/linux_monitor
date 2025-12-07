# 查找Qt5依赖
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# 启用自动MOC
set(CMAKE_AUTOMOC ON)

# 列出所有源文件和头文件
set(SOURCES
    main.cpp
    # 如果还有其他.cpp文件，在这里添加
)

# 列出所有需要MOC的头文件
set(HEADERS
    cpu_load_model.h
    cpu_softirq_model.h
    cpu_stat_model.h
    mem_model.h
    monitor_inter.h
    monitor_widget.h
    net_model.h
)

# 创建可执行文件（包含源文件和头文件）
add_executable(display ${SOURCES} ${HEADERS})

# 包含目录设置
target_include_directories(display PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}       # 当前目录
    ${PROJECT_SOURCE_DIR}/rpc_manager # RPC客户端头文件
    ${Qt5Widgets_INCLUDE_DIRS}        # Qt Widgets包含目录
    ${Qt5Core_INCLUDE_DIRS}           # Qt Core包含目录
)

# 链接库设置
target_link_libraries(display
    Qt5::Widgets           # Qt窗口部件
    Qt5::Core              # Qt核心
    client                 # RPC客户端库
    monitor_proto          # Protobuf库
)