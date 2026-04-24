/**
 * @file Server.hpp
 * @brief Web服务器核心类声明
 *
 * Server类是Web服务器的核心，负责：
 * - TCP网络监听和连接接受
 * - HTTP/HTTPS协议分发
 * - 线程池管理
 * - SSL上下文管理
 *
 * 设计模式：
 * - 代理模式：封装底层网络和SSL操作
 * - 组合模式：组合Socket、ThreadPool、SSLWrapper等组件
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "../network/Socket.hpp"
#include "../concurrent/ThreadPool.hpp"
#include "../ssl/SSLWrapper.hpp"
#include <memory>

/**
 * @brief Web服务器主类
 *
 * 支持HTTP和HTTPS双协议，提供高并发网络服务。
 * 使用线程池避免"每连接一线程"的性能问题。
 */
class Server
{
public:
    /**
     * @brief 构造函数 - 初始化服务器
     *
     * @param port 监听端口
     * @param use_https 是否启用HTTPS模式
     * @param cert_file SSL证书文件路径（HTTPS模式时必需）
     * @param key_file SSL私钥文件路径（HTTPS模式时必需）
     *
     * @throws std::runtime_error 初始化失败时抛出异常
     */
    Server(unsigned short port, bool use_https = false,
           const std::string& cert_file = "", const std::string& key_file = "");

    /**
     * @brief 启动服务器主循环
     *
     * 进入阻塞监听状态，接受客户端连接并分发处理。
     * 每个连接会被提交到线程池中异步处理。
     */
    void run();

private:
    Socket server_socket_;                    ///< 服务器监听socket（RAII管理）
    ThreadPool thread_pool_;                  ///< 工作线程池（4个工作线程）
    std::unique_ptr<SSLWrapper> ssl_wrapper_; ///< SSL上下文管理器（HTTPS模式时使用）
    bool use_https_;                          ///< 是否启用HTTPS模式

    /**
     * @brief 处理HTTP客户端连接
     *
     * @param client_socket 已建立的客户端连接socket
     */
    void handle_client(Socket client_socket);

    /**
     * @brief 处理HTTPS客户端连接
     *
     * 建立SSL连接后，调用HTTP处理逻辑
     *
     * @param client_socket 已建立的客户端TCP连接socket
     */
    void handle_https_client(Socket client_socket);
};

#endif
