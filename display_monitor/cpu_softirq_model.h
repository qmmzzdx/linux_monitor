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
     * @brief CPU软中断监控数据模型类
     *
     * 继承自MonitorInterModel，负责将CPU软中断监控数据适配到Qt表格模型中
     * 显示系统中各CPU核心的软中断统计信息，包括HI、TIMER、NET_TX等10种中断类型
     */
    class MonitorBaseModel : public MonitorInterModel
    {
    private:
        Q_OBJECT  // Qt元对象系统宏，启用信号槽和反射机制

    public:
        /**
         * @brief 构造函数
         * @param parent 父对象指针，用于Qt对象树管理
         */
        explicit MonitorBaseModel(QObject* parent = nullptr) : MonitorInterModel(parent)
        {
            // 初始化表格列标题（11列）
            header_ << tr("cpu");      // CPU核心标识
            header_ << tr("hi");       // 高优先级任务中断
            header_ << tr("timer");    // 定时器中断
            header_ << tr("net_tx");   // 网络发送中断
            header_ << tr("net_rx");   // 网络接收中断
            header_ << tr("block");    // 块设备中断
            header_ << tr("irq_poll"); // IRQ轮询中断
            header_ << tr("tasklet");  // 小任务中断
            header_ << tr("sched");    // 调度器中断
            header_ << tr("hrtimer");  // 高精度定时器中断
            header_ << tr("rcu");      // RCU（读-拷贝-更新）中断
        }

        /// @brief 虚析构函数，确保正确释放资源
        virtual ~MonitorBaseModel() {}

        /**
         * @brief 获取模型行数
         * @param parent 父模型索引，表格模型中通常忽略
         * @return 数据行数（每个CPU核心对应一行）
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
         * @param monito_info Protobuf格式的监控数据
         *
         * 该方法将Protobuf软中断数据转换为模型内部格式，并通知视图更新
         * 每个CPU核心的软中断数据对应表格中的一行
         */
        void UpdateMonitorInfo(const monitor::proto::MonitorInfo& monito_info)
        {
            // 通知视图开始模型重置
            beginResetModel();

            // 清空现有数据
            monitor_data_.clear();

            // 遍历所有CPU核心的软中断数据
            for (int i = 0; i < monito_info.soft_irq_size(); i++)
            {
                // 转换并添加每个CPU核心的软中断数据
                monitor_data_.push_back(insert_one_soft_irq(monito_info.soft_irq(i)));
            }

            // 通知视图重置完成
            endResetModel();

            return;
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
         * @brief 转换单个CPU核心的软中断数据
         * @param soft_irq Protobuf的SoftIrq消息
         * @return 包含11个字段的QVariant向量
         *
         * 辅助方法，将Protobuf消息转换为模型内部数据结构
         * 处理CPU名称和10种软中断类型的数值
         */
        std::vector<QVariant> insert_one_soft_irq(const monitor::proto::SoftIrq& soft_irq)
        {
            std::vector<QVariant> soft_irq_list;

            // 遍历所有列，按枚举顺序处理数据
            for (int i = SoftIrqInfo::CPU_NAME; i < SoftIrqInfo::COLUMN_MAX; i++)
            {
                switch (i)
                {
                    case SoftIrqInfo::CPU_NAME:
                        // CPU名称需要转换为QString
                        soft_irq_list.push_back(
                            QVariant(QString::fromStdString(soft_irq.cpu())));
                        break;
                    case SoftIrqInfo::HI:
                        soft_irq_list.push_back(QVariant(soft_irq.hi()));
                        break;
                    case SoftIrqInfo::TIMER:
                        soft_irq_list.push_back(QVariant(soft_irq.timer()));
                        break;
                    case SoftIrqInfo::NET_TX:
                        soft_irq_list.push_back(QVariant(soft_irq.net_tx()));
                        break;
                    case SoftIrqInfo::NET_RX:
                        soft_irq_list.push_back(QVariant(soft_irq.net_rx()));
                        break;
                    case SoftIrqInfo::BLOCK:
                        soft_irq_list.push_back(QVariant(soft_irq.block()));
                        break;
                    case SoftIrqInfo::IRQ_POLL:
                        soft_irq_list.push_back(QVariant(soft_irq.irq_poll()));
                        break;
                    case SoftIrqInfo::TASKLET:
                        soft_irq_list.push_back(QVariant(soft_irq.tasklet()));
                        break;
                    case SoftIrqInfo::SCHED:
                        soft_irq_list.push_back(QVariant(soft_irq.sched()));
                        break;
                    case SoftIrqInfo::HRTIMER:
                        soft_irq_list.push_back(QVariant(soft_irq.hrtimer()));
                        break;
                    case SoftIrqInfo::RCU:
                        soft_irq_list.push_back(QVariant(soft_irq.rcu()));
                        break;
                    default:
                        break;  // 枚举完整性保证不会执行到此处
                }
            }
            return soft_irq_list;
        }

        /// @brief 数据存储容器：二维向量，外层为行（每个CPU核心），内层为列（11个字段）
        std::vector<std::vector<QVariant>> monitor_data_;

        /// @brief 表头字符串列表，支持国际化，包含11个列标题
        QStringList header_;

        /**
         * @brief 软中断信息列枚举
         *
         * 使用枚举代替硬编码数字，提高代码可读性和可维护性
         * COLUMN_MAX作为哨兵值表示总列数
         */
        enum SoftIrqInfo
        {
            CPU_NAME = 0,     ///< CPU核心标识列
            HI,               ///< 高优先级任务中断列
            TIMER,            ///< 定时器中断列
            NET_TX,           ///< 网络发送中断列
            NET_RX,           ///< 网络接收中断列
            BLOCK,            ///< 块设备中断列
            IRQ_POLL,         ///< IRQ轮询中断列
            TASKLET,          ///< 小任务中断列
            SCHED,            ///< 调度器中断列
            HRTIMER,          ///< 高精度定时器中断列
            RCU,              ///< RCU（读-拷贝-更新）中断列
            COLUMN_MAX        ///< 列总数，用于循环终止条件
        };
    };
}  // namespace monitor
