/**
 * @file HttpResponse.cpp
 * @brief HTTP响应生成类实现
 *
 * 实现HTTP响应的构建逻辑，包括状态行、headers和body的组装。
 * 提供常用HTTP状态码的便捷创建方法。
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "HttpResponse.hpp"
#include <sstream>

/**
 * @brief HttpResponse默认构造函数
 *
 * 初始化为200 OK状态，设置默认的服务器和连接headers
 */
HttpResponse::HttpResponse() : status_code_(200), status_message_("OK") {
    set_header("Server", "ModernCPP/1.0");
    set_header("Connection", "close");
}

/**
 * @brief 设置HTTP状态码和消息
 *
 * @param code 状态码（如200, 404, 500）
 * @param message 自定义状态消息，如果为空则使用默认消息
 */
void HttpResponse::set_status(int code, const std::string& message) {
    status_code_ = code;
    status_message_ = message.empty() ? get_status_message(code) : message;
}

/**
 * @brief 设置响应头
 *
 * 添加或更新HTTP响应头
 *
 * @param key header名称
 * @param value header值
 */
void HttpResponse::set_header(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

/**
 * @brief 设置响应体内容
 *
 * 设置响应体并自动更新Content-Length header
 *
 * @param body 响应体字符串
 */
void HttpResponse::set_body(const std::string& body) {
    body_ = body;
    set_header("Content-Length", std::to_string(body_.size()));
}

/**
 * @brief 生成完整的HTTP响应字符串
 *
 * 将状态行、headers和body组装成标准HTTP/1.1响应格式
 *
 * @return std::string 完整的HTTP响应报文
 */
std::string HttpResponse::to_string() const {
    std::ostringstream oss;

    // 状态行：HTTP/1.1 200 OK
    oss << "HTTP/1.1 " << status_code_ << " " << status_message_ << "\r\n";

    // Headers：Key: Value
    for (const auto& header : headers_) {
        oss << header.first << ": " << header.second << "\r\n";
    }

    // 空行分隔headers和body
    oss << "\r\n";

    // Body内容
    oss << body_;

    return oss.str();
}

/**
 * @brief 创建200 OK响应
 *
 * 用于成功请求的响应模板
 *
 * @param content_type 内容类型（默认text/html）
 * @return HttpResponse 配置好的成功响应
 */
HttpResponse HttpResponse::create_200(const std::string& content_type) {
    HttpResponse resp;
    resp.set_status(200);
    resp.set_header("Content-Type", content_type);
    return resp;
}

/**
 * @brief 创建400 Bad Request响应
 *
 * 用于请求格式错误的响应
 *
 * @return HttpResponse 配置好的错误响应
 */
HttpResponse HttpResponse::create_400() {
    HttpResponse resp;
    resp.set_status(400, "Bad Request");
    resp.set_header("Content-Type", "text/html");
    resp.set_body("<h1>400 Bad Request</h1>");
    return resp;
}

/**
 * @brief 创建404 Not Found响应
 *
 * 用于请求资源不存在的响应
 *
 * @return HttpResponse 配置好的错误响应
 */
HttpResponse HttpResponse::create_404() {
    HttpResponse resp;
    resp.set_status(404, "Not Found");
    resp.set_header("Content-Type", "text/html");
    resp.set_body("<h1>404 Not Found</h1>");
    return resp;
}

/**
 * @brief 创建405 Method Not Allowed响应
 *
 * 用于不支持的HTTP方法的响应
 *
 * @return HttpResponse 配置好的错误响应
 */
HttpResponse HttpResponse::create_405() {
    HttpResponse resp;
    resp.set_status(405, "Method Not Allowed");
    resp.set_header("Content-Type", "text/html");
    resp.set_body("<h1>405 Method Not Allowed</h1>");
    return resp;
}

/**
 * @brief 创建500 Internal Server Error响应
 *
 * 用于服务器内部错误的响应
 *
 * @return HttpResponse 配置好的错误响应
 */
HttpResponse HttpResponse::create_500() {
    HttpResponse resp;
    resp.set_status(500, "Internal Server Error");
    resp.set_header("Content-Type", "text/html");
    resp.set_body("<h1>500 Internal Server Error</h1>");
    return resp;
}

/**
 * @brief 获取状态码对应的默认消息
 *
 * 将数字状态码转换为标准HTTP状态消息
 *
 * @param code 状态码
 * @return std::string 状态消息字符串
 */
std::string HttpResponse::get_status_message(int code) const {
    switch (code) {
        case 200: return "OK";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}