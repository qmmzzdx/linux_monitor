// 头文件保护宏，防止重复包含
#pragma once

// C++标准库头文件
#include <fstream>      // 文件流操作，用于读取文件
#include <iostream>     // 输入输出流（虽然这里可能未使用，但保留以支持调试输出）
#include <sstream>      // 字符串流，用于分割字符串
#include <string>       // 字符串操作
#include <vector>       // 动态数组容器

namespace monitor
{
    /**
     * @brief 文件读取工具类
     *
     * 封装了文件读取的常用操作，提供便捷的文件行读取和解析功能。
     * 专门为读取Linux系统文件（如/proc、/sys下的文件）设计，
     * 这些文件通常具有特殊的格式要求（如按空格分隔的字段）。
     *
     * 设计特点：
     * - RAII模式：构造函数打开文件，析构函数自动关闭文件
     * - 流式接口：支持逐行读取和解析
     * - 线程安全：每个实例独立，可在多线程环境中使用
     * - 异常安全：妥善处理文件操作异常
     */
    class ReadFile
    {
    public:
        /**
         * @brief 构造函数，打开指定文件
         * @param name 要读取的文件路径
         *
         * 使用explicit关键字防止隐式类型转换，确保构造意图明确。
         * 使用std::ifstream打开文件，如果文件打开失败，ifstream会处于失败状态。
         *
         * 注意：构造函数不会抛出异常，但后续的读取操作可能会失败。
         */
        explicit ReadFile(const std::string& name) : ifs_(name) {}

        /**
         * @brief 析构函数，自动关闭文件
         *
         * 遵循RAII（资源获取即初始化）原则，
         * 确保文件在对象生命周期结束时自动关闭，
         * 避免资源泄漏。
         */
        ~ReadFile() { ifs_.close(); }

        /**
         * @brief 读取并解析文件的一行
         * @param args 输出参数，存储分割后的字段
         * @return bool 成功读取一行返回true，文件结束或读取失败返回false
         *
         * 功能描述：
         * 1. 从文件中读取一行文本
         * 2. 按空白字符（空格、制表符等）分割行内容
         * 3. 将分割后的字段存入输出向量中
         * 4. 自动跳过空行
         *
         * 典型应用场景：
         * - 读取/proc/meminfo：每行格式为"字段名: 值"
         * - 读取/proc/net/dev：每行包含多个空格分隔的数值
         * - 读取配置文件：key = value格式
         *
         * 注意：不会自动清理args向量，调用者需要确保向量为空或已清理。
         */
        bool ReadLine(std::vector<std::string>* args);

        /**
         * @brief 静态方法：读取文件的前若干行
         * @param stat_file 要读取的文件路径
         * @param line_count 要读取的行数
         * @return std::vector<std::string> 包含指定行数的字符串向量
         *
         * 功能描述：
         * 1. 打开指定文件
         * 2. 读取前line_count行（或直到文件结束）
         * 3. 返回包含这些行的字符串向量
         *
         * 设计特点：
         * - 静态方法：无需创建类实例即可使用
         * - 返回新向量：调用者负责管理内存
         * - 简单直接：适用于只需读取少量行的情况
         *
         * 使用场景：
         * - 读取CPU核心数量：/proc/cpuinfo的前几行
         * - 读取系统版本信息：/proc/version
         * - 读取特定配置文件的前几行
         *
         * 注意：如果文件行数不足line_count，会提前停止读取。
         */
        static std::vector<std::string> GetStatsLines(const std::string& stat_file,
            const int line_count)
        {
            std::vector<std::string> stats_lines;  // 存储结果
            std::ifstream buffer(stat_file);       // 打开文件

            // 读取指定行数或直到文件结束
            for (int line_num = 0; line_num < line_count; ++line_num)
            {
                std::string line;
                std::getline(buffer, line);        // 读取一行

                // 遇到空行或文件结束则停止读取
                if (line.empty())
                {
                    break;
                }
                stats_lines.push_back(line);       // 保存行内容
            }
            return stats_lines;
        }

    private:
        std::ifstream ifs_;  ///< 输入文件流，用于读取文件内容
    };
}  // namespace monitor
