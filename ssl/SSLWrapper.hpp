/**
 * @file SSLWrapper.hpp
 * @brief SSL/TLS封装类声明
 *
 * 提供OpenSSL库的RAII包装器，简化SSL/TLS连接的创建和管理。
 * 支持服务器端SSL握手、加密读写和安全关闭。
 *
 * SSL初始化流程：
 * 1. 创建SSL上下文（SSL_CTX）
 * 2. 加载证书和私钥
 * 3. 为每个连接创建SSL对象
 * 4. 执行SSL握手
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef SSL_WRAPPER_HPP
#define SSL_WRAPPER_HPP

#include "../network/Socket.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>

/**
 * @brief SSL/TLS连接管理封装类
 *
 * 使用RAII模式管理OpenSSL资源，提供安全的SSL/TLS通信接口。
 * 负责SSL上下文的创建、证书加载和SSL连接的生命周期管理。
 */
class SSLWrapper {
public:
    /**
     * @brief 构造函数
     *
     * 初始化SSL库，但不创建上下文
     */
    SSLWrapper();

    /**
     * @brief 析构函数
     *
     * 清理SSL上下文和相关资源
     */
    ~SSLWrapper();

    /**
     * @brief 初始化SSL上下文
     *
     * 创建SSL上下文并加载服务器证书和私钥。
     * 必须在使用其他方法前调用此方法。
     *
     * @param cert_file PEM格式的证书文件路径
     * @param key_file PEM格式的私钥文件路径
     * @return true 初始化成功
     * @return false 初始化失败（证书或私钥错误）
     */
    bool init(const std::string& cert_file, const std::string& key_file);

    /**
     * @brief 接受SSL连接
     *
     * 在已建立的TCP连接上执行SSL握手，返回SSL对象用于后续通信
     *
     * @param socket 已建立连接的客户端socket
     * @return SSL* SSL连接对象，失败时返回nullptr
     */
    SSL* accept(Socket& socket);

    /**
     * @brief SSL加密读取
     *
     * 从SSL连接读取数据，自动处理解密
     *
     * @param ssl SSL连接对象
     * @param buffer 读取缓冲区
     * @param len 缓冲区长度
     * @return int 实际读取的字节数，失败时返回负值
     */
    int read(SSL* ssl, void* buffer, int len);

    /**
     * @brief SSL加密写入
     *
     * 向SSL连接写入数据，自动处理加密
     *
     * @param ssl SSL连接对象
     * @param buffer 写入缓冲区
     * @param len 写入长度
     * @return int 实际写入的字节数，失败时返回负值
     */
    int write(SSL* ssl, const void* buffer, int len);

    /**
     * @brief 关闭SSL连接
     *
     * 执行SSL关闭握手并释放SSL对象
     *
     * @param ssl 要关闭的SSL连接对象
     */
    void shutdown(SSL* ssl);

private:
    SSL_CTX* ctx_;  ///< SSL上下文对象，管理SSL配置和证书
};

#endif // SSL_WRAPPER_HPP