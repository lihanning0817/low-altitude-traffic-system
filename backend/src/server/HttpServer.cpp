#include "HttpServer.h"
#include "config/Config.h"
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>
#include <regex>

namespace server {

HttpServer::HttpServer(const std::string& address, unsigned short port)
    : ioc_(1)
    , acceptor_(ioc_)
    , address_(address)
    , port_(port) {

    // 设置acceptor
    auto const addr = net::ip::make_address(address);
    tcp::endpoint endpoint{addr, port};

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(net::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(net::socket_base::max_listen_connections);

    spdlog::info("HTTP Server initialized on {}:{}", address_, port_);
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::addRoute(http::verb method, const std::string& path, RequestHandler handler) {
    routes_.push_back({method, path, handler});
    spdlog::debug("Added route: {} {}", std::string(http::to_string(method)), path);
}

void HttpServer::get(const std::string& path, RequestHandler handler) {
    addRoute(http::verb::get, path, handler);
}

void HttpServer::post(const std::string& path, RequestHandler handler) {
    addRoute(http::verb::post, path, handler);
}

void HttpServer::put(const std::string& path, RequestHandler handler) {
    addRoute(http::verb::put, path, handler);
}

void HttpServer::del(const std::string& path, RequestHandler handler) {
    addRoute(http::verb::delete_, path, handler);
}

void HttpServer::addMiddleware(const std::string& name, RequestHandler handler) {
    middleware_[name] = handler;
    spdlog::debug("Added middleware: {}", name);
}

void HttpServer::start() {
    doAccept();
    spdlog::info("HTTP Server started on {}:{}", address_, port_);
}

void HttpServer::stop() {
    if (acceptor_.is_open()) {
        acceptor_.close();
        spdlog::info("HTTP Server stopped");
    }
}

void HttpServer::run(int threads) {
    start();

    std::vector<std::thread> workers;
    workers.reserve(threads - 1);

    // 创建工作线程
    for (int i = 1; i < threads; ++i) {
        workers.emplace_back([this] {
            ioc_.run();
        });
    }

    // 主线程也参与处理
    ioc_.run();

    // 等待所有工作线程结束
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void HttpServer::doAccept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<HttpSession>(std::move(socket), this)->run();
            } else {
                spdlog::error("Accept error: {}", ec.message());
            }

            // 继续接受新连接
            doAccept();
        }
    );
}

void HttpServer::handleRequest(
    const http::request<http::string_body>& req,
    http::response<http::string_body>& res) {

    try {
        // 设置基本响应头
        res.version(req.version());
        res.keep_alive(req.keep_alive());

        // 应用CORS中间件
        applyCors(req, res);

        // 处理OPTIONS预检请求
        if (req.method() == http::verb::options) {
            res.result(http::status::ok);
            res.set(http::field::content_length, "0");
            return;
        }

        // 查找路由处理函数
        auto handler = findRoute(req.method(), std::string(req.target()));

        if (handler) {
            // 调用路由处理函数
            handler(req, res);
        } else {
            // 404 Not Found
            res = createErrorResponse(http::status::not_found, "API endpoint not found");
        }

    } catch (const std::exception& e) {
        spdlog::error("Request handling error: {}", e.what());
        res = createErrorResponse(http::status::internal_server_error, "Internal server error");
    }
}

RequestHandler HttpServer::findRoute(http::verb method, const std::string& path) {
    for (const auto& route : routes_) {
        if (route.method == method) {
            // 简单路径匹配（可以扩展为正则表达式匹配）
            if (route.path == path) {
                return route.handler;
            }

            // 支持路径参数匹配（例如：/api/users/{id}）
            std::regex route_regex(std::regex_replace(route.path, std::regex(R"(\{[^}]+\})"), R"([^/]+)"));
            if (std::regex_match(path, route_regex)) {
                return route.handler;
            }
        }
    }
    return nullptr;
}

void HttpServer::applyCors(
    const http::request<http::string_body>& req,
    http::response<http::string_body>& res) {

    auto& config = config::Config::getInstance();

    if (config.getBool("cors.enabled", true)) {
        // Access-Control-Allow-Origin
        auto origins = config.getString("cors.origins", "*");
        res.set(http::field::access_control_allow_origin, origins);

        // Access-Control-Allow-Methods
        auto methods = config.getString("cors.methods", "GET,POST,PUT,DELETE,OPTIONS");
        res.set(http::field::access_control_allow_methods, methods);

        // Access-Control-Allow-Headers
        auto headers = config.getString("cors.headers", "Content-Type,Authorization");
        res.set(http::field::access_control_allow_headers, headers);

        // Access-Control-Max-Age
        res.set("Access-Control-Max-Age", "86400");
    }
}

http::response<http::string_body> HttpServer::createErrorResponse(
    http::status status,
    const std::string& message) {

    nlohmann::json error_json = {
        {"success", false},
        {"error", message},
        {"timestamp", std::time(nullptr)}
    };

    return createJsonResponse(error_json, status);
}

http::response<http::string_body> HttpServer::createJsonResponse(
    const nlohmann::json& json,
    http::status status) {

    http::response<http::string_body> res{status, 11};
    res.set(http::field::server, "Low-Altitude-Traffic-System/1.0");
    res.set(http::field::content_type, "application/json; charset=utf-8");
    res.body() = json.dump();
    res.prepare_payload();

    return res;
}

// HttpSession 实现

HttpSession::HttpSession(tcp::socket&& socket, HttpServer* server)
    : stream_(std::move(socket))
    , server_(server) {
}

void HttpSession::run() {
    doRead();
}

void HttpSession::doRead() {
    req_ = {};

    http::async_read(stream_, buffer_, req_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
            self->onRead(ec, bytes_transferred);
        });
}

void HttpSession::onRead(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream) {
        return doClose();
    }

    if (ec) {
        spdlog::error("Read error: {}", ec.message());
        return;
    }

    // 处理请求
    http::response<http::string_body> res;
    server_->handleRequest(req_, res);

    // 发送响应
    sendResponse(std::move(res));
}

void HttpSession::sendResponse(http::response<http::string_body>&& res) {
    auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
    bool close = sp->need_eof();

    http::async_write(stream_, *sp,
        [self = shared_from_this(), close](beast::error_code ec, std::size_t bytes_transferred) {
            self->onWrite(ec, bytes_transferred, close);
        });
}

void HttpSession::onWrite(beast::error_code ec, std::size_t bytes_transferred, bool close) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        spdlog::error("Write error: {}", ec.message());
        return;
    }

    if (close) {
        return doClose();
    }

    // 继续读取下一个请求
    doRead();
}

void HttpSession::doClose() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    if (ec) {
        spdlog::debug("Socket shutdown error: {}", ec.message());
    }
}

} // namespace server