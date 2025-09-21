#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace server {

/**
 * @brief HTTP请求处理函数类型
 */
using RequestHandler = std::function<void(
    const http::request<http::string_body>&,
    http::response<http::string_body>&
)>;

/**
 * @brief HTTP路由信息
 */
struct Route {
    http::verb method;
    std::string path;
    RequestHandler handler;
};

/**
 * @brief HTTP服务器类
 * 基于Boost.Beast实现的高性能HTTP服务器
 */
class HttpServer {
private:
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    std::string address_;
    unsigned short port_;
    std::vector<Route> routes_;
    std::unordered_map<std::string, RequestHandler> middleware_;

public:
    /**
     * @brief 构造函数
     * @param address 监听地址
     * @param port 监听端口
     */
    HttpServer(const std::string& address, unsigned short port);

    /**
     * @brief 析构函数
     */
    ~HttpServer();

    /**
     * @brief 添加路由
     * @param method HTTP方法
     * @param path 路径
     * @param handler 处理函数
     */
    void addRoute(http::verb method, const std::string& path, RequestHandler handler);

    /**
     * @brief 添加GET路由
     * @param path 路径
     * @param handler 处理函数
     */
    void get(const std::string& path, RequestHandler handler);

    /**
     * @brief 添加POST路由
     * @param path 路径
     * @param handler 处理函数
     */
    void post(const std::string& path, RequestHandler handler);

    /**
     * @brief 添加PUT路由
     * @param path 路径
     * @param handler 处理函数
     */
    void put(const std::string& path, RequestHandler handler);

    /**
     * @brief 添加DELETE路由
     * @param path 路径
     * @param handler 处理函数
     */
    void del(const std::string& path, RequestHandler handler);

    /**
     * @brief 添加中间件
     * @param name 中间件名称
     * @param handler 处理函数
     */
    void addMiddleware(const std::string& name, RequestHandler handler);

    /**
     * @brief 启动服务器
     */
    void start();

    /**
     * @brief 停止服务器
     */
    void stop();

    /**
     * @brief 运行服务器（阻塞）
     * @param threads 线程数
     */
    void run(int threads = 1);

    /**
     * @brief 处理HTTP请求
     * @param req 请求对象
     * @param res 响应对象
     */
    void handleRequest(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    );

private:
    /**
     * @brief 接受新连接
     */
    void doAccept();

    /**
     * @brief 处理HTTP会话
     * @param socket TCP socket
     */
    void handleSession(tcp::socket socket);

    /**
     * @brief 查找匹配的路由
     * @param method HTTP方法
     * @param path 请求路径
     * @return 路由处理函数，如果没找到返回nullptr
     */
    RequestHandler findRoute(http::verb method, const std::string& path);

    /**
     * @brief 应用CORS中间件
     * @param req 请求对象
     * @param res 响应对象
     */
    void applyCors(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    );

    /**
     * @brief 创建错误响应
     * @param status HTTP状态码
     * @param message 错误消息
     * @return 响应对象
     */
    http::response<http::string_body> createErrorResponse(
        http::status status,
        const std::string& message
    );

    /**
     * @brief 创建JSON响应
     * @param json JSON数据
     * @param status HTTP状态码
     * @return 响应对象
     */
    http::response<http::string_body> createJsonResponse(
        const nlohmann::json& json,
        http::status status = http::status::ok
    );
};

/**
 * @brief HTTP会话类
 * 处理单个客户端连接
 */
class HttpSession : public std::enable_shared_from_this<HttpSession> {
private:
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    HttpServer* server_;

public:
    /**
     * @brief 构造函数
     * @param socket TCP socket
     * @param server 服务器实例
     */
    explicit HttpSession(tcp::socket&& socket, HttpServer* server);

    /**
     * @brief 开始处理请求
     */
    void run();

private:
    /**
     * @brief 读取HTTP请求
     */
    void doRead();

    /**
     * @brief 处理HTTP请求
     */
    void onRead(beast::error_code ec, std::size_t bytes_transferred);

    /**
     * @brief 发送HTTP响应
     * @param res 响应对象
     */
    void sendResponse(http::response<http::string_body>&& res);

    /**
     * @brief 写入响应数据
     * @param res 响应对象
     */
    void onWrite(beast::error_code ec, std::size_t bytes_transferred, bool close);

    /**
     * @brief 关闭连接
     */
    void doClose();
};

} // namespace server