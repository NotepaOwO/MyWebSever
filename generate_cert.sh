#!/bin/bash
# SSL证书生成脚本
# 为PoorWebServer生成自签名SSL证书

echo "=== 生成SSL证书 ==="
echo "这将为PoorWebServer创建自签名SSL证书"
echo

# 检查OpenSSL是否安装
if ! command -v openssl &> /dev/null; then
    echo "错误: OpenSSL未安装，请先安装OpenSSL"
    exit 1
fi

# 生成私钥
echo "1. 生成私钥 (server.key)..."
openssl genrsa -out server.key 2048

if [ $? -ne 0 ]; then
    echo "错误: 私钥生成失败"
    exit 1
fi

# 生成证书请求
echo "2. 生成证书请求..."
openssl req -new -key server.key -out server.csr -subj "/C=CN/ST=Beijing/L=Beijing/O=PoorWebServer/CN=localhost"

if [ $? -ne 0 ]; then
    echo "错误: 证书请求生成失败"
    exit 1
fi

# 生成自签名证书（有效期365天）
echo "3. 生成自签名证书 (server.crt)..."
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt

if [ $? -ne 0 ]; then
    echo "错误: 证书生成失败"
    exit 1
fi

# 设置适当权限
chmod 600 server.key
chmod 644 server.crt

# 清理临时文件
rm -f server.csr

echo
echo "=== 证书生成完成 ==="
echo "私钥文件: server.key"
echo "证书文件: server.crt"
echo
echo "文件信息:"
ls -la server.crt server.key
echo
echo "证书详情:"
openssl x509 -in server.crt -text -noout | head -10