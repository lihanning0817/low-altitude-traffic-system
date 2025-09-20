#include "amap_route_service.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace services {

    AmapRouteService::AmapRouteService(const std::string& api_key)
        : api_key_(api_key), base_url_("http://restapi.amap.com") {
        // 初始化HTTP客户端 (使用HTTP而不是HTTPS以避免SSL依赖问题)
        http_client_ = std::make_unique<httplib::Client>(base_url_);
        http_client_->set_connection_timeout(10, 0);  // 10秒连接超时
        http_client_->set_read_timeout(15, 0);        // 15秒读取超时
    }

    AmapRouteService::~AmapRouteService() = default;

    std::optional<RouteResult> AmapRouteService::planRoute(
        double origin_lng, double origin_lat,
        double dest_lng, double dest_lat,
        int strategy) {

        try {
            std::cout << "模拟路线规划: (" << origin_lng << "," << origin_lat
                      << ") -> (" << dest_lng << "," << dest_lat << "), 策略: " << strategy << std::endl;

            // 创建模拟路线结果用于演示
            RouteResult result;

            // 计算大概距离（简单的欧几里得距离转换为米）
            double distance_deg = std::sqrt(std::pow(dest_lng - origin_lng, 2) + std::pow(dest_lat - origin_lat, 2));
            result.total_distance = distance_deg * 111000; // 大概转换为米
            result.total_duration = result.total_distance / 50 * 3.6; // 假设平均速度50km/h

            result.formatted_distance = formatDistance(result.total_distance);
            result.formatted_duration = formatDuration(result.total_duration);

            // 创建简单的直线路径
            int num_points = 20;
            for (int i = 0; i <= num_points; ++i) {
                double t = static_cast<double>(i) / num_points;
                double lng = origin_lng + t * (dest_lng - origin_lng);
                double lat = origin_lat + t * (dest_lat - origin_lat);
                result.overview_polyline.emplace_back(lng, lat);
            }

            // 创建模拟步骤
            RouteStep step;
            step.instruction = "沿当前道路直行";
            step.road_name = "模拟道路";
            step.distance = result.total_distance;
            step.duration = result.total_duration;
            step.polyline = result.overview_polyline;
            result.steps.push_back(step);

            std::cout << "模拟路线规划成功: 距离 " << result.formatted_distance
                      << ", 时间 " << result.formatted_duration << std::endl;

            return result;

        } catch (const std::exception& e) {
            std::cerr << "路线规划请求异常: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<RouteResult> AmapRouteService::planRouteWithWaypoints(
        double origin_lng, double origin_lat,
        double dest_lng, double dest_lat,
        const std::vector<Coordinate>& waypoints,
        int strategy) {

        try {
            // 构建请求URL
            std::ostringstream url;
            url << "/v5/direction/driving?key=" << api_key_
                << "&origin=" << std::fixed << std::setprecision(6) << origin_lng << "," << origin_lat
                << "&destination=" << dest_lng << "," << dest_lat
                << "&strategy=" << strategy
                << "&extensions=all"
                << "&output=json";

            // 添加途经点
            if (!waypoints.empty()) {
                url << "&waypoints=";
                for (size_t i = 0; i < waypoints.size(); ++i) {
                    if (i > 0) url << ";";
                    url << waypoints[i].lng << "," << waypoints[i].lat;
                }
            }

            std::cout << "请求带途经点的高德路线规划API: " << base_url_ << url.str() << std::endl;

            // 发送HTTP GET请求
            auto result = http_client_->Get(url.str());

            if (!result) {
                std::cerr << "HTTP请求失败" << std::endl;
                return std::nullopt;
            }

            if (result->status != 200) {
                std::cerr << "HTTP响应错误，状态码: " << result->status << std::endl;
                return std::nullopt;
            }

            // 解析响应
            return parseAmapResponse(result->body);

        } catch (const std::exception& e) {
            std::cerr << "带途经点的路线规划请求异常: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<RouteResult> AmapRouteService::parseAmapResponse(const std::string& response) {
        try {
            json j = json::parse(response);

            // 检查响应状态
            if (!j.contains("status") || j["status"] != "1") {
                std::string info = j.contains("info") ? j["info"] : "未知错误";
                std::cerr << "高德API返回错误: " << info << std::endl;
                return std::nullopt;
            }

            // 检查路线数据
            if (!j.contains("route") || !j["route"].contains("paths") || j["route"]["paths"].empty()) {
                std::cerr << "高德API返回的路线数据为空" << std::endl;
                return std::nullopt;
            }

            RouteResult result;
            auto& path = j["route"]["paths"][0];  // 取第一条路线

            // 解析基本信息
            result.total_distance = std::stod(path["distance"].get<std::string>());
            result.total_duration = std::stod(path["duration"].get<std::string>());
            result.formatted_distance = formatDistance(result.total_distance);
            result.formatted_duration = formatDuration(result.total_duration);

            // 解析路线步骤
            if (path.contains("steps")) {
                for (const auto& step : path["steps"]) {
                    RouteStep route_step;

                    route_step.instruction = step.contains("instruction") ? step["instruction"] : "";
                    route_step.road_name = step.contains("road") ? step["road"] : "";
                    route_step.distance = std::stod(step["distance"].get<std::string>());
                    route_step.duration = std::stod(step["duration"].get<std::string>());

                    // 解析polyline坐标
                    if (step.contains("polyline")) {
                        std::string polyline = step["polyline"];
                        route_step.polyline = decodePolyline(polyline);

                        // 添加到总体路径
                        result.overview_polyline.insert(result.overview_polyline.end(),
                                                       route_step.polyline.begin(),
                                                       route_step.polyline.end());
                    }

                    result.steps.push_back(route_step);
                }
            }

            std::cout << "路线规划成功: 距离 " << result.formatted_distance
                      << ", 时间 " << result.formatted_duration
                      << ", 步骤数 " << result.steps.size() << std::endl;

            return result;

        } catch (const std::exception& e) {
            std::cerr << "解析高德API响应异常: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<Coordinate> AmapRouteService::decodePolyline(const std::string& polyline) {
        std::vector<Coordinate> coordinates;

        try {
            // 高德地图的polyline格式是经纬度坐标对，以分号分隔
            std::stringstream ss(polyline);
            std::string coordinate_pair;

            while (std::getline(ss, coordinate_pair, ';')) {
                size_t comma_pos = coordinate_pair.find(',');
                if (comma_pos != std::string::npos) {
                    double lng = std::stod(coordinate_pair.substr(0, comma_pos));
                    double lat = std::stod(coordinate_pair.substr(comma_pos + 1));
                    coordinates.emplace_back(lng, lat);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "解码polyline异常: " << e.what() << std::endl;
        }

        return coordinates;
    }

    std::string AmapRouteService::formatDistance(double meters) {
        if (meters < 1000) {
            return std::to_string(static_cast<int>(meters)) + " 米";
        } else {
            double km = meters / 1000.0;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << km << " 公里";
            return oss.str();
        }
    }

    std::string AmapRouteService::formatDuration(double seconds) {
        int total_minutes = static_cast<int>(seconds / 60);
        int hours = total_minutes / 60;
        int minutes = total_minutes % 60;

        if (hours > 0) {
            return std::to_string(hours) + " 小时 " + std::to_string(minutes) + " 分钟";
        } else {
            return std::to_string(minutes) + " 分钟";
        }
    }

    bool AmapRouteService::isServiceAvailable() {
        try {
            // 测试简单的地理编码API
            auto result = http_client_->Get("/v3/geocode/geo?key=" + api_key_ + "&address=北京市");
            return result && result->status == 200;
        } catch (...) {
            return false;
        }
    }
}