#!/bin/bash
# PoorWebServer 测试脚本
# 用于验证服务器的各项功能

SERVER_HOST="localhost"
HTTP_PORT="8080"
HTTPS_PORT="8443"
TEST_DIR="$(dirname "$0")"

echo "=== PoorWebServer 功能验证脚本 ==="
echo "服务器主机: $SERVER_HOST"
echo "HTTP端口: $HTTP_PORT"
echo "HTTPS端口: $HTTPS_PORT"
echo

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试函数
test_http_get() {
    local url="$1"
    local expected_code="$2"
    local description="$3"

    echo -n "测试 $description: "
    response=$(curl -s -o /dev/null -w "%{http_code}" "$url" 2>/dev/null)
    if [ "$response" = "$expected_code" ]; then
        echo -e "${GREEN}通过${NC}"
        return 0
    else
        echo -e "${RED}失败${NC} (期望: $expected_code, 实际: $response)"
        return 1
    fi
}

test_content_contains() {
    local url="$1"
    local expected_text="$2"
    local description="$3"

    echo -n "测试 $description: "
    content=$(curl -s "$url" 2>/dev/null)
    if echo "$content" | grep -q "$expected_text"; then
        echo -e "${GREEN}通过${NC}"
        return 0
    else
        echo -e "${RED}失败${NC} (未找到: $expected_text)"
        return 1
    fi
}

test_cgi_execution() {
    local url="$1"
    local description="$2"

    echo -n "测试 $description: "
    content=$(curl -s "$url" 2>/dev/null)
    if echo "$content" | grep -q "CGI Script Executed Successfully"; then
        echo -e "${GREEN}通过${NC}"
        return 0
    else
        echo -e "${RED}失败${NC} (CGI脚本未正确执行)"
        return 1
    fi
}

test_post_request() {
    local url="$1"
    local data="$2"
    local description="$3"

    echo -n "测试 $description: "
    response=$(curl -s -X POST -d "$data" "$url" 2>/dev/null)
    if echo "$response" | grep -q "POST Data:"; then
        echo -e "${GREEN}通过${NC}"
        return 0
    else
        echo -e "${RED}失败${NC} (POST数据未正确处理)"
        return 1
    fi
}

# 等待服务器启动
wait_for_server() {
    local port="$1"
    local timeout=10
    local count=0

    echo "等待服务器在端口 $port 启动..."
    while ! nc -z $SERVER_HOST $port 2>/dev/null; do
        if [ $count -ge $timeout ]; then
            echo -e "${RED}服务器启动超时${NC}"
            return 1
        fi
        sleep 1
        count=$((count + 1))
    done
    echo -e "${GREEN}服务器已启动${NC}"
    return 0
}

# 主测试流程
main() {
    local test_count=0
    local pass_count=0

    echo "1. 检查服务器是否正在运行..."
    if nc -z $SERVER_HOST $HTTP_PORT 2>/dev/null; then
        echo -e "${YELLOW}警告: 端口 $HTTP_PORT 已被占用，可能有其他服务器运行${NC}"
    fi

    echo
    echo "2. 启动服务器进行测试..."
    echo "请在另一个终端运行以下命令启动服务器:"
    echo "cd \"$TEST_DIR\" && ./myhttp --port $HTTP_PORT"
    echo
    read -p "服务器已启动后按回车键继续测试..."

    if ! wait_for_server $HTTP_PORT; then
        echo -e "${RED}无法连接到服务器，测试终止${NC}"
        exit 1
    fi

    echo
    echo "3. 开始功能测试..."
    echo

    # 基本HTTP测试
    echo "=== 基本HTTP功能测试 ==="

    ((test_count++))
    if test_http_get "http://$SERVER_HOST:$HTTP_PORT/" "200" "根路径访问"; then
        ((pass_count++))
    fi

    ((test_count++))
    if test_http_get "http://$SERVER_HOST:$HTTP_PORT/index.html" "200" "直接访问index.html"; then
        ((pass_count++))
    fi

    ((test_count++))
    if test_content_contains "http://$SERVER_HOST:$HTTP_PORT/" "PoorWebServer" "首页内容"; then
        ((pass_count++))
    fi

    ((test_count++))
    if test_http_get "http://$SERVER_HOST:$HTTP_PORT/nonexistent.html" "404" "404错误页面"; then
        ((pass_count++))
    fi

    ((test_count++))
    if test_http_get "http://$SERVER_HOST:$HTTP_PORT/" "405" "不支持的PUT方法" "PUT"; then
        ((pass_count++))
    fi

    # CGI测试
    echo
    echo "=== CGI功能测试 ==="

    ((test_count++))
    if test_cgi_execution "http://$SERVER_HOST:$HTTP_PORT/post.cgi" "CGI脚本执行"; then
        ((pass_count++))
    fi

    ((test_count++))
    if test_cgi_execution "http://$SERVER_HOST:$HTTP_PORT/post.cgi?name=test&value=123" "CGI带参数执行"; then
        ((pass_count++))
    fi

    ((test_count++))
    if test_post_request "http://$SERVER_HOST:$HTTP_PORT/post.cgi" "name=John&age=25" "POST请求处理"; then
        ((pass_count++))
    fi

    # 并发测试
    echo
    echo "=== 并发测试 ==="
    echo "测试10个并发请求..."
    concurrent_results=$(for i in {1..10}; do
        curl -s "http://$SERVER_HOST:$HTTP_PORT/" | grep -c "PoorWebServer" &
    done | paste -sd+ | bc)

    if [ "$concurrent_results" = "10" ]; then
        echo -e "并发测试: ${GREEN}通过${NC} (10/10 成功)"
        ((pass_count++))
    else
        echo -e "并发测试: ${RED}失败${NC} ($concurrent_results/10 成功)"
    fi
    ((test_count++))

    # 静态文件测试
    echo
    echo "=== 静态文件测试 ==="

    ((test_count++))
    if test_http_get "http://$SERVER_HOST:$HTTP_PORT/test.html" "200" "静态HTML文件"; then
        ((pass_count++))
    fi

    # 测试结果汇总
    echo
    echo "=== 测试结果汇总 ==="
    echo "总测试数: $test_count"
    echo "通过测试: $pass_count"
    echo "失败测试: $((test_count - pass_count))"

    if [ $pass_count -eq $test_count ]; then
        echo -e "${GREEN}所有测试通过！服务器功能正常。${NC}"
        exit 0
    else
        echo -e "${RED}部分测试失败，请检查服务器配置和日志。${NC}"
        exit 1
    fi
}

# HTTPS测试函数（可选）
test_https() {
    echo
    echo "=== HTTPS功能测试 ==="
    echo "注意: HTTPS测试需要SSL证书文件"

    if [ ! -f "server.crt" ] || [ ! -f "server.key" ]; then
        echo -e "${YELLOW}SSL证书文件不存在，跳过HTTPS测试${NC}"
        return
    fi

    echo "请在另一个终端运行以下命令启动HTTPS服务器:"
    echo "cd \"$TEST_DIR\" && ./myhttp --port $HTTPS_PORT --https --cert server.crt --key server.key"
    echo
    read -p "HTTPS服务器已启动后按回车键继续测试..."

    if ! wait_for_server $HTTPS_PORT; then
        echo -e "${RED}无法连接到HTTPS服务器${NC}"
        return
    fi

    echo "测试HTTPS连接..."
    # 注意: curl需要跳过证书验证用于自签名证书
    if curl -k -s "https://$SERVER_HOST:$HTTPS_PORT/" | grep -q "PoorWebServer"; then
        echo -e "HTTPS测试: ${GREEN}通过${NC}"
    else
        echo -e "HTTPS测试: ${RED}失败${NC}"
    fi
}

# 脚本参数处理
case "$1" in
    "https")
        test_https
        ;;
    "full")
        main
        test_https
        ;;
    *)
        main
        ;;
esac