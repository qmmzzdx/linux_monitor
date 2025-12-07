// 包含对应的头文件
#include "monitor/cpu_stat_monitor.h"

// 包含工具类头文件
#include "utils/read_file.h"

// 包含Protobuf相关头文件
#include "monitor_info.grpc.pb.h"
#include "monitor_info.pb.h"

namespace monitor
{
    /**
     * @brief 更新CPU状态监控信息的具体实现
     * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
     *
     * 详细执行流程：
     * 1. 读取/proc/stat文件，解析各CPU核心的时间片统计
     * 2. 对每个CPU核心：
     *    a. 解析当前采样的时间片数据
     *    b. 查找上一次采样数据
     *    c. 如果找到旧数据，计算各种CPU状态的使用率百分比
     *    d. 将百分比数据填充到Protobuf消息
     *    e. 更新存储的历史数据
     *
     * /proc/stat文件格式示例：
     * cpu  145598 1961 36646 11275927 3070 0 4478 0 0 0
     * cpu0 36951 489 9448 2817880 760 0 1133 0 0 0
     * cpu1 36069 488 9195 2819470 768 0 1108 0 0 0
     * 字段说明（单位：jiffies，通常为10ms）：
     * 1. user: 用户态时间
     * 2. nice: 低优先级用户态时间
     * 3. system: 系统态时间
     * 4. idle: 空闲时间
     * 5. iowait: 等待I/O时间
     * 6. irq: 硬件中断时间
     * 7. softirq: 软中断时间
     * 8. steal: 虚拟化环境偷走的时间
     * 9. guest: 运行虚拟CPU的时间
     * 10. guest_nice: 运行低优先级虚拟CPU的时间
     */
    void CpuStatMonitor::UpdateOnce(monitor::proto::MonitorInfo* monitor_info)
    {
        // 打开并读取/proc/stat文件
        // 该文件包含所有CPU核心的时间片统计信息
        ReadFile cpu_stat_file(std::string("/proc/stat"));

        // 存储从文件读取的原始数据（一行数据）
        std::vector<std::string> cpu_stat_list;

        // 逐行读取文件，直到文件结束
        while (cpu_stat_file.ReadLine(&cpu_stat_list))
        {
            // 检查是否为CPU相关行（以"cpu"开头）
            if (cpu_stat_list[0].find("cpu") != std::string::npos)
            {
                // 调试输出（注释掉的代码）
                // std::cout << cpu_stat_list[0] << cpu_stat_list[1] << std::endl;

                // 创建当前采样的CPU状态数据
                struct CpuStat cpu_stat;

                // 设置CPU核心标识
                // "cpu" 表示所有CPU核心的总体统计
                // "cpu0", "cpu1" 等表示具体的CPU核心
                cpu_stat.cpu_name = cpu_stat_list[0];

                // 解析各种CPU状态的时间片计数值
                // 将字符串转换为浮点数，单位：jiffies（时间片）
                cpu_stat.user = std::stof(cpu_stat_list[1]);        // 用户态时间
                cpu_stat.nice = std::stof(cpu_stat_list[2]);        // 低优先级用户态时间
                cpu_stat.system = std::stof(cpu_stat_list[3]);      // 系统态时间
                cpu_stat.idle = std::stof(cpu_stat_list[4]);        // 空闲时间
                cpu_stat.io_wait = std::stof(cpu_stat_list[5]);     // 等待I/O时间
                cpu_stat.irq = std::stof(cpu_stat_list[6]);         // 硬件中断时间
                cpu_stat.soft_irq = std::stof(cpu_stat_list[7]);    // 软中断时间
                cpu_stat.steal = std::stof(cpu_stat_list[8]);       // 虚拟化偷走时间
                cpu_stat.guest = std::stof(cpu_stat_list[9]);       // 虚拟CPU运行时间
                cpu_stat.guest_nice = std::stof(cpu_stat_list[10]); // 低优先级虚拟CPU时间

                // 查找该CPU核心上一次的采样数据
                auto it = cpu_stat_map_.find(cpu_stat.cpu_name);
                if (it != cpu_stat_map_.end())
                {
                    // 找到旧数据，进行差分计算
                    // 调试输出（注释掉的代码）
                    // std::cout << cpu_stat.cpu_name << std::endl;

                    struct CpuStat old = it->second;

                    // 向Protobuf消息添加一个新的CPU状态条目
                    auto cpu_stat_msg = monitor_info->add_cpu_stat();

                    // ==================== 关键计算部分 ====================
                    // 计算总时间片和繁忙时间片

                    // 计算新的总时间片（不包括guest和guest_nice，因为它们包含在user和nice中）
                    float new_cpu_total_time = cpu_stat.user + cpu_stat.system +
                        cpu_stat.idle + cpu_stat.nice +
                        cpu_stat.io_wait + cpu_stat.irq +
                        cpu_stat.soft_irq + cpu_stat.steal;

                    // 计算旧的总时间片
                    float old_cpu_total_time = old.user + old.system + old.idle + old.nice +
                        old.io_wait + old.irq + old.soft_irq +
                        old.steal;

                    // 计算新的繁忙时间片（CPU在工作的时间）
                    float new_cpu_busy_time = cpu_stat.user + cpu_stat.system +
                        cpu_stat.nice + cpu_stat.irq +
                        cpu_stat.soft_irq + cpu_stat.steal;

                    // 计算旧的繁忙时间片
                    float old_cpu_busy_time = old.user + old.system + old.nice + old.irq +
                        old.soft_irq + old.steal;

                    // 计算时间片差值（两次采样之间的变化）
                    float total_time_diff = new_cpu_total_time - old_cpu_total_time;

                    // 只在时间差为正数时计算百分比（避免除零或负值）
                    if (total_time_diff > 0)
                    {
                        // 计算各种CPU使用率的百分比
                        // 公式：（状态时间变化量 / 总时间变化量）× 100%

                        // 总体CPU使用率（所有工作状态的时间占比）
                        float cpu_percent = (new_cpu_busy_time - old_cpu_busy_time) /
                            total_time_diff * 100.00;

                        // 用户态使用率（普通优先级进程）
                        float cpu_user_percent = (cpu_stat.user - old.user) /
                            total_time_diff * 100.00;

                        // 系统态使用率（内核代码）
                        float cpu_system_percent = (cpu_stat.system - old.system) /
                            total_time_diff * 100.00;

                        // 低优先级用户态使用率
                        float cpu_nice_percent = (cpu_stat.nice - old.nice) /
                            total_time_diff * 100.00;

                        // 空闲时间占比
                        float cpu_idle_percent = (cpu_stat.idle - old.idle) /
                            total_time_diff * 100.00;

                        // 等待I/O时间占比
                        float cpu_io_wait_percent = (cpu_stat.io_wait - old.io_wait) /
                            total_time_diff * 100.00;

                        // 硬件中断时间占比
                        float cpu_irq_percent = (cpu_stat.irq - old.irq) /
                            total_time_diff * 100.00;

                        // 软中断时间占比
                        float cpu_soft_irq_percent = (cpu_stat.soft_irq - old.soft_irq) /
                            total_time_diff * 100.00;

                        // ==================== 填充Protobuf消息 ====================
                        // 设置CPU核心标识
                        cpu_stat_msg->set_cpu_name(cpu_stat.cpu_name);

                        // 设置各种CPU使用率百分比
                        cpu_stat_msg->set_cpu_percent(cpu_percent);          // 总体使用率
                        cpu_stat_msg->set_usr_percent(cpu_user_percent);     // 用户态使用率
                        cpu_stat_msg->set_system_percent(cpu_system_percent); // 系统态使用率
                        cpu_stat_msg->set_nice_percent(cpu_nice_percent);    // 低优先级使用率
                        cpu_stat_msg->set_idle_percent(cpu_idle_percent);    // 空闲时间占比
                        cpu_stat_msg->set_io_wait_percent(cpu_io_wait_percent); // I/O等待占比
                        cpu_stat_msg->set_irq_percent(cpu_irq_percent);      // 硬件中断占比
                        cpu_stat_msg->set_soft_irq_percent(cpu_soft_irq_percent); // 软中断占比
                    }
                }
                // 更新存储的历史数据（用当前采样替换旧数据）
                // 注意：guest和guest_nice字段被包含在user和nice中，所以计算总时间时不重复计算
                cpu_stat_map_[cpu_stat.cpu_name] = cpu_stat;
            }
            // 清空临时容器，准备读取下一行
            cpu_stat_list.clear();
        }
        // 函数返回，CPU状态数据已成功采集并填充到monitor_info中
        return;
    }
}  // namespace monitor
