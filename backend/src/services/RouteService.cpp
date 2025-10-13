#include "RouteService.h"
#include "utils/CurlHandle.h"
#include "utils/ParamParser.h"
#include <spdlog/spdlog.h>
#include <sstream>
#include <iomanip>

namespace services {

RouteService::RouteService(const std::string& api_key)
    : api_key_(api_key), base_url_("https://restapi.amap.com/v3") {
    // 初始化CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

RouteService::~RouteService() {
    // 清理CURL
    curl_global_cleanup();
}

nlohmann::json RouteService::planRoute(
    const std::string& origin,
    const std::string& destination,
    const std::string& strategy) {

    try {
        // 记录输入参数
        spdlog::info("RouteService::planRoute called with origin: {}, destination: {}, strategy: {}", origin, destination, strategy);

        // 构建请求URL
        std::stringstream url;
        url << base_url_ << "/direction/driving"
            << "?origin=" << urlEncode(origin)
            << "&destination=" << urlEncode(destination)
            << "&strategy=" << strategy
            << "&key=" << api_key_
            << "&output=json"
            << "&extensions=all";

        spdlog::debug("Route planning request URL: {}", url.str());

        // 执行HTTP请求
        auto response = httpGet(url.str());

        spdlog::debug("HTTP response status: {}", response.status_code);
        spdlog::debug("HTTP response size: {} bytes", response.data.size());

        if (response.status_code != 200) {
            spdlog::error("HTTP request failed with status: {}", response.status_code);
            spdlog::error("Response data: {}", response.data.substr(0, 500));
            return createErrorResponse("HTTP request failed with status: " + std::to_string(response.status_code));
        }

        // 记录完整的高德API响应（前2000字符）
        spdlog::info("Amap API raw response (first 2000 chars): {}", response.data.substr(0, 2000));

        // 解析JSON响应
        nlohmann::json json_response;
        try {
            json_response = nlohmann::json::parse(response.data);
            spdlog::debug("JSON parsing successful");
        } catch (const nlohmann::json::parse_error& e) {
            spdlog::error("JSON parse error: {}", e.what());
            spdlog::error("Raw response that failed to parse: {}", response.data.substr(0, 1000));
            return createErrorResponse("JSON parse error: " + std::string(e.what()));
        }

        // 检查API响应状态
        auto status = json_response.value("status", "");
        spdlog::debug("Amap API status: {}", status);

        if (status != "1") {
            std::string error_msg = json_response.value("info", "Unknown error");
            std::string error_code = json_response.value("infocode", "");
            spdlog::error("Amap API error - Code: {}, Message: {}", error_code, error_msg);
            spdlog::error("Full error response: {}", json_response.dump());
            return createErrorResponse("Amap API error [" + error_code + "]: " + error_msg);
        }

        // 解析并标准化响应
        return parseRouteResponse(json_response);

    } catch (const std::exception& e) {
        spdlog::error("Route planning error: {}", e.what());
        spdlog::error("Request parameters - Origin: {}, Destination: {}, Strategy: {}", origin, destination, strategy);
        return createErrorResponse("Route planning failed: " + std::string(e.what()));
    }
}

nlohmann::json RouteService::planRouteWithWaypoints(
    const std::string& origin,
    const std::string& destination,
    const std::vector<std::string>& waypoints,
    const std::string& strategy) {

    try {
        spdlog::info("RouteService::planRouteWithWaypoints called with {} waypoints", waypoints.size());

        // 构建途经点字符串
        std::string waypoints_str;
        for (size_t i = 0; i < waypoints.size(); ++i) {
            if (i > 0) waypoints_str += ";";
            waypoints_str += waypoints[i];
        }

        // 构建请求URL
        std::stringstream url;
        url << base_url_ << "/direction/driving"
            << "?origin=" << urlEncode(origin)
            << "&destination=" << urlEncode(destination);

        if (!waypoints_str.empty()) {
            url << "&waypoints=" << urlEncode(waypoints_str);
        }

        url << "&strategy=" << strategy
            << "&key=" << api_key_
            << "&output=json"
            << "&extensions=all";

        spdlog::debug("Route planning with waypoints request URL: {}", url.str());

        // 执行HTTP请求
        auto response = httpGet(url.str());

        if (response.status_code != 200) {
            spdlog::error("HTTP request failed with status: {}", response.status_code);
            return createErrorResponse("HTTP request failed");
        }

        // 解析JSON响应
        auto json_response = nlohmann::json::parse(response.data);

        if (json_response["status"] != "1") {
            std::string error_msg = json_response.value("info", "Unknown error");
            spdlog::error("Amap API error: {}", error_msg);
            return createErrorResponse("Amap API error: " + error_msg);
        }

        // 解析并标准化响应
        return parseRouteResponse(json_response);

    } catch (const std::exception& e) {
        spdlog::error("Route planning with waypoints error: {}", e.what());
        return createErrorResponse("Route planning failed");
    }
}

nlohmann::json RouteService::geocode(const std::string& address) {
    try {
        // 构建请求URL
        std::stringstream url;
        url << base_url_ << "/geocode/geo"
            << "?address=" << urlEncode(address)
            << "&key=" << api_key_
            << "&output=json";

        auto response = httpGet(url.str());

        if (response.status_code != 200) {
            return createErrorResponse("HTTP request failed");
        }

        auto json_response = nlohmann::json::parse(response.data);

        if (json_response["status"] != "1") {
            std::string error_msg = json_response.value("info", "Unknown error");
            return createErrorResponse("Geocoding error: " + error_msg);
        }

        // 标准化响应格式
        nlohmann::json result = {
            {"success", true},
            {"data", json_response["geocodes"]}
        };

        return result;

    } catch (const std::exception& e) {
        spdlog::error("Geocoding error: {}", e.what());
        return createErrorResponse("Geocoding failed");
    }
}

nlohmann::json RouteService::reverseGeocode(double lng, double lat) {
    try {
        // 构建请求URL
        std::stringstream url;
        url << base_url_ << "/geocode/regeo"
            << "?location=" << std::fixed << std::setprecision(6) << lng << "," << lat
            << "&key=" << api_key_
            << "&output=json"
            << "&extensions=all";

        auto response = httpGet(url.str());

        if (response.status_code != 200) {
            return createErrorResponse("HTTP request failed");
        }

        auto json_response = nlohmann::json::parse(response.data);

        if (json_response["status"] != "1") {
            std::string error_msg = json_response.value("info", "Unknown error");
            return createErrorResponse("Reverse geocoding error: " + error_msg);
        }

        // 标准化响应格式
        nlohmann::json result = {
            {"success", true},
            {"data", json_response["regeocode"]}
        };

        return result;

    } catch (const std::exception& e) {
        spdlog::error("Reverse geocoding error: {}", e.what());
        return createErrorResponse("Reverse geocoding failed");
    }
}

RouteService::HttpResponse RouteService::httpGet(const std::string& url) {
    HttpResponse response;
    response.status_code = 0;

    // 使用RAII包装的CurlHandle，自动管理CURL资源
    utils::CurlHandle curl;

    if (!curl.isValid()) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    // 设置CURL选项
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);  // 30秒超时
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);  // 跟随重定向
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);  // 简化SSL验证
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);

    // 执行请求
    CURLcode res = curl.perform();

    if (res == CURLE_OK) {
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.status_code);
    } else {
        spdlog::error("CURL error: {}", utils::CurlHandle::getErrorString(res));
        response.status_code = 0;
    }

    // CurlHandle析构时会自动清理资源
    return response;
}

size_t RouteService::writeCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response) {
    size_t totalSize = size * nmemb;
    response->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string RouteService::urlEncode(const std::string& input) {
    // 使用RAII包装的CurlHandle
    utils::CurlHandle curl;
    if (!curl.isValid()) {
        return input;
    }

    char* encoded = curl_easy_escape(curl.get(), input.c_str(), static_cast<int>(input.length()));
    std::string result(encoded);
    curl_free(encoded);
    // CurlHandle析构时会自动清理资源

    return result;
}

nlohmann::json RouteService::parseRouteResponse(const nlohmann::json& response) {
    try {
        spdlog::debug("Starting to parse route response");

        // 记录响应的基本结构
        spdlog::debug("Response contains keys: {}", [&response]() {
            std::string keys;
            for (auto it = response.begin(); it != response.end(); ++it) {
                if (!keys.empty()) keys += ", ";
                keys += it.key();
            }
            return keys;
        }());

        if (!response.contains("route")) {
            spdlog::error("Response missing 'route' field");
            spdlog::error("Full response: {}", response.dump());
            return createErrorResponse("Invalid response: missing route data");
        }

        auto route = response["route"];
        spdlog::debug("Route object keys: {}", [&route]() {
            std::string keys;
            for (auto it = route.begin(); it != route.end(); ++it) {
                if (!keys.empty()) keys += ", ";
                keys += it.key();
            }
            return keys;
        }());

        if (!route.contains("paths")) {
            spdlog::error("Route missing 'paths' field");
            spdlog::error("Route object: {}", route.dump());
            return createErrorResponse("Invalid response: missing paths data");
        }

        auto paths = route["paths"];
        spdlog::debug("Paths array size: {}", paths.size());

        if (paths.empty()) {
            spdlog::warn("No paths found in response");
            return createErrorResponse("No route found");
        }

        auto path = paths[0];  // 取第一条路径
        spdlog::debug("First path keys: {}", [&path]() {
            std::string keys;
            for (auto it = path.begin(); it != path.end(); ++it) {
                if (!keys.empty()) keys += ", ";
                keys += it.key();
            }
            return keys;
        }());

        // 提取路径坐标
        nlohmann::json coordinates = nlohmann::json::array();

        if (path.contains("steps") && path["steps"].is_array()) {
            spdlog::debug("Processing {} steps", path["steps"].size());

            for (const auto& step : path["steps"]) {
                if (step.contains("polyline")) {
                    std::string polyline = step["polyline"].get<std::string>();

                    // 解析坐标字符串 "lng1,lat1;lng2,lat2;..."
                    std::stringstream ss(polyline);
                    std::string point;

                    while (std::getline(ss, point, ';')) {
                        std::stringstream point_ss(point);
                        std::string lng_str, lat_str;

                        if (std::getline(point_ss, lng_str, ',') && std::getline(point_ss, lat_str)) {
                            // 使用ParamParser安全解析坐标，避免异常
                            auto lng_opt = utils::ParamParser::parseOptionalDouble(lng_str);
                            auto lat_opt = utils::ParamParser::parseOptionalDouble(lat_str);

                            if (lng_opt.has_value() && lat_opt.has_value()) {
                                coordinates.push_back({lng_opt.value(), lat_opt.value()});
                            } else {
                                spdlog::warn("Failed to parse coordinate: {}", point);
                            }
                        }
                    }
                }
            }
        } else {
            spdlog::warn("No steps found in path or steps is not an array");
        }

        spdlog::debug("Extracted {} coordinate points", coordinates.size());

        // 安全地解析数字字段
        int duration_seconds = 0;
        std::string distance_str = "0";
        double distance_meters = 0.0;

        // 处理distance字段
        if (path.contains("distance")) {
            if (path["distance"].is_string()) {
                distance_str = path["distance"].get<std::string>();
                distance_meters = utils::ParamParser::parseDouble(distance_str, 0.0, 0.0, 1000000.0);
                spdlog::debug("Distance (string): {} -> {} meters", distance_str, distance_meters);
            } else if (path["distance"].is_number()) {
                distance_meters = path["distance"].get<double>();
                distance_str = std::to_string(static_cast<int>(distance_meters));
                spdlog::debug("Distance (number): {} meters", distance_meters);
            } else {
                spdlog::warn("Distance field is not string or number, type: {}", path["distance"].type_name());
            }
        } else {
            spdlog::warn("No distance field found in path");
        }

        // 处理duration字段
        if (path.contains("duration")) {
            if (path["duration"].is_string()) {
                std::string duration_str = path["duration"].get<std::string>();
                duration_seconds = utils::ParamParser::parseInt(duration_str, 0, 0, 86400);
                spdlog::debug("Duration (string): {} -> {} seconds", duration_str, duration_seconds);
            } else if (path["duration"].is_number()) {
                duration_seconds = path["duration"].get<int>();
                spdlog::debug("Duration (number): {} seconds", duration_seconds);
            } else {
                spdlog::warn("Duration field is not string or number, type: {}", path["duration"].type_name());
            }
        } else {
            spdlog::warn("No duration field found in path");
        }

        // 构建标准化响应
        nlohmann::json result = {
            {"success", true},
            {"data", {
                {"distance", distance_str + " 米"},
                {"duration", std::to_string(duration_seconds / 60) + " 分钟"},
                {"coordinates", coordinates},
                {"steps", path.value("steps", nlohmann::json::array())},
                {"strategy", path.value("strategy", "0")},
                {"traffic", {
                    {"distance", distance_str},
                    {"duration", duration_seconds}
                }}
            }}
        };

        spdlog::info("Route parsing successful - Distance: {} meters, Duration: {} seconds, Coordinates: {} points",
                    distance_meters, duration_seconds, coordinates.size());

        return result;

    } catch (const std::exception& e) {
        spdlog::error("Failed to parse route response: {}", e.what());
        spdlog::error("Exception type: {}", typeid(e).name());
        spdlog::error("Full response that caused error: {}", response.dump());
        return createErrorResponse("Failed to parse route response: " + std::string(e.what()));
    }
}

nlohmann::json RouteService::createErrorResponse(const std::string& error_message) {
    return nlohmann::json{
        {"success", false},
        {"error", error_message},
        {"timestamp", std::time(nullptr)}
    };
}

} // namespace services