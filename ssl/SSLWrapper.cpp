/**
 * @file SSLWrapper.cpp
 * @brief SSL/TLS封装类实现
 *
 * 实现OpenSSL的RAII包装器，提供安全的SSL/TLS服务器端通信。
 * 处理证书加载、SSL握手和加密数据传输。
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "SSLWrapper.hpp"
#include <iostream>

/**
 * @brief SSLWrapper构造函数
 *
 * 初始化OpenSSL库，加载必要的算法和错误字符串。
 * SSL上下文在init()方法中创建。
 */
SSLWrapper::SSLWrapper() : ctx_(nullptr) {
    // 初始化OpenSSL库
    SSL_library_init();
    // 加载所有加密算法
    OpenSSL_add_all_algorithms();
    // 加载错误字符串用于调试
    SSL_load_error_strings();
}

/**
 * @brief SSLWrapper析构函数
 *
 * 清理SSL上下文，释放所有相关资源
 */
SSLWrapper::~SSLWrapper() {
    if (ctx_) {
        SSL_CTX_free(ctx_);
    }
}

/**
 * @brief 初始化SSL上下文
 *
 * 创建TLS服务器上下文并加载证书和私钥文件。
 * 这是使用SSL功能前的必要步骤。
 *
 * @param cert_file PEM格式的服务器证书文件路径
 * @param key_file PEM格式的私钥文件路径
 * @return true 初始化成功
 * @return false 初始化失败（文件不存在、格式错误或密钥不匹配）
 */
bool SSLWrapper::init(const std::string& cert_file, const std::string& key_file) {
    // 创建TLS服务器方法上下文
    ctx_ = SSL_CTX_new(TLS_server_method());
    if (!ctx_) {
        std::cerr << "SSL_CTX_new failed" << std::endl;
        return false;
    }

    // 加载服务器证书
    if (SSL_CTX_use_certificate_file(ctx_, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "SSL_CTX_use_certificate_file failed" << std::endl;
        return false;
    }

    // 加载私钥
    if (SSL_CTX_use_PrivateKey_file(ctx_, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "SSL_CTX_use_PrivateKey_file failed" << std::endl;
        return false;
    }

    // 验证私钥与证书匹配
    if (!SSL_CTX_check_private_key(ctx_)) {
        std::cerr << "SSL_CTX_check_private_key failed" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief 接受SSL连接
 *
 * 在已建立的TCP连接上执行SSL握手，建立加密通信通道。
 * 返回的SSL对象用于后续的加密读写操作。
 *
 * @param socket 已建立连接的客户端socket
 * @return SSL* SSL连接对象，握手失败时返回nullptr
 */
SSL* SSLWrapper::accept(Socket& socket) {
    // 创建新的SSL对象
    SSL* ssl = SSL_new(ctx_);
    if (!ssl) return nullptr;

    // 将SSL对象与socket文件描述符关联
    SSL_set_fd(ssl, socket.fd());

    // 执行SSL握手
    if (SSL_accept(ssl) <= 0) {
        // 握手失败，打印错误信息
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        return nullptr;
    }

    return ssl;
}

/**
 * @brief SSL加密读取
 *
 * 从SSL连接读取数据，OpenSSL自动处理解密
 *
 * @param ssl SSL连接对象
 * @param buffer 读取缓冲区
 * @param len 缓冲区最大长度
 * @return int 实际读取的字节数，错误时返回负值
 */
int SSLWrapper::read(SSL* ssl, void* buffer, int len) {
    return SSL_read(ssl, buffer, len);
}

/**
 * @brief SSL加密写入
 *
 * 向SSL连接写入数据，OpenSSL自动处理加密
 *
 * @param ssl SSL连接对象
 * @param buffer 写入缓冲区
 * @param len 写入数据长度
 * @return int 实际写入的字节数，错误时返回负值
 */
int SSLWrapper::write(SSL* ssl, const void* buffer, int len) {
    return SSL_write(ssl, buffer, len);
}

/**
 * @brief 关闭SSL连接
 *
 * 执行SSL关闭握手，安全地关闭加密连接并释放资源
 *
 * @param ssl 要关闭的SSL连接对象
 */
void SSLWrapper::shutdown(SSL* ssl) {
    if (ssl) {
        // 执行SSL关闭握手
        SSL_shutdown(ssl);
        // 释放SSL对象
        SSL_free(ssl);
    }
}