#include <iostream>
#include <memory>
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server.h>
#include "rpc_manager.h"

// 服务器监听地址常量
constexpr char kServerPortInfo[] = "0.0.0.0:50051";

/**
 * @brief 初始化并启动gRPC服务器
 *
 * 创建gRPC服务器实例，配置监听端口，注册服务，并启动服务器
 */
void InitServer()
{
    // 创建gRPC服务器构建器
    grpc::ServerBuilder builder;

    // 添加监听端口
    // 0.0.0.0:50051 表示监听所有网络接口的50051端口
    // InsecureServerCredentials() 使用不安全的连接（适合本地测试）
    builder.AddListeningPort(kServerPortInfo, grpc::InsecureServerCredentials());

    // 创建RPC服务实现实例
    monitor::GrpcManagerImpl grpc_server;

    // 向构建器注册服务
    builder.RegisterService(&grpc_server);

    // 构建并启动服务器
    // BuildAndStart() 创建服务器并开始监听
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    // 输出服务器启动信息
    std::cout << "gRPC服务器已启动，监听端口: " << kServerPortInfo << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;

    // 等待服务器停止（阻塞调用）
    server->Wait();

    return;
}

/**
 * @brief 服务器程序主函数
 * @return 程序退出码
 *
 * 程序入口点，启动gRPC监控服务器
 */
int main()
{
    // 初始化并启动服务器
    InitServer();

    return 0;
}
