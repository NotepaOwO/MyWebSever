/**
 * @file Socket.cpp
 * @brief RAII模式的TCP Socket封装类实现
 *
 * 实现跨平台的TCP socket操作，封装POSIX socket API。
 * 提供异常安全的资源管理。
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "Socket.hpp"
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>

/**
 * @brief 默认构造函数
 *
 * 创建IPv4 TCP socket，设置SO_REUSEADDR选项
 */
Socket::Socket() : sock_fd_(-1), valid_(false) {
    // 创建TCP socket (IPv4, TCP, 默认协议)
    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    valid_ = true;
}

/**
 * @brief 从现有文件描述符构造
 *
 * 接管已有的socket，通常用于accept()的结果
 */
Socket::Socket(int fd) : sock_fd_(fd), valid_(true) {}

/**
 * @brief 析构函数
 *
 * 自动清理socket资源，确保无资源泄漏
 */
Socket::~Socket() {
    if (valid_ && sock_fd_ != -1) {
        ::close(sock_fd_);  // 调用系统close函数
    }
}

/**
 * @brief 移动构造函数
 *
 * 高效转移socket所有权，避免资源复制
 */
Socket::Socket(Socket&& other) noexcept : sock_fd_(other.sock_fd_), valid_(other.valid_) {
    // 将原对象置为无效状态
    other.sock_fd_ = -1;
    other.valid_ = false;
}

/**
 * @brief 移动赋值运算符
 *
 * 转移socket所有权，先清理自身资源
 */
Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        // 先清理自身资源
        if (valid_ && sock_fd_ != -1) {
            ::close(sock_fd_);
        }
        // 转移资源所有权
        sock_fd_ = other.sock_fd_;
        valid_ = other.valid_;
        // 将原对象置为无效
        other.sock_fd_ = -1;
        other.valid_ = false;
    }
    return *this;
}

/**
 * @brief 绑定socket到指定地址和端口
 *
 * @param address IPv4地址字符串（如"127.0.0.1"或"0.0.0.0"）
 * @param port 端口号（1-65535）
 * @return true 绑定成功
 * @return false 绑定失败
 */
bool Socket::bind(const std::string& address, unsigned short port) {
    if (!valid_) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;              // IPv4
    addr.sin_port = htons(port);            // 网络字节序转换

    // 将字符串地址转换为网络字节序
    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
        return false;  // 地址格式错误
    }

    // 设置地址重用选项，避免TIME_WAIT状态的端口占用
    int opt = 1;
    setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定socket到地址和端口
    if (::bind(sock_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }
    return true;
}

/**
 * @brief 开始监听连接请求
 *
 * 将socket设置为被动模式，准备接受客户端连接
 *
 * @param backlog 等待队列的最大长度
 * @return true 监听设置成功
 * @return false 设置失败
 */
bool Socket::listen(int backlog) {
    if (!valid_) return false;
    return ::listen(sock_fd_, backlog) >= 0;
}

/**
 * @brief 接受新的客户端连接
 *
 * 阻塞等待客户端连接请求，返回新的socket用于通信
 *
 * @return Socket 新的客户端连接socket，失败时返回无效socket
 */
Socket Socket::accept() {
    if (!valid_) return Socket(-1);

    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    // 接受连接请求
    int client_fd = ::accept(sock_fd_, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        return Socket(-1);  // 接受失败
    }

    // 记录客户端连接信息
    std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr)
              << ":" << ntohs(client_addr.sin_port) << std::endl;

    return Socket(client_fd);
}

/**
 * @brief 连接到远程服务器
 *
 * 主动建立到远程服务器的TCP连接
 *
 * @param address 远程服务器IP地址
 * @param port 远程服务器端口
 * @return true 连接成功
 * @return false 连接失败
 */
bool Socket::connect(const std::string& address, unsigned short port) {
    if (!valid_) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
        return false;
    }

    return ::connect(sock_fd_, (sockaddr*)&addr, sizeof(addr)) >= 0;
}

/**
 * @brief 发送数据
 *
 * 通过socket发送数据到对端
 *
 * @param data 数据缓冲区指针
 * @param len 数据长度（字节）
 * @param flags 发送控制标志（通常为0）
 * @return ssize_t 实际发送的字节数，失败返回-1
 */
ssize_t Socket::send(const void* data, size_t len, int flags) {
    if (!valid_) return -1;
    return ::send(sock_fd_, data, len, flags);
}

/**
 * @brief 接收数据
 *
 * 从socket接收对端发送的数据
 *
 * @param buffer 接收缓冲区
 * @param len 缓冲区最大容量
 * @param flags 接收控制标志（通常为0）
 * @return ssize_t 实际接收的字节数，失败返回-1
 */
ssize_t Socket::recv(void* buffer, size_t len, int flags) {
    if (!valid_) return -1;
    return ::recv(sock_fd_, buffer, len, flags);
}

/**
 * @brief 手动关闭socket
 *
 * 提前释放socket资源，之后该对象变为无效状态
 */
void Socket::close() {
    if (valid_ && sock_fd_ != -1) {
        ::close(sock_fd_);
        sock_fd_ = -1;
        valid_ = false;
    }
}