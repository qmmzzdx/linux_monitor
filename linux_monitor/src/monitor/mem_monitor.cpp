// 包含对应的头文件
#include "monitor/mem_monitor.h"

// 包含工具类头文件
#include "utils/read_file.h"      // 文件读取工具类

namespace monitor
{
    /**
     * @brief KB到GB的转换系数
     *
     * 注意：Linux系统中通常使用1024作为进制（KiB、MiB、GiB），
     * 但这里使用1000进制，可能是为了与某些显示标准保持一致。
     * 1 GB = 1000 * 1000 KB
     * 这种转换在实际显示中可能会导致显示的容量稍小一些。
     */
    static constexpr float KBToGB = 1000 * 1000;

    /**
     * @brief 更新内存监控信息的具体实现
     * @param monitor_info 输出参数，指向要填充的监控信息Protobuf消息
     *
     * 详细执行流程：
     * 1. 打开并读取/proc/meminfo文件
     * 2. 逐行解析文件内容，提取各项内存统计值
     * 3. 计算内存使用率百分比
     * 4. 将所有内存数据从KB转换为GB
     * 5. 填充到Protobuf消息中
     *
     * /proc/meminfo文件格式示例：
     * MemTotal:       16335784 kB
     * MemFree:         3032468 kB
     * MemAvailable:    8411436 kB
     * Buffers:          298416 kB
     * Cached:          5940484 kB
     * ...
     * 每一行格式为：标签: 数值 kB
     * 注意：某些值可能没有"kB"单位，但大多数有。
     */
    void MemMonitor::UpdateOnce(monitor::proto::MonitorInfo* monitor_info)
    {
        // 打开/proc/meminfo文件
        // 该文件包含系统当前内存使用情况的完整统计
        ReadFile mem_file("/proc/meminfo");

        // 创建内存信息结构体，初始化为0（注意：这里没有显式初始化）
        struct MenInfo mem_info;

        // 存储从文件读取的原始数据（一行分割后的字段）
        std::vector<std::string> mem_datas;

        // 逐行读取并解析文件内容
        while (mem_file.ReadLine(&mem_datas))
        {
            // 根据标签名提取对应的内存统计值
            // 注意：mem_datas[0]是标签名，mem_datas[1]是数值，mem_datas[2]可能是"kB"

            if (mem_datas[0] == "MemTotal:")
            {
                mem_info.total = std::stoll(mem_datas[1]);          // 系统总物理内存
            }
            else if (mem_datas[0] == "MemFree:")
            {
                mem_info.free = std::stoll(mem_datas[1]);           // 空闲内存
            }
            else if (mem_datas[0] == "MemAvailable:")
            {
                mem_info.avail = std::stoll(mem_datas[1]);          // 可用内存（估算）
            }
            else if (mem_datas[0] == "Buffers:")
            {
                mem_info.buffers = std::stoll(mem_datas[1]);        // 缓冲区内存
            }
            else if (mem_datas[0] == "Cached:")
            {
                mem_info.cached = std::stoll(mem_datas[1]);         // 页面缓存
            }
            else if (mem_datas[0] == "SwapCached:")
            {
                mem_info.swap_cached = std::stoll(mem_datas[1]);    // 交换区缓存
            }
            else if (mem_datas[0] == "Active:")
            {
                mem_info.active = std::stoll(mem_datas[1]);         // 活跃内存
            }
            else if (mem_datas[0] == "Inactive:")
            {
                mem_info.in_active = std::stoll(mem_datas[1]);      // 非活跃内存
            }
            else if (mem_datas[0] == "Active(anon):")
            {
                mem_info.active_anon = std::stoll(mem_datas[1]);    // 活跃匿名页
            }
            else if (mem_datas[0] == "Inactive(anon):")
            {
                mem_info.inactive_anon = std::stoll(mem_datas[1]);  // 非活跃匿名页
            }
            else if (mem_datas[0] == "Active(file):")
            {
                mem_info.active_file = std::stoll(mem_datas[1]);    // 活跃文件页
            }
            else if (mem_datas[0] == "Inactive(file):")
            {
                mem_info.inactive_file = std::stoll(mem_datas[1]);  // 非活跃文件页
            }
            else if (mem_datas[0] == "Dirty:")
            {
                mem_info.dirty = std::stoll(mem_datas[1]);          // 脏页
            }
            else if (mem_datas[0] == "Writeback:")
            {
                mem_info.writeback = std::stoll(mem_datas[1]);      // 正在写回的页
            }
            else if (mem_datas[0] == "AnonPages:")
            {
                mem_info.anon_pages = std::stoll(mem_datas[1]);     // 匿名页总数
            }
            else if (mem_datas[0] == "Mapped:")
            {
                mem_info.mapped = std::stoll(mem_datas[1]);         // 内存映射文件
            }
            else if (mem_datas[0] == "KReclaimable:")
            {
                mem_info.kReclaimable = std::stoll(mem_datas[1]);   // 内核可回收内存
            }
            else if (mem_datas[0] == "SReclaimable:")
            {
                mem_info.sReclaimable = std::stoll(mem_datas[1]);   // Slab可回收内存
            }
            else if (mem_datas[0] == "SUnreclaim:")
            {
                mem_info.sUnreclaim = std::stoll(mem_datas[1]);     // Slab不可回收内存
            }
            // 注意：还有其他字段如SwapTotal、SwapFree等未被解析
            // 清空临时容器，准备读取下一行
            mem_datas.clear();
        }

        // 获取Protobuf消息中的内存信息子消息
        auto mem_detail = monitor_info->mutable_mem_info();

        // ==================== 关键计算部分 ====================
        // 计算内存使用率百分比
        // 公式：使用率 = (总内存 - 可用内存) / 总内存 × 100%
        // 注意：这里使用MemAvailable而不是MemFree计算可用内存，
        // 因为MemAvailable考虑了缓存可以被回收的部分，更准确地反映实际可用内存
        mem_detail->set_used_percent((mem_info.total - mem_info.avail) * 1.0 /
            mem_info.total * 100.0);

        // 将所有内存数据从KB转换为GB并填充到Protobuf消息
        // 注意：除以KBToGB（1000×1000）进行单位转换

        // 基础内存信息
        mem_detail->set_total(mem_info.total / KBToGB);           // 总内存（GB）
        mem_detail->set_free(mem_info.free / KBToGB);             // 空闲内存（GB）
        mem_detail->set_avail(mem_info.avail / KBToGB);           // 可用内存（GB）
        mem_detail->set_buffers(mem_info.buffers / KBToGB);       // 缓冲区（GB）
        mem_detail->set_cached(mem_info.cached / KBToGB);         // 页面缓存（GB）

        // 交换区相关
        mem_detail->set_swap_cached(mem_info.swap_cached / KBToGB); // 交换区缓存（GB）

        // 活跃/非活跃内存分类
        mem_detail->set_active(mem_info.active / KBToGB);         // 活跃内存（GB）
        mem_detail->set_inactive(mem_info.in_active / KBToGB);    // 非活跃内存（GB）

        // 匿名页分类（应用程序分配的内存）
        mem_detail->set_active_anon(mem_info.active_anon / KBToGB);   // 活跃匿名页（GB）
        mem_detail->set_inactive_anon(mem_info.inactive_anon / KBToGB); // 非活跃匿名页（GB）

        // 文件页分类（文件缓存）
        mem_detail->set_active_file(mem_info.active_file / KBToGB);   // 活跃文件页（GB）
        mem_detail->set_inactive_file(mem_info.inactive_file / KBToGB); // 非活跃文件页（GB）

        // 磁盘I/O相关
        mem_detail->set_dirty(mem_info.dirty / KBToGB);           // 脏页（GB）
        mem_detail->set_writeback(mem_info.writeback / KBToGB);   // 正在写回的页（GB）

        // 特殊内存类型
        mem_detail->set_anon_pages(mem_info.anon_pages / KBToGB); // 匿名页总数（GB）
        mem_detail->set_mapped(mem_info.mapped / KBToGB);         // 内存映射文件（GB）

        // 内核内存管理
        mem_detail->set_kreclaimable(mem_info.kReclaimable / KBToGB); // 内核可回收内存（GB）
        mem_detail->set_sreclaimable(mem_info.sReclaimable / KBToGB); // Slab可回收内存（GB）
        mem_detail->set_sunreclaim(mem_info.sUnreclaim / KBToGB);     // Slab不可回收内存（GB）

        // 函数返回，内存监控数据已成功采集并填充到monitor_info中
        return;
    }
}  // namespace monitor
