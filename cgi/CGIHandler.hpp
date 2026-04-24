/**
 * @file CGIHandler.hpp
 * @brief CGI脚本处理器声明
 *
 * 实现Common Gateway Interface (CGI) 1.1规范，支持动态内容生成。
 * 通过fork/exec执行外部CGI脚本，设置适当的环境变量并处理输出。
 *
 * CGI执行流程：
 * 1. 设置CGI环境变量（QUERY_STRING, REQUEST_METHOD等）
 * 2. 创建管道用于父子进程通信
 * 3. fork子进程执行CGI脚本
 * 4. 父进程读取脚本输出
 * 5. 返回脚本生成的动态内容
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <unordered_map>

/**
 * @brief CGI脚本处理器类
 *
 * 静态类，负责执行CGI脚本并返回动态生成的HTML内容。
 * 遵循CGI 1.1规范，正确设置环境变量和处理脚本I/O。
 */
class CGIHandler {
public:
    /**
     * @brief 执行CGI脚本
     *
     * 执行指定的CGI脚本，传递HTTP请求信息并返回脚本输出。
     * 脚本必须是可执行文件且遵循CGI规范。
     *
     * @param script_path CGI脚本的绝对或相对路径
     * @param query_string URL查询字符串（GET参数）
     * @param request_method HTTP请求方法（GET/POST）
     * @param headers HTTP请求头映射
     * @param body HTTP请求体（POST数据）
     * @return std::string CGI脚本的标准输出内容
     */
    static std::string execute(const std::string& script_path,
                              const std::string& query_string,
                              const std::string& request_method,
                              const std::unordered_map<std::string, std::string>& headers,
                              const std::string& body = "");

private:
    /**
     * @brief 设置CGI环境变量
     *
     * 根据CGI 1.1规范设置必要的环境变量：
     * - QUERY_STRING: URL查询参数
     * - REQUEST_METHOD: HTTP方法
     * - HTTP_* headers转换为环境变量
     * - CONTENT_LENGTH: 请求体长度
     *
     * @param query_string URL查询字符串
     * @param request_method HTTP请求方法
     * @param headers HTTP请求头映射
     */
    static void set_environment(const std::string& query_string,
                               const std::string& request_method,
                               const std::unordered_map<std::string, std::string>& headers);
};

#endif // CGI_HANDLER_HPP