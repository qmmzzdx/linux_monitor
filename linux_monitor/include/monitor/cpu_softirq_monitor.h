// 头文件保护宏，防止重复包含
#pragma once

// C++标准库头文件
#include <string>            // 字符串操作
#include <unordered_map>     // 哈希映射容器，用于存储CPU软中断数据
#include <chrono>            // 高精度时间库，用于计算网络速率

// 项目自定义头文件
#include "monitor/monitor_inter.h"    // 监控器接口基类
#include "monitor_info.grpc.pb.h"     // gRPC生成代码（间接需要）
#include "monitor_info.pb.h"          // Protobuf消息定义

namespace monitor
{
    /**
     * @brief CPU软中断监控器类
     *
     * 继承自MonitorInter接口，专门用于监控系统中各CPU核心的软中断统计信息。
     * 软中断是Linux内核中处理中断的机制之一，用于处理时间要求不那么紧迫的任务。
     *
     * 该类通过读取/proc/softirqs文件获取软中断计数，并计算中断速率。
     *
     * 特点：
     * 1. 使用差分计算：通过前后两次采样的差值计算中断速率
     * 2. 支持多CPU核心：监控系统中所有CPU核心的软中断情况
     * 3. 包含10种软中断类型：HI、TIMER、NET_TX等
     */
    class CpuSoftIrqMonitor : public MonitorInter
    {
        /**
         * @brief 软中断数据结构体
         *
         * 用于存储单个CPU核心的软中断统计信息。
         * 包含10种软中断类型的计数和时间戳，用于速率计算。
         */
        struct SoftIrq
        {
            std::string cpu_name;                             ///< CPU核心标识，如"CPU0", "CPU1"
            int64_t hi;                                       ///< 高优先级任务软中断计数
            int64_t timer;                                    ///< 定时器软中断计数
            int64_t net_tx;                                   ///< 网络发送软中断计数
            int64_t net_rx;                                   ///< 网络接收软中断计数
            int64_t block;                                    ///< 块设备软中断计数
            int64_t irq_poll;                                 ///< IRQ轮询软中断计数
            int64_t tasklet;                                  ///< 小任务软中断计数
            int64_t sched;                                    ///< 调度器软中断计数
            int64_t hrtimer;                                  ///< 高精度定时器软中断计数
            int64_t rcu;                                      ///< RCU（读-拷贝-更新）软中断计数
            std::chrono::steady_clock::time_point timepoint;  ///< 采样时间点
        };

    public:
        /**
         * @brief 默认构造函数
         */
        CpuSoftIrqMonitor() {}

        /**
         * @brief 更新监控信息（实现抽象基类接口）
         * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
         *
         * 从/proc/softirqs文件读取各CPU核心的软中断统计，
         * 计算中断速率并填充到Protobuf消息中。
         */
        void UpdateOnce(monitor::proto::MonitorInfo* monitor_info) override;

        /**
         * @brief 停止监控（实现抽象基类接口）
         *
         * 对于软中断监控器，不需要特殊的清理操作，
         * 因此实现为空。
         */
        void Stop() override {}

    private:
        /**
         * @brief CPU软中断数据存储
         *
         * 哈希映射，键为CPU核心名称（如"CPU0"），
         * 值为SoftIrq结构体，存储上一次采样的数据。
         * 用于计算两次采样之间的中断速率。
         */
        std::unordered_map<std::string, struct SoftIrq> cpu_softirqs_;
    };
}  // namespace monitor
