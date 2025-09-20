#ifndef WEBSOCKET_CONTROLLER_H
#define WEBSOCKET_CONTROLLER_H

#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXConnectionState.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

using json = nlohmann::json;

class WebSocketController {
public:
    WebSocketController(int port = 8080);
    ~WebSocketController();

    // 启动WebSocket服务器
    void start();
    
    // 停止WebSocket服务器
    void stop();

    // 广播消息给所有连接的客户端
    void broadcast(const std::string& message);
    
    // 发送消息给特定客户端
    void sendToClient(int clientId, const std::string& message);

private:
    // 处理新的WebSocket连接
    void setupConnectionHandlers();
    
    // 处理接收到的消息
    void handleMessage(int clientId, const std::string& message);
    
    // 处理客户端断开连接
    void handleDisconnection(int clientId);
    
    // 处理客户端连接
    void handleConnection(int clientId);
    
    // 发送飞行任务更新
    void sendFlightTaskUpdate(int taskId, const std::string& status);
    
    // 发送交通状态更新
    void sendTrafficUpdate(const json& trafficData);
    
    // 发送紧急事件通知
    void sendEmergencyAlert(const json& alertData);
    
    // 验证客户端身份
    bool authenticateClient(const json& authData);

private:
    std::unique_ptr<ix::WebSocketServer> m_server;
    std::unordered_map<int, std::string> m_connectedClients; // clientId -> session info
    std::mutex m_clientsMutex;
    int m_nextClientId;
};

#endif // WEBSOCKET_CONTROLLER_H
