/**
 * @file HttpResponse.hpp
 * @brief HTTP响应生成类声明
 *
 * 生成HTTP/1.1响应报文，支持状态码、headers和body。
 * 提供静态文件和动态内容的响应构建。
 *
 * 响应格式：
 * HTTP/1.1 200 OK
 * Content-Type: text/html
 * Content-Length: 123
 *
 * <html>...</html>
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <unordered_map>

/**
 * @brief HTTP响应生成类
 *
 * 构建完整的HTTP响应报文，包括状态行、响应头和响应体。
 * 支持常见的HTTP状态码和MIME类型。
 */
class HttpResponse {
public:
    /**
     * @brief 默认构造函数
     */
    HttpResponse();

    /**
     * @brief 设置HTTP状态码和消息
     *
     * @param code 状态码（如200, 404, 500）
     * @param message 自定义状态消息（可选）
     */
    void set_status(int code, const std::string& message = "");

    /**
     * @brief 设置响应头
     *
     * @param key header名称
     * @param value header值
     */
    void set_header(const std::string& key, const std::string& value);

    /**
     * @brief 设置响应体内容
     *
     * @param body 响应体字符串
     */
    void set_body(const std::string& body);

    /**
     * @brief 生成完整的HTTP响应字符串
     *
     * 将所有组件组装成标准HTTP响应格式
     *
     * @return std::string 完整的HTTP响应报文
     */
    std::string to_string() const;

    /**
     * @brief 创建200 OK响应
     *
     * @param content_type 内容类型（默认text/html）
     * @return HttpResponse 配置好的响应对象
     */
    static HttpResponse create_200(const std::string& content_type = "text/html");

    /**
     * @brief 创建400 Bad Request响应
     *
     * @return HttpResponse 配置好的错误响应
     */
    static HttpResponse create_400();

    /**
     * @brief 创建404 Not Found响应
     *
     * @return HttpResponse 配置好的错误响应
     */
    static HttpResponse create_404();

    /**
     * @brief 创建405 Method Not Allowed响应
     *
     * @return HttpResponse 配置好的错误响应
     */
    static HttpResponse create_405();

    /**
     * @brief 创建500 Internal Server Error响应
     *
     * @return HttpResponse 配置好的错误响应
     */
    static HttpResponse create_500();

private:
    int status_code_;                                           ///< HTTP状态码
    std::string status_message_;                               ///< 状态消息
    std::unordered_map<std::string, std::string> headers_;     ///< 响应头集合
    std::string body_;                                          ///< 响应体内容

    /**
     * @brief 获取状态码对应的默认消息
     *
     * @param code 状态码
     * @return std::string 状态消息字符串
     */
    std::string get_status_message(int code) const;
};

#endif // HTTP_RESPONSE_HPP