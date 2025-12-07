// 头文件保护宏，防止重复包含
#pragma once

#include <string>
#include "monitor/monitor_inter.h"    // 监控器接口基类

// Protobuf和gRPC生成的头文件
#include "monitor_info.grpc.pb.h"     // gRPC服务相关定义（可能间接需要）
#include "monitor_info.pb.h"          // Protobuf消息定义

namespace monitor
{
    /**
     * @brief CPU负载监控器类
     *
     * 继承自MonitorInter抽象基类，专门用于监控系统CPU负载。
     * 该类负责从Linux系统的/proc/loadavg文件中读取CPU平均负载数据，
     * 包括1分钟、3分钟和15分钟的平均负载值。
     *
     * 设计模式：策略模式（具体策略实现）
     * 职责：单一职责原则，只负责CPU负载数据采集
     */
    class CpuLoadMonitor : public MonitorInter
    {
    public:
        /**
         * @brief 默认构造函数
         *
         * 初始化CPU负载监控器，成员变量会在UpdateOnce方法中被赋值。
         */
        CpuLoadMonitor() {}

        /**
         * @brief 更新监控信息（实现抽象基类接口）
         * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
         *
         * 从/proc/loadavg文件读取当前系统CPU负载数据，
         * 并将读取到的值填充到提供的Protobuf消息中。
         *
         * 具体步骤：
         * 1. 打开并读取/proc/loadavg文件
         * 2. 解析文件内容获取三个负载值
         * 3. 将值设置到Protobuf消息的对应字段
         *
         * @note 此方法会修改monitor_info指向的消息内容
         * @note 需要在Linux系统上运行（依赖/proc文件系统）
         */
        void UpdateOnce(monitor::proto::MonitorInfo* monitor_info) override;

        /**
         * @brief 停止监控（实现抽象基类接口）
         *
         * 对于CPU负载监控器，不需要特殊的清理操作，
         * 因此实现为空。保留此接口是为了保持接口一致性。
         */
        void Stop() override {}

    private:
        float load_avg_1_;    ///< 存储1分钟平均负载值
        float load_avg_3_;    ///< 存储3分钟平均负载值
        float load_avg_15_;   ///< 存储15分钟平均负载值
    };

}  // namespace monitor
