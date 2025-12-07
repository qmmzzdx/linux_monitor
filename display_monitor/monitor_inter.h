// 头文件保护宏，防止重复包含
#pragma once

#include <QAbstractTableModel>  // Qt抽象表格模型基类
#include <QObject>              // Qt对象基类
#include <QColor>               // Qt颜色类
#include <QFont>                // Qt字体类

namespace monitor
{
    /**
     * @brief 监控基础模型类（抽象基类）
     *
     * 继承自QAbstractTableModel，为所有具体监控模型提供统一的样式和基础实现
     * 定义监控表格的默认外观：表头样式、单元格对齐方式、颜色等
     */
    class MonitorInterModel : public QAbstractTableModel
    {
    private:
        Q_OBJECT  // Qt元对象系统宏，启用信号槽和反射机制

    public:
        /**
         * @brief 构造函数
         * @param parent 父对象指针，用于Qt对象树管理
         */
        explicit MonitorInterModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

        /// @brief 虚析构函数，确保正确释放资源
        virtual ~MonitorInterModel() {}

        /**
         * @brief 获取表头数据（样式定制）
         * @param section 列索引
         * @param orientation 表头方向
         * @param role 数据角色
         * @return 表头样式信息
         *
         * 重写基类方法，为监控表格提供统一的表头样式：
         * 1. 字体：微软雅黑，10号，粗体
         * 2. 背景色：浅灰色
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override
        {
            // 设置表头字体样式
            if (role == Qt::FontRole)
            {
                // 使用微软雅黑字体，10号，粗体
                // QVariant::fromValue将QFont对象包装为QVariant
                return QVariant::fromValue(QFont("Microsoft YaHei", 10, QFont::Bold));
            }

            // 设置表头背景颜色
            if (role == Qt::BackgroundRole)
            {
                // 使用浅灰色作为表头背景
                return QVariant::fromValue(QColor(Qt::lightGray));
            }

            // 其他样式角色委托给基类处理
            return QAbstractTableModel::headerData(section, orientation, role);
        }

        /**
         * @brief 获取单元格数据（样式定制）
         * @param index 单元格索引
         * @param role 数据角色
         * @return 单元格样式信息
         *
         * 重写基类方法，为监控表格提供统一的单元格样式：
         * 1. 文本对齐：左对齐，垂直居中
         * 2. 文本颜色：黑色
         * 3. 背景颜色：白色
         *
         * 注意：此方法仅处理样式角色，具体数据由派生类实现
         */
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
        {
            // 设置文本对齐方式
            if (role == Qt::TextAlignmentRole)
            {
                // 左对齐 + 垂直居中
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            }

            // 设置文本颜色
            if (role == Qt::TextColorRole)
            {
                // 使用黑色文本
                return QVariant::fromValue(QColor(Qt::black));
            }

            // 设置单元格背景颜色
            if (role == Qt::BackgroundRole)
            {
                // 使用白色背景
                return QVariant::fromValue(QColor(Qt::white));
            }

            // 对于DisplayRole等数据角色，由派生类实现
            return QVariant();
        }
    };
}  // namespace monitor
