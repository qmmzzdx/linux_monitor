// 头文件保护宏，防止重复包含
#pragma once

#include <string>
#include <unordered_map>              // 哈希映射容器，用于存储CPU状态历史数据

// 项目自定义头文件
#include "monitor/monitor_inter.h"    // 监控器接口基类
#include "monitor_info.grpc.pb.h"     // gRPC生成代码（间接需要）
#include "monitor_info.pb.h"          // Protobuf消息定义

namespace monitor
{
    /**
     * @brief CPU状态监控器类
     *
     * 继承自MonitorInter接口，专门用于监控系统中各CPU核心的使用率统计。
     * 通过读取/proc/stat文件获取CPU时间片的详细分配情况，计算各种状态的百分比。
     *
     * 该类监控CPU在各种状态下的时间分布，包括用户态、系统态、空闲、等待I/O等。
     * 使用差分计算方式，通过前后两次采样的差值计算CPU使用率百分比。
     */
    class CpuStatMonitor : public MonitorInter
    {
        /**
         * @brief CPU状态数据结构体
         *
         * 用于存储单个CPU核心的时间片统计信息（单位：jiffies，通常为10ms）。
         * 这些值表示从系统启动开始，CPU在各种状态下消耗的时间计数。
         */
        struct CpuStat
        {
            std::string cpu_name;    ///< CPU核心标识，"cpu"表示总体，"cpu0"表示第一个核心
            float user;              ///< 用户态时间（普通优先级进程）
            float system;            ///< 系统态时间（内核代码）
            float idle;              ///< 空闲时间
            float nice;              ///< 用户态时间（低优先级进程）
            float io_wait;           ///< 等待I/O完成的时间
            float irq;               ///< 处理硬件中断的时间
            float soft_irq;          ///< 处理软中断的时间
            float steal;             ///< 被虚拟化环境偷走的时间（虚拟化环境）
            float guest;             ///< 运行虚拟CPU的时间（虚拟化环境）
            float guest_nice;        ///< 运行低优先级虚拟CPU的时间（虚拟化环境）
        };

    public:
        /**
         * @brief 默认构造函数
         */
        CpuStatMonitor() {}

        /**
         * @brief 更新监控信息（实现抽象基类接口）
         * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
         *
         * 从/proc/stat文件读取各CPU核心的时间片统计，
         * 通过差分计算各种状态的CPU使用率百分比，
         * 并填充到Protobuf消息中。
         */
        void UpdateOnce(monitor::proto::MonitorInfo* monitor_info) override;

        /**
         * @brief 停止监控（实现抽象基类接口）
         *
         * 对于CPU状态监控器，不需要特殊的清理操作，
         * 因此实现为空。
         */
        void Stop() override {}

    private:
        /**
         * @brief CPU状态历史数据存储
         *
         * 哈希映射，键为CPU核心名称（如"cpu"、"cpu0"、"cpu1"），
         * 值为CpuStat结构体，存储上一次采样的数据。
         * 用于计算两次采样之间的CPU使用率变化。
         */
        std::unordered_map<std::string, struct CpuStat> cpu_stat_map_;
    };
}  // namespace monitor
