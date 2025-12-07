#include <iostream>
#include <string>
#include "rpc_client.h"
#include "monitor_info.grpc.pb.h"
#include "monitor_info.pb.h"

int main(int argc, char* argv[])
{
    std::cout << "=== RPC客户端测试 ===" << std::endl;

    monitor::proto::MonitorInfo monitor_info;
    auto soft_irq = monitor_info.add_soft_irq();
    soft_irq->set_cpu("cpu1");
    auto soft_irq2 = monitor_info.add_soft_irq();
    soft_irq2->set_cpu("cpu2");

    monitor::RpcClient rpc_client;

    try
    {
        rpc_client.SetMonitorInfo(monitor_info);
        std::cout << "✓ 测试通过：数据发送成功" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "✗ 测试失败: " << e.what() << std::endl;
        return 1;
    }
}
