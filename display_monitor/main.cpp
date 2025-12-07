#include <QApplication>        // Qt应用程序类
#include <thread>              // C++11线程支持
#include "client/rpc_client.h" // RPC客户端
#include "monitor_widget.h"    // 监控主窗口

/**
 * @brief 监控显示程序主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 *
 * 程序入口点，负责：
 * 1. 初始化Qt应用程序
 * 2. 连接RPC服务器
 * 3. 创建并显示监控界面
 * 4. 启动数据更新线程
 */
int main(int argc, char* argv[])
{
    // 创建Qt应用程序对象，管理GUI事件循环
    QApplication app(argc, argv);

    // 设置默认的gRPC服务器地址
    std::string server_address = "localhost:50051";

    // 支持命令行参数指定服务器地址
    // 用法: ./display server_address:50051
    if (argc > 1)
    {
        server_address = argv[1];
    }

    // 创建监控窗口部件
    monitor::MonitorWidget moitor_widget;

    // 创建RPC客户端，连接到指定服务器
    monitor::RpcClient rpc_client(server_address);

    // 创建Protobuf消息对象，用于存储监控数据
    monitor::proto::MonitorInfo monitor_info;

    // 首次获取监控信息，主要用于获取主机名
    // 注意：这里存在潜在问题，如果服务器未运行会阻塞
    rpc_client.GetMonitorInfo(&monitor_info);
    std::string name = monitor_info.name();  // 获取主机名用于界面显示

    // 创建并显示完整的监控界面
    QWidget* widget = moitor_widget.ShowAllMonitorWidget(name);
    widget->show();  // 显示窗口

    // 创建数据更新线程（使用智能指针管理）
    std::unique_ptr<std::thread> thread_;
    thread_ = std::make_unique<std::thread>([&]() {
        // 线程主循环
        while (true)
        {
            // 清空上一次的监控数据
            monitor_info.Clear();

            // 从服务器获取最新的监控数据
            rpc_client.GetMonitorInfo(&monitor_info);

            // 更新监控界面显示
            moitor_widget.UpdateData(monitor_info);

            // 休眠2秒，控制更新频率
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });

    // 分离线程，让它在后台运行
    thread_->detach();

    // 启动Qt事件循环，等待用户交互
    return app.exec();
}
