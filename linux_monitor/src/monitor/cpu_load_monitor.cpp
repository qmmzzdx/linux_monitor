// 包含对应的头文件
#include "monitor/cpu_load_monitor.h"

// 包含工具类头文件
#include "utils/read_file.h"          // 文件读取工具类

// 包含Protobuf相关头文件
#include "monitor_info.grpc.pb.h"     // gRPC相关（可能不需要但保持一致性）
#include "monitor_info.pb.h"          // Protobuf消息操作

namespace monitor
{
    /**
     * @brief 更新CPU负载监控信息的具体实现
     * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
     *
     * 详细执行流程：
     * 1. 创建ReadFile对象打开/proc/loadavg文件
     * 2. 读取文件内容到字符串向量
     * 3. 解析字符串为浮点数并存储到成员变量
     * 4. 将值设置到Protobuf消息的对应字段
     *
     * /proc/loadavg文件格式：
     *   0.45 0.67 0.89 2/345 12345
     *   ↑     ↑     ↑     ↑      ↑
     *   1分钟 3分钟 15分钟 运行进程信息 最后PID
     *
     * @warning 此方法假设/proc/loadavg文件存在且格式正确
     * @warning 没有异常处理，文件读取失败可能导致程序崩溃
     */
    void CpuLoadMonitor::UpdateOnce(monitor::proto::MonitorInfo* monitor_info)
    {
        // 创建文件读取对象，打开/proc/loadavg文件
        // /proc/loadavg是Linux内核提供的虚拟文件，包含系统负载信息
        ReadFile cpu_load_file(std::string("/proc/loadavg"));

        // 读取文件内容到字符串向量
        // cpu_load向量将包含从文件读取的所有以空格分隔的字段
        std::vector<std::string> cpu_load;
        cpu_load_file.ReadLine(&cpu_load);

        // 解析字符串为浮点数
        // 将字符串转换为浮点数，存储到对应的成员变量中
        // cpu_load[0]: 1分钟平均负载
        // cpu_load[1]: 3分钟平均负载  
        // cpu_load[2]: 15分钟平均负载
        load_avg_1_ = std::stof(cpu_load[0]);   // 字符串转浮点数
        load_avg_3_ = std::stof(cpu_load[1]);
        load_avg_15_ = std::stof(cpu_load[2]);

        // 将数据填充到Protobuf消息中
        // mutable_cpu_load()返回cpu_load字段的可变指针，如果字段不存在则创建
        auto cpu_load_msg = monitor_info->mutable_cpu_load();

        // 设置Protobuf消息的各个字段
        // set_load_avg_1()是Protobuf生成的setter方法
        cpu_load_msg->set_load_avg_1(load_avg_1_);    // 设置1分钟平均负载
        cpu_load_msg->set_load_avg_3(load_avg_3_);    // 设置3分钟平均负载
        cpu_load_msg->set_load_avg_15(load_avg_15_);  // 设置15分钟平均负载

        // 函数返回，监控数据已成功采集并填充到monitor_info中
        return;
    }
}  // namespace monitor
