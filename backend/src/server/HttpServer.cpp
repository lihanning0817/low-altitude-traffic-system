#include "HttpServer.h"
#include "config/Config.h"
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>
#include <regex>
#include <sstream>

namespace server {

HttpServer::HttpServer(const std::string& address, unsigned short port)
    : ioc_()  // 移除硬编码的线程数1
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
            try {
                ioc_.run();
            } catch (const std::exception& e) {
                spdlog::error("Worker thread error: {}", e.what());
            }
        });
        spdlog::debug("Started worker thread {}/{}", i, threads);
    }

    spdlog::info("Running server with {} threads", threads);

    // 主线程也参与处理
    try {
        ioc_.run();
    } catch (const std::exception& e) {
        spdlog::error("Main thread error: {}", e.what());
    }

    // 等待所有工作线程结束
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    spdlog::info("All threads terminated");
}

void HttpServer::doAccept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                spdlog::debug("New connection accepted from: {}", socket.remote_endpoint().address().to_string());
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
            spdlog::debug("OPTIONS request handled for: {}", std::string(req.target()));
            return;
        }

        // 查找路由处理函数
        auto handler = findRoute(req.method(), std::string(req.target()));

        if (handler) {
            spdlog::debug("Route found for: {} {}", std::string(http::to_string(req.method())), std::string(req.target()));
            // 调用路由处理函数
            handler(req, res);
        } else {
            spdlog::warn("No route found for: {} {}", std::string(http::to_string(req.method())), std::string(req.target()));
            // 404 Not Found
            auto error_res = createErrorResponse(http::status::not_found, "API endpoint not found");
            // 保留CORS头信息
            applyCors(req, error_res);
            res = std::move(error_res);
        }

    } catch (const std::exception& e) {
        spdlog::error("Request handling error: {}", e.what());
        auto error_res = createErrorResponse(http::status::internal_server_error, "Internal server error");
        // 保留CORS头信息
        applyCors(req, error_res);
        res = std::move(error_res);
    }
}

RequestHandler HttpServer::findRoute(http::verb method, const std::string& path) {
    // 提取路径部分，忽略查询参数
    std::string clean_path = path;
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        clean_path = path.substr(0, query_pos);
    }

    for (const auto& route : routes_) {
        if (route.method == method) {
            // 简单路径匹配（可以扩展为正则表达式匹配）
            if (route.path == clean_path) {
                return route.handler;
            }

            // 支持路径参数匹配（例如：/api/users/{id}）
            std::regex route_regex(std::regex_replace(route.path, std::regex(R"(\{[^}]+\})"), R"([^/]+)"));
            if (std::regex_match(clean_path, route_regex)) {
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
        // 获取请求的Origin头
        std::string requestOrigin;
        auto originHeader = req.find(http::field::origin);
        if (originHeader != req.end()) {
            requestOrigin = std::string(originHeader->value());
        }

        // Access-Control-Allow-Origin - 支持动态Origin
        auto configOrigins = config.getString("cors.origins", "*");
        if (configOrigins == "*") {
            res.set(http::field::access_control_allow_origin, "*");
        } else if (!requestOrigin.empty()) {
            // 检查是否在允许的origin列表中
            std::vector<std::string> allowedOrigins;
            std::stringstream ss(configOrigins);
            std::string origin;
            while (std::getline(ss, origin, ',')) {
                // 去除空格
                origin.erase(0, origin.find_first_not_of(" \t"));
                origin.erase(origin.find_last_not_of(" \t") + 1);
                allowedOrigins.push_back(origin);
            }

            bool originAllowed = false;
            for (const auto& allowed : allowedOrigins) {
                if (requestOrigin == allowed) {
                    originAllowed = true;
                    break;
                }
            }

            if (originAllowed) {
                res.set(http::field::access_control_allow_origin, requestOrigin);
                res.set(http::field::access_control_allow_credentials, "true");
            }
        }

        // Access-Control-Allow-Methods
        auto methods = config.getString("cors.methods", "GET,POST,PUT,DELETE,OPTIONS,PATCH");
        res.set(http::field::access_control_allow_methods, methods);

        // Access-Control-Allow-Headers - 支持更多常用头
        auto headers = config.getString("cors.headers",
            "Accept,Authorization,Cache-Control,Content-Type,DNT,If-Modified-Since,Keep-Alive,Origin,User-Agent,X-Requested-With");
        res.set(http::field::access_control_allow_headers, headers);

        // Access-Control-Expose-Headers - 允许前端访问的响应头
        auto exposeHeaders = config.getString("cors.expose_headers", "Content-Length,Content-Range");
        if (!exposeHeaders.empty()) {
            res.set(http::field::access_control_expose_headers, exposeHeaders);
        }

        // Access-Control-Max-Age - 预检缓存时间
        auto maxAge = config.getString("cors.max_age", "86400");
        res.set("Access-Control-Max-Age", maxAge);

        spdlog::debug("CORS headers applied for origin: {}", requestOrigin.empty() ? "none" : requestOrigin);
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
    spdlog::debug("HttpSession created");
}

void HttpSession::run() {
    spdlog::debug("HttpSession started");
    doRead();
}

void HttpSession::doRead() {
    req_ = {};

    spdlog::debug("Starting async read for new request");

    // 设置读取超时
    stream_.expires_after(std::chrono::seconds(30));

    http::async_read(stream_, buffer_, req_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
            self->onRead(ec, bytes_transferred);
        });
}

void HttpSession::onRead(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream) {
        spdlog::debug("End of stream detected");
        return doClose();
    }

    if (ec) {
        spdlog::error("Read error: {}", ec.message());
        return doClose();
    }

    std::ostringstream thread_id_str;
    thread_id_str << std::this_thread::get_id();
    spdlog::debug("Request received: {} {} from thread: {}",
                  std::string(http::to_string(req_.method())),
                  std::string(req_.target()),
                  thread_id_str.str());

    // 处理请求
    http::response<http::string_body> res;
    server_->handleRequest(req_, res);

    spdlog::debug("Response prepared with status: {}", static_cast<int>(res.result()));

    // 发送响应
    sendResponse(std::move(res));
}

void HttpSession::sendResponse(http::response<http::string_body>&& res) {
    auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
    bool close = sp->need_eof();

    // 设置写入超时
    stream_.expires_after(std::chrono::seconds(30));

    http::async_write(stream_, *sp,
        [self = shared_from_this(), close](beast::error_code ec, std::size_t bytes_transferred) {
            self->onWrite(ec, bytes_transferred, close);
        });
}

void HttpSession::onWrite(beast::error_code ec, std::size_t bytes_transferred, bool close) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        spdlog::error("Write error: {}", ec.message());
        return doClose();
    }

    spdlog::debug("Response sent successfully, bytes: {}", bytes_transferred);

    if (close) {
        spdlog::debug("Closing connection as requested");
        return doClose();
    }

    // 继续读取下一个请求
    doRead();
}

void HttpSession::doClose() {
    spdlog::debug("Closing HTTP session");
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    if (ec) {
        spdlog::debug("Socket shutdown error: {}", ec.message());
    }
}

} // namespace server