/**
 * @file main.cpp
 * @brief PoorWebServer主程序入口
 *
 * 此文件包含Web服务器的主函数，负责：
 * - 命令行参数解析
 * - 服务器配置和启动
 * - HTTP/HTTPS模式选择
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "Server.hpp"
#include <iostream>
#include <string>

/**
 * @brief 主函数 - 程序入口点
 *
 * 解析命令行参数并启动Web服务器
 *
 * 支持的参数：
 * - --port <port>: 指定监听端口（默认8080）
 * - --https: 启用HTTPS模式
 * - --cert <file>: HTTPS证书文件路径
 * - --key <file>: HTTPS私钥文件路径
 * - --help: 显示帮助信息
 *
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return int 程序退出码，0表示成功
 */
int main(int argc, char* argv[])
{
    // 默认配置
    unsigned short port = 8080;           // 默认监听端口
    bool use_https = false;               // 是否启用HTTPS
    std::string cert_file, key_file;      // SSL证书和密钥文件路径

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            // 设置监听端口
            port = std::stoi(argv[++i]);
        } else if (arg == "--https") {
            // 启用HTTPS模式
            use_https = true;
        } else if (arg == "--cert" && i + 1 < argc) {
            // 指定SSL证书文件
            cert_file = argv[++i];
        } else if (arg == "--key" && i + 1 < argc) {
            // 指定SSL私钥文件
            key_file = argv[++i];
        } else if (arg == "--help") {
            // 显示帮助信息
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --port <port>     Port to listen on (default: 8080)\n"
                      << "  --https           Enable HTTPS\n"
                      << "  --cert <file>     Certificate file for HTTPS\n"
                      << "  --key <file>      Private key file for HTTPS\n"
                      << "  --help            Show this help\n";
            return 0;
        }
    }

    // HTTPS模式验证：需要同时提供证书和密钥文件
    if (use_https && (cert_file.empty() || key_file.empty())) {
        std::cerr << "HTTPS requires --cert and --key options\n";
        return 1;
    }

    try
    {
        // 创建并启动服务器
        // Server构造函数会初始化网络监听和SSL上下文（如果启用HTTPS）
        Server server(port, use_https, cert_file, key_file);
        server.run();  // 进入主循环，处理客户端请求

        // 正常情况下不会到达这里，因为run()是阻塞的
    }
    catch (const std::exception &e)
    {
        // 捕获并显示服务器运行时的异常
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

//Auto mode failed: no available model found in known endpoints.