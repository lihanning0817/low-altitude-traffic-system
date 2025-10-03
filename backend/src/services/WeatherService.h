#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>

namespace services {

/**
 * @brief 天气服务类
 * 负责与OpenWeatherMap API交互，获取天气数据
 */
class WeatherService {
private:
    std::string apiKey_;
    std::string baseUrl_;

    // HTTP客户端（使用libcurl或其他HTTP库）
    std::string makeHttpRequest(const std::string& url);

public:
    /**
     * @brief 构造函数
     * @param apiKey OpenWeatherMap API密钥
     */
    explicit WeatherService(const std::string& apiKey);

    /**
     * @brief 析构函数
     */
    ~WeatherService() = default;

    /**
     * @brief 根据城市名称获取当前天气
     * @param cityName 城市名称
     * @param countryCode 国家代码（可选，如CN）
     * @return 天气数据JSON
     */
    nlohmann::json getCurrentWeatherByCity(
        const std::string& cityName,
        const std::string& countryCode = ""
    );

    /**
     * @brief 根据坐标获取当前天气
     * @param lat 纬度
     * @param lon 经度
     * @return 天气数据JSON
     */
    nlohmann::json getCurrentWeatherByCoords(double lat, double lon);

    /**
     * @brief 根据坐标获取天气预报（5天）
     * @param lat 纬度
     * @param lon 经度
     * @return 天气预报数据JSON
     */
    nlohmann::json getForecastByCoords(double lat, double lon);

    /**
     * @brief 检查天气是否适合飞行
     * @param weatherData 天气数据
     * @return JSON格式的检查结果，包含是否适合飞行和原因
     */
    nlohmann::json checkFlightSafety(const nlohmann::json& weatherData);

    /**
     * @brief 格式化天气数据为用户友好的格式
     * @param weatherData 原始天气数据
     * @return 格式化后的天气数据
     */
    nlohmann::json formatWeatherData(const nlohmann::json& weatherData);

private:
    /**
     * @brief 解析天气API响应
     * @param response HTTP响应字符串
     * @return 解析后的JSON数据
     */
    nlohmann::json parseWeatherResponse(const std::string& response);

    /**
     * @brief 构建API请求URL
     * @param endpoint API端点
     * @param params 请求参数
     * @return 完整的URL
     */
    std::string buildApiUrl(
        const std::string& endpoint,
        const nlohmann::json& params
    );
};

} // namespace services
