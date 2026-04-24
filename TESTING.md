# PoorWebServer 测试指南

## 项目验证方法

### 1. 基本功能验证

#### 启动服务器
```bash
# HTTP模式
./myhttp --port 8080

# HTTPS模式（需要证书）
./myhttp --port 8443 --https --cert server.crt --key server.key
```

#### 基本HTTP测试
```bash
# 测试首页访问
curl http://localhost:8080/

# 测试静态文件
curl http://localhost:8080/index.html
curl http://localhost:8080/test.html

# 测试404错误
curl http://localhost:8080/nonexistent.html

# 测试不支持的方法
curl -X PUT http://localhost:8080/
```

### 2. CGI功能测试

#### GET请求CGI
```bash
# 基本CGI执行
curl http://localhost:8080/post.cgi

# 带参数的CGI
curl "http://localhost:8080/post.cgi?name=test&value=123"
```

#### POST请求CGI
```bash
# POST数据测试
curl -X POST -d "name=John&age=25" http://localhost:8080/post.cgi

# 表单数据测试
curl -X POST -d "username=admin&password=123456" http://localhost:8080/post.cgi
```

### 3. 并发测试

#### 多连接测试
```bash
# 并发请求测试
for i in {1..10}; do
    curl -s http://localhost:8080/ &
done
wait
```

#### 压力测试
```bash
# 使用ab进行压力测试
ab -n 1000 -c 10 http://localhost:8080/
```

### 4. HTTPS测试

#### SSL证书生成（自签名）
```bash
# 生成私钥
openssl genrsa -out server.key 2048

# 生成证书请求
openssl req -new -key server.key -out server.csr

# 生成自签名证书
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt
```

#### HTTPS连接测试
```bash
# HTTPS访问（跳过证书验证）
curl -k https://localhost:8443/

# 查看SSL信息
openssl s_client -connect localhost:8443 -servername localhost
```

### 5. 自动化测试

#### 使用测试脚本
```bash
# 运行完整测试套件
./test_server.sh

# 只测试HTTPS功能
./test_server.sh https

# 运行完整测试（包括HTTPS）
./test_server.sh full
```

### 6. 错误处理测试

#### 各种错误情况
```bash
# 400 Bad Request（发送无效请求）
echo -e "INVALID REQUEST\r\n\r\n" | nc localhost 8080

# 405 Method Not Allowed
curl -X DELETE http://localhost:8080/

# 500 Internal Server Error（如果CGI脚本出错）
curl http://localhost:8080/broken.cgi
```

### 7. 性能监控

#### 检查服务器资源使用
```bash
# 查看进程
ps aux | grep myhttp

# 监控网络连接
netstat -tlnp | grep :8080

# 查看线程数
ps -T -p $(pgrep myhttp) | wc -l
```

#### 日志分析
```bash
# 如果服务器有日志输出，重定向到文件
./myhttp --port 8080 > server.log 2>&1

# 分析日志
tail -f server.log
```

### 8. 浏览器测试

#### 手动测试
1. 打开浏览器访问 `http://localhost:8080/`
2. 测试不同页面和功能
3. 使用开发者工具检查响应头和状态码
4. 测试HTTPS版本 `https://localhost:8443/`

### 9. 边界情况测试

#### 大文件测试
```bash
# 创建大文件
dd if=/dev/zero of=httpdocs/largefile.dat bs=1M count=10

# 测试大文件下载
curl -o /tmp/downloaded.dat http://localhost:8080/largefile.dat
```

#### 特殊字符和编码
```bash
# 测试URL编码
curl "http://localhost:8080/post.cgi?data=hello%20world%21"

# 测试特殊字符
curl "http://localhost:8080/post.cgi?data=中文测试"
```

### 10. 安全测试

#### 基本安全检查
```bash
# 测试目录遍历攻击
curl http://localhost:8080/../../../etc/passwd

# 测试CGI安全
curl "http://localhost:8080/post.cgi?cmd=ls"
```

## 预期结果

### 成功标准
- [ ] 服务器正常启动，无错误输出
- [ ] HTTP请求返回正确的状态码和内容
- [ ] CGI脚本正确执行并返回动态内容
- [ ] HTTPS连接建立成功（需要证书）
- [ ] 并发请求被正确处理
- [ ] 错误情况返回适当的HTTP状态码
- [ ] 服务器资源使用合理，无内存泄漏

### 性能基准
- 单个请求响应时间 < 100ms
- 支持至少100个并发连接
- 内存使用 < 50MB（基本负载）
- CPU使用率 < 80%（高负载时）

## 故障排除

### 常见问题
1. **端口被占用**: 更换端口或停止其他服务
2. **权限问题**: 确保CGI脚本有执行权限 `chmod +x post.cgi`
3. **SSL证书问题**: 检查证书文件路径和格式
4. **编译错误**: 确保安装了OpenSSL开发库

### 调试技巧
- 使用 `strace` 跟踪系统调用
- 添加调试输出到服务器代码
- 使用Wireshark分析网络流量
- 检查系统日志 `dmesg | tail`