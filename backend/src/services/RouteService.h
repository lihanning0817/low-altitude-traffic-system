#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

namespace services {

/**
 * @brief 路线规划服务类
 * 集成高德地图API进行路线规划
 */
class RouteService {
private:
    std::string api_key_;
    std::string base_url_;

    /**
     * @brief HTTP响应数据结构
     */
    struct HttpResponse {
        std::string data;
        long status_code;
    };

public:
    /**
     * @brief 构造函数
     * @param api_key 高德地图API密钥
     */
    explicit RouteService(const std::string& api_key);

    /**
     * @brief 析构函数
     */
    ~RouteService();

    /**
     * @brief 规划路线
     * @param origin 起点坐标 "lng,lat"
     * @param destination 终点坐标 "lng,lat"
     * @param strategy 路线策略 (0=最快路线, 1=最短路线, 2=避免拥堵)
     * @return JSON格式的路线数据
     */
    nlohmann::json planRoute(
        const std::string& origin,
        const std::string& destination,
        const std::string& strategy = "0"
    );

    /**
     * @brief 批量路线规划（支持途经点）
     * @param origin 起点坐标
     * @param destination 终点坐标
     * @param waypoints 途经点列表
     * @param strategy 路线策略
     * @return JSON格式的路线数据
     */
    nlohmann::json planRouteWithWaypoints(
        const std::string& origin,
        const std::string& destination,
        const std::vector<std::string>& waypoints,
        const std::string& strategy = "0"
    );

    /**
     * @brief 地理编码 - 地址转坐标
     * @param address 地址字符串
     * @return JSON格式的坐标数据
     */
    nlohmann::json geocode(const std::string& address);

    /**
     * @brief 逆地理编码 - 坐标转地址
     * @param lng 经度
     * @param lat 纬度
     * @return JSON格式的地址数据
     */
    nlohmann::json reverseGeocode(double lng, double lat);

private:
    /**
     * @brief 执行HTTP GET请求
     * @param url 请求URL
     * @return HTTP响应
     */
    HttpResponse httpGet(const std::string& url);

    /**
     * @brief CURL写入回调函数
     */
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response);

    /**
     * @brief URL编码
     * @param input 输入字符串
     * @return 编码后的字符串
     */
    std::string urlEncode(const std::string& input);

    /**
     * @brief 解析高德地图API响应
     * @param response API响应JSON
     * @return 标准化的路线数据
     */
    nlohmann::json parseRouteResponse(const nlohmann::json& response);

    /**
     * @brief 创建错误响应
     * @param error_message 错误消息
     * @return 错误响应JSON
     */
    nlohmann::json createErrorResponse(const std::string& error_message);
};

} // namespace services