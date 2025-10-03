#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <string>
#include "services/WeatherService.h"
#include "auth/JwtService.h"

namespace beast = boost::beast;
namespace http = beast::http;

namespace controllers {

/**
 * @brief 天气控制器类
 * 处理天气相关的HTTP请求
 */
class WeatherController {
private:
    std::shared_ptr<services::WeatherService> weatherService_;
    std::shared_ptr<auth::JwtService> jwtService_;

public:
    /**
     * @brief 构造函数
     * @param weatherService 天气服务
     * @param jwtService JWT服务
     */
    WeatherController(
        std::shared_ptr<services::WeatherService> weatherService,
        std::shared_ptr<auth::JwtService> jwtService
    );

    /**
     * @brief 根据城市获取当前天气
     * GET /api/v1/weather/current?city=Beijing
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getCurrentWeather(
        const http::request<http::string_body>& req
    );

    /**
     * @brief 根据坐标获取当前天气
     * GET /api/v1/weather/current/coords?lat=39.9&lon=116.4
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getCurrentWeatherByCoords(
        const http::request<http::string_body>& req
    );

    /**
     * @brief 根据坐标获取天气预报
     * GET /api/v1/weather/forecast?lat=39.9&lon=116.4
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> getForecast(
        const http::request<http::string_body>& req
    );

    /**
     * @brief 检查指定位置的飞行安全性
     * GET /api/v1/weather/flight-safety?lat=39.9&lon=116.4
     *
     * @param req HTTP请求对象
     * @return HTTP响应对象
     */
    http::response<http::string_body> checkFlightSafety(
        const http::request<http::string_body>& req
    );

private:
    /**
     * @brief 从请求中提取Bearer Token
     * @param req HTTP请求对象
     * @return Token字符串
     */
    std::string extractBearerToken(const http::request<http::string_body>& req);

    /**
     * @brief 验证JWT Token并获取用户ID
     * @param req HTTP请求对象
     * @return 用户ID（可选）
     */
    std::optional<int64_t> validateTokenAndGetUserId(
        const http::request<http::string_body>& req
    );

    /**
     * @brief 解析URL查询参数
     * @param req HTTP请求对象
     * @return 查询参数映射
     */
    std::map<std::string, std::string> parseQueryParams(
        const http::request<http::string_body>& req
    );
};

} // namespace controllers
