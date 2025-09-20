#include "WebSocketController.h"
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>
#include <sstream>

WebSocketController::WebSocketController(int port) 
    : m_server(std::make_unique<ix::WebSocketServer>(port))
    , m_nextClientId(1) {
    ix::initNetSystem();
    setupConnectionHandlers();
}

WebSocketController::~WebSocketController() {
    stop();
    ix::uninitNetSystem();
}

void WebSocketController::start() {
    if (m_server->listen().first) {
        m_server->start();
        std::cout << "WebSocket server started on port " << m_server->getPort() << std::endl;
    } else {
        std::cerr << "Failed to start WebSocket server" << std::endl;
    }
}

void WebSocketController::stop() {
    m_server->stop();
}

void WebSocketController::broadcast(const std::string& message) {
    // 获取当前连接的客户端列表
    auto clients = m_server->getClients();
    // 使用迭代器遍历客户端
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        // 注意：ix::WebSocketServer::getClients() 返回的是 std::set<std::shared_ptr<ix::WebSocket>>
        std::shared_ptr<ix::WebSocket> ws = *it;
        if (ws) {
            ws->sendText(message);
        }
    }
}

void WebSocketController::sendToClient(int clientId, const std::string& message) {
    // 在实际实现中，我们需要通过WebSocket连接发送消息
    // 这里简化处理，直接广播
    broadcast(message);
}

void WebSocketController::setupConnectionHandlers() {
    m_server->setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket> webSocket,
               std::shared_ptr<ix::ConnectionState> connectionState) {
            // 处理新连接
            if (auto ws = webSocket.lock()) {
                int clientId = m_nextClientId++;
                handleConnection(clientId);
                
                // 设置消息处理回调
                ws->setOnMessageCallback([this, clientId](const ix::WebSocketMessagePtr& msg) {
                    if (msg->type == ix::WebSocketMessageType::Message) {
                        handleMessage(clientId, msg->str);
                    } else if (msg->type == ix::WebSocketMessageType::Close) {
                        handleDisconnection(clientId);
                    }
                });
            }
        });
}

void WebSocketController::handleMessage(int clientId, const std::string& message) {
    try {
        json jsonData = json::parse(message);
        
        // 处理不同类型的消息
        if (jsonData.contains("type")) {
            std::string type = jsonData["type"];
            if (type == "auth") {
                if (authenticateClient(jsonData)) {
                    // 认证成功，发送欢迎消息
                    json response = {
                        {"type", "auth_success"},
                        {"message", "Authentication successful"}
                    };
                    sendToClient(clientId, response.dump());
                } else {
                    // 认证失败，断开连接
                    json response = {
                        {"type", "auth_failed"},
                        {"message", "Authentication failed"}
                    };
                    sendToClient(clientId, response.dump());
                }
            } else if (type == "subscribe_flight_updates") {
                // 客户端订阅飞行任务更新
                json response = {
                    {"type", "subscription_confirmed"},
                    {"subscription", "flight_updates"}
                };
                sendToClient(clientId, response.dump());
            } else if (type == "subscribe_traffic") {
                // 客户端订阅交通状态更新
                json response = {
                    {"type", "subscription_confirmed"},
                    {"subscription", "traffic"}
                };
                sendToClient(clientId, response.dump());
            } else if (type == "emergency_alert") {
                // 处理紧急事件报告
                sendEmergencyAlert(jsonData);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

void WebSocketController::handleDisconnection(int clientId) {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    m_connectedClients.erase(clientId);
    std::cout << "Client " << clientId << " disconnected" << std::endl;
}

void WebSocketController::handleConnection(int clientId) {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    m_connectedClients[clientId] = ""; // 初始化客户端会话信息
    std::cout << "Client " << clientId << " connected" << std::endl;
}

void WebSocketController::sendFlightTaskUpdate(int taskId, const std::string& status) {
    json update = {
        {"type", "flight_task_update"},
        {"taskId", taskId},
        {"status", status},
        {"timestamp", std::time(nullptr)}
    };
    broadcast(update.dump());
}

void WebSocketController::sendTrafficUpdate(const json& trafficData) {
    json update = {
        {"type", "traffic_update"},
        {"data", trafficData},
        {"timestamp", std::time(nullptr)}
    };
    broadcast(update.dump());
}

void WebSocketController::sendEmergencyAlert(const json& alertData) {
    json alert = {
        {"type", "emergency_alert"},
        {"data", alertData},
        {"timestamp", std::time(nullptr)}
    };
    broadcast(alert.dump());
}

bool WebSocketController::authenticateClient(const json& authData) {
    // 简化的认证逻辑
    // 在实际实现中，应该验证JWT token或会话信息
    if (authData.contains("token")) {
        std::string token = authData["token"];
        // 这里应该验证token的有效性
        // 暂时假设所有token都有效
        return !token.empty();
    }
    return false;
}
