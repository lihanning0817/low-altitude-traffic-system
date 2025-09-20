#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "httplib.h"

namespace services {

    // 坐标点结构
    struct Coordinate {
        double lng;
        double lat;

        Coordinate(double longitude, double latitude) : lng(longitude), lat(latitude) {}
    };

    // 路线段结构
    struct RouteStep {
        std::string instruction;  // 路线指示
        std::string road_name;    // 道路名称
        double distance;          // 距离（米）
        double duration;          // 时间（秒）
        std::vector<Coordinate> polyline;  // 路径坐标点
    };

    // 路线结果结构
    struct RouteResult {
        double total_distance;    // 总距离（米）
        double total_duration;    // 总时间（秒）
        std::vector<RouteStep> steps;     // 路线步骤
        std::vector<Coordinate> overview_polyline;  // 整体路径坐标
        std::string formatted_distance;   // 格式化距离
        std::string formatted_duration;   // 格式化时间
    };

    // 高德路线规划服务类
    class AmapRouteService {
    private:
        std::string api_key_;
        std::string base_url_;
        std::unique_ptr<httplib::Client> http_client_;

        // 解析高德API响应
        std::optional<RouteResult> parseAmapResponse(const std::string& response);

        // 解码polyline字符串为坐标数组
        std::vector<Coordinate> decodePolyline(const std::string& polyline);

        // 格式化距离显示
        std::string formatDistance(double meters);

        // 格式化时间显示
        std::string formatDuration(double seconds);

    public:
        // 构造函数
        explicit AmapRouteService(const std::string& api_key);

        // 析构函数
        ~AmapRouteService();

        // 路线规划
        std::optional<RouteResult> planRoute(
            double origin_lng, double origin_lat,
            double dest_lng, double dest_lat,
            int strategy = 0  // 0:最快路线 1:最短路线 2:避免拥堵
        );

        // 带途经点的路线规划
        std::optional<RouteResult> planRouteWithWaypoints(
            double origin_lng, double origin_lat,
            double dest_lng, double dest_lat,
            const std::vector<Coordinate>& waypoints,
            int strategy = 0
        );

        // 检查服务可用性
        bool isServiceAvailable();
    };
}