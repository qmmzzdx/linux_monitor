// 包含对应的头文件
#include "utils/read_file.h"

namespace monitor
{
    /**
     * @brief 读取并解析文件的一行的具体实现
     * @param args 输出参数，存储分割后的字段
     * @return bool 成功读取一行返回true，文件结束或读取失败返回false
     *
     * 详细执行流程：
     * 1. 从文件流中读取一行文本
     * 2. 检查是否到达文件末尾或读取到空行
     * 3. 使用字符串流分割行内容
     * 4. 将分割后的单词存入输出向量
     * 5. 返回读取状态
     *
     * 技术要点：
     * - 使用std::getline读取整行，保留空白字符
     * - 使用std::istringstream进行分割，自动处理连续空白
     * - 使用>>操作符提取单词，自动跳过空白字符
     *
     * 示例：对于行 "MemTotal: 16335784 kB"
     * 分割结果为：["MemTotal:", "16335784", "kB"]
     */
    bool ReadFile::ReadLine(std::vector<std::string>* args)
    {
        // 从文件流中读取一行
        std::string line;
        std::getline(ifs_, line);

        // 检查读取状态
        // 条件1：ifs_.eof() - 到达文件末尾
        // 条件2：line.empty() - 读取到空行（可能是空行或读取失败）
        // 注意：这里将空行视为读取结束，但某些文件可能包含空行作为有效分隔
        if (ifs_.eof() || line.empty())
        {
            return false;  // 读取结束或失败
        }

        // 创建字符串流用于分割
        std::istringstream line_ss(line);

        // 循环提取单词
        // 注意：使用!line_ss.eof()作为条件，但更好的做法是检查提取操作本身
        while (!line_ss.eof())
        {
            std::string word;
            line_ss >> word;  // 提取一个单词（自动跳过空白字符）

            // 将单词添加到输出向量
            // 注意：即使word可能为空（如果行尾有空白），也会被添加
            args->push_back(word);
        }

        // 返回成功状态
        return true;
    }
}  // namespace monitor
