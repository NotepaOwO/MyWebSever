/**
 * @file Server.cpp
 * @brief Web服务器核心类实现
 *
 * 实现服务器的网络监听、连接管理和请求分发功能。
 * 支持多线程并发处理，提供高性能网络服务。
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "Server.hpp"
#include "../http/HttpHandler.hpp"
#include <iostream>
#include <stdexcept>

/**
 * @brief Server构造函数
 *
 * 初始化服务器组件：
 * 1. 创建监听socket并绑定到指定端口
 * 2. 初始化线程池（4个工作线程）
 * 3. 如果启用HTTPS，初始化SSL上下文
 *
 * @param port 监听端口号
 * @param use_https 是否启用HTTPS安全传输
 * @param cert_file SSL证书文件路径
 * @param key_file SSL私钥文件路径
 *
 * @throws std::runtime_error socket绑定失败或SSL初始化失败
 */
Server::Server(unsigned short port, bool use_https,
               const std::string& cert_file, const std::string& key_file)
    : server_socket_(), thread_pool_(4), ssl_wrapper_(nullptr), use_https_(use_https)
{
    // 绑定服务器socket到指定端口并开始监听
    // 使用"0.0.0.0"表示监听所有网络接口
    if (!server_socket_.bind("0.0.0.0", port)) {
        throw std::runtime_error("Bind failed");
    }
    if (!server_socket_.listen()) {
        throw std::runtime_error("Listen failed");
    }

    // 显示服务器启动信息
    std::cout << "Server running on port " << port
              << (use_https ? " (HTTPS)" : " (HTTP)") << std::endl;

    // 如果启用HTTPS，初始化SSL上下文
    if (use_https_) {
        ssl_wrapper_ = std::make_unique<SSLWrapper>();
        if (!ssl_wrapper_->init(cert_file, key_file)) {
            throw std::runtime_error("SSL initialization failed");
        }
    }
}

/**
 * @brief 服务器主循环
 *
 * 进入无限循环，持续接受客户端连接：
 * 1. 阻塞等待客户端连接
 * 2. 为每个连接创建处理任务
 * 3. 将任务提交到线程池异步执行
 *
 * 使用线程池避免创建过多线程，提高并发性能。
 */
void Server::run()
{
    while (true)
    {
        // 接受新的客户端连接（阻塞操作）
        Socket client_socket = server_socket_.accept();
        if (client_socket.fd() == -1) {
            std::cerr << "Accept failed" << std::endl;
            continue;  // 继续等待下一个连接
        }

        // 根据协议类型选择处理函数
        // 使用lambda表达式捕获this指针和socket对象
        if (use_https_) {
            // HTTPS模式：处理SSL握手和加密通信
            thread_pool_.submit([this, client_socket = std::move(client_socket)]() mutable {
                handle_https_client(std::move(client_socket));
            });
        } else {
            // HTTP模式：直接处理明文通信
            thread_pool_.submit([this, client_socket = std::move(client_socket)]() mutable {
                handle_client(std::move(client_socket));
            });
        }
    }
}

/**
 * @brief 处理HTTP客户端连接
 *
 * 将连接socket传递给HTTP处理器进行业务逻辑处理
 *
 * @param client_socket 已建立的客户端TCP连接
 */
void Server::handle_client(Socket client_socket)
{
    HttpHandler::handle_client(std::move(client_socket));
}

/**
 * @brief 处理HTTPS客户端连接
 *
 * 1. 建立SSL连接（TLS握手）
 * 2. 调用HTTP处理器处理应用层协议
 * 3. 关闭SSL连接
 *
 * @param client_socket 已建立的客户端TCP连接
 */
void Server::handle_https_client(Socket client_socket)
{
    if (!ssl_wrapper_) return;

    // 建立SSL连接（包括证书验证和密钥交换）
    SSL* ssl = ssl_wrapper_->accept(client_socket);
    if (!ssl) {
        client_socket.close();
        return;
    }

    // 处理HTTP协议（在加密通道上）
    HttpHandler::handle_https_client(std::move(client_socket), ssl, *ssl_wrapper_);
}
