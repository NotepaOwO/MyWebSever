# PoorWebServer 模块说明

## 模块架构概述

项目采用模块化设计，将不同功能划分到独立的目录中，提高代码的可维护性和可扩展性。

## 模块说明

### core/ - 核心模块
**职责**：程序入口和服务器主逻辑
- `main.cpp`: 命令行参数解析，服务器启动
- `Server.hpp/cpp`: 服务器主类，管理网络监听和请求分发

### network/ - 网络模块
**职责**：底层网络通信封装
- `Socket.hpp/cpp`: RAII模式的Socket封装，提供TCP连接管理

**设计模式**：
- RAII模式：自动管理socket资源，避免内存泄漏

### http/ - HTTP协议模块
**职责**：HTTP协议解析和处理
- `HttpRequest.hpp/cpp`: HTTP请求解析（方法、URL、headers、body）
- `HttpResponse.hpp/cpp`: HTTP响应生成（状态码、headers、body）
- `HttpHandler.hpp/cpp`: 请求处理逻辑，路由分发

**设计模式**：
- 工厂模式：HttpResponse的静态创建方法

### ssl/ - SSL安全模块
**职责**：HTTPS安全传输
- `SSLWrapper.hpp/cpp`: OpenSSL API封装，提供TLS握手和加密通信

**设计模式**：
- 代理模式：封装复杂的OpenSSL库调用

### concurrent/ - 并发模块
**职责**：并发请求处理
- `ThreadPool.hpp/cpp`: 线程池实现，避免"每连接一线程"问题

**设计模式**：
- 线程池模式：高效管理线程资源

### cgi/ - CGI处理模块
**职责**：动态内容生成
- `CGIHandler.hpp/cpp`: CGI脚本执行，通过fork/exec和管道通信

## 模块依赖关系

```
core
├── network (Socket)
├── concurrent (ThreadPool)
├── ssl (SSLWrapper)
└── http (HttpHandler)

http
├── network (Socket)
├── ssl (SSLWrapper)
└── cgi (CGIHandler)

ssl
└── network (Socket)

cgi
└── (独立模块)
```

## 编译和构建

使用Makefile进行模块化编译：
- 每个模块独立编译为.o文件
- 最终链接所有模块生成可执行文件
- 支持增量编译，提高开发效率

## 扩展性

模块化设计便于功能扩展：
- 新增协议：可在http/模块中添加
- 新增安全机制：可在ssl/模块中扩展
- 新增并发模型：可在concurrent/模块中替换
- 新增动态处理：可在cgi/模块中添加其他脚本支持