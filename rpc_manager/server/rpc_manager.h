// 头文件保护宏，防止重复包含
#pragma once

// gRPC相关头文件
#include <grpcpp/support/status.h>
#include <grpcpp/server_context.h>

// C++标准库头文件
#include <unordered_map>
#include <iostream>

// Protobuf和gRPC生成的头文件
#include "monitor_info.grpc.pb.h"
#include "monitor_info.pb.h"

namespace monitor
{
    /**
     * @brief gRPC服务实现类
     *
     * 继承自Protobuf生成的GrpcManager::Service基类
     * 实现监控数据的接收和提供功能
     */
    class GrpcManagerImpl : public monitor::proto::GrpcManager::Service
    {
    public:
        /**
         * @brief 构造函数
         */
        GrpcManagerImpl() {}

        /**
         * @brief 析构函数
         */
        virtual ~GrpcManagerImpl() {}

        /**
         * @brief 设置监控信息RPC方法
         * @param context gRPC服务器上下文
         * @param request 客户端发送的监控信息
         * @param response 空响应
         * @return gRPC状态码
         *
         * 客户端调用此方法将监控数据发送到服务器
         */
        ::grpc::Status SetMonitorInfo(
            ::grpc::ServerContext* context,
            const ::monitor::proto::MonitorInfo* request,
            ::google::protobuf::Empty* response) override
        {

            // 清空旧数据并存储新数据
            monitor_infos_.Clear();
            monitor_infos_ = *request;

            // 调试输出
            std::cout << "[SetMonitorInfo] request->soft_irq_size(): " << request->soft_irq_size() << std::endl;

            return grpc::Status::OK;
        }

        /**
         * @brief 获取监控信息RPC方法
         * @param context gRPC服务器上下文
         * @param request 空请求
         * @param response 服务器返回的监控信息
         * @return gRPC状态码
         *
         * 客户端调用此方法从服务器获取监控数据
         */
        ::grpc::Status GetMonitorInfo(
            ::grpc::ServerContext* context,
            const ::google::protobuf::Empty* request,
            ::monitor::proto::MonitorInfo* response) override
        {
            // 返回存储的监控数据
            *response = monitor_infos_;

            return grpc::Status::OK;
        }

    private:
        /// @brief 存储最新的监控信息
        monitor::proto::MonitorInfo monitor_infos_;
    };
}  // namespace monitor
