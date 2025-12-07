// 头文件保护宏，防止重复包含
#pragma once

#include <QAbstractTableModel>    // Qt抽象表格模型基类
#include <vector>                 // C++标准向量容器
#include "monitor_inter.h"        // 监控基础模型接口
#include "monitor_info.grpc.pb.h" // gRPC生成代码
#include "monitor_info.pb.h"      // Protobuf生成代码

namespace monitor
{
    /**
     * @brief CPU负载数据模型类
     *
     * 继承自MonitorInterModel，负责将CPU负载监控数据适配到Qt表格模型中
     * 显示1分钟、3分钟、15分钟三个时间维度的CPU平均负载
     */
    class CpuLoadModel : public MonitorInterModel
    {
    private:
        Q_OBJECT  // Qt元对象系统宏，启用信号槽和反射机制

    public:
        /**
         * @brief 构造函数
         * @param parent 父对象指针，用于Qt对象树管理
         */
        explicit CpuLoadModel(QObject* parent = nullptr) : MonitorInterModel(parent)
        {
            // 初始化表格列标题
            header_ << tr("load_1");   // 1分钟平均负载
            header_ << tr("load_3");   // 3分钟平均负载  
            header_ << tr("load_15");  // 15分钟平均负载
        }

        /// @brief 虚析构函数，确保正确释放资源
        virtual ~CpuLoadModel() {}

        /**
         * @brief 获取模型行数
         * @param parent 父模型索引，表格模型中通常忽略
         * @return 数据行数（CPU负载数据通常为1行）
         */
        int rowCount(const QModelIndex& parent = QModelIndex()) const override
        {
            return monitor_data_.size();
        }

        /**
         * @brief 获取模型列数
         * @param parent 父模型索引
         * @return 固定列数（由COLUMN_MAX枚举定义）
         */
        int columnCount(const QModelIndex& parent = QModelIndex()) const override
        {
            return COLUMN_MAX;
        }

        /**
         * @brief 获取表头数据
         * @param section 列索引
         * @param orientation 表头方向
         * @param role 数据角色
         * @return 表头显示内容
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override
        {
            // 只处理水平表头的显示角色
            if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
            {
                return header_[section];
            }

            // 其他情况委托给父类处理（如字体、颜色等样式）
            return MonitorInterModel::headerData(section, orientation, role);
        }

        /**
         * @brief 获取单元格数据
         * @param index 单元格索引
         * @param role 数据角色
         * @return 单元格显示数据
         */
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
        {
            // 边界检查：确保列索引有效
            if (index.column() < 0 || index.column() >= COLUMN_MAX)
            {
                return QVariant();  // 返回空值表示无效索引
            }

            // 仅处理显示角色
            if (role == Qt::DisplayRole)
            {
                // 双重边界检查确保数据访问安全
                if (index.row() < monitor_data_.size() && index.column() < COLUMN_MAX)
                {
                    return monitor_data_[index.row()][index.column()];
                }
            }
            return QVariant();
        }

        /**
         * @brief 更新监控信息（核心方法）
         * @param monitor_info Protobuf格式的监控数据
         *
         * 该方法将Protobuf数据转换为模型内部格式，并通知视图更新
         * 使用beginResetModel/endResetModel实现完整数据刷新
         */
        void UpdateMonitorInfo(const monitor::proto::MonitorInfo& monitor_info)
        {
            // 通知视图开始模型重置
            beginResetModel();

            // 清空现有数据
            monitor_data_.clear();

            // 转换并添加新的CPU负载数据
            monitor_data_.push_back(insert_one_cpu_load(monitor_info.cpu_load()));

            // 通知视图重置完成
            endResetModel();
        }

    signals:
        /**
         * @brief 数据变化信号
         * @param topLeft 变化区域左上角索引
         * @param bottomRight 变化区域右下角索引
         * @param roles 变化的角色列表
         *
         * 当模型数据发生变化时发射，通知连接的视图更新显示
         */
        void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
            const QVector<int>& roles);

    private:
        /**
         * @brief 转换单个CPU负载数据
         * @param cpu_load Protobuf的CpuLoad消息
         * @return 包含三个负载值的QVariant向量
         *
         * 辅助方法，将Protobuf消息转换为模型内部数据结构
         */
        std::vector<QVariant> insert_one_cpu_load(const monitor::proto::CpuLoad& cpu_load)
        {
            std::vector<QVariant> cpu_load_list;

            // 遍历所有列，按枚举顺序处理数据
            for (int i = CPU_AVG_1; i < COLUMN_MAX; i++)
            {
                switch (i)
                {
                    case CPU_AVG_1:
                        cpu_load_list.push_back(QVariant(cpu_load.load_avg_1()));
                        break;
                    case CPU_AVG_3:
                        cpu_load_list.push_back(QVariant(cpu_load.load_avg_3()));
                        break;
                    case CPU_AVG_15:
                        cpu_load_list.push_back(QVariant(cpu_load.load_avg_15()));
                        break;
                    default:
                        break;  // 枚举完整性保证不会执行到此处
                }
            }
            return cpu_load_list;
        }

        /// @brief 数据存储容器：二维向量，外层为行，内层为列
        std::vector<std::vector<QVariant>> monitor_data_;

        /// @brief 表头字符串列表，支持国际化
        QStringList header_;

        /**
         * @brief CPU负载列枚举
         *
         * 使用枚举代替硬编码数字，提高代码可读性和可维护性
         * COLUMN_MAX作为哨兵值表示总列数
         */
        enum CpuLoad
        {
            CPU_AVG_1 = 0,     ///< 1分钟平均负载列
            CPU_AVG_3,         ///< 3分钟平均负载列
            CPU_AVG_15,        ///< 15分钟平均负载列
            COLUMN_MAX         ///< 列总数，用于循环终止条件
        };
    };
}  // namespace monitor
