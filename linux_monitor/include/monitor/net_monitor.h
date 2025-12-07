// 头文件保护宏，防止重复包含
#pragma once

// C++标准库头文件
#include <string>            // 字符串操作
#include <unordered_map>     // 哈希映射容器，用于存储网络接口历史数据
#include <chrono>            // 高精度时间库，用于计算网络速率

// 项目自定义头文件
#include "monitor/monitor_inter.h"    // 监控器接口基类
#include "monitor_info.grpc.pb.h"     // gRPC生成代码（间接需要）
#include "monitor_info.pb.h"          // Protobuf消息定义

namespace monitor
{
    /**
     * @brief 网络监控器类
     *
     * 继承自MonitorInter接口，专门用于监控系统网络接口的流量统计。
     * 通过读取/proc/net/dev文件获取各网络接口的详细收发统计信息，
     * 计算网络带宽使用率、丢包率、错误率等关键指标。
     *
     * 该类采用差分计算方法，通过记录前后两次采样的数据和时间戳，
     * 计算出精确的网络速率（如：KB/s, packets/s）。
     */
    class NetMonitor : public MonitorInter
    {
        /**
         * @brief 网络接口信息数据结构体
         *
         * 用于存储单个网络接口的统计信息，包括：
         * - 接收和发送的字节数、数据包数
         * - 接收和发送的错误数、丢包数
         * - 采样时间点（用于计算速率）
         */
        struct NetInfo
        {
            std::string name;                                  ///< 网络接口名称（如eth0、lo、wlan0）
            int64_t rcv_bytes;                                 ///< 累计接收字节数（bytes）
            int64_t rcv_packets;                               ///< 累计接收数据包数（packets）
            int64_t err_in;                                    ///< 累计接收错误数
            int64_t drop_in;                                   ///< 累计接收丢包数
            int64_t snd_bytes;                                 ///< 累计发送字节数（bytes）
            int64_t snd_packets;                               ///< 累计发送数据包数（packets）
            int64_t err_out;                                   ///< 累计发送错误数
            int64_t drop_out;                                  ///< 累计发送丢包数
            std::chrono::steady_clock::time_point timepoint;   ///< 数据采样时间点
        };

    public:
        /**
         * @brief 默认构造函数
         */
        NetMonitor() {}

        /**
         * @brief 更新网络监控信息（实现抽象基类接口）
         * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
         *
         * 从/proc/net/dev文件读取各网络接口的累计统计，
         * 通过差分计算网络传输速率（字节/秒、包/秒），
         * 并填充到Protobuf消息中。
         */
        void UpdateOnce(monitor::proto::MonitorInfo* monitor_info) override;

        /**
         * @brief 停止监控（实现抽象基类接口）
         *
         * 对于网络监控器，不需要特殊的清理操作，
         * 因此实现为空。
         */
        void Stop() override {}

    private:
        /**
         * @brief 网络接口历史数据存储
         *
         * 哈希映射，键为网络接口名称（如"eth0"、"lo"），
         * 值为NetInfo结构体，存储上一次采样的数据和时间戳。
         * 用于计算两次采样之间的网络速率变化。
         */
        std::unordered_map<std::string, struct NetInfo> net_info_;
    };
}  // namespace monitor
