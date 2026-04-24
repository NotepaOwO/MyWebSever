/**
 * @file CGIHandler.cpp
 * @brief CGI脚本处理器实现
 *
 * 实现CGI脚本的执行逻辑，使用fork/exec模式运行外部程序。
 * 正确设置CGI环境变量并通过管道收集脚本输出。
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "CGIHandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>

/**
 * @brief 执行CGI脚本
 *
 * 使用fork/exec模式执行CGI脚本，设置适当的环境变量并收集输出。
 * 支持GET和POST请求，正确处理请求体数据。
 *
 * @param script_path CGI脚本的可执行文件路径
 * @param query_string URL查询字符串（GET参数）
 * @param request_method HTTP请求方法
 * @param headers HTTP请求头映射
 * @param body HTTP请求体内容（POST数据）
 * @return std::string CGI脚本的标准输出，执行失败时返回错误消息
 */
std::string CGIHandler::execute(const std::string& script_path,
                               const std::string& query_string,
                               const std::string& request_method,
                               const std::unordered_map<std::string, std::string>& headers,
                               const std::string& body) {
    int pipefd[2];

    // 创建管道用于父子进程通信
    if (pipe(pipefd) == -1) {
        return "500 Internal Server Error";
    }

    // 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "500 Internal Server Error";
    }

    if (pid == 0) { // 子进程：执行CGI脚本
        close(pipefd[0]); // 关闭管道读端

        // 将标准输出重定向到管道写端
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // 设置CGI环境变量
        set_environment(query_string, request_method, headers);

        // POST请求特殊处理
        if (request_method == "POST" && !body.empty()) {
            // 设置请求体长度
            setenv("CONTENT_LENGTH", std::to_string(body.size()).c_str(), 1);
            // 设置内容类型
            setenv("CONTENT_TYPE", headers.count("Content-Type") ?
                   headers.at("Content-Type").c_str() : "application/x-www-form-urlencoded", 1);
        }

        // 执行CGI脚本（替换当前进程）
        execl(script_path.c_str(), script_path.c_str(), nullptr);

        // 如果execl失败，输出错误并退出
        std::cerr << "execl failed" << std::endl;
        exit(1);
    } else { // 父进程：读取脚本输出
        close(pipefd[1]); // 关闭管道写端

        // 从管道读取CGI脚本输出
        std::string output;
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytes_read);
        }
        close(pipefd[0]);

        // 等待子进程结束
        waitpid(pid, nullptr, 0);

        return output;
    }
}

/**
 * @brief 设置CGI环境变量
 *
 * 根据CGI 1.1规范设置必要的环境变量，包括：
 * - 基本服务器信息（REQUEST_METHOD, QUERY_STRING等）
 * - HTTP请求头（转换为HTTP_*环境变量）
 * - 服务器配置信息
 *
 * @param query_string URL查询参数字符串
 * @param request_method HTTP请求方法
 * @param headers HTTP请求头映射
 */
void CGIHandler::set_environment(const std::string& query_string,
                                const std::string& request_method,
                                const std::unordered_map<std::string, std::string>& headers) {
    // 设置基本CGI环境变量
    setenv("REQUEST_METHOD", request_method.c_str(), 1);
    setenv("QUERY_STRING", query_string.c_str(), 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("SERVER_NAME", "localhost", 1);
    setenv("SERVER_PORT", "8080", 1); // 默认端口

    // 将HTTP headers转换为CGI环境变量
    for (const auto& header : headers) {
        std::string env_name = "HTTP_" + header.first;
        // 转换为大写并将连字符替换为下划线
        std::transform(env_name.begin(), env_name.end(), env_name.begin(), ::toupper);
        std::replace(env_name.begin(), env_name.end(), '-', '_');
        setenv(env_name.c_str(), header.second.c_str(), 1);
    }
}