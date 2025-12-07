// 包含对应的头文件
#include "monitor/cpu_softirq_monitor.h"

// 包含工具类头文件
#include "utils/read_file.h"      // 文件读取工具类
#include "utils/utils.h"          // 工具函数，包含时间计算

// 包含Protobuf相关头文件
#include "monitor_info.grpc.pb.h"
#include "monitor_info.pb.h"

namespace monitor
{
    /**
     * @brief 更新CPU软中断监控信息的具体实现
     * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
     *
     * 详细执行流程：
     * 1. 读取/proc/softirqs文件，解析各CPU核心的软中断计数
     * 2. 对每个CPU核心：
     *    a. 创建当前采样数据
     *    b. 查找上一次采样数据
     *    c. 如果找到旧数据，计算中断速率（差值/时间间隔）
     *    d. 将速率数据填充到Protobuf消息
     *    e. 更新存储的采样数据
     *
     * /proc/softirqs文件格式示例：
     *                     CPU0       CPU1       CPU2       CPU3
     *           HI:          0          0          0          0
     *        TIMER:     434247     331510     331046     329751
     *       NET_TX:          0          0          0          0
     *       NET_RX:     169553          0          0          0
     *        BLOCK:          0          0          0          0
     *     IRQ_POLL:          0          0          0          0
     *      TASKLET:         36          0          0          0
     *        SCHED:     313190     308517     308399     307518
     *      HRTIMER:          0          0          0          0
     *          RCU:     385834     379541     379504     378660
     */
    void CpuSoftIrqMonitor::UpdateOnce(monitor::proto::MonitorInfo* monitor_info)
    {
        // 打开并读取/proc/softirqs文件
        // 该文件包含所有CPU核心的各种软中断计数
        ReadFile softirqs_file(std::string("/proc/softirqs"));

        // 存储从文件读取的原始数据
        std::vector<std::string> one_softirq;           // 存储一行的数据
        std::vector<std::vector<std::string>> softirq;  // 存储所有行的数据，二维向量

        // 逐行读取文件，直到文件结束
        // 每行包含一种软中断类型在各个CPU核心上的计数
        while (softirqs_file.ReadLine(&one_softirq))
        {
            softirq.push_back(one_softirq);  // 将一行数据添加到二维向量
            one_softirq.clear();             // 清空临时容器，准备读取下一行
        }

        // 处理每个CPU核心的数据
        // softirq[0] 是第一行，包含CPU核心名称（如"CPU0", "CPU1"）
        // softirq[0].size() - 1 是因为第一列是软中断类型名称
        for (int i = 0; i < softirq[0].size() - 1; i++)
        {
            // 获取CPU核心名称（如"CPU0"）
            std::string name = softirq[0][i];

            // 创建当前采样的软中断数据
            struct SoftIrq info;
            info.cpu_name = name;  // 设置CPU核心标识

            // 从文件数据中解析各种软中断的计数值
            // softirq[1]到softirq[10]对应10种软中断类型
            // i+1 是因为第0列是软中断类型名称
            info.hi = std::stoll(softirq[1][i + 1]);        // 高优先级任务中断
            info.timer = std::stoll(softirq[2][i + 1]);     // 定时器中断
            info.net_tx = std::stoll(softirq[3][i + 1]);    // 网络发送中断
            info.net_rx = std::stoll(softirq[4][i + 1]);    // 网络接收中断
            info.block = std::stoll(softirq[5][i + 1]);     // 块设备中断
            info.irq_poll = std::stoll(softirq[6][i + 1]);  // IRQ轮询中断
            info.tasklet = std::stoll(softirq[7][i + 1]);   // 小任务中断
            info.sched = std::stoll(softirq[8][i + 1]);     // 调度器中断
            info.hrtimer = std::stoll(softirq[9][i + 1]);   // 高精度定时器中断
            info.rcu = std::stoll(softirq[10][i + 1]);      // RCU中断

            // 记录当前采样时间点，用于计算时间间隔
            info.timepoint = std::chrono::steady_clock::now();

            // 查找该CPU核心上一次的采样数据
            auto iter = cpu_softirqs_.find(name);
            if (iter != cpu_softirqs_.end())
            {
                // 找到旧数据，计算中断速率
                struct SoftIrq& old = (*iter).second;

                // 计算两次采样的时间间隔（秒）
                double period = Utils::SteadyTimeSecond(info.timepoint, old.timepoint);

                // 只在时间间隔有效时计算速率
                if (period > 0)
                {
                    // 向Protobuf消息添加一个新的软中断条目
                    auto one_softirq_msg = monitor_info->add_soft_irq();

                    // 设置CPU核心标识
                    one_softirq_msg->set_cpu(info.cpu_name);

                    // 计算并设置各种软中断的速率（中断次数/秒）
                    // 公式：(当前计数 - 上次计数) / 时间间隔
                    one_softirq_msg->set_hi((info.hi - old.hi) / period);
                    one_softirq_msg->set_timer((info.timer - old.timer) / period);
                    one_softirq_msg->set_net_tx((info.net_tx - old.net_tx) / period);
                    one_softirq_msg->set_net_rx((info.net_rx - old.net_rx) / period);
                    one_softirq_msg->set_block((info.block - old.block) / period);
                    one_softirq_msg->set_irq_poll((info.irq_poll - old.irq_poll) / period);
                    one_softirq_msg->set_tasklet((info.tasklet - old.tasklet) / period);
                    one_softirq_msg->set_sched((info.sched - old.sched) / period);
                    one_softirq_msg->set_hrtimer((info.hrtimer - old.hrtimer) / period);
                    one_softirq_msg->set_rcu((info.rcu - old.rcu) / period);
                }
            }

            // 更新存储的数据（用当前采样替换旧数据）
            // 如果是第一次采样，直接插入；否则更新现有数据
            cpu_softirqs_[name] = info;
        }
        // 函数返回，软中断数据已成功采集并填充到monitor_info中
        return;
    }
}  // namespace monitor
