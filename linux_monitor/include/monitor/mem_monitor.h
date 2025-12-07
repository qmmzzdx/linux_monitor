// 头文件保护宏，防止重复包含
#pragma once

#include <string>
#include <unordered_map>              // 哈希映射容器

// 项目自定义头文件
#include "monitor/monitor_inter.h"    // 监控器接口基类
#include "monitor_info.grpc.pb.h"     // gRPC生成代码（间接需要）
#include "monitor_info.pb.h"          // Protobuf消息定义

namespace monitor
{
    /**
     * @brief 内存监控器类
     *
     * 继承自MonitorInter接口，专门用于监控系统内存使用情况。
     * 通过读取/proc/meminfo文件获取详细的内存统计信息，
     * 包括物理内存、交换空间、缓存、缓冲区等各个维度的数据。
     *
     * 该类提供了完整的内存使用情况分析，包括内存总量、空闲内存、
     * 可用内存、缓存、缓冲区以及各种特定内存分配情况的监控。
     */
    class MemMonitor : public MonitorInter
    {
        /**
         * @brief 内存信息数据结构体
         *
         * 用于存储从/proc/meminfo文件中解析出的各项内存统计值。
         * 所有值都以KB为单位存储原始数据，后续会转换为GB用于输出。
         */
        struct MenInfo
        {
            int64_t total;           ///< 系统总物理内存（KB）
            int64_t free;            ///< 空闲内存（KB）
            int64_t avail;           ///< 可用内存（估计可用给应用程序的内存，KB）
            int64_t buffers;         ///< 缓冲区内存（块设备缓存，KB）
            int64_t cached;          ///< 页面缓存（文件系统缓存，KB）
            int64_t swap_cached;     ///< 已交换出的内存但被缓存在交换区（KB）
            int64_t active;          ///< 活跃内存（最近使用的内存，KB）
            int64_t in_active;       ///< 非活跃内存（最近未使用的内存，KB）
            int64_t active_anon;     ///< 活跃匿名页（无文件背景的内存，如堆、栈，KB）
            int64_t inactive_anon;   ///< 非活跃匿名页（KB）
            int64_t active_file;     ///< 活跃文件页（文件缓存，KB）
            int64_t inactive_file;   ///< 非活跃文件页（KB）
            int64_t dirty;           ///< 等待写回磁盘的脏页（KB）
            int64_t writeback;       ///< 正在写回磁盘的页（KB）
            int64_t anon_pages;      ///< 所有匿名页（KB）
            int64_t mapped;          ///< 内存映射文件（mmap，KB）
            int64_t kReclaimable;    ///< 内核可回收内存（KB）
            int64_t sReclaimable;    ///< Slab可回收内存（KB）
            int64_t sUnreclaim;      ///< Slab不可回收内存（KB）
        };

    public:
        /**
         * @brief 默认构造函数
         */
        MemMonitor() {}

        /**
         * @brief 更新内存监控信息（实现抽象基类接口）
         * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
         *
         * 从/proc/meminfo文件读取内存使用统计，
         * 计算内存使用率并将各项内存数据填充到Protobuf消息中。
         * 注意：数据从KB转换为GB后输出。
         */
        void UpdateOnce(monitor::proto::MonitorInfo* monitor_info);

        /**
         * @brief 停止监控（实现抽象基类接口）
         *
         * 对于内存监控器，不需要特殊的清理操作，
         * 因此实现为空。
         */
        void Stop() override {}
    };
}  // namespace monitor
