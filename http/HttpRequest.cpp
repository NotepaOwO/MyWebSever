/**
 * @file HttpRequest.cpp
 * @brief HTTP请求解析类实现
 *
 * 实现HTTP请求的完整解析逻辑，包括请求行、headers和body。
 * 支持标准HTTP/1.1协议格式验证。
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "HttpRequest.hpp"
#include <sstream>
#include <algorithm>

/**
 * @brief HttpRequest默认构造函数
 *
 * 初始化所有成员变量为空状态
 */
HttpRequest::HttpRequest() = default;

/**
 * @brief 解析完整的HTTP请求数据
 *
 * 解析流程：
 * 1. 使用字符串流分割请求行和headers
 * 2. 解析请求行（方法、URL、版本）
 * 3. 解析所有请求头
 * 4. 提取请求体内容
 *
 * @param request_data 原始HTTP请求报文
 * @return true 解析成功且格式正确
 * @return false 解析失败或格式错误
 */
bool HttpRequest::parse(const std::string& request_data) {
    std::istringstream iss(request_data);
    std::string line;
    std::vector<std::string> header_lines;

    // 步骤1：读取并解析请求行
    if (!std::getline(iss, line)) return false;
    if (line.back() == '\r') line.pop_back();  // 去除\r字符
    if (!parse_request_line(line)) return false;

    // 步骤2：读取所有header行，直到空行
    while (std::getline(iss, line)) {
        if (line.back() == '\r') line.pop_back();
        if (line.empty()) break;  // 空行表示headers结束
        header_lines.push_back(line);
    }
    if (!parse_headers(header_lines)) return false;

    // 步骤3：读取请求体（剩余内容）
    std::string body;
    std::string remaining;
    while (std::getline(iss, line)) {
        if (line.back() == '\r') line.pop_back();
        body += line + "\n";
    }
    if (!body.empty() && body.back() == '\n') body.pop_back();
    body_ = body;

    return true;
}

/**
 * @brief 解析HTTP请求行
 *
 * 请求行格式："METHOD URL HTTP/VERSION"
 * 示例："GET /index.html HTTP/1.1"
 *
 * @param line 请求行字符串
 * @return true 解析成功
 * @return false 格式错误或字段不完整
 */
bool HttpRequest::parse_request_line(const std::string& line) {
    std::istringstream iss(line);
    if (!(iss >> method_ >> url_ >> version_)) return false;
    return true;
}

/**
 * @brief 解析HTTP请求头
 *
 * 每个header行格式："Key: Value"
 * 自动去除首尾空格，转换为统一的键值对格式
 *
 * @param header_lines 所有header行数组
 * @return true 所有header解析成功
 * @return false 存在格式错误的header
 */
bool HttpRequest::parse_headers(const std::vector<std::string>& header_lines) {
    for (const auto& line : header_lines) {
        // 查找冒号分隔符
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) return false;

        // 分割key和value
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);

        // 去除前后的空格
        key.erase(key.begin(), std::find_if(key.begin(), key.end(),
              [](int ch) { return !std::isspace(ch); }));
        key.erase(std::find_if(key.rbegin(), key.rend(),
              [](int ch) { return !std::isspace(ch); }).base(), key.end());

        value.erase(value.begin(), std::find_if(value.begin(), value.end(),
              [](int ch) { return !std::isspace(ch); }));
        value.erase(std::find_if(value.rbegin(), value.rend(),
              [](int ch) { return !std::isspace(ch); }).base(), value.end());

        headers_[key] = value;
    }
    return true;
}

/**
 * @brief 获取请求路径部分
 *
 * 从完整URL中提取路径，格式：/path/to/resource
 * 去除查询参数部分
 *
 * @return const std::string& 请求路径
 */
const std::string& HttpRequest::path() const {
    size_t query_pos = url_.find('?');
    static std::string path;
    path = (query_pos != std::string::npos) ? url_.substr(0, query_pos) : url_;
    return path;
}

/**
 * @brief 获取查询字符串部分
 *
 * 从完整URL中提取查询参数，格式：key1=value1&key2=value2
 * 不包含前导的'?'
 *
 * @return const std::string& 查询字符串
 */
const std::string& HttpRequest::query_string() const {
    size_t query_pos = url_.find('?');
    static std::string query;
    query = (query_pos != std::string::npos) ? url_.substr(query_pos + 1) : "";
    return query;
}

/**
 * @brief 检查是否为CGI请求
 *
 * 根据URL是否以".cgi"结尾判断是否需要CGI处理
 * CGI请求将通过外部程序处理，而不是返回静态文件
 *
 * @return true 需要CGI处理
 * @return false 静态文件请求
 */
bool HttpRequest::is_cgi() const {
    // 检查URL是否以.cgi结尾
    return url_.find(".cgi") != std::string::npos;
}