#pragma once

#include <chrono>

namespace monitor
{
    /**
     * @brief 工具函数集合类
     */
    class Utils
    {
    public:
        /**
         * @brief 计算两个时间点之间的时间差（单位：秒）
         * @param t1 结束时间点
         * @param t2 开始时间点
         * @return double 时间差（秒）
         */
        static double SteadyTimeSecond(
            const std::chrono::steady_clock::time_point& t1,
            const std::chrono::steady_clock::time_point& t2)
        {
            std::chrono::duration<double> time_second = t1 - t2;
            return time_second.count();
        }
    };
}  // namespace monitor
