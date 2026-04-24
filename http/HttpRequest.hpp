/**
 * @file HttpRequest.hpp
 * @brief HTTP请求解析类声明
 *
 * 解析HTTP/1.1请求报文，支持GET/POST方法。
 * 提取请求行、headers和body信息。
 *
 * 解析流程：
 * 1. 解析请求行（方法、URL、版本）
 * 2. 解析请求头（键值对）
 * 3. 提取请求体（POST数据）
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief HTTP请求解析类
 *
 * 将原始HTTP请求数据解析为结构化信息。
 * 支持标准HTTP/1.1协议格式。
 */
class HttpRequest {
public:
    /**
     * @brief 默认构造函数
     */
    HttpRequest();

    /**
     * @brief 解析HTTP请求数据
     *
     * 从原始字符串解析完整的HTTP请求
     *
     * @param request_data 完整的HTTP请求报文
     * @return true 解析成功
     * @return false 解析失败（格式错误）
     */
    bool parse(const std::string& request_data);

    /**
     * @brief 获取请求方法
     *
     * @return const std::string& HTTP方法（GET/POST等）
     */
    const std::string& method() const { return method_; }

    /**
     * @brief 获取请求URL
     *
     * @return const std::string& 完整的请求URL
     */
    const std::string& url() const { return url_; }

    /**
     * @brief 获取HTTP版本
     *
     * @return const std::string& 版本字符串（如"HTTP/1.1"）
     */
    const std::string& version() const { return version_; }

    /**
     * @brief 获取请求路径
     *
     * 从URL中提取路径部分（去除查询参数）
     *
     * @return const std::string& 请求路径
     */
    const std::string& path() const;

    /**
     * @brief 获取查询字符串
     *
     * 从URL中提取查询参数部分
     *
     * @return const std::string& 查询字符串（不含'?'）
     */
    const std::string& query_string() const;

    /**
     * @brief 获取请求头
     *
     * @return const std::unordered_map<std::string, std::string>&
     *         所有请求头的键值对
     */
    const std::unordered_map<std::string, std::string>& headers() const { return headers_; }

    /**
     * @brief 获取请求体
     *
     * @return const std::string& 请求体内容（POST数据）
     */
    const std::string& body() const { return body_; }

    /**
     * @brief 检查是否为CGI请求
     *
     * 根据URL后缀判断是否需要CGI处理
     *
     * @return true 需要CGI处理
     * @return false 静态文件请求
     */
    bool is_cgi() const;

private:
    std::string method_;                                           ///< HTTP请求方法
    std::string url_;                                              ///< 完整请求URL
    std::string version_;                                          ///< HTTP版本
    std::unordered_map<std::string, std::string> headers_;        ///< 请求头集合
    std::string body_;                                             ///< 请求体内容

    /**
     * @brief 解析请求行
     *
     * 解析"GET /path HTTP/1.1"格式的请求行
     *
     * @param line 请求行字符串
     * @return true 解析成功
     * @return false 格式错误
     */
    bool parse_request_line(const std::string& line);

    /**
     * @brief 解析请求头
     *
     * 解析多行header，格式为"Key: Value"
     *
     * @param header_lines 所有header行
     * @return true 解析成功
     * @return false 格式错误
     */
    bool parse_headers(const std::vector<std::string>& header_lines);
};

#endif // HTTP_REQUEST_HPP