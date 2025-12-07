// 包含对应的头文件
#include "monitor/net_monitor.h"

// 包含工具类头文件
#include "utils/read_file.h"      // 文件读取工具类
#include "utils/utils.h"          // 工具函数，如时间计算

namespace monitor
{
    /**
     * @brief 更新网络监控信息的具体实现
     * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
     *
     * 详细执行流程：
     * 1. 读取/proc/net/dev文件，解析各网络接口的累计统计
     * 2. 对每个网络接口：
     *    a. 解析当前采样的累计数据
     *    b. 查找上一次采样数据和采样时间
     *    c. 如果找到旧数据，计算各种网络速率
     *    d. 将速率数据填充到Protobuf消息
     *    e. 更新存储的历史数据
     *
     * /proc/net/dev文件格式示例：
     * Inter-|   Receive                                                |  Transmit
     *  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
     *    lo: 1000000    5000    0    0    0     0          0         0  1000000    5000    0    0    0     0       0          0
     *  eth0: 20000000  100000    5    2    0     0          0         0  15000000   80000    3    1    0     0       0          0
     *
     * 字段顺序（接收端）：
     * 1. bytes: 接收字节数
     * 2. packets: 接收数据包数
     * 3. errs: 接收错误数
     * 4. drop: 接收丢包数
     * 5. fifo: FIFO缓冲区错误
     * 6. frame: 帧对齐错误
     * 7. compressed: 压缩包数
     * 8. multicast: 多播包数
     *
     * 字段顺序（发送端，从第9个开始）：
     * 9. bytes: 发送字节数
     * 10. packets: 发送数据包数
     * 11. errs: 发送错误数
     * 12. drop: 发送丢包数
     * 13. fifo: FIFO缓冲区错误
     * 14. colls: 冲突数
     * 15. carrier: 载波错误
     * 16. compressed: 压缩包数
     */
    void NetMonitor::UpdateOnce(monitor::proto::MonitorInfo* monitor_info)
    {
        // 打开并读取/proc/net/dev文件
        // 该文件包含所有网络接口的累计统计数据
        ReadFile net_file(std::string("/proc/net/dev"));

        // 存储从文件读取的原始数据（一行分割后的字段）
        std::vector<std::string> net_datas;

        // 逐行读取文件，直到文件结束
        while (net_file.ReadLine(&net_datas))
        {
            // 检查是否为有效的网络接口行
            // 有效行的特征：接口名以':'结尾，且至少包含10个字段（确保有发送端数据）
            std::string name = net_datas[0];
            if (name.find(':') == name.size() - 1 && net_datas.size() >= 10)
            {
                // 创建当前采样的网络接口信息
                struct NetInfo net_info;

                // 处理接口名称：去掉末尾的':'
                name.pop_back();
                net_info.name = name;  // 如："eth0", "lo", "wlan0"

                // ==================== 解析接收端统计数据 ====================
                // 注意：所有值都是累计值，从系统启动或接口启动开始累加
                net_info.rcv_bytes = std::stoll(net_datas[1]);      // 接收字节总数
                net_info.rcv_packets = std::stoll(net_datas[2]);    // 接收数据包总数
                net_info.err_in = std::stoll(net_datas[3]);         // 接收错误总数
                net_info.drop_in = std::stoll(net_datas[4]);        // 接收丢包总数

                // ==================== 解析发送端统计数据 ====================
                // 发送端数据从第9个字段开始（索引8，因为是0-based）
                net_info.snd_bytes = std::stoll(net_datas[9]);      // 发送字节总数
                net_info.snd_packets = std::stoll(net_datas[10]);   // 发送数据包总数
                net_info.err_out = std::stoll(net_datas[11]);       // 发送错误总数
                net_info.drop_out = std::stoll(net_datas[12]);      // 发送丢包总数

                // 记录当前采样时间点，用于计算速率
                net_info.timepoint = std::chrono::steady_clock::now();

                // 查找该网络接口上一次的采样数据
                auto iter = net_info_.find(name);
                if (iter != net_info_.end())
                {
                    // 找到旧数据，进行差分计算得到速率
                    struct NetInfo old = std::move(iter->second);

                    // 计算两次采样之间的时间差（单位：秒）
                    // 使用steady_clock确保时间不受系统时钟调整影响
                    double period = Utils::SteadyTimeSecond(net_info.timepoint, old.timepoint);

                    // 只在时间差为正数时计算速率（避免除零或负值）
                    if (period > 0)
                    {
                        // 向Protobuf消息添加一个新的网络接口条目
                        auto one_net_msg = monitor_info->add_net_info();

                        // 设置网络接口名称
                        one_net_msg->set_name(net_info.name);

                        // ==================== 关键计算部分 ====================
                        // 计算发送速率（KB/s）：(新字节数 - 旧字节数) / 时间间隔 / 1024
                        // 注意：这里计算的是千字节/秒（KB/s），不是千比特/秒（Kbps）
                        one_net_msg->set_send_rate((net_info.snd_bytes - old.snd_bytes) /
                            1024.0 / period);

                        // 计算接收速率（KB/s）
                        one_net_msg->set_rcv_rate((net_info.rcv_bytes - old.rcv_bytes) /
                            1024.0 / period);

                        // 计算发送包速率（packets/s）
                        one_net_msg->set_send_packets_rate(
                            (net_info.snd_packets - old.snd_packets) / period);

                        // 计算接收包速率（packets/s）
                        one_net_msg->set_rcv_packets_rate(
                            (net_info.rcv_packets - old.rcv_packets) / period);

                        // 注意：这里没有计算错误率和丢包率，可以根据需要添加：
                        // 错误率 = (新错误数 - 旧错误数) / (新包数 - 旧包数)
                        // 丢包率 = (新丢包数 - 旧丢包数) / (新包数 - 旧包数)
                    }
                }
                // 更新存储的历史数据（用当前采样替换旧数据）
                // 使用std::move移动语义，避免不必要的拷贝
                net_info_[name] = std::move(net_info);
            }
            // 清空临时容器，准备读取下一行
            net_datas.clear();
        }
        // 函数返回，网络监控数据已成功采集并填充到monitor_info中
        return;
    }
}  // namespace monitor
