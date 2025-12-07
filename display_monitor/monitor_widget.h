// 头文件保护宏，防止重复包含
#pragma once

#include <QStandardItemModel>    // Qt标准项模型（未直接使用，可移除）
#include <QtWidgets>             // Qt窗口部件模块（包含常用UI组件）
#include <thread>                // C++线程支持
#include <string>                // C++字符串支持

#include "cpu_load_model.h"      // CPU负载数据模型
#include "cpu_softirq_model.h"   // CPU软中断数据模型（未直接使用）
#include "cpu_stat_model.h"      // CPU状态数据模型
#include "mem_model.h"           // 内存数据模型
#include "net_model.h"           // 网络数据模型

// 包含Protobuf和gRPC生成的头文件
#include "monitor_info.grpc.pb.h"
#include "monitor_info.pb.h"

namespace monitor
{
    /**
     * @brief 监控主窗口类
     *
     * 继承自QWidget，是监控系统的用户界面主控制器
     * 负责整合所有监控模型和视图，提供页面切换和数据显示功能
     */
    class MonitorWidget : public QWidget
    {
    private:
        Q_OBJECT  // Qt元对象系统宏，启用信号槽机制

    public:
        /**
         * @brief 构造函数
         * @param parent 父窗口部件指针
         */
        explicit MonitorWidget(QWidget* parent = nullptr);

        /// @brief 析构函数，确保正确释放资源
        ~MonitorWidget() {}

        /**
         * @brief 创建并显示完整的监控界面
         * @param name 主机名或用户标识，用于按钮标签
         * @return 返回包含完整界面的QWidget指针
         */
        QWidget* ShowAllMonitorWidget(const std::string& name);

        /**
         * @brief 初始化CPU监控页面
         * @return 包含CPU负载和状态监控的窗口部件
         */
        QWidget* InitCpuMonitorWidget();

        /**
         * @brief 初始化软中断监控页面
         * @return 包含软中断监控的窗口部件
         */
        QWidget* InitSoftIrqMonitorWidget();

        /**
         * @brief 初始化内存监控页面
         * @return 包含内存监控的窗口部件
         */
        QWidget* InitMemMonitorWidget();

        /**
         * @brief 初始化网络监控页面
         * @return 包含网络监控的窗口部件
         */
        QWidget* InitNetMonitorWidget();

        /**
         * @brief 初始化按钮菜单
         * @param name 主机名或用户标识
         * @return 包含导航按钮的窗口部件
         */
        QWidget* InitButtonMenu(const std::string& name);

        /**
         * @brief 更新所有监控数据
         * @param monitor_info Protobuf格式的监控数据
         *
         * 将新的监控数据分发到各个模型，触发视图更新
         */
        void UpdateData(const monitor::proto::MonitorInfo& monitor_info);

    private slots:
        /// @brief CPU按钮点击槽函数，切换到CPU监控页面
        void ClickCpuButton();

        /// @brief 软中断按钮点击槽函数，切换到软中断监控页面
        void ClickSoftIrqButton();

        /// @brief 内存按钮点击槽函数，切换到内存监控页面
        void ClickMemButton();

        /// @brief 网络按钮点击槽函数，切换到网络监控页面
        void ClickNetButton();

    private:
        // 表格视图指针
        QTableView* monitor_view_ = nullptr;        ///< 软中断监控表格视图
        QTableView* cpu_load_monitor_view_ = nullptr; ///< CPU负载监控表格视图
        QTableView* cpu_stat_monitor_view_ = nullptr; ///< CPU状态监控表格视图
        QTableView* mem_monitor_view_ = nullptr;    ///< 内存监控表格视图
        QTableView* net_monitor_view_ = nullptr;    ///< 网络监控表格视图

        // 数据模型指针
        MonitorBaseModel* monitor_model_ = nullptr; ///< 软中断数据模型
        CpuLoadModel* cpu_load_model_ = nullptr;    ///< CPU负载数据模型
        CpuStatModel* cpu_stat_model_ = nullptr;    ///< CPU状态数据模型
        MemModel* mem_model_ = nullptr;             ///< 内存数据模型
        NetModel* net_model_ = nullptr;             ///< 网络数据模型

        /// @brief 堆叠布局，用于页面切换
        QStackedLayout* stack_menu_ = nullptr;
    };

    // ==================== 实现部分 ====================

    /**
     * @brief 构造函数实现
     * @param parent 父窗口部件指针
     */
    inline MonitorWidget::MonitorWidget(QWidget* parent) : QWidget(parent)
    {
        // 空实现，具体初始化在ShowAllMonitorWidget中完成
    }

    /**
     * @brief 创建并显示完整的监控界面
     * @param name 主机名或用户标识
     * @return 包含完整界面的QWidget指针
     */
    inline QWidget* MonitorWidget::ShowAllMonitorWidget(const std::string& name)
    {
        // 创建主窗口部件
        QWidget* widget = new QWidget();

        // 创建堆叠布局，用于切换不同监控页面
        stack_menu_ = new QStackedLayout();

        // 添加各个监控页面到堆叠布局
        stack_menu_->addWidget(InitCpuMonitorWidget());       // 索引0: CPU监控
        stack_menu_->addWidget(InitSoftIrqMonitorWidget());   // 索引1: 软中断监控
        stack_menu_->addWidget(InitMemMonitorWidget());       // 索引2: 内存监控
        stack_menu_->addWidget(InitNetMonitorWidget());       // 索引3: 网络监控

        // 创建网格布局作为主布局
        QGridLayout* layout = new QGridLayout(this);

        // 第一行：按钮菜单
        layout->addWidget(InitButtonMenu(name), 1, 0);

        // 第二行：监控内容区域
        layout->addLayout(stack_menu_, 2, 0);

        // 设置窗口布局并返回
        widget->setLayout(layout);
        return widget;
    }

    /**
     * @brief 初始化按钮菜单
     * @param name 主机名或用户标识
     * @return 包含导航按钮的窗口部件
     */
    inline QWidget* MonitorWidget::InitButtonMenu(const std::string& name)
    {
        // 创建四个导航按钮
        QPushButton* cpu_button = new QPushButton(
            tr("CPU (%1)").arg(QString::fromStdString(name)), this);
        QPushButton* soft_irq_button = new QPushButton(
            tr("SoftIRQ (%1)").arg(QString::fromStdString(name)), this);
        QPushButton* mem_button = new QPushButton(
            tr("Memory (%1)").arg(QString::fromStdString(name)), this);
        QPushButton* net_button = new QPushButton(
            tr("Network (%1)").arg(QString::fromStdString(name)), this);

        // 设置按钮字体
        QFont* font = new QFont("Microsoft YaHei", 15, 40);  // 微软雅黑，15号，正常粗细
        cpu_button->setFont(*font);
        soft_irq_button->setFont(*font);
        mem_button->setFont(*font);
        net_button->setFont(*font);

        // 创建水平布局放置按钮
        QHBoxLayout* layout = new QHBoxLayout();
        layout->addWidget(cpu_button);
        layout->addWidget(soft_irq_button);
        layout->addWidget(mem_button);
        layout->addWidget(net_button);

        // 创建容器部件并设置布局
        QWidget* widget = new QWidget();
        widget->setLayout(layout);

        // 连接按钮点击信号到槽函数
        connect(cpu_button, SIGNAL(clicked()), this, SLOT(ClickCpuButton()));
        connect(soft_irq_button, SIGNAL(clicked()), this, SLOT(ClickSoftIrqButton()));
        connect(mem_button, SIGNAL(clicked()), this, SLOT(ClickMemButton()));
        connect(net_button, SIGNAL(clicked()), this, SLOT(ClickNetButton()));

        return widget;
    }

    /**
     * @brief 初始化CPU监控页面
     * @return 包含CPU负载和状态监控的窗口部件
     */
    inline QWidget* MonitorWidget::InitCpuMonitorWidget()
    {
        QWidget* widget = new QWidget();

        // CPU负载监控标签
        QLabel* cpu_load_label = new QLabel(this);
        cpu_load_label->setText(tr("Monitor CpuLoad:"));
        cpu_load_label->setFont(QFont("Microsoft YaHei", 10, 40));

        // CPU负载监控表格
        cpu_load_monitor_view_ = new QTableView;
        cpu_load_model_ = new CpuLoadModel;
        cpu_load_monitor_view_->setModel(cpu_load_model_);
        cpu_load_monitor_view_->show();

        // CPU状态监控标签
        QLabel* cpu_stat_label = new QLabel(this);
        cpu_stat_label->setText(tr("Monitor CpuStat:"));
        cpu_stat_label->setFont(QFont("Microsoft YaHei", 10, 40));

        // CPU状态监控表格
        cpu_stat_monitor_view_ = new QTableView;
        cpu_stat_model_ = new CpuStatModel;
        cpu_stat_monitor_view_->setModel(cpu_stat_model_);
        cpu_stat_monitor_view_->show();

        // 创建网格布局
        QGridLayout* layout = new QGridLayout();

        // 布局安排（注意：行列索引有些跳跃，可能有特殊布局考虑）
        layout->addWidget(cpu_stat_label, 1, 0, 1, 1);      // 第1行第0列
        layout->addWidget(cpu_stat_monitor_view_, 2, 0, 1, 2); // 第2行，跨2列

        layout->addWidget(cpu_load_label, 3, 0);            // 第3行第0列
        layout->addWidget(cpu_load_monitor_view_, 4, 0, 2, 2); // 第4-5行，跨2列

        widget->setLayout(layout);
        return widget;
    }

    /**
     * @brief 初始化软中断监控页面
     * @return 包含软中断监控的窗口部件
     */
    inline QWidget* MonitorWidget::InitSoftIrqMonitorWidget()
    {
        QWidget* widget = new QWidget();

        // 软中断监控标签
        QLabel* monitor_label = new QLabel(this);
        monitor_label->setText(tr("Monitor softirq:"));
        monitor_label->setFont(QFont("Microsoft YaHei", 10, 40));

        // 软中断监控表格（带排序功能）
        monitor_view_ = new QTableView;
        monitor_model_ = new MonitorBaseModel;

        // 使用代理模型实现排序功能
        QSortFilterProxyModel* sort_proxy = new QSortFilterProxyModel(this);
        sort_proxy->setSourceModel(monitor_model_);
        monitor_view_->setModel(sort_proxy);
        monitor_view_->setSortingEnabled(true);  // 启用列排序
        monitor_view_->show();

        // 创建网格布局
        QGridLayout* layout = new QGridLayout();
        layout->addWidget(monitor_label, 1, 0);
        layout->addWidget(monitor_view_, 2, 0, 1, 2);

        widget->setLayout(layout);
        return widget;
    }

    /**
     * @brief 初始化内存监控页面
     * @return 包含内存监控的窗口部件
     */
    inline QWidget* MonitorWidget::InitMemMonitorWidget()
    {
        QWidget* widget = new QWidget();

        // 内存监控标签
        QLabel* mem_label = new QLabel(this);
        mem_label->setText(tr("Monitor mem:"));
        mem_label->setFont(QFont("Microsoft YaHei", 10, 40));

        // 内存监控表格
        mem_monitor_view_ = new QTableView;
        mem_model_ = new MemModel;
        mem_monitor_view_->setModel(mem_model_);
        mem_monitor_view_->show();

        // 创建网格布局
        QGridLayout* layout = new QGridLayout();
        layout->addWidget(mem_label, 1, 0);
        layout->addWidget(mem_monitor_view_, 2, 0, 1, 1);

        widget->setLayout(layout);
        return widget;
    }

    /**
     * @brief 初始化网络监控页面
     * @return 包含网络监控的窗口部件
     */
    inline QWidget* MonitorWidget::InitNetMonitorWidget()
    {
        QWidget* widget = new QWidget();

        // 网络监控标签
        QLabel* net_label = new QLabel(this);
        net_label->setText(tr("Monitor net:"));
        net_label->setFont(QFont("Microsoft YaHei", 10, 40));

        // 网络监控表格
        net_monitor_view_ = new QTableView;
        net_model_ = new NetModel;
        net_monitor_view_->setModel(net_model_);
        net_monitor_view_->show();

        // 创建网格布局
        QGridLayout* layout = new QGridLayout();
        layout->addWidget(net_label, 1, 0);
        layout->addWidget(net_monitor_view_, 2, 0, 1, 1);

        widget->setLayout(layout);
        return widget;
    }

    /**
     * @brief 更新所有监控数据
     * @param monitor_info Protobuf格式的监控数据
     */
    inline void MonitorWidget::UpdateData(
        const monitor::proto::MonitorInfo& monitor_info)
    {
        // 依次更新各个模型的数据
        monitor_model_->UpdateMonitorInfo(monitor_info);      // 软中断
        cpu_load_model_->UpdateMonitorInfo(monitor_info);     // CPU负载
        cpu_stat_model_->UpdateMonitorInfo(monitor_info);     // CPU状态
        mem_model_->UpdateMonitorInfo(monitor_info);          // 内存
        net_model_->UpdateMonitorInfo(monitor_info);          // 网络
    }

    /**
     * @brief CPU按钮点击槽函数
     * 切换到CPU监控页面（堆叠布局索引0）
     */
    inline void MonitorWidget::ClickCpuButton()
    {
        stack_menu_->setCurrentIndex(0);
    }

    /**
     * @brief 软中断按钮点击槽函数
     * 切换到软中断监控页面（堆叠布局索引1）
     */
    inline void MonitorWidget::ClickSoftIrqButton()
    {
        stack_menu_->setCurrentIndex(1);
    }

    /**
     * @brief 内存按钮点击槽函数
     * 切换到内存监控页面（堆叠布局索引2）
     */
    inline void MonitorWidget::ClickMemButton()
    {
        stack_menu_->setCurrentIndex(2);
    }

    /**
     * @brief 网络按钮点击槽函数
     * 切换到网络监控页面（堆叠布局索引3）
     */
    inline void MonitorWidget::ClickNetButton()
    {
        stack_menu_->setCurrentIndex(3);
    }
}  // namespace monitor
