#include "HttpHandler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

void HttpHandler::handle_client(Socket client_socket)
{
    std::string request_data = read_request(client_socket);
    if (request_data.empty()) {
        client_socket.close();
        return;
    }

    HttpRequest request;
    if (!request.parse(request_data)) {
        HttpResponse response = HttpResponse::create_400();
        send_response(client_socket, response);
        client_socket.close();
        return;
    }

    HttpResponse response = process_request(request);
    send_response(client_socket, response);
    client_socket.close();
}

void HttpHandler::handle_https_client(Socket client_socket, SSL* ssl, SSLWrapper& ssl_wrapper)
{
    std::string request_data = read_request_ssl(ssl, ssl_wrapper);
    if (request_data.empty()) {
        ssl_wrapper.shutdown(ssl);
        client_socket.close();
        return;
    }

    HttpRequest request;
    if (!request.parse(request_data)) {
        HttpResponse response = HttpResponse::create_400();
        send_response_ssl(ssl, ssl_wrapper, response);
        ssl_wrapper.shutdown(ssl);
        client_socket.close();
        return;
    }

    HttpResponse response = process_request(request);
    send_response_ssl(ssl, ssl_wrapper, response);
    ssl_wrapper.shutdown(ssl);
    client_socket.close();
}

std::string HttpHandler::read_request(Socket& socket)
{
    std::string request;
    char buffer[1024];
    ssize_t bytes_read;

    // 读取请求行
    bytes_read = socket.recv(buffer, sizeof(buffer));
    if (bytes_read <= 0) return "";

    request.append(buffer, bytes_read);

    // 简单检查是否是HTTP请求
    if (request.find("HTTP/") == std::string::npos) return "";

    // 如果第一次读取就包含完整的请求头，直接返回
    if (request.find("\r\n\r\n") != std::string::npos) {
        return request;
    }

    // 继续读取直到空行
    while ((bytes_read = socket.recv(buffer, sizeof(buffer))) > 0) {
        request.append(buffer, bytes_read);
        if (request.find("\r\n\r\n") != std::string::npos) break;
    }

    return request;
}

std::string HttpHandler::read_request_ssl(SSL* ssl, SSLWrapper& ssl_wrapper)
{
    std::string request;
    char buffer[1024];
    int bytes_read;

    // 读取请求行
    bytes_read = ssl_wrapper.read(ssl, buffer, sizeof(buffer));
    if (bytes_read <= 0) return "";

    request.append(buffer, bytes_read);

    // 简单检查是否是HTTP请求
    if (request.find("HTTP/") == std::string::npos) return "";

    // 继续读取直到空行
    while ((bytes_read = ssl_wrapper.read(ssl, buffer, sizeof(buffer))) > 0) {
        request.append(buffer, bytes_read);
        if (request.find("\r\n\r\n") != std::string::npos) break;
    }

    return request;
}

void HttpHandler::send_response(Socket& socket, const HttpResponse& response)
{
    std::string response_str = response.to_string();
    socket.send(response_str.c_str(), response_str.size());
}

void HttpHandler::send_response_ssl(SSL* ssl, SSLWrapper& ssl_wrapper, const HttpResponse& response)
{
    std::string response_str = response.to_string();
    ssl_wrapper.write(ssl, response_str.c_str(), response_str.size());
}

HttpResponse HttpHandler::process_request(const HttpRequest& request)
{
    std::string method = request.method();
    std::string path = request.path();

    // 只支持GET和POST
    if (method != "GET" && method != "POST") {
        return HttpResponse::create_405();
    }

    // 构建文件路径
    std::string file_path = "httpdocs" + path;
    if (file_path.back() == '/') {
        file_path += "index.html";
    }

    // 检查是否是CGI请求
    if (request.is_cgi()) {
        return execute_cgi(request);
    }

    // 检查文件是否存在
    struct stat st;
    if (stat(file_path.c_str(), &st) == -1) {
        return HttpResponse::create_404();
    }

    // 如果是目录，重定向到index.html
    if (S_ISDIR(st.st_mode)) {
        file_path += "/index.html";
        if (stat(file_path.c_str(), &st) == -1) {
            return HttpResponse::create_404();
        }
    }

    return serve_file(file_path);
}

HttpResponse HttpHandler::serve_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return HttpResponse::create_404();
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    HttpResponse response = HttpResponse::create_200();
    response.set_body(content);

    // 设置Content-Type
    if (path.find(".html") != std::string::npos) {
        response.set_header("Content-Type", "text/html");
    } else if (path.find(".css") != std::string::npos) {
        response.set_header("Content-Type", "text/css");
    } else if (path.find(".js") != std::string::npos) {
        response.set_header("Content-Type", "application/javascript");
    } else if (path.find(".png") != std::string::npos) {
        response.set_header("Content-Type", "image/png");
    } else {
        response.set_header("Content-Type", "application/octet-stream");
    }

    return response;
}

/**
 * @brief 执行CGI脚本
 *
 * 执行指定的CGI脚本，传递请求参数和数据。
 * CGI脚本必须是可执行文件且位于httpdocs目录下。
 *
 * @param request HTTP请求对象（包含查询参数和POST数据）
 * @return HttpResponse 包含CGI执行结果的HTTP响应
 */
HttpResponse HttpHandler::execute_cgi(const HttpRequest& request)
{
    std::string script_path = "httpdocs" + request.path();

    // 检查脚本是否存在且可执行
    if (access(script_path.c_str(), X_OK) == -1) {
        return HttpResponse::create_404();
    }

    // 执行CGI脚本并获取输出
    std::string output = CGIHandler::execute(script_path,
                                           request.query_string(),
                                           request.method(),
                                           request.headers(),
                                           request.body());

    // 创建包含CGI输出的响应
    HttpResponse response = HttpResponse::create_200("text/html");
    response.set_body(output);
    return response;
}
