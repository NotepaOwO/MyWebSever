/**
 * @file HttpHandler.hpp
 * @brief HTTP请求处理类声明
 *
 * 处理HTTP请求的核心逻辑，包括请求解析、路由分发和响应生成。
 * 支持静态文件服务和CGI脚本执行。
 *
 * 处理流程：
 * 1. 读取客户端请求数据
 * 2. 解析HTTP请求
 * 3. 根据请求类型路由到相应处理函数
 * 4. 生成并发送HTTP响应
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "../network/Socket.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "../cgi/CGIHandler.hpp"
#include "../ssl/SSLWrapper.hpp"
#include <openssl/ssl.h>

/**
 * @brief HTTP请求处理器类
 *
 * 静态类，负责处理单个HTTP连接的完整生命周期。
 * 支持HTTP和HTTPS两种协议的请求处理。
 */
class HttpHandler
{
public:
    /**
     * @brief 处理HTTP客户端连接
     *
     * 处理单个HTTP客户端连接的完整流程：
     * 读取请求 -> 解析请求 -> 处理请求 -> 发送响应
     *
     * @param client_socket 已连接的客户端socket
     */
    static void handle_client(Socket client_socket);

    /**
     * @brief 处理HTTPS客户端连接
     *
     * 处理单个HTTPS客户端连接的完整流程：
     * 使用SSL读取请求 -> 解析请求 -> 处理请求 -> SSL发送响应
     *
     * @param client_socket 已连接的客户端socket
     * @param ssl SSL连接对象
     * @param ssl_wrapper SSL包装器引用
     */
    static void handle_https_client(Socket client_socket, SSL* ssl, SSLWrapper& ssl_wrapper);

private:
    /**
     * @brief 从socket读取HTTP请求
     *
     * 读取完整的HTTP请求报文，直到遇到空行
     *
     * @param socket 客户端socket引用
     * @return std::string 完整的HTTP请求字符串
     */
    static std::string read_request(Socket& socket);

    /**
     * @brief 从SSL连接读取HTTP请求
     *
     * 通过SSL连接读取完整的HTTP请求报文
     *
     * @param ssl SSL连接对象
     * @param ssl_wrapper SSL包装器引用
     * @return std::string 完整的HTTP请求字符串
     */
    static std::string read_request_ssl(SSL* ssl, SSLWrapper& ssl_wrapper);

    /**
     * @brief 通过socket发送HTTP响应
     *
     * 将HTTP响应发送到客户端socket
     *
     * @param socket 客户端socket引用
     * @param response HTTP响应对象
     */
    static void send_response(Socket& socket, const HttpResponse& response);

    /**
     * @brief 通过SSL连接发送HTTP响应
     *
     * 通过SSL连接将HTTP响应发送到客户端
     *
     * @param ssl SSL连接对象
     * @param ssl_wrapper SSL包装器引用
     * @param response HTTP响应对象
     */
    static void send_response_ssl(SSL* ssl, SSLWrapper& ssl_wrapper, const HttpResponse& response);

    /**
     * @brief 处理HTTP请求
     *
     * 根据请求类型（静态文件或CGI）路由到相应处理函数
     *
     * @param request 解析后的HTTP请求对象
     * @return HttpResponse 处理结果响应对象
     */
    static HttpResponse process_request(const HttpRequest& request);

    /**
     * @brief 服务静态文件
     *
     * 读取并返回请求的静态文件内容
     *
     * @param path 请求的文件路径
     * @return HttpResponse 包含文件内容的响应
     */
    static HttpResponse serve_file(const std::string& path);

    /**
     * @brief 执行CGI脚本
     *
     * 执行CGI脚本并返回执行结果
     *
     * @param request HTTP请求对象（包含CGI参数）
     * @return HttpResponse 包含CGI执行结果的响应
     */
    static HttpResponse execute_cgi(const HttpRequest& request);
};

#endif
