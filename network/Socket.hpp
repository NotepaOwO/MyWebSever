/**
 * @file Socket.hpp
 * @brief RAII模式的TCP Socket封装类声明
 *
 * 提供面向对象的TCP socket接口，自动管理资源生命周期。
 * 支持TCP连接的建立、监听、接受和数据传输。
 *
 * 设计模式：
 * - RAII模式：构造函数获取资源，析构函数释放资源
 * - 移动语义：支持高效的资源转移
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <netinet/in.h>

/**
 * @brief RAII模式的TCP Socket封装类
 *
 * 封装POSIX socket API，提供类型安全的网络编程接口。
 * 自动管理socket文件描述符，避免资源泄漏。
 */
class Socket {
public:
    /**
     * @brief 默认构造函数
     *
     * 创建TCP socket，自动分配文件描述符
     *
     * @throws std::runtime_error socket创建失败
     */
    Socket();

    /**
     * @brief 从现有文件描述符构造
     *
     * 接管已有的socket文件描述符（通常用于accept的结果）
     *
     * @param fd 已打开的socket文件描述符
     */
    explicit Socket(int fd);

    /**
     * @brief 析构函数
     *
     * 自动关闭socket，释放系统资源
     */
    ~Socket();

    // 禁用拷贝构造和赋值（socket不可拷贝）
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    /**
     * @brief 移动构造函数
     *
     * 转移socket所有权，避免资源复制
     */
    Socket(Socket&& other) noexcept;

    /**
     * @brief 移动赋值运算符
     *
     * 转移socket所有权
     */
    Socket& operator=(Socket&& other) noexcept;

    /**
     * @brief 绑定socket到指定地址和端口
     *
     * @param address IP地址（支持IPv4）
     * @param port 端口号
     * @return bool 绑定是否成功
     */
    bool bind(const std::string& address, unsigned short port);

    /**
     * @brief 开始监听连接请求
     *
     * @param backlog 等待队列的最大长度
     * @return bool 监听设置是否成功
     */
    bool listen(int backlog = 10);

    /**
     * @brief 接受新的客户端连接
     *
     * 阻塞等待客户端连接，返回新的socket用于通信
     *
     * @return Socket 新连接的socket对象
     */
    Socket accept();

    /**
     * @brief 连接到远程服务器
     *
     * @param address 远程服务器IP地址
     * @param port 远程服务器端口
     * @return bool 连接是否成功
     */
    bool connect(const std::string& address, unsigned short port);

    /**
     * @brief 发送数据
     *
     * @param data 要发送的数据缓冲区
     * @param len 数据长度（字节）
     * @param flags 发送标志（通常为0）
     * @return ssize_t 实际发送的字节数，失败返回-1
     */
    ssize_t send(const void* data, size_t len, int flags = 0);

    /**
     * @brief 接收数据
     *
     * @param buffer 接收缓冲区
     * @param len 缓冲区大小
     * @param flags 接收标志（通常为0）
     * @return ssize_t 实际接收的字节数，失败返回-1
     */
    ssize_t recv(void* buffer, size_t len, int flags = 0);

    /**
     * @brief 获取socket文件描述符
     *
     * @return int socket文件描述符
     */
    int fd() const { return sock_fd_; }

    /**
     * @brief 手动关闭socket
     *
     * 提前释放socket资源（析构函数也会自动调用）
     */
    void close();

private:
    int sock_fd_;   ///< socket文件描述符
    bool valid_;    ///< socket是否有效
};

#endif // SOCKET_HPP