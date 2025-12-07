// 头文件保护宏，防止重复包含
#pragma once

// gRPC核心头文件
#include <grpc/grpc.h>                // gRPC C API
#include <grpcpp/create_channel.h>    // 创建gRPC通道
#include <grpcpp/grpcpp.h>            // gRPC C++ API

// 项目生成的Protobuf和gRPC代码
#include "monitor_info.grpc.pb.h"     // gRPC服务存根定义
#include "monitor_info.pb.h"          // Protobuf消息定义

// 标准库头文件
#include <iostream>                   // 错误输出
#include <string>                     // 字符串处理

namespace monitor
{
    /**
     * @brief RPC客户端类
     *
     * 封装了与监控服务器的gRPC通信功能
     * 提供设置和获取监控信息的接口
     */
    class RpcClient
    {
    public:
        /**
         * @brief 构造函数
         * @param server_address gRPC服务器地址，格式："ip:port"
         *
         * 示例："localhost:50051"
         */
        explicit RpcClient(const std::string& server_address = "localhost:50051")
        {
            // 创建gRPC通道（使用不安全连接，适合本地或测试环境）
            // InsecureChannelCredentials：不加密的连接凭证
            auto channel = grpc::CreateChannel(server_address,
                grpc::InsecureChannelCredentials());

            // 创建gRPC服务存根（Stub）
            // Stub是客户端调用远程服务的代理对象
            stub_ptr_ = monitor::proto::GrpcManager::NewStub(channel);
        }

        /**
         * @brief 析构函数
         */
        ~RpcClient() = default;  // 使用默认析构，unique_ptr会自动管理资源

        /**
         * @brief 设置监控信息（客户端→服务器）
         * @param monito_info 要设置的监控信息
         *
         * 将本地采集的监控数据发送到服务器
         * 单向调用，不需要返回值
         */
        void SetMonitorInfo(const monitor::proto::MonitorInfo& monito_info)
        {
            // 创建gRPC客户端上下文（包含调用元数据、超时设置等）
            ::grpc::ClientContext context;

            // 响应消息（使用Google的空消息类型）
            ::google::protobuf::Empty response;

            // 调用远程RPC方法
            ::grpc::Status status = stub_ptr_->SetMonitorInfo(&context, monito_info, &response);

            // 检查RPC调用状态
            if (!status.ok())
            {
                // 输出错误信息（生产环境应使用日志库而非cout）
                std::cout << "RPC SetMonitorInfo 调用失败:" << std::endl;
                std::cout << "  错误详情: " << status.error_details() << std::endl;
                std::cout << "  错误消息: " << status.error_message() << std::endl;
                std::cout << "  错误代码: " << status.error_code() << std::endl;
            }
            // 成功时不做处理
        }

        /**
         * @brief 获取监控信息（服务器→客户端）
         * @param monito_info 输出参数，用于接收服务器返回的监控信息
         * @return 调用是否成功（可以根据需求改为bool或Status）
         *
         * 从服务器获取最新的监控数据
         */
        void GetMonitorInfo(monitor::proto::MonitorInfo* monito_info)
        {
            // 参数检查
            if (monito_info == nullptr)
            {
                std::cerr << "错误: monito_info 参数为空指针" << std::endl;
                return;
            }

            // 创建gRPC客户端上下文
            ::grpc::ClientContext context;

            // 请求消息（空消息，因为不需要参数）
            ::google::protobuf::Empty request;

            // 调用远程RPC方法
            ::grpc::Status status = stub_ptr_->GetMonitorInfo(&context, request, monito_info);

            // 检查RPC调用状态
            if (!status.ok())
            {
                // 输出错误信息
                std::cout << "RPC GetMonitorInfo 调用失败:" << std::endl;
                std::cout << "  错误详情: " << status.error_details() << std::endl;
                std::cout << "  错误消息: " << status.error_message() << std::endl;
                std::cout << "  错误代码: " << status.error_code() << std::endl;

                // 清空输出参数或设置错误标志
                monito_info->Clear();
            }
        }

    private:
        /// @brief gRPC服务存根智能指针，自动管理资源
        std::unique_ptr<monitor::proto::GrpcManager::Stub> stub_ptr_;
    };
}  // namespace monitor
